////////////////////////////////////////////////////////////////////////////////
// Name:      command-set.cpp
// Purpose:   Implementation of class wex::ex
//            http://pubs.opengroup.org/onlinepubs/9699919799/utilities/ex.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/cmdline.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/ex/ex.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/ui/statusbar.h>

bool wex::ex::command_set(const std::string& command)
{
  const bool modeline = command.back() == '*';

  wex::cmdline cmdline(
    // switches
    {{{"ac", _("Auto complete")}, nullptr},
     {{"autoindent,ai", "ex-set.ai"},
      [&](bool on)
      {
        if (!modeline)
          config(_("stc.Auto indent")).set(on);
      }},
     {{"aw", _("stc.Auto write")},
      [&](bool on)
      {
        m_auto_write = on;
      }},
     {{"errorbells,eb", _("stc.Error bells")}, nullptr},
     {{"el", "ex-set.el"},
      [&](bool on)
      {
        if (!modeline)
          config(_("stc.Edge line"))
            .set(on ? (long)wxSTC_EDGE_LINE : (long)wxSTC_EDGE_NONE);
        else
          get_stc()->SetEdgeMode(wxSTC_EDGE_LINE);
      }},
     {{"ignorecase,ic", "ex-set.ignorecase"},
      [&](bool on)
      {
        if (!on)
          m_search_flags |= wxSTC_FIND_MATCHCASE;
        else
          m_search_flags &= ~wxSTC_FIND_MATCHCASE;
        wex::find_replace_data::get()->set_match_case(!on);
      }},
     {{"magic", "ex-set.magic,1"},
      [&](bool on)
      {
        if (on)
          m_search_flags |= m_search_flags_regex;
        else
          m_search_flags &= ~m_search_flags_regex;
      }},
     {{"mw", "ex-set.matchwords"},
      [&](bool on)
      {
        if (on)
          m_search_flags |= wxSTC_FIND_WHOLEWORD;
        else
          m_search_flags &= ~wxSTC_FIND_WHOLEWORD;
        wex::find_replace_data::get()->set_match_word(on);
      }},
     {{"number,nu", _("stc.Line numbers")},
      [&](bool on)
      {
        if (modeline)
          get_stc()->show_line_numbers(on);
      }},
     {{"readonly", "ex-set.readonly"},
      [&](bool on)
      {
        get_stc()->SetReadOnly(on);
      }},
     {{"showmode,sm", _("stc.Show mode")},
      [&](bool on)
      {
        m_frame->get_statusbar()->pane_show("PaneMode", on);
      }},
     {{"sm", _("stc.Show match")}, nullptr},
     {{"sws", "ex-set.showwhitespace"},
      [&](bool on)
      {
        if (!modeline)
        {
          config(_("stc.Whitespace visible"))
            .set(on ? wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE);
          config(_("stc.End of line")).set(on);
        }
        else
        {
          get_stc()->SetViewWhiteSpace(wxSTC_WS_VISIBLEALWAYS);
          get_stc()->SetViewEOL(true);
        }
      }},
     {{"expandtab,et", _("stc.Expand tabs")},
      [&](bool on)
      {
        get_stc()->SetUseTabs(!on);
      }},
     {{"wm", "ex-set.wm"},
      [&](bool on)
      {
        if (!modeline)
          config(_("stc.Wrap line"))
            .set(on ? wxSTC_WRAP_CHAR : wxSTC_WRAP_NONE);
        else
          get_stc()->SetWrapMode(wxSTC_WRAP_CHAR);
      }},
     {{"ws", _("stc.Wrap scan"), "1"}, nullptr}},
    // options
    {{{"dir", "ex-set.dir", wex::path::current().string()},
      {cmdline::STRING,
       [&](const std::any& val)
       {
         wex::path::current(path(std::any_cast<std::string>(val)));
       }}},
     {{"ec", _("stc.Edge column"), std::to_string(get_stc()->GetEdgeColumn())},
      {cmdline::INT,
       [&](const std::any& val)
       {
         if (!modeline)
           config(_("stc.Edge column")).set(std::any_cast<int>(val));
         else
           get_stc()->SetEdgeColumn(std::any_cast<int>(val));
       }}},
     {{"report",
       "stc.Reported lines",
       std::to_string(config("stc.Reported lines").get(5))},
      {cmdline::INT,
       [&](const std::any& val)
       {
         config("stc.Reported lines").set(std::any_cast<int>(val));
       }}},
     {{"shiftwidth,sw",
       _("stc.Indent"),
       std::to_string(get_stc()->GetIndent())},
      {cmdline::INT,
       [&](const std::any& val)
       {
         if (!modeline)
           config(_("stc.Indent")).set(std::any_cast<int>(val));
         else
           get_stc()->SetIndent(std::any_cast<int>(val));
       }}},
     {{"sy", "ex-set.syntax"},
      {cmdline::STRING,
       [&](const std::any& val)
       {
         if (std::any_cast<std::string>(val) != "off")
           get_stc()->get_lexer().set(
             std::any_cast<std::string>(val),
             true); // allow folding
         else
           get_stc()->get_lexer().clear();
       }}},
     {{"softtabstop,ts",
       _("stc.Tab width"),
       std::to_string(get_stc()->GetTabWidth())},
      {cmdline::INT,
       [&](const std::any& val)
       {
         if (modeline)
           get_stc()->SetTabWidth(std::any_cast<int>(val));
       }}},
     {{"ve",
       "ex-set.verbosity",
       std::to_string(static_cast<int>(log::get_level()))},
      {cmdline::INT,
       [&](const std::any& val)
       {
         log::set_level((log::level_t)std::any_cast<int>(val));
         config("ex-set.verbosity").set(static_cast<int>(log::get_level()));
       }}}},
    /// params
    {},
    // no standard options
    false);

  bool          found;
  data::cmdline cmdl(command.substr(
    4,
    command.back() == '*' ? command.size() - 5 : std::string::npos));
  cmdl.save(!modeline);

  if ((found = cmdline.parse_set(cmdl)) && !modeline)
  {
    m_frame->on_command_item_dialog(
      wxID_PREFERENCES,
      wxCommandEvent(wxEVT_BUTTON, wxOK));
  }

  if (!cmdl.help().empty())
  {
    m_frame->output(cmdl.help());

    if (!modeline)
    {
      show_dialog("Options", cmdl.help(), lexer_props().scintilla_lexer());
    }
  }

  if (found)
  {
    log::trace(":set") << command;
  }

  return found;
}
