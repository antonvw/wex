////////////////////////////////////////////////////////////////////////////////
// Name:      vcsentry.cpp
// Purpose:   Implementation of wxExVCSEntry class
// Author:    Anton van Wezenbeek
// Created:   2010-08-27
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vcsentry.h>
#include <wx/extension/defs.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>

int wxExVCSEntry::m_Instances = wxExVCS::VCS_AUTO + 1;

wxExVCSEntry::wxExVCSEntry()
  : m_No(-1)
  , m_Name()
  , m_SupportKeywordExpansion(false)
{
}

wxExVCSEntry::wxExVCSEntry(const wxXmlNode* node)
  : m_No(m_Instances++)
  , m_Name(node->GetAttribute("name"))
  , m_SupportKeywordExpansion(
      node->GetAttribute("keyword-expansion") == "true")
{
  if (m_Name.empty())
  {
    wxLogError(_("Missing vcs on line: %d"), node->GetLineNumber());
  }
  else
  {
    wxXmlNode *child = node->GetChildren();

    while (child)
    {
      if (child->GetName() == "commands")
      {
        m_Commands = ParseNodeCommands(child);
      }
      
      child = child->GetNext();
    }
  }
}

#if wxUSE_GUI
int wxExVCSEntry::BuildMenu(int base_id, wxMenu* menu, bool is_popup) const
{
  wxMenu* submenu = NULL;
  
  wxString prev_menu = "XXXXX";

  for (
    auto it = m_Commands.begin();
    it != m_Commands.end();
    ++it)
  {
    bool add = false;

    if (!it->GetSubMenu().empty() && prev_menu != it->GetSubMenu())
    {
      if (submenu == NULL)
      {
        submenu = new wxMenu();
        prev_menu = it->GetSubMenu();
        menu->AppendSubMenu(submenu, it->GetSubMenu());
      }
    }
    else if (submenu != NULL)
    {
      submenu = NULL;
    }

    switch (it->GetType())
    {
      case wxExVCSCommand::VCS_COMMAND_IS_BOTH: add = true; break;
      case wxExVCSCommand::VCS_COMMAND_IS_POPUP: add = is_popup; break;
      case wxExVCSCommand::VCS_COMMAND_IS_MAIN: add = !is_popup; break;
      case wxExVCSCommand::VCS_COMMAND_IS_UNKNOWN: add = false; break;
      default: wxFAIL;
    }

    if (add)
    {
      wxMenu* usemenu = (submenu == NULL ? menu: submenu);
      usemenu->Append(
        base_id + it->GetNo(), 
        wxExEllipsed(it->GetCommand(false, true))); // use no sub and do accel
    }
  }

  return menu->GetMenuItemCount();
}

#endif

const wxExVCSCommand wxExVCSEntry::GetCommand(int command_id) const
{
  if (command_id >= m_Commands.size() || command_id < 0)
  {
    return wxExVCSCommand();
  }
  else
  {
    return m_Commands.at(command_id);
  }
}
  
const std::vector<wxExVCSCommand> wxExVCSEntry::ParseNodeCommands(
  const wxXmlNode* node) const
{
  std::vector<wxExVCSCommand> v;
  
  wxExVCSCommand::ResetInstances();

  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "command")
    {
      if (v.size() == VCS_MAX_COMMANDS)
      {
        wxLogError(_("Reached commands limit: %d"), VCS_MAX_COMMANDS);
      }
      else
      {
        const wxString content = child->GetNodeContent().Strip(wxString::both);
        const wxString attrib = child->GetAttribute("type");
        const wxString submenu = child->GetAttribute("submenu");
        const wxString subcommand = child->GetAttribute("subcommand");
        v.push_back(wxExVCSCommand(content, attrib, submenu, subcommand));
      }
    }
    
    child = child->GetNext();
  }

  return v;
}

void wxExVCSEntry::ResetInstances()
{
  m_Instances = wxExVCS::VCS_AUTO + 1;
}
