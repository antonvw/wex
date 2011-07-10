////////////////////////////////////////////////////////////////////////////////
// Name:      configitem.cpp
// Purpose:   Implementation of wxExConfigItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/button.h>
#include <wx/clrpicker.h> // for wxColourPickerWidget
#include <wx/commandlinkbutton.h>
#include <wx/config.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/hyperlink.h>
#include <wx/spinctrl.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/extension/configitem.h>
#include <wx/extension/frd.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

wxExConfigItem::wxExConfigItem(
  const wxString& label,
  wxExConfigType type,
  const wxString& page,
  bool is_required,
  int id,
  int max_items,
  bool add_label,
  int cols,
  long style)
  : m_Control(NULL)
  , m_Id(id)
  , m_IsRequired(is_required)
  , m_Min(0)
  , m_Max(1)
  , m_MaxItems(max_items)
  , m_MajorDimension(1)
  , m_Label(label)
  , m_Page(page)
  , m_Style(style)
  , m_Type(type)
  , m_Cols(cols)
  , m_AddLabel(
      type == CONFIG_BUTTON ||
      type == CONFIG_CHECKBOX ||
      type == CONFIG_COMMAND_LINK_BUTTON ||
      type == CONFIG_STATICLINE || 
      type == CONFIG_TOGGLEBUTTON ? false: add_label)
  , m_Inc(1)
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& label,
  wxControl* control,
  const wxString& page,
  bool is_required,
  bool add_label,
  int cols)
  : m_Control(control)
  , m_Id(control->GetId())
  , m_IsRequired(is_required)
  , m_Min(0)
  , m_Max(1)
  , m_MaxItems(1)
  , m_MajorDimension(1)
  , m_Label(label)
  , m_Page(page)
  , m_Style(0)
  , m_Type(CONFIG_USER)
  , m_Cols(cols)
  , m_AddLabel(add_label)
  , m_Inc(1)
{
}
    
wxExConfigItem::wxExConfigItem(
  const wxString& label,
  double min,
  double max,
  const wxString& page,
  wxExConfigType type,
  long style,
  double inc,
  int cols)
  : m_Control(NULL)
  , m_Id(wxID_ANY)
  , m_IsRequired(false)
  , m_MaxItems(0)
  , m_Inc(inc)
  , m_Label(label)
  , m_Page(page)
  , m_Style(style)
  , m_Type(type)
  , m_Cols(cols)
  , m_AddLabel(true)
  , m_Min(min)
  , m_Max(max)
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& label,
  const wxString& value,
  const wxString& page,
  long style,
  wxExConfigType type,
  bool is_required,
  bool add_label,
  int cols)
  : m_Control(NULL)
  , m_Id(wxID_ANY)
  , m_IsRequired(is_required)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Label(label)
  , m_Page(page)
  , m_Style(style)
  , m_Type(type)
  , m_Cols(cols)
  , m_AddLabel(
      (type != CONFIG_STATICTEXT && 
       type != CONFIG_HYPERLINKCTRL ? add_label: false))
  , m_Inc(1)
  , m_Default(value)
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& label,
  const std::map<long, const wxString> & choices,
  bool use_radiobox,
  const wxString& page,
  int majorDimension,
  long style,
  int cols)
  : m_Control(NULL)
  , m_Id(wxID_ANY)
  , m_IsRequired(false)
  , m_MajorDimension(majorDimension)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Label(label)
  , m_Page(page)
  , m_Style(style)
  , m_Type(use_radiobox ? CONFIG_RADIOBOX: CONFIG_CHECKLISTBOX)
  , m_Choices(choices)
  , m_Cols(cols)
  , m_AddLabel(false)
  , m_Inc(1)
{
}

wxExConfigItem::wxExConfigItem(
  const std::set<wxString> & choices,
  const wxString& page,
  int cols)
  : m_Control(NULL)
  , m_Id(wxID_ANY)
  , m_IsRequired(false)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Label("checklistbox_noname")
  , m_Page(page)
  , m_Style(0)
  , m_Type(CONFIG_CHECKLISTBOX_NONAME)
  , m_ChoicesBool(choices) 
  , m_Cols(cols)
  , m_AddLabel(false)
  , m_Inc(1)
{
}

wxFlexGridSizer* wxExConfigItem::AddBrowseButton(wxSizer* sizer) const
{
  wxASSERT(m_Control != NULL);

  wxFlexGridSizer* fgz = new wxFlexGridSizer(3, 0, 0);

  fgz->AddGrowableCol(1);

  AddStaticText(fgz);

  fgz->Add(m_Control, m_ControlFlags);

  // Tried to use a wxDirPickerCtrl here, is nice,
  // but does not use a combobox for keeping old values, so this is better.
  // And the text box that is used is not resizable as well.
  fgz->Add(
    new wxButton(
      m_Control->GetParent(),
      m_Control->GetId(),
      _(wxDirPickerWidgetLabel)),
    wxSizerFlags().Center().Border());

  sizer->Add(fgz, wxSizerFlags().Left().Expand()); // no border

  return fgz;
}

void wxExConfigItem::AddStaticText(wxSizer* sizer) const
{
  wxASSERT(!m_Label.empty());
  wxASSERT(m_Control != NULL);

  sizer->Add(
    new wxStaticText(m_Control->GetParent(), 
      wxID_ANY, 
      m_Label + ":"), 
      wxSizerFlags().Right().Border());
}

void wxExConfigItem::CreateControl(wxWindow* parent, bool readonly)
{
  const int width = 200;
  const int width_numeric = 75;

  // Default control is expanded when laid out, 
  // override in switch if not wanted.
  bool expand = true;

  switch (m_Type)
  {
    case CONFIG_BUTTON:
      m_Control = new wxButton(parent, m_Id);
      ((wxButton *)m_Control)->SetLabelMarkup(m_Label);
      expand = false;
      break;

    case CONFIG_CHECKBOX:
      m_Control = new wxCheckBox(parent, m_Id, m_Label);
      break;

    case CONFIG_CHECKLISTBOX:
      {
      wxArrayString arraychoices;

      for (
        auto it = m_Choices.begin();
        it != m_Choices.end();
       ++it)
      {
        arraychoices.Add(it->second);
      }

      m_Control = new wxCheckListBox(parent,
        m_Id, wxDefaultPosition, wxDefaultSize, arraychoices);
      }
      break;

    case CONFIG_CHECKLISTBOX_NONAME:
      {
      wxArrayString arraychoices;
      arraychoices.resize(m_ChoicesBool.size()); // required!
      copy (m_ChoicesBool.begin(), m_ChoicesBool.end(), arraychoices.begin());
      m_Control = new wxCheckListBox(parent,
        m_Id, wxDefaultPosition, wxDefaultSize, arraychoices);
      }
      break;

    case CONFIG_COLOUR:
      m_Control = new wxColourPickerWidget(parent, m_Id);
      expand = false;
      break;

    case CONFIG_COMBOBOX:
    case CONFIG_COMBOBOXDIR:
      m_Control = new wxComboBox(
        parent, 
        m_Id,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(250, wxDefaultCoord));
      break;

    case CONFIG_COMMAND_LINK_BUTTON:
      m_Control = new wxCommandLinkButton(
        parent, 
        m_Id, 
        m_Label.BeforeFirst('\t'), 
        m_Label.AfterFirst('\t'));
      expand = false;
      break;

    case CONFIG_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = new wxDirPickerCtrl(parent,
        m_Id,
        wxEmptyString,
        _(wxDirSelectorPromptStr),
        wxDefaultPosition,
        wxSize(width, wxDefaultCoord));

      m_Control = pc;

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

      wxFilePickerCtrl* pc = new wxFilePickerCtrl(parent,
        m_Id,
        wxEmptyString,
        wxFileSelectorPromptStr,
        wc,
        wxDefaultPosition,
        wxSize(width, wxDefaultCoord));

      m_Control = pc;

      if (pc->GetTextCtrl() != NULL && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case CONFIG_FONTPICKERCTRL:
      {
      wxFontPickerCtrl* pc = new wxFontPickerCtrl(parent,
        m_Id,
        wxNullFont,
        wxDefaultPosition,
        wxSize(width, wxDefaultCoord));

      m_Control = pc;

      if (pc->GetTextCtrl() != NULL && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case CONFIG_HYPERLINKCTRL:
      {
#if wxUSE_HYPERLINKCTRL
      m_Control = new wxHyperlinkCtrl(parent,
        m_Id,
        m_Label,
        m_Default,
        wxDefaultPosition,
        wxSize(width, wxDefaultCoord));
      expand = false;
#endif      
      }
      break;

    case CONFIG_INT:
      m_Control = new wxTextCtrl(parent,
        m_Id,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(width_numeric, wxDefaultCoord),
        m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
        wxTextValidator(wxFILTER_NUMERIC));
      expand = false;
      break;

    case CONFIG_RADIOBOX:
      {
      wxArrayString arraychoices;

      for (
        auto it = m_Choices.begin();
        it != m_Choices.end();
        ++it)
      {
        arraychoices.Add(it->second);
      } 

      m_Control = new wxRadioBox(parent,
        m_Id, 
        m_Label, 
        wxDefaultPosition, 
        wxDefaultSize, 
        arraychoices, 
        m_MajorDimension,
        m_Style);
      }

      break;

    case CONFIG_SLIDER:
      m_Control = new wxSlider(parent,
        m_Id,
        0,
        m_Min,
        m_Max,
        wxDefaultPosition,
        wxSize(width_numeric, wxDefaultCoord),
        m_Style);
      expand = false;
      break;

    case CONFIG_SPINCTRL:
      m_Control = new wxSpinCtrl(parent,
        m_Id,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(width_numeric, wxDefaultCoord),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        m_Min,
        m_Max);
      expand = false;
      break;

    case CONFIG_SPINCTRL_DOUBLE:
      m_Control = new wxSpinCtrlDouble(parent,
        m_Id,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(width_numeric, wxDefaultCoord),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        m_Min,
        m_Max,
        m_Min,
        m_Inc);
      expand = false;
      break;

    case CONFIG_STATICLINE:
      m_Control = new wxStaticLine(
        parent,
        m_Id,
        wxDefaultPosition,
        wxDefaultSize,
        (m_Style == 0 ? wxLI_HORIZONTAL: wxLI_VERTICAL));
      break;

    case CONFIG_STATICTEXT:
      m_Control = new wxStaticText(parent,
        m_Id,
        wxEmptyString,
        wxDefaultPosition,
        (m_Style & wxTE_MULTILINE ?
           wxSize(width, 200):
           wxSize(width, wxDefaultCoord)),
        m_Style);
      ((wxStaticText* )m_Control)->SetLabelMarkup(m_Label);
      break;

    case CONFIG_STRING:
      m_Control = new wxTextCtrl(parent,
        m_Id,
        wxEmptyString,
        wxDefaultPosition,
        (m_Style & wxTE_MULTILINE ?
           wxSize(width, 200):
           wxSize(width, wxDefaultCoord)),
        m_Style | 
          (readonly ? wxTE_READONLY: 0));
      break;

    case CONFIG_TOGGLEBUTTON:
      m_Control = new wxToggleButton(parent, m_Id, m_Label);
      expand = false;
      break;

    case CONFIG_USER:
      wxASSERT(m_Control != NULL);
      UserControlCreate(parent, readonly);
      break;
  
    default: wxFAIL;
  }

  m_ControlFlags = wxSizerFlags().Border().Left();
  if (expand) m_ControlFlags.Expand();

  wxASSERT(m_Control != NULL);
}

wxFlexGridSizer* wxExConfigItem::Layout(
  wxWindow* parent, 
  wxSizer* sizer, 
  bool readonly,
  wxFlexGridSizer* fgz)
{
  CreateControl(parent, readonly);
  
  wxFlexGridSizer* use = fgz;

  if (m_Type == CONFIG_COMBOBOXDIR)
  {
    use = AddBrowseButton(sizer);
  }
  else
  {
    if (m_AddLabel)
    {
      // Construct a child flex grid sizer.
      if (fgz == NULL)
      {
        use = new wxFlexGridSizer(2, 0, 0);
        use->AddGrowableCol(1); // the control
        use->AddGrowableRow(0);
      
        // Add label and control.
        AddStaticText(use);
        use->Add(m_Control, m_ControlFlags);

        // Add to the sizer.
        sizer->Add(use, wxSizerFlags().Expand());
      }
      else
      {
        AddStaticText(fgz);
        fgz->Add(m_Control, m_ControlFlags);
      }
    }
    else
    {
      sizer->Add(m_Control, m_ControlFlags);
    }
  }
  
  ToConfig(false);
  
  return use;
}

bool wxExConfigItem::ToConfig(bool save) const
{
  wxASSERT(m_Control != NULL);

  switch (m_Type)
  {
    case CONFIG_BUTTON:
    case CONFIG_COMMAND_LINK_BUTTON:
    case CONFIG_HYPERLINKCTRL:
    case CONFIG_STATICLINE:
    case CONFIG_STATICTEXT:
      // these controls have no persistent info
      return false;
      break;

    case CONFIG_CHECKBOX:
      {
      wxCheckBox* cb = (wxCheckBox*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Label, cb->GetValue());
      else
        cb->SetValue(wxConfigBase::Get()->ReadBool(m_Label, false));
      }
      break;

    case CONFIG_CHECKLISTBOX:
      {
      wxCheckListBox* clb = (wxCheckListBox*)m_Control;

      long value = 0;
      if (!save)
        value = wxConfigBase::Get()->ReadLong(m_Label, 0);
      int item = 0;

      for (
        auto b = m_Choices.begin();
        b != m_Choices.end();
        ++b)
      {
        if (save)
        {
          if (clb->IsChecked(item))
          {
            value |= b->first;
          }
        }
        else
        {
          clb->Check(item, (value & b->first) > 0);
        }

        item++;
      }

      if (save)
        wxConfigBase::Get()->Write(m_Label, value);
      }
      break;

    case CONFIG_CHECKLISTBOX_NONAME:
      {
      wxCheckListBox* clb = (wxCheckListBox*)m_Control;

      for (size_t i = 0; i < clb->GetCount(); i++)
      {
        if (save)
        {
          if (!wxExFindReplaceData::Get()->Set(
            clb->GetString(i),
            clb->IsChecked(i)))
          {
            wxConfigBase::Get()->Write(clb->GetString(i), clb->IsChecked(i));
          }
        }
        else
        {
          if (!wxExFindReplaceData::Get()->Get(clb->GetString(i), clb, i))
          {
            clb->Check(i, wxConfigBase::Get()->ReadBool(clb->GetString(i), false));
          }
        }
      }
      }
      break;

    case CONFIG_COLOUR:
      {
      wxColourPickerWidget* gcb = (wxColourPickerWidget*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Label, gcb->GetColour());
      else
        gcb->SetColour(wxConfigBase::Get()->ReadObject(m_Label, *wxWHITE));
      }
      break;

    case CONFIG_COMBOBOX:
    case CONFIG_COMBOBOXDIR:
      {
      wxComboBox* cb = (wxComboBox*)m_Control;

      if (save)
      {
        const auto l = wxExComboBoxToList(cb, m_MaxItems);

        wxExListToConfig(l, m_Label);

        if (m_Label == wxExFindReplaceData::Get()->GetTextFindWhat())
        {
          wxExFindReplaceData::Get()->SetFindStrings(l);
        }
        else if (m_Label == wxExFindReplaceData::Get()->GetTextReplaceWith())
        {
          wxExFindReplaceData::Get()->SetReplaceStrings(l);
        }
      }
      else
      {
        wxExComboBoxFromList(cb, wxExListFromConfig(m_Label));
      }
      }
      break;

    case CONFIG_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = (wxDirPickerCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Label, pc->GetPath());
      else
        pc->SetPath(wxConfigBase::Get()->Read(m_Label));
      }
      break;

    case CONFIG_FILEPICKERCTRL:
      {
      wxFilePickerCtrl* pc = (wxFilePickerCtrl*)m_Control;
      
      if (save)
      {
        wxConfigBase::Get()->Write(m_Label, pc->GetPath());
      }
      else
      {
        const wxString val(wxConfigBase::Get()->Read(m_Label));
        
        if (!val.empty())
        {
          pc->SetPath(val);
        }
      }
      }
      break;

    case CONFIG_FONTPICKERCTRL:
      {
      wxFontPickerCtrl* pc = (wxFontPickerCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Label, pc->GetSelectedFont());
      else
        pc->SetFont(
          wxConfigBase::Get()->ReadObject(m_Label, 
          wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)));
      }
      break;

    case CONFIG_INT:
      {
      wxTextCtrl* tc = (wxTextCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Label, atol(tc->GetValue().c_str()));
      else
        tc->SetValue(
          wxString::Format("%ld", wxConfigBase::Get()->ReadLong(m_Label, 0)));
      }
      break;

    case CONFIG_RADIOBOX:
      {
      wxRadioBox* rb = (wxRadioBox*)m_Control;

      if (save)
      {
        for (
          auto b = m_Choices.begin();
          b != m_Choices.end();
          ++b)
        {
          if (b->second == rb->GetStringSelection())
          {
            wxConfigBase::Get()->Write(m_Label, b->first);
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

    case CONFIG_SLIDER:
      {
      wxSlider* sl = (wxSlider*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Label, sl->GetValue());
      else
        sl->SetValue(wxConfigBase::Get()->ReadLong(m_Label, m_Min));
      }
      break;

    case CONFIG_SPINCTRL:
      {
      wxSpinCtrl* sc = (wxSpinCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Label, sc->GetValue());
      else
        sc->SetValue(wxConfigBase::Get()->ReadLong(m_Label, m_Min));
      }
      break;

    case CONFIG_SPINCTRL_DOUBLE:
      {
      wxSpinCtrlDouble* sc = (wxSpinCtrlDouble*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Label, sc->GetValue());
      else
        sc->SetValue(wxConfigBase::Get()->ReadDouble(m_Label, m_Min));
      }
      break;

    case CONFIG_STRING:
      {
      wxTextCtrl* tc = (wxTextCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Label, tc->GetValue());
      else
        tc->SetValue(wxConfigBase::Get()->Read(m_Label));
      }
      break;

    case CONFIG_TOGGLEBUTTON:
      {
      wxToggleButton* ctrl = (wxToggleButton*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Label, ctrl->GetValue());
      else
        ctrl->SetValue(wxConfigBase::Get()->ReadBool(m_Label, false));
      }
      break;
      
    case CONFIG_USER:
      return UserControlToConfig(save);
      break;
      
    default:
      wxFAIL;
      return false;
      break;
  }

  return true;
}
#endif // wxUSE_GUI
