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

void wxExConfigItem::ToConfig(bool save) const
{
    switch (m_Type)
    {
    case CONFIG_CHECKBOX:
      {
      wxCheckBox* cb = (wxCheckBox*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(cb->GetName(), cb->GetValue());
      else
        cb->SetValue(wxConfigBase::Get()->ReadBool(cb->GetName(), false));
      }
      break;

    case CONFIG_CHECKLISTBOX:
      {
      wxCheckListBox* clb = (wxCheckListBox*)m_Control;

      long value = 0;
      if (!save)
        value = wxConfigBase::Get()->ReadLong(clb->GetName(), 0);
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
        wxConfigBase::Get()->Write(clb->GetName(), value);
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
          // Special case, should be taken from find replace data.
          if (*b == wxExFindReplaceData::Get()->GetTextMatchWholeWord())
          {
            wxExFindReplaceData::Get()->SetMatchWord(clb->IsChecked(item));
          }
          else if (*b == wxExFindReplaceData::Get()->GetTextMatchCase())
          {
            wxExFindReplaceData::Get()->SetMatchCase(clb->IsChecked(item));
          }
          else if (*b == wxExFindReplaceData::Get()->GetTextRegEx())
          {
            wxExFindReplaceData::Get()->SetIsRegularExpression(clb->IsChecked(item));
          }
          else
          {
            wxConfigBase::Get()->Write(*b, clb->IsChecked(item));
          }
        }
        else
        {
          // Special cases, should be taken from the find replace data.
          if (*b == wxExFindReplaceData::Get()->GetTextMatchWholeWord())
          {
            clb->Check(item, wxExFindReplaceData::Get()->MatchWord());
          }
          else if (*b == wxExFindReplaceData::Get()->GetTextMatchCase())
          {
            clb->Check(item, wxExFindReplaceData::Get()->MatchCase());
          }
          else if (*b == wxExFindReplaceData::Get()->GetTextRegEx())
          {
            clb->Check(item, wxExFindReplaceData::Get()->IsRegularExpression());
          }
          else
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
        wxConfigBase::Get()->Write(gcb->GetName(), gcb->GetColour());
      else
        gcb->SetColour(wxConfigBase::Get()->ReadObject(gcb->GetName(), *wxWHITE));
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

        wxExListToConfig(l, cb->GetName());

        if (cb->GetName() == wxExFindReplaceData::Get()->GetTextFindWhat())
        {
          wxExFindReplaceData::Get()->SetFindStrings(l);
        }
        else if (cb->GetName() == wxExFindReplaceData::Get()->GetTextReplaceWith())
        {
          wxExFindReplaceData::Get()->SetReplaceStrings(l);
        }
      }
      else
      {
        wxExComboBoxFromList(cb, wxExListFromConfig(cb->GetName()));
      }
      }
      break;

    case CONFIG_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = (wxDirPickerCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(pc->GetName(), pc->GetPath());
      else
        pc->SetPath(wxConfigBase::Get()->Read(pc->GetName()));
      }
      break;

    case CONFIG_FILEPICKERCTRL:
      {
      wxFilePickerCtrl* pc = (wxFilePickerCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(pc->GetName(), pc->GetPath());
      else
        pc->SetPath(wxConfigBase::Get()->Read(pc->GetName()));
      }
      break;

    case CONFIG_FONTPICKERCTRL:
      {
      wxFontPickerCtrl* pc = (wxFontPickerCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(pc->GetName(), pc->GetSelectedFont());
      else
        pc->SetFont(
          wxConfigBase::Get()->ReadObject(pc->GetName(), 
          wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)));
      }
      break;

    case CONFIG_INT:
      {
      wxTextCtrl* tc = (wxTextCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(tc->GetName(), atol(tc->GetValue().c_str()));
      else
        tc->SetValue(
          wxString::Format("%ld", wxConfigBase::Get()->ReadLong(tc->GetName(), 0)));
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
            wxConfigBase::Get()->Write(rb->GetName(), b->first);
          }
        }
      }
      else
      {
        std::map<long, const wxString>::const_iterator c = 
          m_Choices.find(wxConfigBase::Get()->ReadLong(rb->GetName(), 0));

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
        wxConfigBase::Get()->Write(tc->GetName(), tc->GetValue());
      else
        tc->SetValue(wxConfigBase::Get()->Read(tc->GetName()));
      }
      break;

    case CONFIG_SPINCTRL:
      {
      wxSpinCtrl* sc = (wxSpinCtrl*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(sc->GetName(), sc->GetValue());
      else
        sc->SetValue(wxConfigBase::Get()->ReadLong(sc->GetName(), m_Min));
      }
      break;

    case CONFIG_SPINCTRL_DOUBLE:
      {
      wxSpinCtrlDouble* sc = (wxSpinCtrlDouble*)m_Control;
      if (save)
        wxConfigBase::Get()->Write(sc->GetName(), sc->GetValue());
      else
        sc->SetValue(wxConfigBase::Get()->ReadDouble(sc->GetName(), m_MinDouble));
      }
      break;

    default:
      wxFAIL;
      break;
    }
}

#endif // wxUSE_GUI
