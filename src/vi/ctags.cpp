////////////////////////////////////////////////////////////////////////////////
// Name:      ctags.cpp
// Purpose:   Implementation of class wex::ctags
//            https://github.com/universal-ctags/ctags
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <readtags.h>

#include <wex/config.h>
#include <wex/ctags.h>
#include <wex/ex.h>
#include <wex/factory/stc.h>
#include <wex/frame.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wx/app.h>
#include <wx/choicdlg.h>

namespace wex
{
auto* get_frame()
{
  return dynamic_cast<frame*>(wxTheApp->GetTopWindow());
}

const std::string tag_name(const path& p)
{
  return config(_("stc.vi tag fullpath")).get(false) ? p.string() :
                                                       p.filename();
};

// Support class.
class ctags_info
{
public:
  // Constructor.
  explicit ctags_info(const tagEntry& entry)
    : m_line_number(entry.address.lineNumber)
    , m_path(entry.file)
    , m_pattern(
        [](const tagEntry& entry)
        {
          std::string pattern(
            entry.address.pattern != nullptr ?
              // prepend colon to force ex command
              ":" + std::string(entry.address.pattern) :
              std::string());
          // the pattern generated by
          // ctags mixes regex with non regex....
          boost::algorithm::replace_all(pattern, "*", "\\*");
          boost::algorithm::replace_all(pattern, "(", "\\(");
          boost::algorithm::replace_all(pattern, ")", "\\)");
          return pattern;
        }(entry))
  {
  }

  // Returns name, being fullpath or path name depending on
  // config settings.
  const std::string name() const { return tag_name(m_path); }

  // Opens file in specified frame.
  auto open_file(factory::frame* frame) const
  {
    return frame->open_file(
      m_path,
      data::control().line(m_line_number).command(m_pattern));
  }

private:
  const path        m_path;
  const int         m_line_number;
  const std::string m_pattern;
};
}; // namespace wex

std::map<std::string, wex::ctags_info>           wex::ctags::m_matches;
std::map<std::string, wex::ctags_info>::iterator wex::ctags::m_iterator;

wex::ctags::ctags(wex::ex* ex, bool open_file)
  : m_ex(ex)
{
  if (open_file)
  {
    open();
  }
}

const std::string
wex::ctags::auto_complete(const std::string& text, const ctags_entry& filter)
{
  if (m_file == nullptr)
  {
    return std::string();
  }

  ctags_entry entry(filter);

  if (text.empty())
  {
    if (tagsFirst(m_file, &entry.entry()) == TagFailure)
    {
      return std::string();
    }
  }
  else if (
    tagsFind(
      m_file,
      &entry.entry(),
      text.c_str(),
      TAG_PARTIALMATCH | TAG_OBSERVECASE) == TagFailure)
  {
    return std::string();
  }

  if (!m_is_prepared)
  {
    auto_complete_prepare();
  }

  std::string s, prev_tag;

  const int min_size{3}, max{100};
  int       count{0};
  tagResult result = TagSuccess;

  do
  {
    if (const auto& tag(entry.entry_string(min_size));
        !tag.empty() && tag != prev_tag)
    {
      s.append(tag);
      s.push_back(m_separator);
      count++;
      prev_tag = tag;
    }

    result =
      (text.empty() ? tagsNext(m_file, &entry.entry()) :
                      tagsFindNext(m_file, &entry.entry()));
  } while (result == TagSuccess && count < max);

  log::trace("ctags::auto_complete count") << count;

  return s;
}

void wex::ctags::auto_complete_prepare()
{
  m_ex->get_stc()->AutoCompSetIgnoreCase(false);
  m_ex->get_stc()->AutoCompSetAutoHide(false);

  ctags_entry::register_image(m_ex->get_stc());

  m_is_prepared = true;
}

bool wex::ctags::close()
{
  if (m_file == nullptr || tagsClose(m_file) == TagFailure)
  {
    return false;
  }

  m_file = nullptr;

  return true;
}

bool wex::ctags::do_open(const std::string& path)
{
  if (tagFileInfo info; (m_file = tagsOpen(path.c_str(), &info)) != nullptr)
  {
    log::trace("ctags file") << path;
    return true;
  }

  return false;
}

bool wex::ctags::find(const std::string& tag, ex* ex)
{
  if (m_file == nullptr)
  {
    return false;
  }

  if (tag.empty())
  {
    log::trace("ctags::find empty tag") << m_matches.size();
    return next();
  }

  ctags_entry entry;

  if (
    tagsFind(m_file, &entry.entry(), tag.c_str(), TAG_FULLMATCH) == TagFailure)
  {
    log::status("Tag not found") << tag;
    return false;
  }

  m_matches.clear();

  do
  {
    if (const ctags_info ct(entry.entry());
        ex == nullptr || (ct.name() != tag_name(ex->get_stc()->path())))
    {
      m_matches.insert({ct.name(), ct});
    }
  } while (tagsFindNext(m_file, &entry.entry()) == TagSuccess);

  m_iterator = m_matches.begin();

  log::trace("ctags::find matches") << m_matches.size();

  switch (m_matches.size())
  {
    case 0:
      if (ex != nullptr)
      {
        ex->get_stc()->find(tag);
      }
      break;

    case 1:
      m_matches.begin()->second.open_file(get_frame());
      break;

    default:
    {
      wxArrayString as;

      for (const auto& it : m_matches)
        as.Add(it.second.name());

      wxMultiChoiceDialog dialog(
        get_frame(),
        _("Input") + ":",
        _("Select File"),
        as);

      if (dialog.ShowModal() != wxID_OK)
        return false;

      for (const auto& sel : dialog.GetSelections())
      {
        m_iterator = m_matches.find(as[sel]);
        m_iterator->second.open_file(get_frame());
      }
    }
  }

  find_replace_data::get()->set_find_string(tag);

  return true;
}

bool wex::ctags::find(const std::string& tag, wex::ctags_entry& filter)
{
  if (m_file == nullptr)
  {
    return false;
  }

  ctags_entry entry;

  // Find first entry. This entry determines which kind of
  // filter will be set.
  if (
    tagsFind(m_file, &entry.entry(), tag.c_str(), TAG_FULLMATCH) == TagFailure)
  {
    return false;
  }

  filter.clear();

  do
  {
    if (entry.is_master())
    {
      filter.filter(entry);
      return true;
    }
  } while (!entry.is_master() &&
           tagsFindNext(m_file, &entry.entry()) == TagSuccess);

  return false;
}

bool wex::ctags::next()
{
  if (m_matches.size() <= 1)
  {
    log::trace("ctags::next no next match") << m_matches.size();
    return false;
  }

  if (++m_iterator == m_matches.end())
  {
    m_iterator = m_matches.begin();
  }

  m_iterator->second.open_file(get_frame());

  return true;
}

void wex::ctags::open(const std::string& filename)
{
  if (m_file != nullptr)
  {
    return;
  }

  m_iterator = m_matches.begin();

  if (filename != DEFAULT_TAGFILE)
  {
    do_open(filename);
  }
  else if (const std::vector<std::string> v{"./", config::dir().string() + "/"};
           !std::any_of(
             v.begin(),
             v.end(),
             [filename](const auto& p)
             {
               return do_open(p + filename);
             }))
  {
    if (filename != DEFAULT_TAGFILE && m_file == nullptr)
    {
      log("could not locate ctags file") << filename;
    }
  }
}

bool wex::ctags::previous()
{
  if (m_matches.size() <= 1)
  {
    log::trace("ctags::previous no previous match") << m_matches.size();
    return false;
  }

  if (m_iterator == m_matches.begin())
  {
    m_iterator = m_matches.end();
  }

  --m_iterator;

  m_iterator->second.open_file(get_frame());

  return true;
}
