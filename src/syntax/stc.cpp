////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wex::syntax::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/stc.h>

bool wex::syntax::stc::set_indicator(
  const indicator& indicator,
  int              start,
  int              end)
{
  if (start == -1)
    start = GetTargetStart();
  if (end == -1)
    end = GetTargetEnd();

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
