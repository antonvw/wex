////////////////////////////////////////////////////////////////////////////////
// Name:      item.cpp
// Purpose:   Implementation of wxExItem class
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
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/hyperlink.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/valnum.h>
#include <wx/valtext.h>
#include <wx/window.h>
#include <wx/extension/item.h>
#include <wx/extension/listview.h>
#include <wx/extension/stc.h>

#if wxUSE_GUI

wxExItem::wxExItem(wxExItemType type, long style,
  const wxString& page, const wxString& label, const wxString& value, const wxString& info,
  bool is_required, bool add_label,
  int id, int cols, int major_dimension,
  double min, double max, double inc,
  const std::map<long, const wxString> & choices,
  const std::set<wxString> & choices_bool,
  wxWindow* window, wxExUserWindowCreate create)
  : m_Type(type)
  , m_Style(style)
  , m_AddLabel(add_label)
  , m_Label(label)
  , m_Value(value)
  , m_Info(info)
  , m_IsRequired(is_required)
  , m_Id(id)
  , m_Min(min)
  , m_Max(max)
  , m_Inc(inc)
  , m_MajorDimension(major_dimension)
  , m_UserWindowCreate(create)
  , m_Validator(NULL)
  , m_Window(window)
  , m_Cols(cols)
  , m_IsRowGrowable(false)
  , m_Page(page)
  , m_PageCols(-1)
  , m_SizerFlags(wxSizerFlags().Border().Left())
  , m_Choices(choices)
  , m_ChoicesBool(choices_bool)
{
  if (m_Page.Contains(":"))
  {
    m_PageCols = atoi(m_Page.AfterFirst(':'));
    m_Page = m_Page.BeforeFirst(':');
  }
  
  switch (m_Type)
  {
    case ITEM_CHECKLISTBOX:
    case ITEM_CHECKLISTBOX_NONAME:
    case ITEM_LISTVIEW_FOLDER:
    case ITEM_RADIOBOX:
    case ITEM_STC:
      m_IsRowGrowable = true;
      m_SizerFlags.Expand();
      break;

    case ITEM_STATICTEXT:
    case ITEM_STRING:
      m_IsRowGrowable = (m_Style & wxTE_MULTILINE) > 0;
      m_SizerFlags.Expand();
      break;
  
    case ITEM_CHECKBOX:
    case ITEM_COMBOBOX:
    case ITEM_COMBOBOXDIR:
    case ITEM_DIRPICKERCTRL:
    case ITEM_FILEPICKERCTRL:
    case ITEM_SPACER:
    case ITEM_STATICLINE:
    case ITEM_USER:
      m_SizerFlags.Expand();
      break;
  }
}

wxFlexGridSizer* wxExItem::AddBrowseButton(wxSizer* sizer) const
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

void wxExItem::AddStaticText(wxSizer* sizer) const
{
  wxASSERT(!m_Label.empty());
  wxASSERT(m_Window != NULL);

  sizer->Add(
    new wxStaticText(m_Window->GetParent(), 
      wxID_ANY, 
      m_Label + ":"), 
      wxSizerFlags().Right().Border().Align(wxALIGN_LEFT));
}

void wxExItem::CreateWindow(wxWindow* parent, bool readonly)
{
  if (m_Type != ITEM_USER)
  {
    wxASSERT(m_Window == NULL);
  }
  
  const int width = 200;
  const int width_numeric = 75;
  const int width_numeric_spin = 125;
  
  switch (m_Type)
  {
    case ITEM_EMPTY:
    case ITEM_SPACER:
      break;
      
    case ITEM_BUTTON:
      // Using a label is necessary for wxGTK.
      m_Window = new wxButton(parent, m_Id, "default", 
        wxDefaultPosition, wxDefaultSize, m_Style);
      ((wxButton *)m_Window)->SetLabelMarkup(m_Label);
      break;

    case ITEM_CHECKBOX:
      m_Window = new wxCheckBox(parent, m_Id, m_Label, 
        wxDefaultPosition, wxDefaultSize, m_Style);
      break;

    case ITEM_CHECKLISTBOX:
      {
      wxArrayString arraychoices;

      for (const auto& it : m_Choices)
      {
        arraychoices.Add(it.second);
      }

      m_Window = new wxCheckListBox(parent,
        m_Id, wxDefaultPosition, wxDefaultSize, arraychoices, m_Style);
      }
      break;

    case ITEM_CHECKLISTBOX_NONAME:
      {
      wxArrayString arraychoices;
      arraychoices.resize(m_ChoicesBool.size()); // required!
      copy (m_ChoicesBool.begin(), m_ChoicesBool.end(), arraychoices.begin());
      m_Window = new wxCheckListBox(parent,
        m_Id, wxDefaultPosition, wxDefaultSize, arraychoices, m_Style);
      }
      break;

    case ITEM_COLOUR:
      m_Window = new wxColourPickerWidget(parent, m_Id, 
        *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRBTN_DEFAULT_STYLE); // no m_Style
      break;

    case ITEM_COMBOBOX:
    case ITEM_COMBOBOXDIR:
      m_Window = new wxComboBox(parent, m_Id, wxEmptyString,
        wxDefaultPosition, wxSize(250, wxDefaultCoord),
        0,
        NULL,
        0,
        (m_Validator != NULL ? *m_Validator: wxDefaultValidator));
      break;

    case ITEM_COMMAND_LINK_BUTTON:
      m_Window = new wxCommandLinkButton(parent, m_Id, 
        m_Label.BeforeFirst('\t'), m_Label.AfterFirst('\t'),
        wxDefaultPosition, wxDefaultSize, m_Style);
      break;

    case ITEM_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = new wxDirPickerCtrl(parent, m_Id, wxEmptyString,
        wxDirSelectorPromptStr, wxDefaultPosition, wxSize(width, wxDefaultCoord), 
        m_Style == 0 ? wxDIRP_DEFAULT_STYLE: m_Style);

      m_Window = pc;

      if (pc->GetTextCtrl() != NULL && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case ITEM_FILEPICKERCTRL:
      {
#if defined(__WXMSW__) || defined(__OS2__)
      const wxString wc = "*.exe";
#else // Unix/Mac
      const wxString wc(wxFileSelectorDefaultWildcardStr);
#endif

      wxFilePickerCtrl* pc = new wxFilePickerCtrl(parent, m_Id, wxEmptyString,
        wxFileSelectorPromptStr, wc,
        wxDefaultPosition, wxSize(width, wxDefaultCoord),
        m_Style == 0 ? wxFLP_DEFAULT_STYLE: m_Style);

      m_Window = pc;

      if (pc->GetTextCtrl() != NULL && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case ITEM_FLOAT:
      // See also ITEM_INT, validator cannot be set using ?.
      if (m_Validator == NULL)
      {
        m_Window = new wxTextCtrl(parent, m_Id, m_Value,
          wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord),
          m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
          wxFloatingPointValidator<float>());
      }
      else
      {
        m_Window = new wxTextCtrl(parent, m_Id, m_Value,
          wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord),
          m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
          *m_Validator);
      }
      break;
      
    case ITEM_FONTPICKERCTRL:
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

    case ITEM_HYPERLINKCTRL:
      {
#if wxUSE_HYPERLINKCTRL
      m_Window = new wxHyperlinkCtrl(parent, m_Id, m_Label,
        m_Info, wxDefaultPosition, wxSize(width, wxDefaultCoord), wxHL_DEFAULT_STYLE); // no m_Style
#endif      
      }
      break;

    case ITEM_INT:
      if (m_Validator == NULL)
      {
        m_Window = new wxTextCtrl(parent, m_Id, m_Value,
          wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord),
          m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
          wxIntegerValidator<int>());
      }
      else
      {
        m_Window = new wxTextCtrl(parent, m_Id, m_Value,
          wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord),
          m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
          *m_Validator);
      }
      break;

    case ITEM_LISTVIEW_FOLDER:
      m_Window = new wxExListViewFileName(parent, wxExListViewFileName::LIST_FOLDER, m_Id,
        NULL,
        wxDefaultPosition, wxSize(width, 200));
      break;

    case ITEM_RADIOBOX:
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

    case ITEM_SLIDER:
      m_Window = new wxSlider(parent, m_Id, 0,
        m_Min, m_Max,
        wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord), m_Style);
      break;

    case ITEM_SPINCTRL:
    case ITEM_SPINCTRL_HEX:
      m_Window = new wxSpinCtrl(parent, m_Id, wxEmptyString,
        wxDefaultPosition, wxSize(width_numeric_spin, wxDefaultCoord),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        m_Min, m_Max, m_Min);
      if (m_Type == ITEM_SPINCTRL_HEX)
      {
        ((wxSpinCtrl* )m_Window)->SetBase(16);
      }
      break;

    case ITEM_SPINCTRL_DOUBLE:
      m_Window = new wxSpinCtrlDouble(parent, m_Id, wxEmptyString,
        wxDefaultPosition, wxSize(width_numeric_spin, wxDefaultCoord),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        m_Min, m_Max, m_Min, m_Inc);
      break;

    case ITEM_STATICLINE:
      m_Window = new wxStaticLine(parent, m_Id,
        wxDefaultPosition, wxDefaultSize, m_Style);
      break;

    case ITEM_STATICTEXT:
      m_Window = new wxStaticText(parent, m_Id, wxEmptyString,
        wxDefaultPosition,
        (m_Style & wxTE_MULTILINE ?
           wxSize(width, 200):
           wxSize(width, wxDefaultCoord)),
        m_Style);
      ((wxStaticText* )m_Window)->SetLabelMarkup(m_Label);
      break;

    case ITEM_STC:
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

    case ITEM_STRING:
      m_Window = new wxTextCtrl(parent, m_Id, m_Value,
        wxDefaultPosition,
        (m_Style & wxTE_MULTILINE ?
           wxSize(width, 200):
           wxSize(width, wxDefaultCoord)),
        m_Style | 
          (readonly ? wxTE_READONLY: 0),
        (m_Validator != NULL ? *m_Validator: wxDefaultValidator));
      break;

    case ITEM_TOGGLEBUTTON:
      m_Window = new wxToggleButton(parent, m_Id, m_Label,
        wxDefaultPosition, wxDefaultSize, m_Style);
      break;

    case ITEM_USER:
      wxASSERT(m_Window != NULL);
    
      if (m_UserWindowCreate != NULL)
      {
        (m_UserWindowCreate)(m_Window, parent, readonly);
      }
      break;
  
    default: wxFAIL;
  }

  if (m_Type != ITEM_EMPTY && m_Type != ITEM_SPACER)
  {
    wxASSERT(m_Window != NULL);
  }
}

wxFlexGridSizer* wxExItem::Layout(
  wxWindow* parent, 
  wxSizer* sizer, 
  bool readonly,
  wxFlexGridSizer* fgz)
{
  CreateWindow(parent, readonly);
  
  wxFlexGridSizer* use = fgz;

  switch (m_Type)
  {
    case ITEM_COMBOBOXDIR: use = AddBrowseButton(sizer); break;
    case ITEM_EMPTY: break;
    case ITEM_SPACER: sizer->AddSpacer(m_Style); break;

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
  
  return use;
}
#endif // wxUSE_GUI
