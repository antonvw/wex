////////////////////////////////////////////////////////////////////////////////
// Name:      vcsentry.cpp
// Purpose:   Implementation of wxExVCSEntry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/vcsentry.h>
#include <wx/extension/defs.h> // for VCS_MAX_COMMANDS
#include <wx/extension/itemdlg.h>
#include <wx/extension/shell.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>

wxExVCSEntry::wxExVCSEntry(
  const wxString& name,
  const wxString& admin_dir,
  std::vector<wxExVCSCommand> commands,
  int flags_location)
  : wxExProcess()
  , m_CommandIndex(0)
  , m_AdminDir(admin_dir)
  , m_Name(name)
  , m_FlagsLocation(flags_location)
  , m_AdminDirIsTopLevel(false)
  , m_Commands(commands)
{
}

wxExVCSEntry::wxExVCSEntry(const wxXmlNode* node)
  : wxExProcess()
  , m_CommandIndex(0)
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
  wxMenu* submenu = nullptr;

  const wxString unused = "XXXXX";  
  wxString prev_menu = unused;
  
  int i = 0;

  for (const auto& it : m_Commands)
  {
    bool add = false;

    if (!it.GetSubMenu().empty() && prev_menu != it.GetSubMenu())
    {
      submenu = new wxMenu();
      prev_menu = it.GetSubMenu();
      menu->AppendSeparator();
      menu->AppendSubMenu(submenu, it.GetSubMenu());
    }
    else if (it.GetSubMenu().empty())
    {
      if (prev_menu != unused)
      {
        prev_menu = unused;
        menu->AppendSeparator();
      }

      submenu = nullptr;
    }
    
    const long type = it.GetType() & 0x000F;

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
      wxMenu* usemenu = (submenu == nullptr ? menu: submenu);
      usemenu->Append(
        base_id + i, 
        wxExEllipsed(it.GetCommand(false, true))); // use no sub and do accel
        
      const long sep = it.GetType() & 0x00F0;
      
      if (sep)
      {
        usemenu->AppendSeparator();
      }
    }
      
    i++;
  }

  return menu->GetMenuItemCount();
}

#endif

bool wxExVCSEntry::Execute(
  const wxString& args,
  const wxExLexer& lexer,
  int exec_flags,
  const wxString& wd)
{
  if (GetBin().empty())
  {
#if defined(__WXMSW__) || defined(__OS2__)
    const wxString wc = "*.exe";
#else // Unix/Mac
    const wxString wc(wxFileSelectorDefaultWildcardStr);
#endif

    wxFileDialog dlg(nullptr, 
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
    exec_flags,
    wd);
}

const wxString wxExVCSEntry::GetBin() const
{
  return wxConfigBase::Get()->Read(m_Name, 
#ifdef __UNIX__
    "/usr/bin/"
#else
    ""
#endif
    + m_Name);
}

const wxString wxExVCSEntry::GetFlags() const
{
  return wxConfigBase::Get()->Read(_("Flags"));
}

bool wxExVCSEntry::SetCommand(int command_no)
{
  if (command_no < 0 || command_no >= (int)m_Commands.size())
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
  
  if (GetCommand().UseFlags())
  {
    wxConfigBase::Get()->Write(
      _("Flags"), 
      wxConfigBase::Get()->Read(m_FlagsKey));
  }
  
  const int retValue = wxExItemDialog(parent,
    std::vector<wxExItem> {
      (GetCommand().IsCommit() ? wxExItem(
        _("Revision comment"), 
        ITEM_COMBOBOX,
        wxAny(),
        true) : wxExItem()),
      (add_folder && !GetCommand().IsHelp() ? wxExItem(
        _("Base folder"), 
        ITEM_COMBOBOX_DIR, 
        wxAny(), 
        true,
        wxWindow::NewControlId()) : wxExItem()),
      (add_folder && !GetCommand().IsHelp() && GetCommand().IsAdd() ? wxExItem(
        _("Path"), 
        ITEM_COMBOBOX,
        wxAny(), 
        true) : wxExItem()),
      (GetCommand().UseFlags() ?  wxExItem(
        _("Flags"), wxEmptyString): wxExItem()),
      (m_FlagsLocation == VCS_FLAGS_LOCATION_PREFIX ? wxExItem(
        _("Prefix flags"), wxEmptyString): wxExItem()),
      (GetCommand().UseSubcommand() ? wxExItem(
        _("Subcommand"), wxEmptyString): wxExItem())},
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
  if (!GetError() && GetShell() != nullptr)
  {
    if (GetFlags().Contains("xml"))
    {
      GetShell()->SetLexer("xml");
    }
    else
    {
      wxExVCSCommandOnSTC(
        GetCommand(), 
        m_Lexer, 
        GetShell());
    }
  }

  wxExProcess::ShowOutput(caption);
}
#endif
