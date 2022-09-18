////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wex::link and stc::link... methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/stc/link.h>
#include <wex/stc/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/item-vector.h>
#include <wx/app.h>

wex::link::link()
  : factory::link()
{
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

bool wex::stc::link_open(link_t mode, std::string* filename)
{
  const auto& sel(GetSelectedText().ToStdString());

  if (sel.size() > 200 || (!sel.empty() && sel.find('\n') != std::string::npos))
  {
    return false;
  }

  const auto& text(!sel.empty() ? sel : GetCurLine().ToStdString());

  if (mode[LINK_OPEN])
  {
    data::control data;

    if (const wex::path path(m_link->get_path(text, data, this));
        !path.string().empty())
    {
      if (filename != nullptr)
      {
        *filename = path.filename();
      }
      else if (!mode[LINK_CHECK])
      {
        m_frame->open_file(path, data);
      }

      return true;
    }
  }

  if (mode[LINK_OPEN_MIME])
  {
    if (const wex::path path(m_link->get_path(
          text,
          data::control().line(link::LINE_OPEN_URL),
          this));
        !path.string().empty())
    {
      if (!mode[LINK_CHECK])
      {
        browser(path.string());
      }

      return true;
    }
    else if (const wex::path mime(m_link->get_path(
               text,
               data::control().line(link::LINE_OPEN_MIME),
               this));
             !mime.string().empty())
    {
      if (!mode[LINK_CHECK])
      {
        return mime.open_mime();
      }

      return true;
    }
  }

  return false;
}
