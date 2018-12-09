////////////////////////////////////////////////////////////////////////////////
// Name:      debug.cpp
// Purpose:   Implementation of class wex::debug
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/numdlg.h>
#include <wex/debug.h>
#include <wex/config.h>
#include <wex/defs.h>
#include <wex/itemdlg.h>
#include <wex/listview.h>
#include <wex/managedframe.h>
#include <wex/menu.h>
#include <wex/menus.h>
#include <wex/process.h>
#include <wex/shell.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <wex/util.h>

#ifdef __WXGTK__
namespace wex
{
  // This class adds name and pid of running processes to
  // a listview. Each process has an entry in /proc,
  // with a subdir of the pid as name. In this dir there is 
  // a file comm that contains the name of that process.
  class process_dir : public dir
  {
  public:
    process_dir(listview* lv, bool init)
      : dir("/proc", "[0-9]+", type_t().set(dir::DIRS))
      , m_ListView(lv) {
      if (init)
      {
        m_ListView->append_columns({{"Name", column::STRING, 200}, {"Pid"}});
      }
      else
      {
        m_ListView->clear();
      }};
   ~process_dir() {
      m_ListView->sort_column("Name", SORT_ASCENDING);};
  private:
    virtual bool on_dir(const path& p) override {
      std::ifstream ifs(wex::path(p.data(), "comm").data());
      if (std::string line; ifs.is_open() && std::getline(ifs, line))
      {
        m_ListView->insert_item({line, p.name()});
      }
      return true;};

    listview* m_ListView;
  };
};
#endif

wex::debug::debug(wex::managed_frame* frame, wex::process* debug)
  : m_Frame(frame)
  , m_Process(debug)
{
  if (std::vector< wex::menu_commands<wex::menu_command>> entries;
    wex::menus::load("debug", entries))
  {
    const size_t use = config("DEBUG").get((long)0);
    m_Entry = entries[use < entries.size() ? use: 0];
  }
}
  
int wex::debug::add_menu(wex::menu* menu, bool popup) const
{
  wex::menu* sub = (popup ? new wex::menu: nullptr);
  wex::menu* use = (popup ? sub: menu);
  
  const auto ret = wex::menus::build_menu(
    m_Entry.get_commands(), ID_EDIT_DEBUG_FIRST, use, popup);
  
  if (ret > 0 && popup)
  {
    menu->append_separator();
    menu->append_submenu(sub, "debug");
  }

  return ret;
}
  
bool wex::debug::DeleteAllBreakpoints(const std::string& text)
{
  if (std::vector<std::string> v;
    wex::match("(d|del|delete|Delete) (all )?breakpoints", text, v) >= 1)
  {
    for (const auto& it: m_Breakpoints)
    {
      if (m_Frame->is_open(std::get<0>(it.second)))
      {
        auto* stc = m_Frame->open_file(std::get<0>(it.second));
        stc->MarkerDeleteAll(m_MarkerBreakpoint.number());
      }
    }

    m_Breakpoints.clear();
    
    return true;
  }
  
  return false;
}
  
bool wex::debug::execute(const std::string& action, wex::stc* stc)
{
  std::string args;

  if (!GetArgs(action, args, stc) ||
     (m_Process == nullptr &&
     (m_Process = m_Frame->get_process(m_Entry.name())) == nullptr))
     return false;

  m_Frame->show_pane("PROCESS"); 

  if (!m_Process->is_running())
  {
    m_Process->execute(m_Entry.name());
  }

  return m_Process->write(action == "interrupt" ?
    std::string(1, 3) : action + args);
}

bool wex::debug::GetArgs(
  const std::string& command, std::string& args, stc* stc)
{
  if (std::vector<std::string> v;
    match("^(at|attach)", command, v) == 1)
  {
    static listview* lv = nullptr;
    bool init = false;

    if (m_Dialog == nullptr)
    {
      init = true;
      m_Dialog = new item_dialog({
#ifdef __WXGTK__
      {"processes", listview_data(), std::any(), item::LABEL_NONE,
        [&](wxWindow* user, const std::any& value, bool save) {
        lv = ((wex::listview *)user);
        if (save && lv->GetFirstSelected() != -1)
        {
          args = " " + lv->get_item_text(lv->GetFirstSelected(), "Pid");
        }
        }}},
#else
      {"pid", item::TEXTCTRL_INT, std::any(), control_data().is_required(true),
        item::LABEL_LEFT,
        [&](wxWindow* user, const std::any& value, bool save) {
           if (save) args += " " + std::to_string(std::any_cast<long>(value));}}},
#endif
      window_data().title("Attach").size({400, 400}).parent(m_Frame));
    }

    if (lv != nullptr)
    {
#ifdef __WXGTK__
      process_dir(lv, init).find_files();
#endif
    }

    return m_Dialog->ShowModal() != wxID_CANCEL;
  }
  else if ((match("^(br|break)", command, v) == 1) && stc != nullptr)
  {
    args += " " +
      stc->get_filename().data().string() + ":" + 
      std::to_string(stc->GetCurrentLine() + 1); 
  }
  else if ((match("^(d|del|delete) (br|breakpoint)", command, v) > 0) && stc != nullptr)
  {
    for (const auto& it: m_Breakpoints)
    {
      if (
        stc->get_filename() == std::get<0>(it.second) &&
        stc->GetCurrentLine() == std::get<2>(it.second))
      {
        args += " " + it.first;
        break;
      }
    }
  }
  else if (DeleteAllBreakpoints(command)) {}
  else if (command == "file")
  {
    return item_dialog(
      {{"File", item::COMBOBOX_FILE, std::any(), control_data().is_required(true),
          item::LABEL_LEFT,
          [&](wxWindow* user, const std::any& value, bool save) {
             if (save) args += " " + std::any_cast<wxArrayString>(value)[0];}},
       {m_Entry.name(), item::FILEPICKERCTRL}},
      window_data().title("Debug").parent(m_Frame)).ShowModal() != wxID_CANCEL;
  }
  else if ((match("^(p|print)", command, v) == 1) && stc != nullptr)
  {
    args += " " + stc->GetSelectedText(); 
  }
  else if ((match("(u|until)", command, v) == 1) && stc != nullptr)
  {
    args += " " + std::to_string(stc->GetCurrentLine());
  }
  
  return true;
}

void wex::debug::process_stdin(const std::string& text)
{
  if (std::vector<std::string> v;
    match("(d|del|delete) +([0-9 ]*)", text, v) > 0)
  {
    switch (v.size())
    {
      case 1:
        DeleteAllBreakpoints(text);
      break;

      case 2:
        for (tokenizer tkz(v[1], " "); tkz.has_more_tokens(); )
        {
          if (const auto& it = m_Breakpoints.find(tkz.get_next_token());
            it != m_Breakpoints.end() && m_Frame->is_open(std::get<0>(it->second)))
          {
            auto* stc = m_Frame->open_file(std::get<0>(it->second));
            stc->MarkerDeleteHandle(std::get<1>(it->second));
          }
        }
      break;
    }
  }
}

void wex::debug::process_stdout(const std::string& text)
{
  control_data data;

  if (std::vector<std::string> v;
    match("Breakpoint ([0-9]+) at 0x[0-9a-f]+: file (.*), line ([0-9]+)", text, v) == 3 || 
    match("Breakpoint ([0-9]+) at 0x[0-9a-f]+: (.*):([0-9]+)", text, v) == 3)
  {
    wex::path filename(m_Path.get_path(), v[1]);
  
    if (filename.file_exists())
    {
      if (auto* stc = m_Frame->open_file(filename); stc != nullptr)
      {
        const int line = std::stoi(v[2]) - 1;
        const auto id = stc->MarkerAdd(line, m_MarkerBreakpoint.number());
        m_Breakpoints[v[0]] = std::make_tuple(filename, id, line);
        return;
      }
    }
  }
  else if (std::vector<std::string> v;
    match("Reading symbols from (.*)\\.\\.\\.done", text, v) == 1)
  {
    m_Path = path(v[0]);
  }
  else if (DeleteAllBreakpoints(text)) {}
  else if (match("at (.*):([0-9]+)", text, v) > 1)
  {
    m_Path = path(m_Path.get_path(), v[0]);
    data.line(std::stoi(v[1]));
  }
  else if (match("(^|\\n)([0-9]+)", text, v) > 1)
  {
    data.line(std::stoi(v[1]));
  }

  if (data.line() > 0 && m_Path.file_exists())
  {
    m_Frame->open_file(m_Path, data);
    m_Process->get_shell()->SetFocus();
  }
}
