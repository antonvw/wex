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
#include <wx/extension/configdlg.h>
#include <wx/extension/defs.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>

wxExVCSEntry::wxExVCSEntry()
  : m_No(-1)
  , m_CommandId(0)
  , m_Name()
  , m_FlagsLocation(VCS_FLAGS_LOCATION_POSTFIX)
  , m_SupportKeywordExpansion(false)
{
  m_Commands.push_back(wxExVCSCommand());
}

wxExVCSEntry::wxExVCSEntry(const wxXmlNode* node, int no)
  : m_No(no)
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
    
    while (child)
    {
      if (child->GetName() == "commands")
      {
        AddCommands(child);
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
  if (GetBin().empty()) 
  {
    return 0;
  }
  
  wxMenu* submenu = NULL;

  const wxString unused = "XXXXX";  
  wxString prev_menu = unused;

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
      if (prev_menu != unused)
      {
        prev_menu = unused;
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
      case wxExVCSCommand::VCS_COMMAND_IS_NONE: add = false; break;
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
  m_FileName = filename;
  
  wxString prefix;
  
  if (m_FlagsLocation == VCS_FLAGS_LOCATION_PREFIX)
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
    flags = GetFlags();

    if (!flags.empty())
    {
      flags += " ";
    }
  }
  
  // If we specified help (flags), we do not need a file argument.      
  if (GetCommand().IsHelp() || flags.Contains("help"))
  {
    my_args.clear();
  }

  return wxExCommand::Execute(
    GetBin() + " " + 
    prefix + 
    GetCommand().GetCommand() + " " + 
    subcommand + flags + comment + my_args, wd);
}

const wxString wxExVCSEntry::GetBin() const
{
  return wxConfigBase::Get()->Read(m_Name, "svn");
}

const wxString wxExVCSEntry::GetFlags() const
{
  return wxConfigBase::Get()->Read(_("Flags"));
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
  
  m_FlagsKey = wxString::Format(
    "vcsflags/%s%d", 
    m_Name.c_str(), 
    GetCommand().GetNo());
}

#if wxUSE_GUI
int wxExVCSEntry::ShowDialog(
  wxWindow* parent, 
  const wxString& caption,
  bool add_folder) const
{
  std::vector<wxExConfigItem> v;

  if (GetCommand().IsCommit())
  {
    v.push_back(wxExConfigItem(
      _("Revision comment"), 
      CONFIG_COMBOBOX,
      wxEmptyString,
      true)); // required
  }

  if (add_folder && !GetCommand().IsHelp())
  {
    v.push_back(wxExConfigItem(
      _("Base folder"), 
      CONFIG_COMBOBOXDIR, 
      wxEmptyString, 
      true,
      1000));

    if (GetCommand().IsAdd())
    {
      v.push_back(wxExConfigItem(
        _("Path"), 
        CONFIG_COMBOBOX,
        wxEmptyString, 
        true)); // required
    }
  }

  if (GetCommand().UseFlags())
  {
    wxConfigBase::Get()->Write(
      _("Flags"), 
      wxConfigBase::Get()->Read(m_FlagsKey));

    v.push_back(wxExConfigItem(_("Flags")));
  }

  if (m_FlagsLocation == VCS_FLAGS_LOCATION_PREFIX)
  {
    v.push_back(wxExConfigItem(_("Prefix flags")));
  }
  
  if (GetCommand().UseSubcommand())
  {
    v.push_back(wxExConfigItem(_("Subcommand")));
  }
  
  const int retValue = wxExConfigDialog(parent,
    v,
    caption).ShowModal();
    
  if (retValue == wxID_OK)
  {
    if (GetCommand().UseFlags())
    {
      wxConfigBase::Get()->Write(m_FlagsKey, GetFlags());
    }
  }
  
  return retValue;
}
#endif
  
#if wxUSE_GUI
void wxExVCSEntry::ShowOutput(const wxString& caption) const
{
  if (!GetError() && GetDialog() != NULL)
  {
    if (GetFlags().Contains("xml"))
    {
      GetDialog()->SetLexer("xml");
    }
    else
    {
      wxExVCSCommandOnSTC(
        GetCommand(), 
        m_FileName.GetLexer(), 
        GetDialog()->GetSTC());
    }
  }

  wxExCommand::ShowOutput(caption);
}
#endif
