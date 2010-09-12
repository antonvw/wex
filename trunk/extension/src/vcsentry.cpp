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

int wxExVCSCommand::m_Instances = 0;

wxExVCSCommand::wxExVCSCommand()
  : m_Command()
  , m_SubMenu()
  , m_SubMenuIsCommand(false)
  , m_No(0)
  , m_Type(VCS_COMMAND_IS_UNKNOWN) 
{
}

wxExVCSCommand::wxExVCSCommand(
  const wxString& command,
  const wxString& type,
  const wxString& submenu,
  const wxString& subcommand)
  : m_Command(command)
  , m_SubMenu(!submenu.empty() ? submenu: subcommand)
  , m_SubMenuIsCommand(!subcommand.empty())
  , m_No(m_Instances++)
  , m_Type(From(type))
{
}
  
int wxExVCSCommand::From(const wxString& type) const
{
  if (type.IsEmpty())
  {
    return VCS_COMMAND_IS_BOTH;
  }
  else if (type == "popup")
  {
    return VCS_COMMAND_IS_POPUP;
  }
  else if (type == "main")
  {
    return VCS_COMMAND_IS_MAIN;
  }
  else
  {
    return VCS_COMMAND_IS_UNKNOWN;
  }
}

const wxString wxExVCSCommand::GetCommand(
  bool include_subcommand,
  bool include_accelerators) const
{
  wxString command = m_Command;

  if (m_SubMenuIsCommand && include_subcommand)
  {
    command += " " + m_SubMenu;
  }

  if (command.Contains("&") && !include_accelerators)
  {
    command.Replace("&", wxEmptyString);
  }

  return command;
}

bool wxExVCSCommand::IsAdd() const
{
  return 
    GetCommand(false) == "add";
}

bool wxExVCSCommand::IsCheckout() const
{
  return 
    GetCommand(false) == "checkout" ||
    GetCommand(false) == "co";
}

bool wxExVCSCommand::IsCommit() const
{
  return 
    GetCommand(false) == "commit" ||
    GetCommand(false) == "ci";
}

bool wxExVCSCommand::IsDiff() const
{
  return 
    GetCommand(false) == "diff";
}

bool wxExVCSCommand::IsHelp() const
{
  return 
    GetCommand(false) == "help";
}

bool wxExVCSCommand::IsOpen() const
{
  return
    GetCommand(false) == "blame" ||
    GetCommand(false) == "cat" ||
    GetCommand(false) == "diff";
}

bool wxExVCSCommand::IsUpdate() const
{
  return 
    GetCommand(false) == "update" ||
    GetCommand(false) == "up";
}

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
      else if (child->GetName() == "comment")
      {
        // Ignore comments.
      }
      else
      {
        wxLogError(_("Undefined tag: %s on line: %d"),
          child->GetName().c_str(),
          child->GetLineNumber());
      }

      child = child->GetNext();
    }
  }
}

#if wxUSE_GUI
void wxExVCSEntry::BuildMenu(int base_id, wxMenu* menu, bool is_popup) const
{
  wxMenu* submenu = NULL;
  
  wxString prev_menu = "XXXXX";

  for (
    auto it = m_Commands.begin();
    it != m_Commands.end();
    ++it)
  {
    bool add = false;

    if (!it->SubMenu().empty() && prev_menu != it->SubMenu())
    {
      if (submenu == NULL)
      {
        submenu = new wxMenu();
        prev_menu = it->SubMenu();
        menu->AppendSubMenu(submenu, it->SubMenu());
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
    else if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else
    {
      wxLogError(_("Undefined tag: %s on line: %d"),
        child->GetName().c_str(),
        child->GetLineNumber());
    }

    child = child->GetNext();
  }

  return v;
}
