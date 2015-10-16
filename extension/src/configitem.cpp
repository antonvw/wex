////////////////////////////////////////////////////////////////////////////////
// Name:      configitem.cpp
// Purpose:   Implementation of wxExConfigItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/button.h>
#include <wx/clrpicker.h> // for wxColourPickerWidget
#include <wx/commandlinkbutton.h>
#include <wx/checklst.h>
#include <wx/config.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/hyperlink.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/valnum.h>
#include <wx/valtext.h>
#include <wx/window.h>
#include <wx/extension/configitem.h>
#include <wx/extension/frd.h>
#include <wx/extension/listview.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

#define PERSISTENT(CONTROL, SETVALUE, GETVALUE, READ, DEFAULT)      \
{                                                                   \
  CONTROL* ctrl = (CONTROL*)m_Window;                               \
  if (save)                                                         \
    wxConfigBase::Get()->Write(m_Label, ctrl->GETVALUE());          \
  else                                                              \
    ctrl->SETVALUE(wxConfigBase::Get()->READ(m_Label, DEFAULT));    \
}                                                                   \

bool Update(wxExFindReplaceData* frd, wxCheckListBox* clb, int item, bool save, bool value)
{
  const wxString field(clb->GetString(item));
  
  if (field == frd->GetTextMatchWholeWord())
  {
    !save ? clb->Check(item, frd->MatchWord()): frd->SetMatchWord(value);
  }
  else if (field == frd->GetTextMatchCase())
  {
    !save ? clb->Check(item, frd->MatchCase()): frd->SetMatchCase(value);
  }
  else if (field == frd->GetTextRegEx())
  {
    !save ? clb->Check(item, frd->UseRegEx()): frd->SetUseRegEx(value);
  }
  else if (field == frd->GetTextSearchDown())
  {
    frd->SetFlags(value ? wxFR_DOWN: ~wxFR_DOWN);
  }
  else
  {
    return false;
  }

  return true;
}

wxExConfigItem::wxExConfigItem(wxExConfigType type, long style,
  const wxString& page, const wxString& label, const wxString& info,
  bool is_required, bool add_label,
  int id, int cols, int max_items, int major_dimension,
  double min, double max, double inc,
  wxWindow* window, wxExUserWindowCreate create, wxExUserWindowToConfig config)
  : m_Type(type)
  , m_Style(style)
  , m_AddLabel(add_label)
  , m_Label(label)
  , m_Info(info)
  , m_IsRequired(is_required)
  , m_Id(id)
  , m_Min(min)
  , m_Max(max)
  , m_Inc(inc)
  , m_MajorDimension(major_dimension)
  , m_UserWindowCreate(create)
  , m_UserWindowToConfig(config)
  , m_Validator(NULL)
  , m_Window(window)
  , m_Cols(cols)
  , m_MaxItems(max_items)
  , m_IsRowGrowable(false)
  , m_Page(page)
  , m_PageCols(-1)
  , m_SizerFlags(wxSizerFlags().Border().Left())
{
  if (m_Page.Contains(":"))
  {
    m_PageCols = atoi(m_Page.AfterFirst(':'));
    m_Page = m_Page.BeforeFirst(':');
  }
  
  switch (m_Type)
  {
    case CONFIG_CHECKLISTBOX:
    case CONFIG_CHECKLISTBOX_NONAME:
    case CONFIG_LISTVIEW_FOLDER:
    case CONFIG_RADIOBOX:
    case CONFIG_STC:
      m_IsRowGrowable = true;
      m_SizerFlags.Expand();
      break;

    case CONFIG_STATICTEXT:
    case CONFIG_STRING:
      m_IsRowGrowable = (m_Style & wxTE_MULTILINE) > 0;
      m_SizerFlags.Expand();
      break;
  
    case CONFIG_CHECKBOX:
    case CONFIG_COMBOBOX:
    case CONFIG_COMBOBOXDIR:
    case CONFIG_DIRPICKERCTRL:
    case CONFIG_FILEPICKERCTRL:
    case CONFIG_SPACER:
    case CONFIG_STATICLINE:
    case CONFIG_USER:
      m_SizerFlags.Expand();
      break;
  }
}

wxFlexGridSizer* wxExConfigItem::AddBrowseButton(wxSizer* sizer) const
{
  wxASSERT(m_Window != NULL);

  wxFlexGridSizer* fgz = new wxFlexGridSizer(3, 0, 0);

  fgz->AddGrowableCol(1);

  AddStaticText(fgz);

  fgz->Add(m_Window, m_SizerFlags);

  // Tried to use a wxDirPickerCtrl here, is nice,
  // but does not use a combobox for keeping old values, so this is better.
  // And the text box that is used is not resizable as well.
  fgz->Add(
    new wxButton(
      m_Window->GetParent(),
      m_Window->GetId(),
      _(wxDirPickerWidgetLabel)),
    wxSizerFlags().Center().Border());

  sizer->Add(fgz, wxSizerFlags().Left().Expand()); // no border

  return fgz;
}

void wxExConfigItem::AddStaticText(wxSizer* sizer) const
{
  wxASSERT(!m_Label.empty());
  wxASSERT(m_Window != NULL);

  sizer->Add(
    new wxStaticText(m_Window->GetParent(), 
      wxID_ANY, 
      m_Label + ":"), 
      wxSizerFlags().Right().Border().Align(wxALIGN_LEFT));
}

void wxExConfigItem::CreateWindow(wxWindow* parent, bool readonly)
{
  const int width = 200;
  const int width_numeric = 75;
  const int width_numeric_spin = 125;
  
  switch (m_Type)
  {
    case CONFIG_EMPTY:
    case CONFIG_SPACER:
      break;
      
    case CONFIG_BUTTON:
      // Using a label is necessary for wxGTK.
      m_Window = new wxButton(parent, m_Id, "default");
      ((wxButton *)m_Window)->SetLabelMarkup(m_Label);
      break;

    case CONFIG_CHECKBOX:
      m_Window = new wxCheckBox(parent, m_Id, m_Label);
      break;

    case CONFIG_CHECKLISTBOX:
      {
      wxArrayString arraychoices;

      for (const auto& it : m_Choices)
      {
        arraychoices.Add(it.second);
      }

      m_Window = new wxCheckListBox(parent,
        m_Id, wxDefaultPosition, wxDefaultSize, arraychoices);
      }
      break;

    case CONFIG_CHECKLISTBOX_NONAME:
      {
      wxArrayString arraychoices;
      arraychoices.resize(m_ChoicesBool.size()); // required!
      copy (m_ChoicesBool.begin(), m_ChoicesBool.end(), arraychoices.begin());
      m_Window = new wxCheckListBox(parent,
        m_Id, wxDefaultPosition, wxDefaultSize, arraychoices);
      }
      break;

    case CONFIG_COLOUR:
      m_Window = new wxColourPickerWidget(parent, m_Id);
      break;

    case CONFIG_COMBOBOX:
    case CONFIG_COMBOBOXDIR:
      m_Window = new wxComboBox(parent, m_Id, wxEmptyString,
        wxDefaultPosition, wxSize(250, wxDefaultCoord),
        0,
        NULL,
        0,
        (m_Validator != NULL ? *m_Validator: wxDefaultValidator));
      break;

    case CONFIG_COMMAND_LINK_BUTTON:
      m_Window = new wxCommandLinkButton(parent, m_Id, 
        m_Label.BeforeFirst('\t'), m_Label.AfterFirst('\t'));
      break;

    case CONFIG_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = new wxDirPickerCtrl(parent, m_Id, wxEmptyString,
        wxDirSelectorPromptStr, wxDefaultPosition, wxSize(width, wxDefaultCoord));

      m_Window = pc;

      if (pc->GetTextCtrl() != NULL && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case CONFIG_FILEPICKERCTRL:
      {
#if defined(__WXMSW__) || defined(__OS2__)
      const wxString wc = "*.exe";
#else // Unix/Mac
      const wxString wc(wxFileSelectorDefaultWildcardStr);
#endif

      wxFilePickerCtrl* pc = new wxFilePickerCtrl(parent, m_Id, wxEmptyString,
        wxFileSelectorPromptStr, wc,
        wxDefaultPosition, wxSize(width, wxDefaultCoord));

      m_Window = pc;

      if (pc->GetTextCtrl() != NULL && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case CONFIG_FLOAT:
      // See also CONFIG_INT, validator cannot be set using ?.
      if (m_Validator == NULL)
      {
        m_Window = new wxTextCtrl(parent, m_Id, wxEmptyString,
          wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord),
          m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
          wxFloatingPointValidator<float>());
      }
      else
      {
        m_Window = new wxTextCtrl(parent, m_Id, wxEmptyString,
          wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord),
          m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
          *m_Validator);
      }
      break;
      
    case CONFIG_FONTPICKERCTRL:
      {
      wxFontPickerCtrl* pc = new wxFontPickerCtrl(parent, m_Id, wxNullFont,
        wxDefaultPosition, wxDefaultSize,
        wxFNTP_FONTDESC_AS_LABEL); // no wxFNTP_USEFONT_FOR_LABEL

      m_Window = pc;

      if (pc->GetTextCtrl() != NULL && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case CONFIG_HYPERLINKCTRL:
      {
#if wxUSE_HYPERLINKCTRL
      m_Window = new wxHyperlinkCtrl(parent, m_Id, m_Label,
        m_Info, wxDefaultPosition, wxSize(width, wxDefaultCoord));
#endif      
      }
      break;

    case CONFIG_INT:
      if (m_Validator == NULL)
      {
        m_Window = new wxTextCtrl(parent, m_Id, wxEmptyString,
          wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord),
          m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
          wxTextValidator(wxFILTER_NUMERIC));
      }
      else
      {
        m_Window = new wxTextCtrl(parent, m_Id, wxEmptyString,
          wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord),
          m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
          *m_Validator);
      }
      break;

    case CONFIG_LISTVIEW_FOLDER:
      m_Window = new wxExListViewFileName(parent, wxExListViewFileName::LIST_FOLDER, m_Id,
        NULL,
        wxDefaultPosition, wxSize(width, 200));
      break;

    case CONFIG_RADIOBOX:
      {
      wxArrayString arraychoices;

      for (const auto& it : m_Choices)
      {
        arraychoices.Add(it.second);
      } 

      m_Window = new wxRadioBox(parent, m_Id, m_Label, 
        wxDefaultPosition, wxDefaultSize, arraychoices, m_MajorDimension, m_Style);
      }
      break;

    case CONFIG_SLIDER:
      m_Window = new wxSlider(parent, m_Id, 0,
        m_Min, m_Max,
        wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord), m_Style);
      break;

    case CONFIG_SPINCTRL:
    case CONFIG_SPINCTRL_HEX:
      m_Window = new wxSpinCtrl(parent, m_Id, wxEmptyString,
        wxDefaultPosition, wxSize(width_numeric_spin, wxDefaultCoord),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        m_Min, m_Max, m_Min);
      if (m_Type == CONFIG_SPINCTRL_HEX)
      {
        ((wxSpinCtrl* )m_Window)->SetBase(16);
      }
      break;

    case CONFIG_SPINCTRL_DOUBLE:
      m_Window = new wxSpinCtrlDouble(parent, m_Id, wxEmptyString,
        wxDefaultPosition, wxSize(width_numeric_spin, wxDefaultCoord),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        m_Min, m_Max, m_Min, m_Inc);
      break;

    case CONFIG_STATICLINE:
      m_Window = new wxStaticLine(parent, m_Id,
        wxDefaultPosition, wxDefaultSize, m_Style);
      break;

    case CONFIG_STATICTEXT:
      m_Window = new wxStaticText(parent, m_Id, wxEmptyString,
        wxDefaultPosition,
        (m_Style & wxTE_MULTILINE ?
           wxSize(width, 200):
           wxSize(width, wxDefaultCoord)),
        m_Style);
      ((wxStaticText* )m_Window)->SetLabelMarkup(m_Label);
      break;

    case CONFIG_STC:
      m_Window = new wxExSTC(parent, wxEmptyString, 0,
        wxEmptyString, wxExSTC::STC_MENU_DEFAULT, m_Id);
      
      // Do not use vi mode, as ESC should cancel the dialog,
      // and would not be interpreted by vi.
      ((wxExSTC* )m_Window)->GetVi().Use(false);

      if (!m_Info.empty())
      {
        ((wxExSTC* )m_Window)->SetLexer(m_Info);
      }
      break;

    case CONFIG_STRING:
      m_Window = new wxTextCtrl(parent, m_Id, wxEmptyString,
        wxDefaultPosition,
        (m_Style & wxTE_MULTILINE ?
           wxSize(width, 200):
           wxSize(width, wxDefaultCoord)),
        m_Style | 
          (readonly ? wxTE_READONLY: 0),
        (m_Validator != NULL ? *m_Validator: wxDefaultValidator));
      break;

    case CONFIG_TOGGLEBUTTON:
      m_Window = new wxToggleButton(parent, m_Id, m_Label);
      break;

    case CONFIG_USER:
      wxASSERT(m_Window != NULL);
      if (m_UserWindowCreate != NULL)
      {
        (m_UserWindowCreate)(m_Window, parent, readonly);
      }
      break;
  
    default: wxFAIL;
  }

  if (m_Type != CONFIG_EMPTY && m_Type != CONFIG_SPACER)
  {
    wxASSERT(m_Window != NULL);
  }
}

wxFlexGridSizer* wxExConfigItem::Layout(
  wxWindow* parent, 
  wxSizer* sizer, 
  bool readonly,
  wxFlexGridSizer* fgz)
{
  CreateWindow(parent, readonly);
  
  wxFlexGridSizer* use = fgz;

  switch (m_Type)
  {
    case CONFIG_COMBOBOXDIR: use = AddBrowseButton(sizer); break;
    case CONFIG_EMPTY: break;
    case CONFIG_SPACER: sizer->AddSpacer(m_Style); break;

    default:
    if (m_AddLabel)
    {
      // Construct a child flex grid sizer.
      if (fgz == NULL)
      {
        use = new wxFlexGridSizer(
          (m_Cols == 1 ? 1: 2), 0, 0);
        
        use->AddGrowableCol(use->GetCols() - 1); // the control
        
        if (m_IsRowGrowable)
        {
          use->AddGrowableRow(0);
        }
      
        // Add label and control.
        AddStaticText(use);
        use->Add(m_Window, m_SizerFlags);

        // Add to the sizer.
        sizer->Add(use, wxSizerFlags().Expand());
      }
      else
      {
        AddStaticText(fgz);
        fgz->Add(m_Window, m_SizerFlags);
        
        if (m_IsRowGrowable && fgz->GetEffectiveRowsCount() >= 1)
        {
          fgz->AddGrowableRow(fgz->GetEffectiveRowsCount() - 1);
        }
      }
    }
    else
    {
      sizer->Add(m_Window, m_SizerFlags);
    }
  }
  
  ToConfig(false);
  
  return use;
}

bool wxExConfigItem::ToConfig(bool save) const
{
  switch (m_Type)
  {
    case CONFIG_CHECKBOX:        PERSISTENT(wxCheckBox, SetValue, IsChecked, ReadBool, false); break;
    case CONFIG_COLOUR:          PERSISTENT(wxColourPickerWidget, SetColour, GetColour, ReadObject, *wxWHITE); break;
    case CONFIG_DIRPICKERCTRL:   PERSISTENT(wxDirPickerCtrl, SetPath, GetPath, Read, m_Label); break;
    case CONFIG_FONTPICKERCTRL:  PERSISTENT(wxFontPickerCtrl, SetSelectedFont, GetSelectedFont, ReadObject, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)); break;
    case CONFIG_SLIDER:          PERSISTENT(wxSlider, SetValue, GetValue, ReadLong, m_Min); break;
    case CONFIG_SPINCTRL:
    case CONFIG_SPINCTRL_HEX:    PERSISTENT(wxSpinCtrl, SetValue, GetValue, ReadLong, m_Min); break;
    case CONFIG_SPINCTRL_DOUBLE: PERSISTENT(wxSpinCtrlDouble, SetValue, GetValue, ReadDouble, m_Min); break;
    case CONFIG_STC:             PERSISTENT(wxExSTC, SetValue, GetValue, Read, ""); break;
    case CONFIG_STRING:          PERSISTENT(wxTextCtrl,SetValue,  GetValue, Read, ""); break;
    case CONFIG_TOGGLEBUTTON:    PERSISTENT(wxToggleButton, SetValue, GetValue, ReadBool, false); break;

    case CONFIG_CHECKLISTBOX:
      {
      wxCheckListBox* clb = (wxCheckListBox*)m_Window;

      long value = 0;
      if (!save)
        value = wxConfigBase::Get()->ReadLong(m_Label, 0);
      int item = 0;

      for (const auto& b : m_Choices)
      {
        if (save)
        {
          if (clb->IsChecked(item))
          {
            value |= b.first;
          }
        }
        else
        {
          clb->Check(item, (value & b.first) > 0);
        }

        item++;
      }

      if (save)
        wxConfigBase::Get()->Write(m_Label, value);
      }
      break;

    case CONFIG_CHECKLISTBOX_NONAME:
      {
      wxCheckListBox* clb = (wxCheckListBox*)m_Window;

      for (size_t i = 0; i < clb->GetCount(); i++)
      {
        if (!Update(wxExFindReplaceData::Get(), clb, i, save, clb->IsChecked(i)))
        {
          if (save)
            wxConfigBase::Get()->Write(clb->GetString(i), clb->IsChecked(i));
          else
            clb->Check(i, wxConfigBase::Get()->ReadBool(clb->GetString(i), false));
        }
      }}
      break;

    case CONFIG_COMBOBOX:
    case CONFIG_COMBOBOXDIR:
      {
      wxComboBox* cb = (wxComboBox*)m_Window;

      if (save)
      {
        const auto& l = wxExComboBoxToList(cb, m_MaxItems);

        if (m_Label == wxExFindReplaceData::Get()->GetTextFindWhat())
        {
          wxExFindReplaceData::Get()->SetFindStrings(l);
        }
        else if (m_Label == wxExFindReplaceData::Get()->GetTextReplaceWith())
        {
          wxExFindReplaceData::Get()->SetReplaceStrings(l);
        }
        else
        {
          wxExListToConfig(l, m_Label);
        }
      }
      else
      {
        wxExComboBoxFromList(cb, wxExListFromConfig(m_Label));
      }
      }
      break;

    case CONFIG_FILEPICKERCTRL:
      {
      wxFilePickerCtrl* pc = (wxFilePickerCtrl*)m_Window;
      
      if (save)
      {
        wxConfigBase::Get()->Write(m_Label, pc->GetPath());
      }
      else
      {
        wxString initial;

#ifdef __WXGTK__
        initial = "/usr/bin/" + m_Label;
        if (!wxFileExists(initial))
        {
          initial.clear();
        }
#endif
        const wxString val(wxConfigBase::Get()->Read(m_Label, initial));
        
        if (!val.empty())
        {
          pc->SetPath(val);
        }
      }
      }
      break;

    case CONFIG_FLOAT:
      {
      wxTextCtrl* tc = (wxTextCtrl*)m_Window;
      
      if (save)
        wxConfigBase::Get()->Write(m_Label, atof(tc->GetValue().c_str()));
      else
        tc->SetValue(
          wxString::Format("%lf", wxConfigBase::Get()->ReadDouble(m_Label, 0)));
      }
      break;
      
    case CONFIG_INT:
      {
      wxTextCtrl* tc = (wxTextCtrl*)m_Window;
      if (save)
        wxConfigBase::Get()->Write(m_Label, atol(tc->GetValue().c_str()));
      else
        tc->SetValue(
          wxString::Format("%ld", wxConfigBase::Get()->ReadLong(m_Label, 0)));
      }
      break;

    case CONFIG_LISTVIEW_FOLDER:
      {
      wxExListViewFileName* win = (wxExListViewFileName*)m_Window;
      if (save)
        wxConfigBase::Get()->Write(m_Label, win->ItemToText(-1));
      else
      {
        win->DeleteAllItems();
        win->ItemFromText(wxConfigBase::Get()->Read(m_Label));
      }
      }
      break;

    case CONFIG_RADIOBOX:
      {
      wxRadioBox* rb = (wxRadioBox*)m_Window;
      if (save)
      {
        for (const auto& b : m_Choices)
        {
          if (b.second == rb->GetStringSelection())
          {
            wxConfigBase::Get()->Write(m_Label, b.first);
          }
        }
      }
      else
      {
        const auto c = 
          m_Choices.find(wxConfigBase::Get()->ReadLong(m_Label, 0));

        if (c != m_Choices.end())
        {
          rb->SetStringSelection(c->second);
        }
      }
      }
      break;

    case CONFIG_USER:
      if (m_UserWindowToConfig != NULL)
      {
        return (m_UserWindowToConfig)(m_Window, save);
      }
      break;
      
    default:
      // the other types have no persistent info
      return false;
      break;
  }

  return true;
}
#endif // wxUSE_GUI
