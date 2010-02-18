////////////////////////////////////////////////////////////////////////////////
// Name:      configitem.cpp
// Purpose:   Implementation of wxExConfigItem class
// Author:    Anton van Wezenbeek
// Created:   2009-11-10
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/clrpicker.h> // for wxColourPickerWidget
#include <wx/config.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/spinctrl.h>
#include <wx/extension/configitem.h>
#include <wx/extension/frd.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

wxExConfigItem::wxExConfigItem()
  : m_Name("spacer")
  , m_Page(wxEmptyString)
  , m_Type(CONFIG_SPACER) 
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& name,
  int min,
  int max,
  const wxString& page)
  : m_IsRequired(false)
  , m_Min(min)
  , m_Max(max)
  , m_MaxItems(0)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(CONFIG_SPINCTRL)
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& name,
  double min,
  double max,
  double inc,
  const wxString& page)
  : m_IsRequired(false)
  , m_MaxItems(0)
  , m_MinDouble(min)
  , m_MaxDouble(max)
  , m_Inc(inc)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(CONFIG_SPINCTRL_DOUBLE)
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& name,
  const wxString& page,
  long style,
  bool is_required)
  : m_IsRequired(is_required)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Name(name)
  , m_Page(page)
  , m_Style(style)
  , m_Type(CONFIG_STRING)
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& name,
  const std::map<long, const wxString> & choices,
  bool use_radiobox,
  const wxString& page)
  : m_IsRequired(false)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(use_radiobox ? CONFIG_RADIOBOX: CONFIG_CHECKLISTBOX)
  , m_Choices(choices)
{
}

wxExConfigItem::wxExConfigItem(
  const std::set<wxString> & choices,
  const wxString& page)
  : m_IsRequired(false)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Name("checklistbox_noname")
  , m_Page(page)
  , m_Style(0)
  , m_Type(CONFIG_CHECKLISTBOX_NONAME)
  , m_ChoicesBool(choices) 
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& name,
  int type,
  const wxString& page,
  bool is_required,
  int max_items)
  : m_IsRequired(is_required)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(max_items)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(type) 
{
}

void wxExConfigItem::Create(wxWindow* parent, bool readonly)
{
  const int width = 200;
  const int width_numeric = 75;

  switch (m_Type)
  {
    case CONFIG_CHECKBOX:
      m_Control = new wxCheckBox(parent,
        wxID_ANY,
        m_Name,
        wxDefaultPosition,
        wxSize(125, wxDefaultCoord));
      break;

    case CONFIG_CHECKLISTBOX:
      {
      wxArrayString arraychoices;

      for (
        std::map<long, const wxString>::const_iterator it = m_Choices.begin();
        it != m_Choices.end();
       ++it)
      {
        arraychoices.Add(it->second);
      }

      m_Control = (wxCheckListBox*)new wxCheckListBox(parent,
        wxID_ANY, wxDefaultPosition, wxDefaultSize, arraychoices);
      }
      break;

    case CONFIG_CHECKLISTBOX_NONAME:
      {
      wxArrayString arraychoices;
      arraychoices.resize(m_ChoicesBool.size()); // required!
      copy (m_ChoicesBool.begin(), m_ChoicesBool.end(), arraychoices.begin());
      m_Control = new wxCheckListBox(parent,
        wxID_ANY, wxDefaultPosition, wxDefaultSize, arraychoices);
      }
      break;

    case CONFIG_COLOUR:
      m_Control = new wxColourPickerWidget(parent, wxID_ANY);
      break;

    case CONFIG_COMBOBOX:
    case CONFIG_COMBOBOX_NONAME:
    case CONFIG_COMBOBOXDIR:
      m_Control = new wxComboBox(
        parent, 
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(250, wxDefaultCoord));
      break;

    case CONFIG_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = new wxDirPickerCtrl(parent,
        wxID_ANY,
        wxEmptyString,
        wxDirSelectorPromptStr,
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
      wxFilePickerCtrl* pc = new wxFilePickerCtrl(parent,
        wxID_ANY,
        wxEmptyString,
        wxFileSelectorPromptStr,
        wxFileSelectorDefaultWildcardStr,
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
        wxID_ANY,
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

    case CONFIG_INT:
      m_Control = new wxTextCtrl(parent,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(width_numeric, wxDefaultCoord),
        m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
        wxTextValidator(wxFILTER_NUMERIC));
      break;

    case CONFIG_RADIOBOX:
      {
      wxArrayString arraychoices;

      for (
        std::map<long, const wxString>::const_iterator it = m_Choices.begin();
        it != m_Choices.end();
        ++it)
      {
        arraychoices.Add(it->second);
      } 

      m_Control = new wxRadioBox(parent,
        wxID_ANY, 
        m_Name, 
        wxDefaultPosition, 
        wxDefaultSize, 
        arraychoices, 
        0, 
        wxRA_SPECIFY_ROWS);
      }

      break;

    case CONFIG_SPACER: break;

    case CONFIG_SPINCTRL:
      m_Control = new wxSpinCtrl(parent,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(width_numeric, wxDefaultCoord),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        m_Min,
        m_Max);
      break;

    case CONFIG_SPINCTRL_DOUBLE:
      m_Control = new wxSpinCtrlDouble(parent,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxSize(width_numeric, wxDefaultCoord),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        m_MinDouble,
        m_MaxDouble,
        m_MinDouble,
        m_Inc);
      break;

    case CONFIG_STRING:
      m_Control = new wxTextCtrl(parent,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        (m_Style & wxTE_MULTILINE ?
           wxSize(width, 200):
           wxSize(width, wxDefaultCoord)),
        m_Style | (readonly ? wxTE_READONLY: 0));
      break;

    default: wxFAIL;
  }

  wxASSERT(m_Control != NULL);
}

void wxExConfigItem::ToConfig(bool save) const
{
  switch (m_Type)
  {
    case CONFIG_CHECKBOX:
      {
      wxCheckBox* cb = (wxCheckBox*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Name, cb->GetValue());
      else
        cb->SetValue(wxConfigBase::Get()->ReadBool(m_Name, false));
      }
      break;

    case CONFIG_CHECKLISTBOX:
      {
      wxCheckListBox* clb = (wxCheckListBox*)m_Control;

      long value = 0;
      if (!save)
        value = wxConfigBase::Get()->ReadLong(m_Name, 0);
      int item = 0;

      for (
        std::map<long, const wxString>::const_iterator b = m_Choices.begin();
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
        wxConfigBase::Get()->Write(m_Name, value);
      }
      break;

    case CONFIG_CHECKLISTBOX_NONAME:
      {
      wxCheckListBox* clb = (wxCheckListBox*)m_Control;

      int item = 0;

      for (
        std::set<wxString>::const_iterator b = m_ChoicesBool.begin();
        b != m_ChoicesBool.end();
        ++b)
      {
        if (save)
        {
          if (!wxExFindReplaceData::Get()->Set(*b, clb->IsChecked(item)))
          {
            wxConfigBase::Get()->Write(*b, clb->IsChecked(item));
          }
        }
        else
        {
          if (!wxExFindReplaceData::Get()->Get(*b, clb, item))
          {
            clb->Check(item, wxConfigBase::Get()->ReadBool(*b, false));
          }
        }
      
        item++;
      }
      }
      break;

    case CONFIG_COLOUR:
      {
      wxColourPickerWidget* gcb = (wxColourPickerWidget*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Name, gcb->GetColour());
      else
        gcb->SetColour(wxConfigBase::Get()->ReadObject(m_Name, *wxWHITE));
      }
      break;

    case CONFIG_COMBOBOX:
    case CONFIG_COMBOBOXDIR:
    case CONFIG_COMBOBOX_NONAME:
      {
      wxComboBox* cb = (wxComboBox*)m_Control;

      if (save)
      {
        const std::list <wxString> l = wxExComboBoxToList(cb, m_MaxItems);

        wxExListToConfig(l, m_Name);

        if (m_Name == wxExFindReplaceData::Get()->GetTextFindWhat())
        {
          wxExFindReplaceData::Get()->SetFindStrings(l);
        }
        else if (m_Name == wxExFindReplaceData::Get()->GetTextReplaceWith())
        {
          wxExFindReplaceData::Get()->SetReplaceStrings(l);
        }
      }
      else
      {
        wxExComboBoxFromList(cb, wxExListFromConfig(m_Name));
      }
      }
      break;

    case CONFIG_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = (wxDirPickerCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Name, pc->GetPath());
      else
        pc->SetPath(wxConfigBase::Get()->Read(m_Name));
      }
      break;

    case CONFIG_FILEPICKERCTRL:
      {
      wxFilePickerCtrl* pc = (wxFilePickerCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Name, pc->GetPath());
      else
        pc->SetPath(wxConfigBase::Get()->Read(m_Name));
      }
      break;

    case CONFIG_FONTPICKERCTRL:
      {
      wxFontPickerCtrl* pc = (wxFontPickerCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Name, pc->GetSelectedFont());
      else
        pc->SetFont(
          wxConfigBase::Get()->ReadObject(m_Name, 
          wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)));
      }
      break;

    case CONFIG_INT:
      {
      wxTextCtrl* tc = (wxTextCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Name, atol(tc->GetValue().c_str()));
      else
        tc->SetValue(
          wxString::Format("%ld", wxConfigBase::Get()->ReadLong(m_Name, 0)));
      }
      break;

    case CONFIG_RADIOBOX:
      {
      wxRadioBox* rb = (wxRadioBox*)m_Control;

      if (save)
      {
        for (
          std::map<long, const wxString>::const_iterator b = m_Choices.begin();
          b != m_Choices.end();
          ++b)
        {
          if (b->second == rb->GetStringSelection())
          {
            wxConfigBase::Get()->Write(m_Name, b->first);
          }
        }
      }
      else
      {
        std::map<long, const wxString>::const_iterator c = 
          m_Choices.find(wxConfigBase::Get()->ReadLong(m_Name, 0));

        if (c != m_Choices.end())
        {
          rb->SetStringSelection(c->second);
        }
      }
      }
      break;

    case CONFIG_SPACER:
      break;

    case CONFIG_STRING:
      {
      wxTextCtrl* tc = (wxTextCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Name, tc->GetValue());
      else
        tc->SetValue(wxConfigBase::Get()->Read(m_Name));
      }
      break;

    case CONFIG_SPINCTRL:
      {
      wxSpinCtrl* sc = (wxSpinCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Name, sc->GetValue());
      else
        sc->SetValue(wxConfigBase::Get()->ReadLong(m_Name, m_Min));
      }
      break;

    case CONFIG_SPINCTRL_DOUBLE:
      {
      wxSpinCtrlDouble* sc = (wxSpinCtrlDouble*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(m_Name, sc->GetValue());
      else
        sc->SetValue(wxConfigBase::Get()->ReadDouble(m_Name, m_MinDouble));
      }
      break;

    default:
      wxFAIL;
      break;
  }
}

#endif // wxUSE_GUI
