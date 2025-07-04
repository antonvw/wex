////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar-pane.cpp
// Purpose:   Implementation of wex::statusbar_pane class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/statusbar-pane.h>

#include <map>

namespace wex
{
std::string determine_help_text(const std::string& name)
{
  const std::map<std::string, std::string> desc{
    {"PaneDBG", _("Debugger")},
    {"PaneFileType", _("File Type")},
    {"PaneInfo", _("Lines or Items")},
    {"PaneBlameAuthor", _("Current line author annotation")},
    {"PaneBlameComments", _("Current line comments annotation")},
    {"PaneBlameDate", _("Current line date annotation")},
    {"PaneMode", "vi mode"},
    {"PaneTheme", _("Theme")}};

  if (const auto& search = desc.find(name); search != desc.end())
  {
    return search->second;
  }

  return name.substr(name.find('e') + 1);
}
} // namespace wex

wex::statusbar_pane::statusbar_pane(
  const std::string& name,
  int                width,
  bool               show)
  : wxStatusBarPane(wxSB_NORMAL, width)
  , m_help_text(determine_help_text(name))
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
