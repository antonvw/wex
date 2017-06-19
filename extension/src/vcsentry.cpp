////////////////////////////////////////////////////////////////////////////////
// Name:      vcsentry.cpp
// Purpose:   Implementation of wxExVCSEntry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/vcsentry.h>
#include <wx/extension/defs.h> // for VCS_MAX_COMMANDS
#include <wx/extension/itemdlg.h>
#include <wx/extension/menu.h>
#include <wx/extension/menus.h>
#include <wx/extension/shell.h>
#include <wx/extension/vcs.h>

wxExItemDialog* wxExVCSEntry::m_ItemDialog = nullptr;

wxExVCSEntry::wxExVCSEntry(
  const std::string& name,
  const std::string& admin_dir,
  const std::vector<wxExVCSCommand> & commands,
  int flags_location)
  : wxExProcess()
  , wxExMenuCommands(name, commands)
  , m_AdminDir(admin_dir)
  , m_FlagsLocation(flags_location)
{
}

wxExVCSEntry::wxExVCSEntry(const pugi::xml_node& node)
  : wxExProcess()
  , wxExMenuCommands(node)
  , m_AdminDir(node.attribute("admin-dir").value())
  , m_FlagsLocation(
      (strcmp(node.attribute("flags-location").value(), "prefix") == 0 ?
         VCS_FLAGS_LOCATION_PREFIX: VCS_FLAGS_LOCATION_POSTFIX))
  , m_AdminDirIsTopLevel(
      strcmp(node.attribute("toplevel").value(), "true") == 0)
{
}

#if wxUSE_GUI
int wxExVCSEntry::BuildMenu(int base_id, wxExMenu* menu, bool is_popup) const
{
  return wxExMenus::BuildMenu(GetCommands(), base_id, menu, is_popup);
}
#endif

bool wxExVCSEntry::Execute(
  const std::string& args,
  const wxExLexer& lexer,
  bool wait,
  const std::string& wd)
{
  m_Lexer = lexer;
  
  std::string prefix;
  
  if (m_FlagsLocation == VCS_FLAGS_LOCATION_PREFIX)
  {
    prefix = wxConfigBase::Get()->Read(_("Prefix flags"));
    
    if (!prefix.empty())
    {
      prefix += " ";
    }
  }
  
  std::string subcommand;
  
  if (GetCommand().UseSubcommand())
  {
    subcommand = wxConfigBase::Get()->Read(_("Subcommand"));

    if (!subcommand.empty())
    {
      subcommand += " ";
    }
  }

  std::string flags;
  std::string my_args(args);

  if (GetCommand().UseFlags())
  {
    flags = GetFlags();

    // E.g. in git you can do
    // git show HEAD~15:syncped/frame.cpp
    // where flags is HEAD~15:,
    // so there should be no space after it
    if (!flags.empty() && flags.back() != ':')
    {
      flags += " ";
    }
  }
  
  std::string comment;

  if (GetCommand().IsCommit())
  {
    comment = 
      "-m \"" + wxExConfigFirstOf(_("Revision comment")) + "\" ";
  }

  // If we specified help (flags), we do not need a file argument.      
  if (GetCommand().IsHelp() || flags.find("help") != std::string::npos)
  {
    my_args.clear();
  }

  return wxExProcess::Execute(
    wxConfigBase::Get()->Read(GetName(), GetName()).ToStdString() + " " + 
      prefix +
      GetCommand().GetCommand() + " " + 
      subcommand + flags + comment + my_args, 
    wait,
    wd);
}

const std::string wxExVCSEntry::GetFlags() const
{
  return wxConfigBase::Get()->Read(_("Flags")).ToStdString();
}

#if wxUSE_GUI
int wxExVCSEntry::ShowDialog(
  const wxExWindowData& data,
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
      wxConfigBase::Get()->Read(GetFlagsKey()));
  }
  
  if (m_ItemDialog == nullptr)
  {
    m_ItemDialog = new wxExItemDialog({
      GetCommand().IsCommit() ? 
        wxExItem(_("Revision comment"), ITEM_COMBOBOX, std::any(), wxExControlData().Required(true)) : wxExItem(),
      add_folder && !GetCommand().IsHelp() ? 
        wxExItem(_("Base folder"), ITEM_COMBOBOX_DIR, std::any(), wxExControlData().Required(true)) : wxExItem(),
      add_folder && !GetCommand().IsHelp() && GetCommand().IsAdd() ? wxExItem(
        _("Path"), ITEM_COMBOBOX, std::any(), wxExControlData().Required(true)) : wxExItem(), 
      GetCommand().UseFlags() ?  
        wxExItem(_("Flags"), wxEmptyString, ITEM_TEXTCTRL, wxExControlData(), LABEL_LEFT, 
          [=](wxWindow* user, const std::any& value, bool save) {
            wxConfigBase::Get()->Write(GetFlagsKey(), wxString(GetFlags()));}): wxExItem(),
      m_FlagsLocation == VCS_FLAGS_LOCATION_PREFIX ? 
        wxExItem(_("Prefix flags"), wxEmptyString): wxExItem(),
      GetCommand().UseSubcommand() ? 
        wxExItem(_("Subcommand"), wxEmptyString): wxExItem()}, data);
  }
  else
  {
    m_ItemDialog->SetTitle(data.Title());
  }

  return (data.Button() & wxAPPLY) ? 
    m_ItemDialog->Show(): m_ItemDialog->ShowModal();
}
#endif
  
#if wxUSE_GUI
void wxExVCSEntry::ShowOutput(const std::string& caption) const
{
  if (!GetError() && GetShell() != nullptr)
  {
    if (GetFlags().find("xml") != std::string::npos)
    {
      GetShell()->GetLexer().Set("xml");
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
