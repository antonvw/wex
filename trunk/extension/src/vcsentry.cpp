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
#include <wx/config.h>
#include <wx/extension/vcsentry.h>
#include <wx/extension/defs.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>

int wxExVCSEntry::m_Instances = wxExVCS::VCS_AUTO + 1;

wxExVCSEntry::wxExVCSEntry()
  : m_No(-1)
  , m_CommandId(0)
  , m_Name()
  , m_FlagsLocation(VCS_FLAGS_LOCATION_POSTFIX)
  , m_SupportKeywordExpansion(false)
{
  m_Commands.push_back(wxExVCSCommand());
}

wxExVCSEntry::wxExVCSEntry(const wxXmlNode* node)
  : m_No(m_Instances++)
  , m_CommandId(0)
  , m_Name(node->GetAttribute("name"))
  , m_FlagsLocation(
      (node->GetAttribute("flags-location") == "prefix" ?
         VCS_FLAGS_LOCATION_PREFIX: VCS_FLAGS_LOCATION_POSTFIX))
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
    
    bool error = false;

    while (child && !error)
    {
      if (child->GetName() == "commands")
      {
        if (m_Commands.size() == 0)
        {
          AddCommands(child);
        }
        else
        {
          error = true;
        }
      }
      
      child = child->GetNext();
    }
  }
  
  if (m_Commands.size() == 0)
  {
    wxLogError(_("No commands found for: ") + m_Name);
    m_Commands.push_back(wxExVCSCommand());
  }  
}

void wxExVCSEntry::AddCommands(const wxXmlNode* node)
{
  wxExVCSCommand::ResetInstances();

  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "command")
    {
      if (m_Commands.size() == VCS_MAX_COMMANDS)
      {
        wxLogError(_("Reached commands limit: %d"), VCS_MAX_COMMANDS);
      }
      else
      {
        const wxString content = child->GetNodeContent().Strip(wxString::both);
        const wxString attrib = child->GetAttribute("type");
        const wxString submenu = child->GetAttribute("submenu");
        const wxString subcommand = child->GetAttribute("subcommand");
        
        m_Commands.push_back(
          wxExVCSCommand(content, attrib, submenu, subcommand));
      }
    }
    
    child = child->GetNext();
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
      submenu = new wxMenu();
      prev_menu = it->GetSubMenu();
      menu->AppendSeparator();
      menu->AppendSubMenu(submenu, it->GetSubMenu());
    }
    else if (it->GetSubMenu().empty())
    {
      if (prev_menu != "XXXXX")
      {
        prev_menu = "XXXXX";
        menu->AppendSeparator();
      }

      submenu = NULL;
    }
    
    const long type = it->GetType() & 0x000F;

    switch (type)
    {
      case wxExVCSCommand::VCS_COMMAND_IS_BOTH: add = true; break;
      case wxExVCSCommand::VCS_COMMAND_IS_POPUP: add = is_popup; break;
      case wxExVCSCommand::VCS_COMMAND_IS_MAIN: add = !is_popup; break;
      default: wxFAIL;
    }

    if (add)
    {
      wxMenu* usemenu = (submenu == NULL ? menu: submenu);
      usemenu->Append(
        base_id + it->GetNo(), 
        wxExEllipsed(it->GetCommand(false, true))); // use no sub and do accel
        
      const long sep  = it->GetType() & 0x00F0;
      
      if (sep)
      {
        usemenu->AppendSeparator();
      }
    }
  }

  return menu->GetMenuItemCount();
}

#endif

long wxExVCSEntry::Execute(
  const wxExFileName& filename,
  const wxString& args,
  const wxString& wd)
{
  const wxString bin = wxConfigBase::Get()->Read(m_Name, "svn");

  if (bin.empty())
  {
    wxLogError(m_Name + " " + _("path is empty"));
    return -1;
  }
  
  m_FileName = filename;
  
  wxString prefix;
  
  if (m_FlagsLocation == wxExVCSEntry::VCS_FLAGS_LOCATION_PREFIX)
  {
    prefix += wxConfigBase::Get()->Read(_("Prefix flags")) + " ";
  }
  
  wxString comment;

  if (GetCommand().IsCommit())
  {
    comment = 
      "-m \"" + wxExConfigFirstOf(_("Revision comment")) + "\" ";
  }

  wxString subcommand;
  
  if (GetCommand().UseSubcommand())
  {
    subcommand = wxConfigBase::Get()->Read(_("Subcommand"));

    if (!subcommand.empty())
    {
      subcommand += " ";
    }
  }

  wxString flags;
  wxString my_args(args);

  if (GetCommand().UseFlags())
  {
    flags = wxConfigBase::Get()->Read(_("Flags"));

    if (!flags.empty())
    {
      flags += " ";

      // If we specified help flags, we do not need a file argument.      
      if (flags.Contains("help"))
      {
        my_args.clear();
      }
    }
  }

  return wxExCommand::Execute(
    bin + " " + 
    prefix + 
    GetCommand().GetCommand() + " " + 
    subcommand + flags + comment + args, wd);
}

void wxExVCSEntry::ResetInstances()
{
  m_Instances = wxExVCS::VCS_AUTO + 1;
}

void wxExVCSEntry::SetCommand(int menu_id)
{
  if (menu_id > ID_VCS_LOWEST && menu_id < ID_VCS_HIGHEST)
  {
    m_CommandId = menu_id - ID_VCS_LOWEST - 1;
  }
  else if (menu_id > ID_EDIT_VCS_LOWEST && menu_id < ID_EDIT_VCS_HIGHEST)
  {
    m_CommandId = menu_id - ID_EDIT_VCS_LOWEST - 1;
  }
  
  if (m_CommandId < 0 || m_CommandId >= m_Commands.size())
  {
    m_CommandId = 0;
  }
}
  
#if wxUSE_GUI
void wxExVCSEntry::ShowOutput(const wxString& caption) const
{
  if (!GetError())
  {
    wxExVCSCommandOnSTC(
      GetCommand(), 
      m_FileName.GetLexer(), 
      GetDialog()->GetSTC());
  }

  wxExCommand::ShowOutput(caption);
}
#endif
