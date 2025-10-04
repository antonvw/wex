////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/ui/frame.h>

#include "../syntax/test.h"

namespace wex
{
namespace test
{

class ex : public app
{
public:
  // Static methods

  static auto* frame() { return m_frame; }
  static auto* get_stc() { return m_stc; }

  // Virtual interface
  bool OnInit() override;

private:
  inline static wex::frame* m_frame = nullptr;
  inline static test::stc*  m_stc   = nullptr;
};

/// Returns abbreviations.
std::vector<std::pair<std::string, std::string>> get_abbreviations();

std::vector<std::string> get_builtin_variables();
}; // namespace test
}; // namespace wex

/// Returns the frame.
wex::frame* frame();

/// Returns an stc.
wex::syntax::stc* get_stc();
