////////////////////////////////////////////////////////////////////////////////
// Name:      configitem.cpp
// Purpose:   Implementation of wxExItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/checklst.h>
#include <wx/config.h>
#include <wx/spinctrl.h>
#include <wx/window.h>
#include <wx/extension/item.h>
#include <wx/extension/frd.h>
#include <wx/extension/tostring.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

#define PERSISTENT(READ, TYPE, DEFAULT)                            \
{                                                                  \
  if (save)                                                        \
    wxConfigBase::Get()->Write(GetLabel(), GetValue().As<TYPE>()); \
  else                                                             \
    SetValue(wxConfigBase::Get()->READ(GetLabel(), DEFAULT));      \
}                                                                  \

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

bool wxExItem::ToConfig(bool save) const
{
  if (!m_UseConfig)
  {
    return false;
  }
  
  switch (GetType())
  {
    case ITEM_CHECKBOX:           PERSISTENT(ReadBool, bool, false); break;
    case ITEM_CHECKLISTBOX_BIT:   PERSISTENT(ReadLong, long, 0); break;
    case ITEM_COLOURPICKERWIDGET: PERSISTENT(ReadObject, wxColour, *wxWHITE); break;
    case ITEM_DIRPICKERCTRL:      PERSISTENT(Read, wxString, GetLabel()); break;
    case ITEM_TEXTCTRL_FLOAT:     PERSISTENT(ReadDouble, double, 0); break;
    case ITEM_FONTPICKERCTRL:     
#ifdef __WXOSX__
                                  return false;
#else
                                  PERSISTENT(ReadObject, wxFont, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)); break;
#endif
    case ITEM_TEXTCTRL_INT:       PERSISTENT(ReadLong, long, 0); break;
    case ITEM_LISTVIEW:           PERSISTENT(Read, wxString, ""); break;
    case ITEM_SLIDER:             PERSISTENT(ReadLong, int, ((wxSlider* )GetWindow())->GetMin()); break;
    case ITEM_SPINCTRL:           PERSISTENT(ReadLong, int, ((wxSpinCtrl* )GetWindow())->GetMin()); break;
    case ITEM_SPINCTRLDOUBLE:     PERSISTENT(ReadDouble, double, ((wxSpinCtrlDouble* )GetWindow())->GetMin()); break;
    case ITEM_STC:                PERSISTENT(Read, wxString, ""); break;
    case ITEM_TEXTCTRL:           PERSISTENT(Read, wxString, ""); break;
    case ITEM_TOGGLEBUTTON:       PERSISTENT(ReadBool, bool, false); break;

    case ITEM_CHECKLISTBOX_BOOL:
      {
      wxCheckListBox* clb = (wxCheckListBox*)GetWindow();
      wxASSERT(clb != nullptr);

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
    case ITEM_COMBOBOX_DIR:
      {
      wxComboBox* cb = (wxComboBox*)GetWindow();

      if (save)
      {
        const auto l = wxExToListString(cb, m_MaxItems).Get();

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
          wxExListToConfig(l, GetLabel().ToStdString());
        }
      }
      else
      {
        wxExComboBoxFromList(cb, wxExListFromConfig(GetLabel().ToStdString()));
      }
      }
      break;

    case ITEM_FILEPICKERCTRL:
      if (save)
      {
        wxConfigBase::Get()->Write(GetLabel(), GetValue().As<wxString>());
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
        SetValue(wxConfigBase::Get()->Read(GetLabel(), initial));
      }
      break;

    case ITEM_RADIOBOX:
      {
      wxRadioBox* rb = (wxRadioBox*)GetWindow();

      if (save)
      {
        for (const auto& b : GetInitial().As<wxExItem::Choices>())
        {
          if (b.second == rb->GetStringSelection())
          {
            wxConfigBase::Get()->Write(GetLabel(), b.first);
          }
        }
      }
      else
      {
          const wxExItem::Choices & choices(GetInitial().As<wxExItem::Choices>());
        const auto c = choices.find(wxConfigBase::Get()->ReadLong(GetLabel(), 0));
        if (c != choices.end())
        {
          rb->SetStringSelection(c->second);
        }
      }
      }
      break;

    case ITEM_USER:
      if (m_UserWindowToConfig != nullptr &&
        !(m_UserWindowToConfig)(GetWindow(), save))
      { 
        return false;
      }
      break;
      
    default:
      // the other types have no persistent info
      return false;
      break;
  }

  if (m_Apply != nullptr)
  {
    (m_Apply)(m_Window, GetValue(), save);
  }

  return true;
}

wxExConfigDefaults::wxExConfigDefaults(
  const std::vector<std::tuple<wxString, wxExItemType, wxAny>> & items)
  : m_Config(wxConfigBase::Get())
{
  if (!m_Config->Exists(std::get<0>(items.front())))
  {
    m_Config->SetRecordDefaults(true);
    
    for (const auto& it : items)
    {
      switch (std::get<1>(it))
      {
        case ITEM_CHECKBOX:
          m_Config->ReadBool(std::get<0>(it), std::get<2>(it).As<bool>());
          break;
        case ITEM_COLOURPICKERWIDGET:
          m_Config->ReadObject(std::get<0>(it), std::get<2>(it).As<wxColour>());
          break;
        case ITEM_TEXTCTRL_FLOAT:
          m_Config->ReadDouble(std::get<0>(it), std::get<2>(it).As<double>());
          break;
        case ITEM_FONTPICKERCTRL:
          m_Config->ReadObject(std::get<0>(it), std::get<2>(it).As<wxFont>());
          break;
        case ITEM_TEXTCTRL_INT:
          m_Config->ReadLong(std::get<0>(it), std::get<2>(it).As<long>());
          break;
        case ITEM_TEXTCTRL:
          m_Config->Read(std::get<0>(it), std::get<2>(it).As<wxString>());
          break;
        default:
          std::cout << "Unsupported default type for: "  << 
            std::get<0>(it).c_str() << "\n";;
      }
    }
  }
}

wxExConfigDefaults::~wxExConfigDefaults()
{
  if (m_Config->IsRecordingDefaults())
  {
    m_Config->SetRecordDefaults(false);
  }
}
#endif // wxUSE_GUI
