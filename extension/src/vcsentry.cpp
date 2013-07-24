////////////////////////////////////////////////////////////////////////////////
// Name:      vcsentry.cpp
// Purpose:   Implementation of wxExVCSEntry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/vcsentry.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>

wxExVCSEntry::wxExVCSEntry()
  : m_CommandIndex(0)
  , m_AdminDir()
  , m_Name()
  , m_FlagsLocation(VCS_FLAGS_LOCATION_POSTFIX)
  , m_AdminDirIsTopLevel(false)
{
  m_Commands.push_back(wxExVCSCommand());
}

wxExVCSEntry::wxExVCSEntry(const wxXmlNode* node)
  : m_CommandIndex(0)
  , m_AdminDir(node->GetAttribute("admin-dir"))
  , m_Name(node->GetAttribute("name"))
  , m_FlagsLocation(
      (node->GetAttribute("flags-location") == "prefix" ?
         VCS_FLAGS_LOCATION_PREFIX: VCS_FLAGS_LOCATION_POSTFIX))
  , m_AdminDirIsTopLevel(
      node->GetAttribute("toplevel") == "true")
{
  if (m_Name.empty())
  {
    wxLogError("Missing vcs on line: %d", node->GetLineNumber());
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
  
  if (m_Commands.empty())
  {
    wxLogError("No commands found for: " + m_Name);
    m_Commands.push_back(wxExVCSCommand());
  }  
}

void wxExVCSEntry::AddCommands(const wxXmlNode* node)
{
  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "command")
    {
      if (m_Commands.size() == VCS_MAX_COMMANDS)
      {
        wxLogError("Reached commands limit: %d", VCS_MAX_COMMANDS);
      }
      else
      {
        const wxString content = child->GetNodeContent().Strip(wxString::both);
        const wxString attrib = child->GetAttribute("type");
        const wxString submenu = child->GetAttribute("submenu");
        const wxString subcommand = child->GetAttribute("subcommand");
        
        m_Commands.push_back(
          wxExVCSCommand(
            content, 
            attrib, 
            submenu, 
            subcommand));
      }
    }
    
    child = child->GetNext();
  }
}

#if wxUSE_GUI
int wxExVCSEntry::BuildMenu(int base_id, wxMenu* menu, bool is_popup) const
{
  wxMenu* submenu = NULL;

  const wxString unused = "XXXXX";  
  wxString prev_menu = unused;
  
  int i;

  for (
    auto it = m_Commands.begin(), i = 0;
    it != m_Commands.end();
    ++it, i++)
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
        base_id + i, 
        wxExEllipsed(it->GetCommand(false, true))); // use no sub and do accel
        
      const long sep = it->GetType() & 0x00F0;
      
      if (sep)
      {
        usemenu->AppendSeparator();
      }
    }
  }

  return menu->GetMenuItemCount();
}

#endif

bool wxExVCSEntry::Execute(
  const wxString& args,
  const wxExLexer& lexer,
  const wxString& wd)
{
  if (GetBin().empty())
  {
#if defined(__WXMSW__) || defined(__OS2__)
    const wxString wc = "*.exe";
#else // Unix/Mac
    const wxString wc(wxFileSelectorDefaultWildcardStr);
#endif

    wxFileDialog dlg(NULL, 
      _("Select") + " " + m_Name + " bin", 
      "", 
      "", 
      wc, 
      wxFD_OPEN|wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_CANCEL)
    {
      return false;
    }
        
    wxConfigBase::Get()->Write(m_Name, dlg.GetPath());
  }
  
  m_Lexer = lexer;
  
  wxString prefix;
  
  if (m_FlagsLocation == VCS_FLAGS_LOCATION_PREFIX)
  {
    prefix = wxConfigBase::Get()->Read(_("Prefix flags"));
    
    if (!prefix.empty())
    {
      prefix += " ";
    }
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

    // E.g. in git you can do
    // git show HEAD~15:syncped/frame.cpp
    // where flags is HEAD~15:,
    // so there should be no space after it
    if (!flags.empty() && !flags.EndsWith(':'))
    {
      flags += " ";
    }
  }
  
  wxString comment;

  if (GetCommand().IsCommit())
  {
    comment = 
      "-m \"" + wxExConfigFirstOf(_("Revision comment")) + "\" ";
  }

  // If we specified help (flags), we do not need a file argument.      
  if (GetCommand().IsHelp() || flags.Contains("help"))
  {
    my_args.clear();
  }

  return wxExProcess::Execute(
    GetBin() + " " + 
      prefix +
      GetCommand().GetCommand() + " " + 
      subcommand + flags + comment + my_args, 
    wxEXEC_SYNC, // for the moment (problems with wxTimer when ASYC is used)
    wd);
}

const wxString wxExVCSEntry::GetBin() const
{
  return wxConfigBase::Get()->Read(m_Name, "");
}

const wxString wxExVCSEntry::GetFlags() const
{
  return wxConfigBase::Get()->Read(_("Flags"));
}

bool wxExVCSEntry::SetCommand(int command_no)
{
  if (command_no < 0 || command_no >= m_Commands.size())
  {
    return false;
  }
  
  m_CommandIndex = command_no;
  
  m_FlagsKey = wxString::Format(
    "vcsflags/%s%d", 
    m_Name.c_str(), 
    m_CommandIndex);
    
  return true;
}

#if wxUSE_GUI
int wxExVCSEntry::ShowDialog(
  wxWindow* parent, 
  const wxString& caption,
  bool add_folder) const
{
  if (GetCommand().GetCommand().empty())
  {
    return wxID_CANCEL;
  }
  
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
      1005));

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

    v.push_back(wxExConfigItem(_("Flags"), wxEmptyString));
  }

  if (m_FlagsLocation == VCS_FLAGS_LOCATION_PREFIX)
  {
    v.push_back(wxExConfigItem(_("Prefix flags"), wxEmptyString));
  }
  
  if (GetCommand().UseSubcommand())
  {
    v.push_back(wxExConfigItem(_("Subcommand"), wxEmptyString));
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
  if (!GetError() && GetSTC() != NULL)
  {
    if (GetFlags().Contains("xml"))
    {
      GetSTC()->SetLexer("xml");
    }
    else
    {
      wxExVCSCommandOnSTC(
        GetCommand(), 
        m_Lexer, 
        GetSTC());
    }
  }

  wxExProcess::ShowOutput(caption);
}
#endif
