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

int wxExVCSCommand::m_Instances = 0;

wxExVCSCommand::wxExVCSCommand()
  : m_Command()
  , m_SubMenu()
  , m_No(0)
  , m_Type(VCS_COMMAND_IS_UNKNOWN) 
{
}

wxExVCSCommand::wxExVCSCommand(
  const wxString& command,
  const wxString& type,
  const wxString& submenu)
  : m_Command(command)
  , m_SubMenu(submenu)
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

const wxString wxExVCSCommand::GetCommand() const
{
  wxString command = m_Command;

  if (command.Contains("&"))
  {
    command.Replace("&", wxEmptyString);
  }

  return command;
}

bool wxExVCSCommand::IsAdd() const
{
  return 
    m_Command == "add";
}

bool wxExVCSCommand::IsCheckout() const
{
  return 
    m_Command == "checkout" ||
    m_Command == "co";
}

bool wxExVCSCommand::IsCommit() const
{
  return 
    m_Command == "commit" ||
    m_Command == "ci";
}

bool wxExVCSCommand::IsDiff() const
{
  return 
    m_Command == "diff";
}

bool wxExVCSCommand::IsHelp() const
{
  return 
    m_Command == "help";
}

bool wxExVCSCommand::IsOpen() const
{
  return
    m_Command == "blame" ||
    m_Command == "cat" ||
    m_Command == "diff";
}

bool wxExVCSCommand::IsUpdate() const
{
  return 
    m_Command == "update" ||
    m_Command == "up";
}

int wxExVCSEntry::m_Instances = 2; // TODO: VCS_AUTO + 1;

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

  for (
    auto it = m_Commands.begin();
    it != m_Commands.end();
    ++it)
  {
    bool add = false;

    if (!it->SubMenu().empty())
    {
      if (submenu == NULL)
      {
        submenu = new wxMenu();
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
      const wxString text = 
        (it->GetCommand().Contains("&") ? 
           wxExEllipsed(it->GetCommand()) :
          (wxExEllipsed("&" + it->GetCommand())));

      wxMenu* usemenu = (submenu == NULL ? menu: submenu);
      usemenu->Append(base_id + it->GetNo(), text);
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
        v.push_back(wxExVCSCommand(content, attrib, submenu));
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
