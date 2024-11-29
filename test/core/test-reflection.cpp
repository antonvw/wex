////////////////////////////////////////////////////////////////////////////////
// Name:      test-reflection.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/describe.hpp>
#include <wex/core/chrono.h>
#include <wex/core/reflection.h>
#include <wex/test/test.h>

class test_reflect
{
public:
  test_reflect()
    : m_reflect(
        {REFLECT_ADD("bool", m_bool),
         REFLECT_ADD("double", m_double),
         REFLECT_ADD("float", m_float),
         REFLECT_ADD("int", m_int),
         REFLECT_ADD("long", m_long),
         REFLECT_ADD("size", m_size),
         REFLECT_ADD("chrono", m_chrono),
         REFLECT_ADD("time", m_chrono.get_time("2019-02-01 12:20:06"))})
  {
    ;
  };

  void change() { m_int++; };

  const auto log() const { return m_reflect.log(); }

private:
  bool   m_bool{false};
  double m_double{0.0};
  float  m_float{0.1};
  int    m_int{0};
  long   m_long{0};
  size_t m_size{0};

  wex::chrono     m_chrono;
  wex::path       m_path{"hello"};
  wex::reflection m_reflect;

  BOOST_DESCRIBE_CLASS(
    test_reflect,
    (),
    (),
    (),
    (m_bool, m_double, m_float, m_int, m_long, m_size, m_path))
};

using boost::describe::operators::operator<<;

TEST_CASE("wex::reflection")
{
  SUBCASE("constructor")
  {
    REQUIRE(wex::reflection({}).log().str().empty());
  }

  SUBCASE("boost")
  {
    std::stringstream ss;
    ss << test_reflect();
    REQUIRE(ss.str().contains("hello"));
  }

  SUBCASE("log")
  {
    wex::reflection rfl(
      {REFLECT_ADD("x", std::string()), REFLECT_ADD("y", std::string("yyy"))});

    CAPTURE(rfl.log().str());
    REQUIRE(rfl.log().str().contains("x, y: yyy"));
  }

  SUBCASE("log-skip-empty")
  {
    wex::reflection rfl(
      {REFLECT_ADD("x", std::string()), REFLECT_ADD("y", std::string("yyy"))},
      wex::reflection::log_t::SKIP_EMPTY);

    REQUIRE(rfl.log().str() == "y: yyy\n");
  }

  SUBCASE("member")
  {
    test_reflect reflect;

    REQUIRE(reflect.log().str().contains("no cast available"));
    REQUIRE(reflect.log().str().contains("int: 0"));

    reflect.change();
    REQUIRE(reflect.log().str().contains("int: 1"));
  }
}
