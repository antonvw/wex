////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Methods to be used by vi lib
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#define REPEAT(TEXT)                   \
  {                                    \
    for (auto i = 0; i < m_count; i++) \
    {                                  \
      TEXT;                            \
    }                                  \
  }
