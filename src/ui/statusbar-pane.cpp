////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar-pane.cpp
// Purpose:   Implementation of wex::statusbar_pane class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/statusbar-pane.h>

namespace wex
{
std::string
determine_help_text(const std::string& name, const std::string& text)
{
  if (name == "PaneDBG")
  {
    return _("Debugger");
  }
  else if (name == "PaneFileType")
  {
    return _("File Type");
  }
  else if (name == "PaneInfo")
  {
    return _("Lines or Items");
  }
  else if (name == "PaneMode")
  {
    return "vi mode";
  }
  else if (name == "PaneTheme")
  {
    return _("Theme");
  }
  else
  {
    return text.empty() ? name.substr(name.find('e') + 1) : text;
  }
}
} // namespace wex

wex::statusbar_pane::statusbar_pane(
  const std::string& name,
  int                width,
  bool               show)
  : wxStatusBarPane(wxSB_NORMAL, width)
  , m_help_text(determine_help_text(name, std::string()))
  , m_is_shown(show)
  , m_name(name)
{
}

wex::statusbar_pane& wex::statusbar_pane::help(const std::string& rhs)
{
  m_help_text = rhs;
  return *this;
}

wex::statusbar_pane& wex::statusbar_pane::hidden_text(const std::string& rhs)
{
  m_hidden = rhs;
  return *this;
}

wex::statusbar_pane& wex::statusbar_pane::show(bool show)
{
  m_is_shown = show;

  if (show)
  {
    m_hidden.clear();
  }
  else
  {
    m_hidden = GetText();
  }

  return *this;
}

wex::statusbar_pane& wex::statusbar_pane::style(int rhs)
{
  SetStyle(rhs);

  return *this;
}
