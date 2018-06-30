////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.cpp
// Purpose:   Implementation of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/panel.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/debug.h>
#include <wx/extension/defs.h>
#include <wx/extension/ex.h>
#include <wx/extension/frd.h>
#include <wx/extension/process.h>
#include <wx/extension/shell.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/vi-macros.h>
#include <wx/extension/vi-macros-mode.h>

const int ID_REGISTER = wxWindow::NewControlId();

// Support class.
// Offers a text ctrl related to a ex object.
class wxExTextCtrl: public wxTextCtrl
{
public:
  // Constructor. Creates empty control.
  wxExTextCtrl(
    wxExManagedFrame* frame,
    wxStaticText* prefix,
    const wxExWindowData& data);
    
  // Returns ex component.
  auto* GetEx() {return m_ex;};
    
  // Sets ex component.
  // Returns false if command not supported.
  bool SetEx(wxExEx* ex, const std::string& command);
private:
  wxExTextCtrlInput& TCI() {
    switch (m_Command.Type())
    {
      case wxExExCommandType::CALC: return m_Calcs;
      case wxExExCommandType::EXEC: return m_Execs;
      case wxExExCommandType::FIND_MARGIN: return m_FindMargins;
      default: return m_Commands;
    }};
  wxExExCommand m_Command;
  wxExManagedFrame* m_Frame;
  wxExEx* m_ex {nullptr};
  wxStaticText* m_Prefix;
  bool m_ControlR {false}, m_ModeVisual {false}, m_UserInput {false};
  wxExTextCtrlInput 
    m_Calcs {wxExExCommandType::CALC},
    m_Commands {wxExExCommandType::COMMAND},
    m_Execs {wxExExCommandType::EXEC},
    m_FindMargins {wxExExCommandType::FIND_MARGIN};
};

wxExManagedFrame::wxExManagedFrame(size_t maxFiles, const wxExWindowData& data)
  : wxExFrame(data)
  , m_Debug(new wxExDebug(this))
  , m_FileHistory(maxFiles, wxID_FILE1)
  , m_FindBar(new wxExFindToolBar(this))
  , m_OptionsBar(new wxExOptionsToolBar(this))
  , m_ToolBar(new wxExToolBar(this))
  , m_ToggledPanes({
    {{"FINDBAR", _("&Findbar").ToStdString()}, ID_VIEW_LOWEST + 1},
    {{"OPTIONSBAR", _("&Optionsbar").ToStdString()}, ID_VIEW_LOWEST + 2},
    {{"TOOLBAR", _("&Toolbar").ToStdString()}, ID_VIEW_LOWEST + 3},
    {{"PROCESS", _("&Process").ToStdString()}, ID_VIEW_LOWEST + 4}})
{
  m_Manager.SetManagedWindow(this);
  AddToolBarPane(m_ToolBar, "TOOLBAR", _("Toolbar"));
  AddToolBarPane(m_FindBar,  "FINDBAR", _("Findbar"));
  AddToolBarPane(m_OptionsBar, "OPTIONSBAR", _("Optionsbar"));
  AddToolBarPane(CreateExPanel(), "VIBAR");
  m_Manager.Update();
  
  Bind(wxEVT_AUI_PANE_CLOSE, [=](wxAuiManagerEvent& event) {
    wxAuiPaneInfo* info = event.GetPane();
    info->BestSize(info->window->GetSize()).Fixed().Resizable();
    m_Manager.Update();
    // If this pane is a toolbar pane, it might have a checkbox,
    // update that as well.
    m_OptionsBar->Update(info->name, false);
    });
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    m_FileHistory.Save();
    event.Skip();});

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    DoRecent(m_FileHistory, event.GetId() - m_FileHistory.GetBaseId());},
    m_FileHistory.GetBaseId(), 
    m_FileHistory.GetBaseId() + m_FileHistory.GetMaxFiles());

  for (const auto& it : m_ToggledPanes)
  {
    Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
      event.Check(m_Manager.GetPane(it.first.first).IsShown());}, it.second);
    Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
      TogglePane(it.first.first);}, it.second);
  }
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExFindReplaceData::Get()->SetFindStrings(std::list < std::string > {});}, ID_CLEAR_FINDS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExSTC::ConfigDialog(
      wxExWindowData().
        Id(wxID_PREFERENCES).
        Parent(this).
        Title(_("Editor Options").ToStdString()).
        Button(wxAPPLY | wxOK | wxCANCEL));}, wxID_PREFERENCES);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_FileHistory.Clear();}, ID_CLEAR_FILES);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (auto* stc = GetSTC(); stc != nullptr)
    {
      auto it = wxExFindReplaceData::Get()->GetFindStrings().begin();
      std::advance(it, event.GetId() - ID_FIND_FIRST);
      if (const std::string text(*it); stc->FindNext(text, 
        stc->GetVi().GetIsActive()? stc->GetVi().GetSearchFlags(): -1))
      {
        wxExFindReplaceData::Get()->SetFindString(text);
      }
    }}, ID_FIND_FIRST, ID_FIND_LAST);
}

wxExManagedFrame::~wxExManagedFrame()
{
  delete m_Debug;
  
  m_Manager.UnInit();
}

bool wxExManagedFrame::AddToolBarPane(
  wxWindow* window, 
  const std::string& name,
  const wxString& caption)
{
  wxAuiPaneInfo pane;
  
  pane
    .LeftDockable(false)
    .RightDockable(false)
    .Name(name);

  // If the toolbar has a caption, it is at the top, 
  if (!caption.empty())
  {
    pane
      .Top()
      .ToolbarPane()
      .MinSize(-1, 30)
      .Caption(caption);
      
    // Initially hide special bars.
    if (name == "FINDBAR" || name == "OPTIONSBAR" )
    {
      pane.Hide();
    }
  }
  // otherwise (vi) fixed at the bottom and initially hidden.  
  else
  {
    pane
      .Bottom()
      .CloseButton(false)
      .Hide()
      .DockFixed(true)
      .Movable(false)
      .Row(10)
      .CaptionVisible(false);
  }
  
  return m_Manager.AddPane(window, pane);
}

bool wxExManagedFrame::AllowClose(wxWindowID id, wxWindow* page)
{
  // The page will be closed, so do not update find focus now.
  SetFindFocus(nullptr);
  return true;
}

void wxExManagedFrame::AppendPanes(wxMenu* menu) const
{
#ifdef __WXMSW__
  menu->AppendCheckItem(ID_VIEW_MENUBAR, _("&Menubar\tCtrl+I"));
#endif

  menu->AppendCheckItem(ID_VIEW_STATUSBAR, _("&Statusbar"));

  for (const auto& it : m_ToggledPanes)
  {
    if (it.first.first == "PROCESS" && wxExProcess::GetShell() == nullptr)
    {
      continue;
    }
    
    menu->AppendCheckItem(it.second, it.first.second);
  }
}
  
wxPanel* wxExManagedFrame::CreateExPanel()
{
  // An ex panel starts with small static text for : or /, then
  // comes the ex ctrl for getting user input.
  wxPanel* panel = new wxPanel(this);
  wxStaticText* text = new wxStaticText(panel, wxID_ANY, " ");
  m_TextCtrl = new wxExTextCtrl(this, text, 
    wxExWindowData().Style(wxTE_PROCESS_ENTER).Parent(panel));
  
  wxFlexGridSizer* sizer = new wxFlexGridSizer(2);
  sizer->AddGrowableCol(1);
  sizer->Add(text, wxSizerFlags().Center());
  sizer->Add(m_TextCtrl, wxSizerFlags().Expand());
  
  panel->SetSizerAndFit(sizer);

  return panel;
}

void wxExManagedFrame::DoRecent(
  const wxExFileHistory& history, 
  size_t index, 
  wxExSTCWindowFlags flags)
{
  if (const wxExPath file(history.GetHistoryFile(index)); !file.Path().empty())
  {
    OpenFile(file, wxExSTCData().Flags(flags));
  }
}

bool wxExManagedFrame::GetExCommand(wxExEx* ex, const std::string& command)
{
  return ShowPane("VIBAR") && m_TextCtrl->SetEx(ex, command);
}

void wxExManagedFrame::HideExBar(int hide)
{
  if (m_Manager.GetPane("VIBAR").IsShown())
  {
    if (hide == HIDE_BAR_FORCE || hide == HIDE_BAR_FORCE_FOCUS_STC ||
        (GetStatusBar() != nullptr && GetStatusBar()->IsShown()))
    {
      ShowPane("VIBAR", false);
    }
    
    if ((hide == HIDE_BAR_FOCUS_STC || hide == HIDE_BAR_FORCE_FOCUS_STC) && 
         m_TextCtrl != nullptr && 
         m_TextCtrl->GetEx() != nullptr)
    {
      m_TextCtrl->GetEx()->GetSTC()->SetFocus();
    }
  }
}
  
void wxExManagedFrame::OnNotebook(wxWindowID id, wxWindow* page)
{
  if (auto* stc = wxDynamicCast(page, wxExSTC); stc != nullptr)
  {
    SetRecentFile(stc->GetFileName());
  }
}

wxExSTC* wxExManagedFrame::OpenFile(
  const wxExPath& file,
  const wxExSTCData& stc_data)
{
  if (auto* stc = wxExFrame::OpenFile(file, stc_data); stc != nullptr)
  {
    SetRecentFile(file);
    return stc;
  }

  return nullptr;
}

void wxExManagedFrame::PrintEx(wxExEx* ex, const std::string& text)
{
  ex->Print(text);
}

void wxExManagedFrame::SetRecentFile(const wxExPath& path) 
{
  m_FileHistory.AddFileToHistory(path);
}

void wxExManagedFrame::ShowExMessage(const std::string& text)
{
  if (GetStatusBar() != nullptr && GetStatusBar()->IsShown())
  {
    HideExBar();
    GetStatusBar()->SetStatusText(text);
  }
  else
  {
    m_TextCtrl->SetValue(text);
  }
}

bool wxExManagedFrame::ShowPane(const std::string& pane, bool show)
{
  if (wxAuiPaneInfo& info = m_Manager.GetPane(pane); !info.IsOk())
  {
    return false;
  }
  else 
  {
    show ? info.Show(): info.Hide();
    m_Manager.Update();
    m_OptionsBar->Update(pane, show);
    return true;
  }
}
  
void wxExManagedFrame::SyncAll()
{
  if (auto* stc = GetSTC(); stc != nullptr)
  {
    stc->Sync(wxConfigBase::Get()->ReadBool("AllowSync", true));
  }
}

void wxExManagedFrame::SyncCloseAll(wxWindowID id)
{
  SetFindFocus(nullptr);
}

// Implementation of support class.

wxExTextCtrl::wxExTextCtrl(
  wxExManagedFrame* frame,
  wxStaticText* prefix,
  const wxExWindowData& data)
  : wxTextCtrl(
    data.Parent(), data.Id(), wxEmptyString, data.Pos(), data.Size(), data.Style())
  , m_Frame(frame)
  , m_Prefix(prefix)
{
  SetFont(wxConfigBase::Get()->ReadObject(_("Text font"), 
    wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)));

  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (event.GetUnicodeKey() != WXK_NONE)
    {
      if (event.GetKeyCode() == WXK_BACK)
      {
        if (!m_Command.empty()) m_Command.pop_back();
      }
      else if (event.GetKeyCode() != WXK_TAB)
      {
        m_Command.Append(event.GetUnicodeKey());
      }
    }
        
    switch (event.GetKeyCode())
    {
      case WXK_CONTROL_R: 
        m_ControlR = true; 
        break;

      case WXK_TAB: {
        if (m_ex != nullptr && m_ex->GetSTC()->GetFileName().FileExists())
        {
          wxExPath::Current(m_ex->GetSTC()->GetFileName().GetPath());
        }

        // studio not yet: [[maybe_unused]]
        if (const auto& [r, e, v] = wxExAutoCompleteFileName(m_Command.Command());
          r)
        {
          m_Command.Append(e);
          AppendText(e);
        }}
        break;

      default: {
        bool skip = true;
        if (event.GetKeyCode() != WXK_RETURN)
        {
          if (event.GetUnicodeKey() != (wxChar)WXK_NONE && m_ControlR)
          {
            skip = false;
            const char c = event.GetUnicodeKey();
            wxCommandEvent event(wxEVT_MENU, ID_REGISTER);
            if (c == '%')
            {
              if (m_ex != nullptr) event.SetString(m_ex->GetSTC()->GetFileName().GetFullName());
            }
            else 
            {
              event.SetString(wxExEx::GetMacros().GetRegister(c));
            }
            if (!event.GetString().empty())
            {
              wxPostEvent(this, event);
            }
          }
          m_UserInput = true;
        }
        m_ControlR = false;
        if (skip)
        {
          event.Skip();
        }
      }}});

  Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent& event) {
    switch (event.GetKeyCode())
    {
      case WXK_DOWN:
      case WXK_END:
      case WXK_HOME:
      case WXK_PAGEDOWN:
      case WXK_PAGEUP:
      case WXK_UP:
        if ((event.GetKeyCode() == WXK_HOME || event.GetKeyCode() == WXK_END) && 
            !event.ControlDown())
        {
          event.Skip();
        }
        else if (m_Command.Type() == wxExExCommandType::FIND)
        {
          wxExFindReplaceData::Get()->m_FindStrings.Set(event.GetKeyCode(), this); 
        }
        else
        {
          TCI().Set(event.GetKeyCode(), this); 
        }
        break;
        
      case WXK_ESCAPE:
        if (m_ex != nullptr)
        {
          m_ex->GetSTC()->PositionRestore();
        }
        m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC);
        m_ControlR = false;
        m_UserInput = false;
        break;
      
      default: event.Skip();
        break;
      }});
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    WriteText(event.GetString());}, ID_REGISTER);
  
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    event.Skip();
    if (m_ex != nullptr)
    {
      m_ex->GetSTC()->PositionSave();
    }});

  Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
    event.Skip();
    if (m_UserInput && m_ex != nullptr && m_Command.Type() == wxExExCommandType::FIND)
    {
      m_ex->GetSTC()->PositionRestore();
      m_ex->GetSTC()->FindNext(
        GetValue().ToStdString(),
        m_ex->GetSearchFlags(),
        m_Prefix->GetLabel() == "/");
    }});

  Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent& event) {
    if (m_ex == nullptr || GetValue().empty())
    {
      m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC);
      return;
    }

    if (!m_ex->GetMacros().Mode()->IsRecording())
    {
      m_Command.Command(m_Prefix->GetLabel().ToStdString() + GetValue().ToStdString());
    }

    m_Command.IsHandled(m_UserInput && m_Command.Type() == wxExExCommandType::FIND);

    if (m_Command.Exec())
    {
      int focus = (m_Command.Type() == wxExExCommandType::FIND ? 
        wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC: 
        wxExManagedFrame::HIDE_BAR_FOCUS_STC);

      if (m_Command.Type() == wxExExCommandType::FIND)
      {
        wxExFindReplaceData::Get()->SetFindString(GetValue().ToStdString());
      }
      else
      {
        TCI().Set(this); 

        if (m_Command.Type() == wxExExCommandType::COMMAND)
        {
          if (
            GetValue() == "gt" || 
            GetValue() == "n" || 
            GetValue() == "prev" || 
            GetValue().StartsWith("ta")) 
          {
            focus = wxExManagedFrame::HIDE_BAR_FORCE;
          }
          else if (GetValue().find("!") == 0) 
          {
            focus = wxExManagedFrame::HIDE_BAR;
          }
        }
      }

      m_Frame->HideExBar(focus);
    }});
}

bool wxExTextCtrl::SetEx(wxExEx* ex, const std::string& command) 
{
  if (command.empty()) return false;

  m_ex = ex;
  m_UserInput = false;
  const std::string range(command.substr(1));
  m_ModeVisual = !range.empty();
  m_Prefix->SetLabel(command.substr(0, 1));
  m_Command = wxExExCommand(ex->GetCommand()).Command(command);
  m_ControlR = false;

  switch (m_Command.Type())
  {
    case wxExExCommandType::CALC: 
    case wxExExCommandType::EXEC: 
    case wxExExCommandType::FIND_MARGIN: 
      SetValue(TCI().Get()); 
      SelectAll();
      break;

    case wxExExCommandType::COMMAND:
      if (command == ":!")
      {
        SetValue("!");
        SetInsertionPointEnd();
      }
      else if (!TCI().Get().empty())
      {
        SetValue(m_ModeVisual && TCI().Get().find(range) != 0 ? 
          range + TCI().Get(): TCI().Get()); 
        SelectAll();
      }
      else
      {
        SetValue(range); 
        SelectAll();
      }
      break;

    case wxExExCommandType::FIND: 
      SetValue(!m_ModeVisual ? ex->GetSTC()->GetFindString(): std::string()); 
      SelectAll();
      break;

    default:
      return false;
  }
    
  Show();
  SetFocus();

  return true;
}
