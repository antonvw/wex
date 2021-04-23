////////////////////////////////////////////////////////////////////////////////
// Name:      common/util.cpp
// Purpose:   Implementation of wex common utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <pugixml.hpp>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/blame.h>
#include <wex/config.h>
#include <wex/core.h>
#include <wex/dir.h>
#include <wex/factory/frame.h>
#include <wex/factory/link.h>
#include <wex/factory/process.h>
#include <wex/factory/stc.h>
#include <wex/file-dialog.h>
#include <wex/lexer.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wex/regex.h>
#include <wex/tostring.h>
#include <wex/util.h>
#include <wex/vcs-command.h>
#include <wx/app.h>
#include <wx/wupdlock.h>

namespace wex
{
  /// Allows you to easily open all files on specified path.
  /// After constructing, invoke find_files which
  /// causes all found files to be opened using open_file from frame.
  class open_file_dir : public dir
  {
  public:
    /// Constructor.
    open_file_dir(const path& path, const data::dir& data);

    static void set(factory::frame* fr, data::stc::window_t ft)
    {
      m_flags = ft;
      m_frame = fr;
    }

  private:
    /// Opens each found file.
    bool on_file(const path& file) override;

    static inline factory::frame*     m_frame = nullptr;
    static inline data::stc::window_t m_flags{0};
  };
}; // namespace wex

wex::open_file_dir::open_file_dir(const wex::path& path, const data::dir& data)
  : dir(path, data)
{
}

bool wex::open_file_dir::on_file(const wex::path& file)
{
  m_frame->open_file(file, data::stc().flags(m_flags));
  return true;
}

void wex::combobox_from_list(wxComboBox* cb, const std::list<std::string>& text)
{
  combobox_as<const std::list<std::string>>(cb, text);
}

bool wex::compare_file(const path& file1, const path& file2)
{
  if (config(_("list.Comparator")).empty())
  {
    log("compare_file") << "empty comparator";
    return false;
  }

  if (file1.empty() || file2.empty())
  {
    log("compare_file") << "empty arg";
    return false;
  }

  if (const auto arguments =
        (file1.stat().st_mtime < file2.stat().st_mtime) ?
          "\"" + file1.string() + "\" \"" + file2.string() + "\"" :
          "\"" + file2.string() + "\" \"" + file1.string() + "\"";
      factory::process().system(
        config(_("list.Comparator")).get() + " " + arguments) != 0)
  {
    return false;
  }
  else
  {
    log::status(_("Compared")) << arguments;
    return true;
  }
}

bool wex::make(const path& makefile)
{
  auto* process = new wex::factory::process;

  return process->async_system(
    config("Make").get("make") + " " + config("MakeSwitch").get("-f") + " " +
      makefile.string(),
    makefile.get_path());
}

int wex::open_files(
  factory::frame*          frame,
  const std::vector<path>& files,
  const data::stc&         stc,
  const data::dir::type_t& type)
{
  wxWindowUpdateLocker locker(frame);

  int count = 0;

  open_file_dir::set(frame, stc.flags());

  for (const auto& it : files)
  {
    if (
      it.string().find("*") != std::string::npos ||
      it.string().find("?") != std::string::npos)
    {
      count += open_file_dir(
                 path::current(),
                 data::dir().file_spec(it.string()).type(type))
                 .find_files();
    }
    else if (it.dir_exists())
    {
      log("open dir") << it;
    }
    else
    {
      try
      {
        path           fn(it);
        wex::data::stc data(stc);

        if (!it.file_exists() && it.string().find(":") != std::string::npos)
        {
          if (const path &
                val(wex::factory::link().get_path(it.string(), data.control()));
              !val.empty())
          {
            fn = val;
          }
        }

        fn.make_absolute();
        count++;

        frame->open_file(fn, data);

        if (!fn.file_exists())
        {
          log::debug("open file") << fn;
        }
      }
      catch (std::exception& e)
      {
        log(e) << it;
      }
    }
  }

  return count;
}

bool wex::shell_expansion(std::string& command)
{
  regex r("`(.*?)`"); // non-greedy

  while (r.search(command) > 0)
  {
    if (factory::process process; process.system(r[0]) != 0)
    {
      return false;
    }
    else
    {
      r.replace(command, process.get_stdout());
    }
  }

  return true;
}

bool wex::lexers_dialog(factory::stc* stc)
{
  wxArrayString s;

  for (const auto& it : lexers::get()->get_lexers())
  {
    s.Add(it.display_lexer());
  }

  if (auto lexer = stc->get_lexer().display_lexer();
      !single_choice_dialog(stc, _("Enter Lexer"), s, lexer))
  {
    return false;
  }
  else
  {
    lexer.empty() ? stc->get_lexer().clear() :
                    (void)stc->get_lexer().set(lexer, true);
    return true;
  }
}

void wex::vcs_command_stc(
  const vcs_command& command,
  const lexer&       lexer,
  factory::stc*      stc)
{
  if (command.is_blame())
  {
    // Do not show an edge and wrap for blamed documents, they are too wide.
    stc->SetEdgeMode(wxSTC_EDGE_NONE);
    stc->SetWrapMode(wxSTC_WRAP_NONE);
  }

  if (command.is_diff())
  {
    stc->get_lexer().set("diff");
  }
  else if (command.is_open())
  {
    stc->get_lexer().set(lexer, true); // fold
  }
  else
  {
    stc->get_lexer().clear();
  }
}

void wex::xml_error(
  const path&                   filename,
  const pugi::xml_parse_result* result,
  factory::stc*                 stc)
{
  log::status("Xml error") << result->description();
  log(*result) << filename.name();

  // prevent recursion
  if (stc == nullptr && filename != lexers::get()->get_filename())
  {
    if (auto* frame =
          dynamic_cast<wex::factory::frame*>(wxTheApp->GetTopWindow());
        frame != nullptr)
    {
      stc = frame->open_file(filename, data::stc());
    }
  }

  if (stc != nullptr && result->offset != 0)
  {
    stc->vi_command("gg");
    stc->vi_command(std::to_string(result->offset) + "|");
  }
}
