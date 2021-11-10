////////////////////////////////////////////////////////////////////////////////
// Name:      defs.h
// Purpose:   Methods to be used by vi lib
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#define REPEAT(TEXT)                   \
  {                                    \
    for (auto i = 0; i < m_count; i++) \
    {                                  \
      TEXT;                            \
    }                                  \
  }

#define REPEAT_WITH_UNDO(TEXT)    \
  {                               \
    get_stc()->BeginUndoAction(); \
    REPEAT(TEXT);                 \
    get_stc()->EndUndoAction();   \
  }

namespace wex
{
/// Returns escape sequence.
const std::string esc();

/// Returns a string for specified key.
const std::string k_s(wxKeyCode key);

/// Returns whether there is one letter after.
bool one_letter_after(const std::string& text, const std::string& letter);
} // namespace wex
