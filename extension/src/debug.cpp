////////////////////////////////////////////////////////////////////////////////
// Name:      debug.cpp
// Purpose:   Implementation of class wxExDebug
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/numdlg.h>
#include <wx/extension/debug.h>
#include <wx/extension/defs.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/menu.h>
#include <wx/extension/menus.h>
#include <wx/extension/process.h>
#include <wx/extension/shell.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

wxExDebug::wxExDebug(wxExManagedFrame* frame, wxExProcess* debug)
  : m_Frame(frame)
  , m_Process(debug)
{
  std::vector< wxExMenuCommands<wxExMenuCommand>> entries;
  
  if (wxExMenus::Load("debug", entries))
  {
    const size_t use = wxConfigBase::Get()->ReadLong("DEBUG", 0);
    m_Entry = entries[use < entries.size() ? use: 0];
  }
}
  
int wxExDebug::AddMenu(wxExMenu* menu, bool popup) const
{
  wxExMenu* sub = (popup ? new wxExMenu: nullptr);
  wxExMenu* use = (popup ? sub: menu);
  
  const int ret = wxExMenus::BuildMenu(
    m_Entry.GetCommands(), ID_EDIT_DEBUG_FIRST, use, popup);
  
  if (ret > 0 && popup)
  {
    menu->AppendSeparator();
    menu->AppendSubMenu(sub, "debug");
  }

  return ret;
}
  
bool wxExDebug::DeleteAllBreakpoints(const std::string& text)
{
  std::vector<std::string> v;

  if (wxExMatch("(d|del|delete|Delete) (all )?breakpoints", text, v) >= 1)
  {
    for (const auto& it: m_Breakpoints)
    {
      if (m_Frame->IsOpen(std::get<0>(it.second)))
      {
        wxExSTC* stc = m_Frame->OpenFile(std::get<0>(it.second));
        stc->MarkerDeleteAll(m_MarkerBreakpoint.GetNo());
      }
    }

    m_Breakpoints.clear();
    
    return true;
  }
  
  return false;
}
  
bool wxExDebug::Execute(const std::string& action, wxExSTC* stc)
{
  std::string args;

  if (!GetArgs(action, args, stc) ||
     (m_Process == nullptr &&
     (m_Process = m_Frame->Process(m_Entry.GetName())) == nullptr))
     return false;

  m_Frame->ShowPane("PROCESS"); 

  return m_Process->Write(action == "interrupt" ?
    std::string(1, 3) : action + args);
}

bool wxExDebug::GetArgs(
  const std::string& command, std::string& args, wxExSTC* stc)
{
  std::vector<std::string> v;

  if (wxExMatch("^(at|attach)", command, v) == 0)
  {
    const long pid = wxGetNumberFromUser(
      _("Input:"),
      wxEmptyString,
      "pid",
      1, 1, 4194303);

    if (pid < 0) return false;
    
    args += " " + std::to_string(pid);
  }
  else if ((wxExMatch("^(br|break)", command, v) == 1) && stc != nullptr)
  {
    args += " " +
      stc->GetFileName().GetFullPath() + ":" + 
      std::to_string(stc->GetCurrentLine()); 
  }
  else if ((wxExMatch("^(d|del|delete) (br|breakpoint)", command, v) > 0) && stc != nullptr)
  {
    for (const auto& it: m_Breakpoints)
    {
      if (
        stc->GetFileName() == std::get<0>(it.second) &&
        stc->GetCurrentLine() == std::get<2>(it.second))
      {
        args += " " + it.first;
        break;
      }
    }
  }
  else if (DeleteAllBreakpoints(command)) {}
  else if (command == "file")
  {
    if (wxExItemDialog({
        {"File", ITEM_COMBOBOX, wxAny(), true},
        {m_Entry.GetName(), ITEM_FILEPICKERCTRL}},
      wxExWindowData().
        Title("Debug").
        Parent(m_Frame).
        Size(wxSize(500, 180))).ShowModal() == wxID_CANCEL) return false;
    
    args += " " + wxExConfigFirstOf("File"); 
  }
  else if ((wxExMatch("^(p|print)", command, v) == 1) && stc != nullptr)
  {
    args += " " + stc->GetSelectedText(); 
  }
  else if ((wxExMatch("(u|until)", command, v) == 1) && stc != nullptr)
  {
    args += " " + std::to_string(stc->GetCurrentLine());
  }
  
  return true;
}

void wxExDebug::ProcessStdIn(const std::string& text)
{
  std::vector<std::string> v;

  if (wxExMatch("(d|del|delete) (br|breakpoint) ([0-9]+)", text, v) == 3)
  {
    const auto& it = m_Breakpoints.find(v[2]);

    if (it != m_Breakpoints.end() && m_Frame->IsOpen(std::get<0>(it->second)))
    {
      wxExSTC* stc = m_Frame->OpenFile(std::get<0>(it->second));
      stc->MarkerDeleteHandle(std::get<1>(it->second));
    }
  }
  else 
  {
    DeleteAllBreakpoints(text);
  }
}

void wxExDebug::ProcessStdOut(const std::string& text)
{
  std::vector<std::string> v;
  int line = -1;

  if (
    wxExMatch("Breakpoint ([0-9]+) at 0x[0-9a-f]+: file (.*), line ([0-9]+)", text, v) == 3 || 
    wxExMatch("Breakpoint ([0-9]+) at 0x[0-9a-f]+: (.*):([0-9]+)", text, v) == 3)
  {
    wxExPath filename(v[1]);
    filename.MakeAbsolute();
    if (filename.FileExists())
    {
      wxExSTC* stc = m_Frame->OpenFile(filename);
      if (stc != nullptr)
      {
        const int id = stc->MarkerAdd(
          std::stoi(v[2]), m_MarkerBreakpoint.GetNo());
        m_Breakpoints[v[0]] = std::make_tuple(filename, id, std::stoi(v[2]));
      }
    }
  }
  else if (DeleteAllBreakpoints(text)) {}
  else if (wxExMatch("at (.*):([0-9]+)", text, v) > 1)
  {
    m_Path = wxExPath(v[0]);
    m_Path.MakeAbsolute();
    line = std::stoi(v[1]);
  }
  else if (wxExMatch("^([0-9]+)", text, v) > 0)
  {
    line = std::stoi(v[0]);
  }

  if (line > 0 && m_Path.FileExists())
  {
    m_Frame->OpenFile(m_Path, wxExControlData().Line(line));
    m_Process->GetShell()->SetFocus();
  }
}
