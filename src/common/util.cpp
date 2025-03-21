////////////////////////////////////////////////////////////////////////////////
// Name:      common/util.cpp
// Purpose:   Implementation of wex common utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <numeric>

#include <wex/common/dir.h>
#include <wex/common/tostring.h>
#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/path.h>
#include <wex/core/regex.h>
#include <wex/core/vcs-command.h>
#include <wex/factory/frame.h>
#include <wex/factory/link.h>
#include <wex/factory/process.h>
#include <wex/syntax/lexer.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/stc.h>
#include <wex/syntax/util.h>
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
  open_file_dir(const wex::path& path, const data::dir& data);

  static void set(factory::frame* fr, data::stc::window_t ft)
  {
    m_flags = ft;
    m_frame = fr;
  }

private:
  /// Opens each found file.
  bool on_file(const path& file) const override;

  static inline factory::frame*     m_frame = nullptr;
  static inline data::stc::window_t m_flags{0};
};
}; // namespace wex

wex::open_file_dir::open_file_dir(const wex::path& path, const data::dir& data)
  : dir(path, data)
{
}

bool wex::open_file_dir::on_file(const wex::path& file) const
{
  m_frame->open_file(file, data::stc().flags(m_flags));
  return true;
}

std::optional<wex::auto_complete_filename_t>
wex::auto_complete_filename(const std::string& text)
{
  // E.g.:
  // 1) text: src/vi
  // -> should build vector with files in ./src starting with vi
  // path:   src
  // prefix: vi
  // 2) text: /usr/include/s
  // ->should build vector with files in /usr/include starting with s
  // path:   /usr/include
  // prefix: s
  // And text might be prefixed by a command, e.g.: e src/vi
  path path(rfind_after(text, " "));

  if (path.is_relative())
  {
    path.make_absolute();
  }

  auto_complete_filename_t out;

  // alias to filename
  const auto& prefix(path.filename());

  // get all matching files
  const auto& v(get_all_files(
    path.parent_path(),
    data::dir()
      .file_spec(prefix + "*")
      .dir_spec(prefix + "*")
      .type(data::dir::type_t().set().set(data::dir::RECURSIVE, false))));

  if (v.empty())
  {
    return {};
  }

  if (v.size() > 1)
  {
    auto rest_equal_size = 0;
    auto all_ok          = true;

    for (auto i = prefix.length(); i < v[0].size() && all_ok; i++)
    {
      for (size_t j = 1; j < v.size() && all_ok; j++)
      {
        if (i < v[j].size() && v[0][i] != v[j][i])
        {
          all_ok = false;
        }
      }

      if (all_ok)
      {
        rest_equal_size++;
      }
    }

    log::status() << std::accumulate(
      v.begin(),
      v.size() <= 5 ? v.end() : v.begin() + 5,
      std::string(),
      [&](const std::string& a, const std::string& b)
      {
        return a.empty() ? b : a + " " + b;
      });

    return {
      auto_complete_filename_t(v[0].substr(prefix.size(), rest_equal_size), v)};
  }

  return {auto_complete_filename_t(v[0].substr(prefix.size()), v)};
}

void wex::combobox_from_list(wxComboBox* cb, const strings_t& text)
{
  combobox_as<const strings_t>(cb, text);
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

  const auto arguments =
    (file1.stat().get_modification_time() <
     file2.stat().get_modification_time()) ?
      quoted_find(file1.string()) + " " + quoted_find(file2.string()) :
      quoted_find(file2.string()) + " " + quoted_find(file1.string());
  if (
    factory::process().system(
      config(_("list.Comparator")).get() + " " + arguments) != 0)
  {
    return false;
  }

  log::status(_("Compared")) << arguments;
  return true;
}

bool wex::lexers_dialog(syntax::stc* stc)
{
  std::vector<std::string> s;

  std::transform(
    lexers::get()->get_lexers().begin(),
    lexers::get()->get_lexers().end(),
    std::back_inserter(s),
    [](const auto& i)
    {
      return i.display_lexer();
    });

  auto lexer = stc->get_lexer().display_lexer();
  if (!single_choice_dialog(
        data::window().parent(stc).title(_("Enter Lexer")),
        s,
        lexer))
  {
    return false;
  }

  lexer.empty() ? stc->get_lexer().clear() :
                  (void)stc->get_lexer().set(lexer, true);
  return true;
}

wex::strings_t wex::listbox_to_list(wxListBox* lb)
{
  const wxArrayString items(lb->GetStrings());
  wxArrayInt          selections;
  lb->GetSelections(selections);
  strings_t st;

  for (size_t i = 0; i < items.GetCount(); ++i)
  {
    const std::string selected(
      std::ranges::any_of(
        selections,
        [i](const auto& p)
        {
          return p == i;
        }) ?
        "1" :
        "0");

    st.push_back(items[i] + ":" + selected);
  }

  return st;
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
    if (it.string().contains("*") || it.string().contains("?"))
    {
      if (it.paths().size() > 1)
      {
        count +=
          open_file_dir(
            wex::path(path::current()).append(it.data().parent_path()),
            data::dir()
              .file_spec(it.filename())
              .type(
                it.string().starts_with(".") ? data::dir::type_t().set() :
                                               type))
            .find_files();
      }
      else
      {
        count += open_file_dir(
                   path::current(),
                   data::dir().file_spec(it.string()).type(type))
                   .find_files();
      }
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

        if (!it.file_exists() && it.string().contains(":"))
        {
          if (const path &
                val(wex::factory::link().get_path(it.string(), data.control()));
              !val.empty())
          {
            fn = val;
          }
        }

        fn.make_absolute();

        if (!fn.empty())
        {
          count++;

          frame->open_file(fn, data);

          if (!fn.file_exists())
          {
            // this is not an error
            log::debug("file does not exist") << fn.string();
          }
        }
        else
        {
          log("file is illegal") << it.string();
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

void wex::process_match(const path_match& m, wxEvtHandler* eh)
{
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_LIST_MATCH);
  event.SetClientData(new path_match(m));
  wxPostEvent(eh, event);
}

bool wex::shell_expansion(std::string& command)
{
  regex r("`(.*?)`"); // non-greedy

  while (r.search(command) > 0)
  {
    factory::process process;
    if (process.system(r[0]) != 0)
    {
      return false;
    }

    r.replace(command, process.std_out());
  }

  return true;
}

void wex::vcs_command_stc(
  const vcs_command& command,
  const lexer&       lexer,
  syntax::stc*       stc)
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
  syntax::stc*                  stc)
{
  log(*result) << filename.name();
  log::status("xml error") << result->description();

  // prevent recursion
  if (stc == nullptr && filename != lexers::get()->path())
  {
    if (auto* frame =
          dynamic_cast<wex::factory::frame*>(wxTheApp->GetTopWindow());
        frame != nullptr)
    {
      stc = dynamic_cast<syntax::stc*>(frame->open_file(filename, data::stc()));
    }
  }

  if (stc != nullptr && result->offset != 0)
  {
    stc->vi_command(line_data().command("gg"));
    stc->vi_command(line_data().command(std::to_string(result->offset) + "|"));
  }
}
