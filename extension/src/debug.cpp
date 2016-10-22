////////////////////////////////////////////////////////////////////////////////
// Name:      debug.cpp
// Purpose:   Implementation of class wxExDebug
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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
    const int use = wxConfigBase::Get()->ReadLong("DEBUG", 0);
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
  
bool wxExDebug::Execute(const std::string& item, wxExSTC* stc)
{
  for (const auto& it : m_Entry.GetCommands())
  {
    if (it.GetCommand() == item)
    {
      std::string args;

      if (!GetArgs(item, args, stc) ||
        ( m_Process == nullptr &&
         (m_Process = m_Frame->Process(m_Entry.GetName())) == nullptr))
         return false;

      m_Frame->ShowPane("PROCESS"); 

      return m_Process->Command(it.GetCommand() + args);
    }
  }
  
  return false;
}

bool wxExDebug::GetArgs(
  const std::string& command, std::string& args, wxExSTC* stc)
{
  if (command == "attach")
  {
    const long pid = wxGetNumberFromUser(
      _("Input:"),
      wxEmptyString,
      "pid",
      1, 1, 4194303);

    if (pid < 0) return false;
    
    args += " " + std::to_string(pid);
  }
  else if (command == "break" && stc != nullptr)
  {
    args += " " +
      stc->GetFileName().GetFullPath() + ":" + 
      std::to_string(stc->GetCurrentLine()); 
  }
  else if (command == "file")
  {
    if (wxExItemDialog(
      m_Frame, std::vector<wxExItem> {
        {"File", ITEM_COMBOBOX, wxAny(), true},
        {m_Entry.GetName(), ITEM_FILEPICKERCTRL}},
      "Debug").ShowModal() == wxID_CANCEL) return false;
    
    args += " " + wxExConfigFirstOf("File"); 
  }
  else if (command == "print" && stc != nullptr)
  {
    args += " " + stc->GetSelectedText(); 
  }
  else if (command == "until" && stc != nullptr)
  {
    args += " " + std::to_string(stc->GetCurrentLine());
  }
  
  return true;
}

void wxExDebug::ProcessInput(const std::string& text)
{
  std::vector<std::string> v;

  if (wxExMatch("(?:del|delete) breakpoint ([0-9]+)", text, v) > 0)
  {
    const auto& it = m_Breakpoints.find(v[0]);

    if (it != m_Breakpoints.end() && m_Frame->IsOpen(std::get<0>(it->second)))
    {
      wxExSTC* stc = m_Frame->OpenFile(std::get<0>(it->second));
      stc->MarkerDeleteHandle(std::get<1>(it->second));
    }
  }
  else if (wxExMatch("(del|delete) breakpoints", text, v) == 1)
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
  }
}

void wxExDebug::ProcessOutput(const std::string& text)
{
  std::vector<std::string> v;
  int line = -1;

  if (
    wxExMatch("Breakpoint ([0-9]+) at 0x[0-9a-f]+: file (.*), line ([0-9]+)", text, v) == 3 || 
    wxExMatch("Breakpoint ([0-9]+) at 0x[0-9a-f]+: (.*):([0-9]+)", text, v) == 3)
  {
    wxExFileName filename(v[1]);
    filename.MakeAbsolute();
    wxExSTC* stc = m_Frame->OpenFile(filename);
    const int id = stc->MarkerAdd(
      std::stoi(v[2]), m_MarkerBreakpoint.GetNo());
    m_Breakpoints[v[0]] = {filename, id, std::stoi(v[2])};
  }
  else if (wxExMatch("at (.*):([0-9]+)", text, v) > 1)
  {
    m_FileName = wxExFileName(v[0]);
    m_FileName.MakeAbsolute();
    line = std::stoi(v[1]);
  }
  else if (wxExMatch("^([0-9]+)", text, v) > 0)
  {
    line = std::stoi(v[0]);
  }

  if (line > 0 && m_FileName.FileExists())
  {
    m_Frame->OpenFile(m_FileName, line);
    m_Process->GetShell()->SetFocus();
  }
}
