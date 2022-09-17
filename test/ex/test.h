////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>

#include "../test.h"

namespace wex
{
namespace test
{
class ex_stc : public wex::syntax::stc
{
public:
  explicit ex_stc(wxFrame* parent)
  {
    Create(parent, -1);
    Show();
  };

private:
  const wex::path& path() const override { return m_path; };
  wex::path        m_path;
};

class ex : public app
{
public:
  /// Static methods

  static auto* frame() { return m_frame; }
  static auto* get_stc() { return m_stc; }

  /// Virtual interface
  bool OnInit() override;

private:
  inline static wex::frame* m_frame = nullptr;
  inline static ex_stc*     m_stc   = nullptr;
};
}; // namespace test
}; // namespace wex

std::vector<std::string> get_builtin_variables();

/// Returns the frame.
wex::frame* frame();

/// Returns an stc.
wex::syntax::stc* get_stc();
