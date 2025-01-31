////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wex::syntax::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/stc.h>

wex::syntax::stc::stc(const data::window& data)
  : factory::stc(data)
  , m_lexer(this)
{
}

void wex::syntax::stc::fold(bool all)
{
  if (GetProperty("fold") != "1")
  {
    return;
  }

  if (all || get_line_count() > config(_("stc.Auto fold")).get(1500))
  {
    fold_all();
  }
}

void wex::syntax::stc::fold_all()
{
  const bool json = (get_lexer().scintilla_lexer() == "json");
  const bool xml  = (get_lexer().language() == "xml");

  if (!json && !xml)
  {
    FoldAll(wxSTC_FOLDACTION_CONTRACT);
    return;
  }

  const auto current_line = get_current_line();
  int        line         = (json ? 1 : 0);

  while (line < get_line_count())
  {
    if (const auto level = GetFoldLevel(line);
        xml && (level == wxSTC_FOLDLEVELBASE + wxSTC_FOLDLEVELHEADERFLAG))
    {
      line++;
    }
    else if (const auto last_child_line = GetLastChild(line, level);
             last_child_line > line + 1)
    {
      if (GetFoldExpanded(line))
      {
        FoldLine(line, wxSTC_FOLDACTION_CONTRACT);
      }

      line = last_child_line + 1;
    }
    else
    {
      line++;
    }
  }

  goto_line(current_line);
}

bool wex::syntax::stc::set_indicator(
  const indicator& indicator,
  int              start,
  int              end)
{
  if (!lexers::get()->is_loaded())
  {
    return false;
  }

  if (start == -1)
  {
    start = GetTargetStart();
  }

  if (end == -1)
  {
    end = GetTargetEnd();
  }

  if (const bool loaded(lexers::get()->indicator_is_loaded(indicator));
      !loaded || start == -1 || end == -1)
  {
    log("indicator") << indicator.number() << loaded << start << end;
    return false;
  }

  if (end == start)
  {
    return true;
  }

  SetIndicatorCurrent(indicator.number());
  IndicatorFillRange(start, end - start);

  log::trace("indicator") << start << end << GetIndicatorCurrent();

  return true;
}
