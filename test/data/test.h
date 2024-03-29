////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Test file for running unit tests
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

// The actual contents do not matter. This file is not compiled.
// However, it is used for testing matches and testing ctags.
#include <wex/wex.h>

namespace wex
{
/// Derive your application from app.
class test_app : public app
{
public:
  /// Constructor.
  test_app() { ; }

  void method_one() const { ; }

  bool method_two() const { return 1; }

  bool method_three(const std::string& s1) { return 1; }

private:
  /// Override the OnInit.
  virtual bool OnInit();

  auto_complete m_ac;
  auto_indent   m_ai;
};

class helper
{
public:
  void helper_method() { ; }
};
}; // namespace wex

/*
 * this is a TEST for matching words.
 * this is a TEST for matching words.
 * this is a TESTXXX for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 *
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 *
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 *
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 *
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 *
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 *
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 *
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 *
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 *
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 *
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 *
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 * this is a line without a match.
 *
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 * this is a test for matching words.
 */
