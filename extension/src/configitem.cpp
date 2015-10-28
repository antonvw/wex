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
#include <wx/clrpicker.h> // for wxColourPickerWidget
#include <wx/checklst.h>
#include <wx/config.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/hyperlink.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/window.h>
#include <wx/extension/configitem.h>
#include <wx/extension/frd.h>
#include <wx/extension/listview.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

#define PERSISTENT(CONTROL, SETVALUE, GETVALUE, READ, DEFAULT)      \
{                                                                   \
  CONTROL* ctrl = (CONTROL*)GetWindow();                            \
  if (save)                                                         \
    wxConfigBase::Get()->Write(GetLabel(), ctrl->GETVALUE());       \
  else                                                              \
    ctrl->SETVALUE(wxConfigBase::Get()->READ(GetLabel(), DEFAULT)); \
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

wxFlexGridSizer* wxExConfigItem::Layout(
  wxWindow* parent, wxSizer* sizer, bool readonly, wxFlexGridSizer* fgz)
{
  wxFlexGridSizer* ret = wxExItem::Layout(parent, sizer, readonly, fgz);
  
  ToConfig(false);
  
  return ret;
}
  
bool wxExConfigItem::ToConfig(bool save) const
{
  switch (GetType())
  {
    case ITEM_CHECKBOX:        PERSISTENT(wxCheckBox, SetValue, IsChecked, ReadBool, false); break;
    case ITEM_COLOUR:          PERSISTENT(wxColourPickerWidget, SetColour, GetColour, ReadObject, *wxWHITE); break;
    case ITEM_DIRPICKERCTRL:   PERSISTENT(wxDirPickerCtrl, SetPath, GetPath, Read, GetLabel()); break;
    case ITEM_FONTPICKERCTRL:  PERSISTENT(wxFontPickerCtrl, SetSelectedFont, GetSelectedFont, ReadObject, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)); break;
    case ITEM_SLIDER:          PERSISTENT(wxSlider, SetValue, GetValue, ReadLong, ctrl->GetMin()); break;
    case ITEM_SPINCTRL:
    case ITEM_SPINCTRL_HEX:    PERSISTENT(wxSpinCtrl, SetValue, GetValue, ReadLong, ctrl->GetMin()); break;
    case ITEM_SPINCTRL_DOUBLE: PERSISTENT(wxSpinCtrlDouble, SetValue, GetValue, ReadDouble, ctrl->GetMin()); break;
    case ITEM_STC:             PERSISTENT(wxExSTC, SetValue, GetValue, Read, ""); break;
    case ITEM_STRING:          PERSISTENT(wxTextCtrl,SetValue,  GetValue, Read, ""); break;
    case ITEM_TOGGLEBUTTON:    PERSISTENT(wxToggleButton, SetValue, GetValue, ReadBool, false); break;

    case ITEM_CHECKLISTBOX:
      {
      wxCheckListBox* clb = (wxCheckListBox*)GetWindow();

      long value = 0;
      if (!save)
        value = wxConfigBase::Get()->ReadLong(GetLabel(), 0);
      int item = 0;

      for (const auto& b : GetChoices())
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
        wxConfigBase::Get()->Write(GetLabel(), value);
      }
      break;

    case ITEM_CHECKLISTBOX_NONAME:
      {
      wxCheckListBox* clb = (wxCheckListBox*)GetWindow();

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

    case ITEM_COMBOBOX:
    case ITEM_COMBOBOXDIR:
      {
      wxComboBox* cb = (wxComboBox*)GetWindow();

      if (save)
      {
        const auto& l = wxExComboBoxToList(cb, m_MaxItems);

        if (GetLabel() == wxExFindReplaceData::Get()->GetTextFindWhat())
        {
          wxExFindReplaceData::Get()->SetFindStrings(l);
        }
        else if (GetLabel() == wxExFindReplaceData::Get()->GetTextReplaceWith())
        {
          wxExFindReplaceData::Get()->SetReplaceStrings(l);
        }
        else
        {
          wxExListToConfig(l, GetLabel());
        }
      }
      else
      {
        wxExComboBoxFromList(cb, wxExListFromConfig(GetLabel()));
      }
      }
      break;

    case ITEM_FILEPICKERCTRL:
      {
      wxFilePickerCtrl* pc = (wxFilePickerCtrl*)GetWindow();
      
      if (save)
      {
        wxConfigBase::Get()->Write(GetLabel(), pc->GetPath());
      }
      else
      {
        wxString initial;

#ifdef __WXGTK__
        initial = "/usr/bin/" + GetLabel();
        if (!wxFileExists(initial))
        {
          initial.clear();
        }
#endif
        const wxString val(wxConfigBase::Get()->Read(GetLabel(), initial));
        
        if (!val.empty())
        {
          pc->SetPath(val);
        }
      }
      }
      break;

    case ITEM_FLOAT:
      {
      wxTextCtrl* tc = (wxTextCtrl*)GetWindow();
      
      if (save)
        wxConfigBase::Get()->Write(GetLabel(), atof(tc->GetValue().c_str()));
      else
        tc->SetValue(
          wxString::Format("%lf", wxConfigBase::Get()->ReadDouble(GetLabel(), 0)));
      }
      break;
      
    case ITEM_INT:
      {
      wxTextCtrl* tc = (wxTextCtrl*)GetWindow();
      if (save)
        wxConfigBase::Get()->Write(GetLabel(), atol(tc->GetValue().c_str()));
      else
        tc->SetValue(
          wxString::Format("%ld", wxConfigBase::Get()->ReadLong(GetLabel(), 0)));
      }
      break;

    case ITEM_LISTVIEW_FOLDER:
      {
      wxExListViewFileName* win = (wxExListViewFileName*)GetWindow();
      if (save)
        wxConfigBase::Get()->Write(GetLabel(), win->ItemToText(-1));
      else
      {
        win->DeleteAllItems();
        win->ItemFromText(wxConfigBase::Get()->Read(GetLabel()));
      }
      }
      break;

    case ITEM_RADIOBOX:
      {
      wxRadioBox* rb = (wxRadioBox*)GetWindow();
      if (save)
      {
        for (const auto& b : GetChoices())
        {
          if (b.second == rb->GetStringSelection())
          {
            wxConfigBase::Get()->Write(GetLabel(), b.first);
          }
        }
      }
      else
      {
        const auto c = GetChoices().find(wxConfigBase::Get()->ReadLong(GetLabel(), 0));

        if (c != GetChoices().end())
        {
          rb->SetStringSelection(c->second);
        }
      }
      }
      break;

    case ITEM_USER:
      if (m_UserWindowToConfig != NULL)
      {
        return (m_UserWindowToConfig)(GetWindow(), save);
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
