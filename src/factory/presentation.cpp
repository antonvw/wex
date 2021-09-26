////////////////////////////////////////////////////////////////////////////////
// Name:      presentation.cpp
// Purpose:   Implementation of class wex::presentation
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>

#include <wex/core/log.h>
#include <wex/factory/lexers.h>
#include <wex/factory/presentation.h>
#include <wx/stc/stc.h>

import<charconv>;

wex::presentation::presentation(presentation_t type, const pugi::xml_node& node)
  : m_type(type)
{
  if (node.empty())
    return;

  const auto single = lexers::get()->apply_macro(node.attribute("no").value());
  const std::string text(node.text().get());

  try
  {
    boost::tokenizer<boost::char_separator<char>> tok(
      text,
      boost::char_separator<char>(","));

    if (
      std::from_chars(single.data(), single.data() + single.size(), m_no).ec !=
      std::errc())
    {
      log("invalid " + name() + " number") << m_no << node;
    }

    if (auto it = tok.begin(); it != tok.end())
    {
      const auto& applied(lexers::get()->apply_macro(*it));

      if (
        std::from_chars(
          applied.data(),
          applied.data() + applied.size(),
          m_style)
          .ec != std::errc())
      {
        log("invalid " + name() + " style") << applied << node;
      }

      if (++it != tok.end())
      {
        m_foreground_colour = lexers::get()->apply_macro(*it);

        if (m_type == INDICATOR && ++it != tok.end())
        {
          m_under = (*it == "true");
        }

        if (m_type == MARKER && ++it != tok.end())
        {
          m_background_colour = lexers::get()->apply_macro(*it);
        }
      }
    }
  }
  catch (std::exception& e)
  {
    log(e) << name() << single << text;
  }
}

wex::presentation::presentation(presentation_t type, int no, int style)
  : m_type(type)
  , m_no(no)
  , m_style(style)
{
}

void wex::presentation::apply(wxStyledTextCtrl* stc) const
{
  if (is_ok() && stc->GetParent() != nullptr)
  {
    switch (m_type)
    {
      case INDICATOR:
        stc->IndicatorSetStyle(m_no, m_style);

        if (!m_foreground_colour.empty())
        {
          stc->IndicatorSetForeground(m_no, wxString(m_foreground_colour));
        }

        stc->IndicatorSetUnder(m_no, m_under);
        break;

      case MARKER:
        stc->MarkerDefine(
          m_no,
          m_style,
          wxString(m_foreground_colour),
          wxString(m_background_colour));
        break;

      default:
        assert(0);
    }
  }
}

bool wex::presentation::is_ok() const
{
  switch (m_type)
  {
    case INDICATOR:
      return m_no >= 0 && m_no <= wxSTC_INDIC_MAX && m_style >= 0 &&
             m_style <= wxSTC_INDIC_ROUNDBOX;

    case MARKER:
      return m_no >= 0 && m_no <= wxSTC_MARKER_MAX &&
             ((m_style >= 0 && m_style <= wxSTC_MARKER_MAX) ||
              (m_style >= wxSTC_MARK_CHARACTER &&
               m_style <= wxSTC_MARK_CHARACTER + 255));
    default:
      assert(0);
      return false;
  }
}

const std::string wex::presentation::name() const
{
  switch (m_type)
  {
    case INDICATOR:
      return "indicator";

    case MARKER:
      return "marker";

    default:
      assert(0);
      return std::string();
  }
}
