////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wex::link and stc::link... methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <wex/stc/link.h>
#include <wex/stc/stc.h>
#include <wex/syntax/path-lexer.h>
#include <wex/ui/frame.h>
#include <wex/ui/item-vector.h>
#include <wx/app.h>

// prevent very long lines to be returned, e.g. by json config files,
// as that causes:
// std::exception:filesystem error: in posix_stat:
// failed to determine attributes for the specified path: File name too long
std::string get_current_line_text(wex::link* link, wex::factory::stc* stc)
{
  const std::string& line(stc->GetCurLine().ToStdString());

  if (const size_t mp(500); line.size() >= mp)
  {
    const auto pos(stc->GetCurrentPos());
    const auto start(
      stc->FindText(pos, stc->PositionFromLine(stc->get_current_line()), "\""));
    const auto end(stc->FindText(
      pos,
      stc->GetLineEndPosition(stc->get_current_line()),
      "\""));

    if (start != wxSTC_INVALID_POSITION && end != wxSTC_INVALID_POSITION)
    {
      const auto& word(stc->GetTextRange(start + 1, end));

      // So if this word is a valid path, return it.
      wex::line_data data;
      if (const auto& ok(link->get_path(word, data)); !ok.empty())
      {
        return ok.string();
      }
    }

    // Otherwise just return the max allowed size.
    return line.substr(0, mp - 1);
  }

  return line;
}

std::string wex::link::get_link_pairs(const std::string& text) const
{
  for (const auto& p : item_vector(stc::config_items())
                         .find<config::strings_t>(_("stc.link.Pairs")))
  {
    const auto pos1 = text.find(find_before(p, "\t"));
    const auto pos2 = text.rfind(find_after(p, "\t"));

    if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
    {
      // Okay, get everything in between, and make sure we skip white space.
      return boost::algorithm::trim_copy(
        text.substr(pos1 + 1, pos2 - pos1 - 1));
    }
  }

  return std::string();
}

const wex::path
wex::link::get_path(const std::string& text, line_data& data, factory::stc* stc)
{
  auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
  frame->vcs_add_path(this);
  return factory::link::get_path(text, data, stc);
}

bool wex::stc::link_open()
{
  return link_open(link_t().set(LINK_OPEN).set(LINK_OPEN_MIME));
}

bool wex::stc::link_open(link_t mode, std::string* link)
{
  const auto& sel(get_selected_text());

  if (const size_t max_link_size = 10000; sel.size() > max_link_size)
  {
    return false;
  }

  const auto& text(!sel.empty() ? sel : get_current_line_text(m_link, this));
  bool        found(false);

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         text,
         boost::char_separator<char>("\n")))
  {
    if (mode[LINK_OPEN])
    {
      data::control data;

      if (const wex::path path(m_link->get_path(it, data, this));
          !path.string().empty())
      {
        if (link != nullptr)
        {
          if (!link->empty())
          {
            *link += ",";
          }

          *link += path.filename();
        }
        else if (!mode[LINK_CHECK])
        {
          m_frame->open_file(path, data);
        }

        found = true;
      }
    }

    // Open at most one mime link.
    if (!found && mode[LINK_OPEN_MIME])
    {
      if (const wex::path_lexer path(m_link->get_path(
            it,
            data::control().line(link::LINE_OPEN_URL),
            this));
          !path.string().empty())
      {
        if (!mode[LINK_CHECK])
        {
          if (browser(path.string()))
          {
            found = true;
          }
        }
      }
      else if (const wex::path mime(m_link->get_path(
                 it,
                 data::control().line(link::LINE_OPEN_MIME),
                 this));
               !mime.string().empty())
      {
        found = (!mode[LINK_CHECK]) ? mime.open_mime(): true;
      }
    }
  }

  if (link != nullptr && found)
  {
    *link = find_tail(*link, 25);
  }

  return found;
}
