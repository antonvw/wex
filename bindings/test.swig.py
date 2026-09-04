################################################################################
# Name:      test.swig.py
# Purpose:   Test wex python bindings
# Author:    Anton van Wezenbeek
# Copyright: (c) 2026 Anton van Wezenbeek
################################################################################

import wex

# the test methods


def test_common():
    t = wex.tool_info("aha")
    assert t.info() == "aha"


def test_core():
    assert wex.now()
    r = wex.regex("([0-9])+xxx")
    assert r.match("1444xxx") == 1
    assert r.match("1444yyy") == -1

    c = wex.config("config-item")
    c.set("xyz")
    assert c.get() == "xyz"

    rp = wex.regex_part("\\*+ Settings \\*")
    assert rp.regex() == "\\*+ Settings \\*"
    for i in range(0, 6):
        assert rp.match("*") == wex.regex_part.match_t_PART
    assert rp.text() == "******"


def test_data():
    d = wex.menu()
    assert d.help_text() == ""
    d.help_text("dddd")
    assert d.help_text() == "dddd"


def test_factory():
    assert wex.control().find() == ""

    f = wex.find_replace_data()
    assert f.get_find_string() == ""

    s = wex.sort()
    assert s.string("", "") == ""
    assert s.string("", " ") == ""

    assert s.string("x", " ") == "x"
    assert s.string("a b c", " ") == "a b c"
    assert s.string("c b a", " ") == "a b c"

    assert s.string("x yz\nabc\n", "\n") == "abc\nx yz\n"
    assert s.string("x yz\nabc", "\n") == "abc\nx yz"


def test_syntax():
    p = wex.property("x", "y")
    assert p.is_ok()
    assert p.name() == "x"
    assert p.value() == "y"
    p.set("z")
    assert p.value() == "z"


# main, invoking the test methods

if __name__ == "__main__":
    test_core()
    test_common()
    test_data()
    test_factory()
    test_syntax()
    print("Everything passed")
