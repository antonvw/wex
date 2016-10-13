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
  , m_Debug(debug)
{
  wxExMenus::Load("gdb", m_Entries);
}
  
int wxExDebug::AddMenu(wxExMenu* menu, bool popup) const
{
  wxExMenu* sub = (popup ? new wxExMenu: nullptr);
  wxExMenu* use = (popup ? sub: menu);
  
  const int ret = wxExMenus::BuildMenu(
    m_Entries[0].GetCommands(), ID_EDIT_DEBUG_FIRST, use, popup);
  
  if (ret > 0 && popup)
  {
    menu->AppendSeparator();
    menu->AppendSubMenu(sub, "debug");
  }
  
  return ret;
}
  
bool wxExDebug::Execute(int no, wxExSTC* stc)
{
  const wxExMenuCommand item(m_Entries[0].GetCommands().at(no));
  std::string args;

  if (!GetArgs(item.GetCommand(), args, stc)) return false;
  
  if (m_Debug == nullptr)
  {
    if ((m_Debug = m_Frame->Process("gdb")) == nullptr) return false;
  }

  // Execute the command.
  bool success = m_Debug->Command(item.GetCommand() + args);
  
  // Update stc if necessary.
  if (success && stc != nullptr)
  {
    if (item.GetCommand() == "break")
    {
      stc->MarkerAdd(stc->GetCurrentLine(), m_Breakpoint.GetNo());
    }
    else if (item.GetCommand() == "del breakpoints")
    {
      stc->MarkerDeleteAll(m_Breakpoint.GetNo());
    }
  }

  m_Frame->ShowPane("PROCESS"); 

  return success;
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
  else if (command == "break")
  {
    args += " " +
      stc->GetFileName().GetFullPath() + ":" + std::to_string(stc->GetCurrentLine()); 
  }
  else if (command == "file")
  {
    if (wxExItemDialog(m_Frame, 
      std::vector <wxExItem> {{"File", ITEM_COMBOBOX, wxAny(), true}},
      "Debug").ShowModal() == wxID_CANCEL)
    {
      return false;
    }
    
    args += " " + wxExConfigFirstOf("File"); 
  }
  else if (command == "print")
  {
    args += " " + stc->GetSelectedText(); 
  }
  
  return true;
}
  
void wxExDebug::SetDebugProcess(wxExProcess* process)
{
  m_Debug = process;
}
