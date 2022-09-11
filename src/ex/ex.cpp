////////////////////////////////////////////////////////////////////////////////
// Name:      ex.cpp
// Purpose:   Implementation of class wex::ex
//            http://pubs.opengroup.org/onlinepubs/9699919799/utilities/ex.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/ex/command-parser.h>
#include <wex/ex/ctags.h>
#include <wex/ex/ex-stream.h>
#include <wex/ex/ex.h>
#include <wex/ex/macros.h>
#include <wex/factory/defs.h>
#include <wex/factory/lexers.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/ui/statusbar.h>
#include <wx/app.h>

#include "eval.h"

wex::macros wex::ex::m_macros;

wex::ex::ex(wex::factory::stc* stc, mode_t mode)
  : m_command(ex_command(stc))
  , m_frame(dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow()))
  , m_ex_stream(new wex::ex_stream(this))
  , m_mode(mode)
  , m_commands(commands_ex())
  , m_ctags(new wex::ctags(this))
  , m_auto_write(config(_("stc.Auto write")).get(false))
{
  assert(m_frame != nullptr);

  reset_search_flags();
}

wex::ex::~ex()
{
  delete m_ctags;
  delete m_ex_stream;
}

bool wex::ex::auto_write()
{
  if (!m_auto_write || !get_stc()->IsModified())
  {
    return true;
  }

  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE);

  wxPostEvent(wxTheApp->GetTopWindow(), event);

  return true;
}

int wex::ex::calculator(const std::string& text)
{
  const auto& [val, err] = evaluator().eval(this, text);

  if (!err.empty())
  {
    show_dialog("Calculate Error", err);
  }

  return val;
}

bool wex::ex::command(const std::string& cmd)
{
  auto command(cmd);

  if (m_mode == OFF || command.empty() || command.front() != ':')
    return false;

  log::trace("ex command") << cmd;

  const auto& it = m_macros.get_map().find(command);
  command        = (it != m_macros.get_map().end() ? it->second : command);

  if (m_frame->exec_ex_command(m_command.set(command)))
  {
    m_macros.record(command);
    m_command.clear();
    return auto_write();
  }
  else if (
    command == ":" || command == ":" + ex_command::selection_range() ||
    command == ":!")
  {
    return m_frame->show_ex_command(get_stc(), command);
  }
  else if (
    !command_handle(command) &&
    !command_parser(this, command.substr(1)).is_ok())
  {
    m_command.clear();
    return false;
  }
  else
  {
    m_macros.record(command);
  }

  m_command.clear();

  return auto_write();
}

void wex::ex::copy(const wex::ex* ex)
{
  m_marker_identifiers = ex->m_marker_identifiers;
  m_copy               = true; // no char numbers for a copy
}

void wex::ex::cut()
{
  const auto& sel(get_stc()->get_selected_text());

  yank('0');

  get_stc()->ReplaceSelection(wxEmptyString);

  set_registers_delete(sel);

  info_message(sel, wex::info_message_t::DEL);
}

wex::factory::stc* wex::ex::get_stc() const
{
  return m_command.get_stc();
}

void wex::ex::info_message(const std::string& text, wex::info_message_t type)
  const
{
  if (const auto lines = get_number_of_lines(text);
      lines > config("stc.Reported lines").get(5))
  {
    wxString msg;

    switch (type)
    {
      case wex::info_message_t::ADD:
        msg = _("%d lines added");
        break;
      case wex::info_message_t::COPY:
        msg = _("%d lines copied");
        break;
      case wex::info_message_t::DEL:
        msg = _("%d fewer lines");
        break;
      case wex::info_message_t::MOVE:
        msg = _("%d lines moved");
        break;
      case wex::info_message_t::YANK:
        msg = _("%d lines yanked");
        break;
      default:
        return;
    }

    m_frame->show_ex_message(wxString::Format(msg, lines - 1));
  }
}

bool wex::ex::marker_add(char marker, int line)
{
  if (m_copy)
    return false;

  if (!get_stc()->is_visual())
  {
    return m_ex_stream->marker_add(marker, line);
  }

  const wex::marker lm(wex::lexers::get()->get_marker(m_marker_symbol));

  if (!lm.is_ok())
  {
    wex::log("could not find marker")
      << marker << "symbol" << m_marker_symbol.number();
    return false;
  }

  marker_delete(marker);

  int       id;
  const int lin =
    (line == LINE_NUMBER_UNKNOWN ? get_stc()->get_current_line() : line);

  if (lm.symbol() == wxSTC_MARK_CHARACTER)
  {
    if (const auto& it = m_marker_numbers.find(marker);
        it == m_marker_numbers.end())
    {
      // We have symbol:
      // 0: non-char ex marker
      // 1: change marker
      // 2: breakpoint marker
      // 3..: character markers (all markers in m_marker_identifiers)
      const auto marker_offset = 3;
      const auto marker_number = m_marker_identifiers.size() + marker_offset;

      get_stc()->MarkerDefine(
        marker_number,
        wxSTC_MARK_CHARACTER + marker,
        wxColour(lm.foreground_colour()),
        wxColour(lm.background_colour()));

      id                       = get_stc()->MarkerAdd(lin, marker_number);
      m_marker_numbers[marker] = marker_number;
    }
    else
    {
      id = get_stc()->MarkerAdd(lin, it->second);
    }
  }
  else
  {
    id = get_stc()->MarkerAdd(lin, m_marker_symbol.number());
  }

  if (id == -1)
  {
    log("could not add marker") << marker << "to line:" << lin;
    return false;
  }

  m_marker_identifiers[marker] = id;

  return true;
}

bool wex::ex::marker_delete(char marker)
{
  if (!get_stc()->is_visual())
  {
    return m_ex_stream->marker_delete(marker);
  }

  if (const auto& it = m_marker_identifiers.find(marker);
      it != m_marker_identifiers.end())
  {
    get_stc()->MarkerDeleteHandle(it->second);
    m_marker_identifiers.erase(it);
    return true;
  }

  return false;
}

bool wex::ex::marker_goto(char marker)
{
  if (const auto line = marker_line(marker); line != LINE_NUMBER_UNKNOWN)
  {
    get_stc()->inject(data::control().line(line + 1));
    return true;
  }

  return false;
}

int wex::ex::marker_line(char marker) const
{
  if (!get_stc()->is_visual())
  {
    return m_ex_stream->marker_line(marker);
  }

  if (marker == '<')
  {
    if (!get_stc()->get_selected_text().empty())
    {
      return get_stc()->LineFromPosition(get_stc()->GetSelectionStart());
    }
  }
  else if (marker == '>')
  {
    if (!get_stc()->get_selected_text().empty())
    {
      return get_stc()->LineFromPosition(get_stc()->GetSelectionEnd());
    }
  }
  else
  {
    if (const auto& it = m_marker_identifiers.find(marker);
        it != m_marker_identifiers.end())
    {
      if (const auto line = get_stc()->MarkerLineFromHandle(it->second);
          line == -1)
      {
        log::status("Handle for marker") << marker << "invalid";
      }
      else
      {
        return line;
      }
    }
    else
    {
      log::status("Undefined marker") << marker;
    }
  }

  if (config(_("stc.Error bells")).get(true))
  {
    wxBell();
  }

  return LINE_NUMBER_UNKNOWN;
}

void wex::ex::print(const std::string& text)
{
  if (!m_frame->print_ex(get_stc(), text))
  {
    show_dialog("Print", text);
  }
}

const std::string wex::ex::register_insert()
{
  return m_macros.get_register('.');
}

const std::string wex::ex::register_text() const
{
  return m_register ? m_macros.get_register(m_register) :
                      m_macros.get_register('0');
}

void wex::ex::reset_search_flags()
{
  m_search_flags =
    ((find_replace_data::get()->match_case() ? wxSTC_FIND_MATCHCASE : 0) |
     wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX);
}

void wex::ex::set_registers_delete(const std::string& value)
{
  if (value.empty())
  {
    return;
  }

  for (int i = 9; i >= 2; i--)
  {
    if (const auto value(m_macros.get_register(static_cast<char>(48 + i - 1)));
        !value.empty())
    {
      m_macros.set_register(static_cast<char>(48 + i), value);
    }
  }

  m_macros.set_register('1', value);
}

void wex::ex::set_register_insert(const std::string& value)
{
  m_macros.set_register('.', value);
}

void wex::ex::set_register_yank(const std::string& value)
{
  m_macros.set_register('0', value);
}

void wex::ex::show_dialog(
  const std::string& title,
  const std::string& text,
  const std::string& lexer)
{
  if (m_frame->stc_entry_dialog_component() == nullptr)
  {
    return;
  }

  if (title == "Print")
  {
    if (title != m_frame->stc_entry_dialog_title())
    {
      m_frame->stc_entry_dialog_component()->set_text(text);
    }
    else
    {
      m_frame->stc_entry_dialog_component()->AppendText(text);
      m_frame->stc_entry_dialog_component()->DocumentEnd();
    }
  }
  else
  {
    m_frame->stc_entry_dialog_component()->set_text(text);
  }

  m_frame->stc_entry_dialog_title(title);
  m_frame->stc_entry_dialog_component()->get_lexer().set(
    !lexer.empty() ? wex::lexer(lexer) : get_stc()->get_lexer());

  m_frame->show_stc_entry_dialog();
}

void wex::ex::use(mode_t mode)
{
  log::trace("ex mode from")
    << static_cast<int>(m_mode) << "to:" << static_cast<int>(mode);

  m_mode = mode;
}

bool wex::ex::yank(char name) const
{
  const auto& range(get_stc()->get_selected_text());

  if (range.empty())
  {
    return false;
  }
  else if (register_name())
  {
    m_macros.set_register(register_name(), range);
  }
  else if (name != '0')
  {
    m_macros.set_register(name, range);
  }
  else
  {
    set_register_yank(range);
  }

  info_message(range, wex::info_message_t::YANK);

  return true;
}