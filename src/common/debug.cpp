////////////////////////////////////////////////////////////////////////////////
// Name:      debug.cpp
// Purpose:   Implementation of class wex::debug
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <boost/tokenizer.hpp>
#include <wex/bind.h>
#include <wex/config.h>
#include <wex/debug.h>
#include <wex/defs.h>
#include <wex/dir.h>
#include <wex/item-dialog.h>
#include <wex/listview.h>
#include <wex/managed-frame.h>
#include <wex/menu.h>
#include <wex/menus.h>
#include <wex/process.h>
#include <wex/regex.h>
#include <wex/shell.h>
#include <wex/stc.h>

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
      : dir(
          "/proc",
          data::dir().file_spec("[0-9]+").type(
            data::dir::type_t().set(data::dir::DIRS)))
      , m_listview(lv)
    {
      if (init)
      {
        m_listview->append_columns({{"Name", column::STRING, 200}, {"Pid"}});
      }
      else
      {
        m_listview->clear();
      }
    };
    ~process_dir() { m_listview->sort_column("Name", SORT_ASCENDING); };

  private:
    bool on_dir(const path& p) override
    {
      if (!std::filesystem::is_symlink(p.data()))
      {
        std::ifstream ifs(wex::path(p.data(), "comm").data());
        if (std::string line; ifs.is_open() && std::getline(ifs, line))
        {
          m_listview->insert_item({line, p.name()});
        }
      }
      return true;
    };

    listview* m_listview;
  };
}; // namespace wex
#endif

#define MATCH(REGEX)                                          \
  regex v(m_entry.regex_stdout(debug_entry::regex_t::REGEX)); \
  v.match(m_stdout)

wex::debug::debug(wex::managed_frame* frame, wex::process* debug)
  : m_frame(frame)
  , m_process(debug)
{
  set_entry(config("debug.debugger").get());

  bind(this).command(
    {{[=, this](wxCommandEvent& event) {
        is_finished();
      },
      ID_DEBUG_EXIT},
     {[=, this](wxCommandEvent& event) {
        const std::string text(event.GetString());
        // parse delete a breakpoint with text, numbers
        if (regex v("(d|del|delete) +([0-9 ]*)"); v.match(text) > 0)
        {
          switch (v.size())
          {
            case 1:
              clear_breakpoints(text);
              break;

            case 2:
              for (const auto& token :
                   boost::tokenizer<boost::char_separator<char>>(
                     v[1],
                     boost::char_separator<char>(" ")))
              {
                if (const auto& it = m_breakpoints.find(token);
                    it != m_breakpoints.end() &&
                    m_frame->is_open(std::get<0>(it->second)))
                {
                  if (auto* stc = m_frame->open_file(std::get<0>(it->second));
                      stc != nullptr)
                  {
                    stc->MarkerDeleteHandle(std::get<1>(it->second));
                  }
                }
              }
              break;
          }
        }
      },
      ID_DEBUG_STDIN},
     {[=, this](wxCommandEvent& event) {
        m_stdout += event.GetString();
        data::stc data;

        if (MATCH(NO_FILE_LINE) == 3)
        {
          m_stdout.clear();

          if (const wex::path filename(
                path(v[1]).is_absolute() ? path(v[1]) :
                                           path(m_path.get_path(), v[1]));
              allow_open(filename))
          {
            if (auto* stc = m_frame->open_file(filename); stc != nullptr)
            {
              const auto line = std::stoi(v[2]) - 1;
              const auto id =
                stc->MarkerAdd(line, m_marker_breakpoint.number());
              m_breakpoints[v[0]] = std::make_tuple(filename, id, line);
              return;
            }
          }

          log("debug break") << v[1];
        }
        else if (MATCH(PATH) == 1)
        {
          if (path(v[0]).is_absolute())
          {
            log::trace("debug path") << v[0];

            if (m_path.string() == v[0] && m_process != nullptr)
            {
              // Debug same exe as before, so
              // reapply all breakpoints.
              for (auto& it : m_breakpoints)
              {
                m_process->write(
                  m_entry.break_set() + " " + std::get<0>(it.second).string() +
                  ":" + std::to_string(std::get<2>(it.second) + 1));
              }
            }

            m_path = path(v[0]);
          }
          m_stdout.clear();
        }
        else if (regex v(
                   {m_entry.regex_stdout(debug_entry::regex_t::AT_LINE).c_str(),
                    m_entry.regex_stdout(debug_entry::regex_t::AT_PATH_LINE)});
                 v.match(m_stdout) > 0)
        {
          if (v.size() == 2)
          {
            m_path                 = path(m_path.get_path(), v[0]);
            m_path_execution_point = m_path;
            log::trace("debug path and exec") << m_path.string();
          }
          data.indicator_no(data::stc::IND_DEBUG);
          data.control().line(std::stoi(v.matches().back()));
          m_stdout.clear();
        }
        else if (regex v(
                   {m_entry.regex_stdout(debug_entry::regex_t::VARIABLE_MULTI)
                      .c_str(),
                    m_entry.regex_stdout(debug_entry::regex_t::VARIABLE)});
                 v.match(m_stdout) > 0)
        {
          m_stdout.clear();

          if (allow_open(m_path))
          {
            if (auto* stc = m_frame->open_file(m_path); stc != nullptr)
            {
              wxCommandEvent event(
                wxEVT_COMMAND_MENU_SELECTED,
                ID_EDIT_DEBUG_VARIABLE);
              event.SetString(v[0]);
              wxPostEvent(stc, event);
              return;
            }
          }
        }
        else if (MATCH(EXIT) >= 0)
        {
          is_finished();
          m_stdout.clear();
        }
        else if (clear_breakpoints(m_stdout))
        {
          m_stdout.clear();
        }
        else if (regex v("'(.*)'"); v.match(m_stdout) == 1)
        {
          if (wex::path filename(v[0]); allow_open(filename))
          {
            m_path = v[0];
            log::trace("debug path") << v[0];
          }
          m_stdout.clear();
        }
        else if (m_stdout.find("{") == std::string::npos)
        {
          m_stdout.clear();
        }

        if (data.control().line() > 0 && allow_open(m_path))
        {
          m_frame->open_file(m_path, data);
        }
      },
      ID_DEBUG_STDOUT}});
}

int wex::debug::add_menu(wex::menu* menu, bool popup) const
{
  if (popup && m_process == nullptr)
  {
    return 0;
  }

  wex::menu* sub = (popup ? new wex::menu : nullptr);
  wex::menu* use = (popup ? sub : menu);

  if (popup)
  {
    use->style().set(menu::IS_POPUP);
  }

  const auto ret =
    wex::menus::build_menu(m_entry.get_commands(), ID_EDIT_DEBUG_FIRST, use);

  if (ret > 0 && popup)
  {
    menu->append({{}, {sub, "debug"}});
  }

  return ret;
}

bool wex::debug::allow_open(const path& p) const
{
  return p.file_exists() &&
         matches_one_of(p.extension(), debug_entry().extensions());
}

bool wex::debug::apply_breakpoints(stc* stc) const
{
  bool found = false;

  for (const auto& it : m_breakpoints)
  {
    if (std::get<0>(it.second) == stc->get_filename())
    {
      stc->MarkerAdd(std::get<2>(it.second), m_marker_breakpoint.number());
      found = true;
    }
  }

  return found;
}

bool wex::debug::clear_breakpoints(const std::string& text)
{
  if (regex v("(d|del|delete|Delete) (all )?breakpoints"); v.match(text) >= 1)
  {
    for (const auto& it : m_breakpoints)
    {
      if (m_frame->is_open(std::get<0>(it.second)))
      {
        auto* stc = m_frame->open_file(std::get<0>(it.second));
        stc->MarkerDeleteAll(m_marker_breakpoint.number());
      }
    }

    m_breakpoints.clear();

    return true;
  }

  return false;
}

bool wex::debug::execute(const std::string& action, wex::stc* stc)
{
  const auto& exe(
    m_entry.name() + (!m_entry.flags().empty() ?
                        std::string(1, ' ') + m_entry.flags() :
                        std::string()));

  if (const auto& [r, args] = get_args(action, stc); !r)
  {
    log::debug("debug") << m_entry.name() << "failed args";
    return false;
  }
  else
  {
    if (
      m_process == nullptr &&
      ((m_process = m_frame->get_process(exe)) == nullptr))
    {
      log::debug("debug") << m_entry.name() << "no process";
      return false;
    }

    if (!m_process->is_running() && !m_process->execute(exe))
    {
      log::debug("debug") << m_entry.name() << "process no execute" << exe;
      return false;
    }

    m_active = true;

    return m_process->write(
      action == "interrupt" ? std::string(1, 3) : action + args);
  }
}

bool wex::debug::execute(int item, stc* stc)
{
  return item >= 0 && item < (int)m_entry.get_commands().size() &&
         execute(m_entry.get_commands().at(item).get_command(), stc);
};

std::tuple<bool, std::string>
wex::debug::get_args(const std::string& command, stc* stc)
{
  std::string args;

  if (regex v("^(at|attach)"); v.match(command) == 1)
  {
    static listview* lv   = nullptr;
    bool             init = false;

    if (m_dialog == nullptr)
    {
      init     = true;
      m_dialog = new item_dialog(
        {
#ifdef __WXGTK__
          {"debug.processes",
           data::listview(),
           std::any(),
           data::item()
             .label_type(data::item::LABEL_NONE)
             .apply([&](wxWindow* user, const std::any& value, bool save) {
               lv = ((wex::listview*)user);
               if (save && lv->GetFirstSelected() != -1)
               {
                 args = " " + lv->get_item_text(lv->GetFirstSelected(), "Pid");
               }
             })}},
#else
          {"debug.pid",
           item::TEXTCTRL_INT,
           std::any(),
           data::item(data::control().is_required(true))
             .label_type(data::item::LABEL_LEFT)
             .apply([&](wxWindow* user, const std::any& value, bool save) {
               if (save)
                 args += " " + std::to_string(std::any_cast<long>(value));
             })}},
#endif
        data::window().title("Attach").size({400, 400}).parent(m_frame));
    }

#ifdef __WXGTK__
    if (lv != nullptr)
    {
      process_dir(lv, init).find_files();
    }
#endif

    return {m_dialog->ShowModal() != wxID_CANCEL, args};
  }
  else if (regex r("^(b|break)"); r.match(command) == 1 && stc != nullptr)
  {
    args += " " + stc->get_filename().string() + ":" +
            std::to_string(stc->get_current_line() + 1);
  }
  else if (regex r("^(d|del|delete) (br|breakpoint)");
           r.match(command) > 0 && stc != nullptr)
  {
    for (auto& it : m_breakpoints)
    {
      if (
        stc->get_filename() == std::get<0>(it.second) &&
        stc->get_current_line() == std::get<2>(it.second))
      {
        args += " " + it.first;
        stc->MarkerDeleteHandle(std::get<1>(it.second));
        m_breakpoints.erase(it.first);
        break;
      }
    }
  }
  else if (clear_breakpoints(command))
  {
  }
  else if (command == "file")
  {
    return {
      item_dialog(
        {{"debug.File",
          item::COMBOBOX_FILE,
          std::any(),
          data::item(data::control().is_required(true))
            .apply([&](wxWindow* user, const std::any& value, bool save) {
              if (save)
                args += " " + std::any_cast<wxArrayString>(value)[0];
            })},
         {"debug." + m_entry.name(), item::FILEPICKERCTRL}},
        data::window().title("Debug").parent(m_frame))
          .ShowModal() != wxID_CANCEL,
      args};
  }
  else if (regex r("^(p|print)"); r.match(command) == 1 && stc != nullptr)
  {
    args += " " + stc->GetSelectedText();
  }
  else if (regex r("^(u|until|thread until)");
           r.match(command) == 1 && stc != nullptr)
  {
    args += " " + std::to_string(stc->get_current_line());
  }

  return {true, args};
}

bool wex::debug::is_active() const
{
  return m_active && m_process != nullptr && m_process->is_running();
}

void wex::debug::is_finished()
{
  m_active = false;

  log::trace("debug") << m_entry.name() << "finished";

  if (!m_frame->is_closing() && allow_open(m_path_execution_point))
  {
    if (auto* stc = m_frame->open_file(m_path_execution_point); stc != nullptr)
    {
      stc->SetIndicatorCurrent(data::stc::IND_DEBUG);
      stc->IndicatorClearRange(0, stc->GetTextLength() - 1);
      stc->AnnotationClearAll();
    }
  }
}

bool wex::debug::print(const std::string& variable) const
{
  return is_active() && m_process->write("print " + variable);
}

void wex::debug::set_entry(const std::string& debugger)
{
  if (std::vector<wex::debug_entry> v; menus::load("debug", v))
  {
    if (debugger.empty())
    {
      m_entry = v[0];
    }
    else
    {
      bool found = false;

      for (const auto& it : v)
      {
        if (it.name() == debugger)
        {
          m_entry = it;
          found   = true;
          break;
        }
      }

      if (!found)
      {
        log("unknown debugger") << debugger;
      }
    }

    log::trace("debug entries") << v.size() << "debugger:" << m_entry.name();
  }
}

bool wex::debug::show_dialog(frame* parent)
{
  wxArrayString                 s;
  std::vector<wex::debug_entry> v;
  menus::load("debug", v);

  for (const auto& it : v)
  {
    s.Add(it.name());
  }

  if (auto debugger = m_entry.name();
      !single_choice_dialog(parent, _("Enter Debugger"), s, debugger))
  {
    return false;
  }
  else
  {
    config("debug.debugger").set(debugger);
    set_entry(debugger);
    return true;
  }
}

bool wex::debug::toggle_breakpoint(int line, stc* stc)
{
  if (m_process == nullptr)
  {
    return false;
  }

  for (auto& it : m_breakpoints)
  {
    if (
      line == std::get<2>(it.second) &&
      std::get<0>(it.second) == stc->get_filename().string())
    {
      stc->MarkerDeleteHandle(std::get<1>(it.second));
      m_breakpoints.erase(it.first);
      return m_process->write(m_entry.break_del() + " " + it.first);
    }
  }

  m_path = stc->get_filename();

  return m_process->write(
    m_entry.break_set() + " " + stc->get_filename().string() + ":" +
    std::to_string(line + 1));
}
