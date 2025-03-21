# wex library

This library is written in the c++ language, and offers classes
to add vi or ex functionality as specified in
"The Open Group Base Specifications Issue 8, 2024 edition"
to your apps.

## wex c++ libraries

lib         | sub-lib   | src     | info
------------|-----------|---------|------
wex-core    |           | core    | core classes
wex-test    |           | test    | test classes
wex-factory |           | factory | virtual base classes
wex-syntax  |           | syntax  | syntax highlighting classes
wex-data    |           | data    | data classes
wex-common  |           | common  | common classes
wex-ui      | wex-ctags | ui      | user interface classes
wex-ex      | wex-vi    | ex      | ex and vi classes
wex-stc     |           | stc     | stc classes
wex-vcs     |           | vcs     | version control system classes
wex-del     |           | del     | delivered classes

To ensure proper implementation of c++ code there is the excellent
tool clang-tidy that checks for several c++ topics and also
has an option to fix anomalies. Run it:

```bash
  ./build-gen.sh -T -d tidy
  cd tidy
  run-clang-tidy -quiet
```

possibly followed by:

```bash
  run-clang-tidy -fix
```

It benefits from the following wxWidgets libraries:

## wxWidgets libraries

- all gui classes are derived from / use wxWidgets base classes

lib         | info
------------|------
wxbase      | base
wxcore      | core
wxaui       | advanced user interface
wxHTML      | HTML
wxscintilla | stc
wxstc       | stc

It benefits from the following boost libraries:

## boost c++ libraries

lib                       | info
--------------------------|------
boost::algorithm          | uses find_tail, icontains, iequals, replace_all, to_upper, trim
boost::describe           | to add reflection
boost::json               | to implement wex::config
boost::log                | to implement wex::log
boost::process            | to implement wex::process
boost::program_options    | to implement wex::cmdline
boost::regular expression | to implement the wex::regex_part
boost::spirit             | to implement the wex::evaluator
boost::statechart         | to implement the statemachine for vi mode and macro mode
boost::tokenizer          | to tokenize expressions
boost::URL                | to handle URLs

It benefits from the following c++ features:

## c++ libraries

### Algorithms library

```cpp
  std::ranges::all_of (c++11)
```

  E.g. when doing a global command on all of it's subcommands

  example:

```cpp
bool wex::global_env::for_each(const block_lines& match) const
{
  return !has_commands() ? match.set_indicator(m_ar.find_indicator()) :
                           std::ranges::all_of(
                             m_commands,
                             [this, match](const std::string& it)
                             {
                               return command(match, it);
                             );

}
```

### Containers library

```cpp
  std::span
```

  This container is used e.g. to pass buffers around.
  Example:

```cpp
  /// Writes file from buffer.
  bool write(std::span<const char> buffer);
```

### Filesystem library (c++17)

```cpp
  std::filesystem
  std::filesystem::directory_iterator
  std::filesystem::recursive_directory_iterator
```

### Formatting library (c++20)

```cpp
  std::format
```

  example:

```cpp
  std::string
  wex::get_lines(factory::stc* stc, int start, int end, const std::string& flags)
  ...

    if (flags.contains("#"))
    {
#ifndef __WXOSX__
      text += std::format("{:6} ", i + 1);
```

### Input/output library

```cpp
  std::fstream
```

  The base of all io uses a std::fstream class.

### Numerics library

```cpp
  std::accumulate
```

  The std::accumulate is used e.g. within the vi macros to
  return a string containing all elements of the requested
  register (the find returns a std::vector<std::string>).

  example:

```cpp
  const auto& it = m_macros.find(std::string(1, name));
  return it != m_macros.end() ? std::accumulate(
                                  it->second.begin(),
                                  it->second.end(),
                                  std::string()) :
                                std::string();
```

### Regular expressions library (c++11)

```cpp
  std::regex
  std::regex_match
  std::regex_replace
  std::regex_search
```

  E.g. here is code to parse a:

```cpp
  // :set [option[=[value]] ...][nooption ...][option? ...][all]
```

  that implements the ex :set OpenSource specs.

  example:

```cpp
  regex r(
    {"all",
     // [nooption ...]
     "no([a-z0-9]+)(.*)",
     // [option? ...]
     "([a-z0-9]+)[ \t]*\\?(.*)",
     // [option[=[value]] ...]
     "([a-z0-9]+)(=[a-z0-9]+)?(.*)"});

  std::string help;
  bool        found = false;

  for (auto line(boost::algorithm::trim_copy(data.string())); !line.empty();)
  {
    switch (r.search(line); r.which_no())
```

  and search:

```cpp
  int wex::regex::find(const std::string& text, find_t how)
  ...
  for (const auto& reg : m_regex)
  {
    try
    {
      if (std::match_results<std::string::const_iterator> m;
          ((how == REGEX_MATCH && std::regex_match(text, m, reg.first)) ||
           (how == REGEX_SEARCH && std::regex_search(text, m, reg.first))))
      {
        if (m.size() > 1)
        {
          m_matches.clear();
          std::copy(++m.begin(), m.end(), std::back_inserter(m_matches));
        }

        m_which    = reg;
        m_which_no = index;

        return m_matches.size();
      }

      index++;
    }
    catch (std::regex_error& e)
    {
      log(e) << reg.second << "code:" << (int)e.code();
    }
  }
```

### Strings library

```cpp
  std::stoi (c++11)
  std::to_string (c++11)
```

  These functions are used heavily, the advice is to be sure that
  you should be aware that a std::exception might be raised.
  And, if performance is an issue prefer using from_chars.

```cpp
  starts_with (c++20)
```

  vi/vi.cpp:

```cpp
  if (command.starts_with(k_s(WXK_CONTROL_R) + "="))
```

```cpp
  contains (c++23)
```

  syntax/lexer.cpp:

```cpp
    if (line.contains(":"))
```

### Thread support library (c++17)

```cpp
  std::thread
  std::future (together with boost) (c++11)
```

  See next.

### Utilities library

```cpp
  std::any (c++17)
```

  The std::any container is used as general container for
  gui elements.

```cpp
  std::expected (c++23)
```

  See e.g. eval.h:

```cpp
  /// Returns calculated value.
  std::expected<int, std::string> eval(
    /// the ex component, e.g. for line number (.) if present in text
    const ex* ex,
    /// text containing the expression to be evaluated
    const std::string& text) const;
```

```cpp
  std::function (c++11)
```

  A lot used for callbacks, e.g.:

  example: in lexers:

```cpp
    void wex::lexers::apply_default_style(
      std::function<void(const std::string&)> back,
      std::function<void(const std::string&)> fore) const
    {
      if (std::vector<std::string> v;
          back != nullptr && match(",back:(.*),", m_default_style.value(), v) > 0)
      {
        back(v[0]);
      }

      if (std::vector<std::string> v;
          fore != nullptr && match(",fore:(.*)", m_default_style.value(), v) > 0)
      {
        fore(v[0]);
      }
    }
```

  used in listview

```cpp
      lexers::get()->apply_default_style([=, this](const std::string& back) {
        SetBackgroundColour(wxColour(back));
      });
```

```cpp
  std::make_unique (c++14)
  std::shared_ptr (c++11)
  std::unique_ptr (c++11)

  std::chrono
```

  example:

```cpp
      std::thread u([debug   = m_debug.load(),
                     io      = m_io,
                     &os     = m_os,
                     process = m_process,
                     queue   = m_queue] {
        while (os.good() && !io->stopped())
        {
          io->run_one_for(std::chrono::milliseconds(10));

          if (!queue->empty())
          {
            const std::string text(queue->front());
            queue->pop();

            log::verbose("async", 2) << "write:" << text;

            os << text << std::endl;

            if (debug && process->get_frame() != nullptr)
            {
              WEX_POST(ID_DEBUG_STDIN, text, process->get_frame()->get_debug())
            }
          }
        }
      });
      u.detach();
```

```cpp
  std::from_chars (c++17)
  std::to_chars (c++17)
```

  This method can be used as replacement for e.g. stol.
  example used in textctrl-input.cpp:

```cpp
    for (const auto& v : m_values)
    {
      // If this value is an int, ignore value if we reached max
      if (int value = 0;
          std::from_chars(v.data(), v.data() + v.size(), value).ec ==
          std::errc())
      {
        if (current++ >= max_ints)
        {
          continue;
        }
      }

      filtered.emplace_back(v);
    }
```

```cpp
  std::optional (c++17)
```

  See e.g. chrono.h:

```cpp
  std::optional<time_t> get_time(const std::string& time) const;
```

```cpp
  std::to_underlying (c++23)
```

  Converts an enumeration to its underlying type.
  example in ex.cpp:

```cpp
    log::trace("ex mode from")
      << std::to_underlying(m_mode) << "to:" << std::to_underlying(mode);
```

## c++ language

- init_statement in if, case statements (c++17), and for range (c++20)
  ui/textctrl-input.cpp:

```cpp
      switch (const int page = 10; key)
```

  vi/command-ex.cpp:

```cpp
      if (const std::string line(it); !line.empty())
```

  ui/item.cpp

```cpp
    for (int item  = 0; const auto& b : std::any_cast<choices_t>(m_data.initial()))
```

- initializer_list (c++11)

- lambda expressions (c++11)
  e.g. useful to assign result of expression to a constant class
  member in a constructor.
  core/regex.cpp:

```cpp
wex::regex::regex(
  const std::vector<std::string>& regex,
  std::regex::flag_type           flags)
  : m_regex(
      [](const std::vector<std::string>& reg_str, std::regex::flag_type flags) {
        regex_t v;

        for (const auto& r : reg_str)
        {
          v.emplace_back(std::regex(r, flags), r);
        }

        return v;
      }(regex, flags))
{
}
```

- nested namespaces (c++17)

```cpp
  namespace wex::core
  {
    class stc;
  }

```

  as forward declaration

- override or final specifier (c++11)

  vi.h:

```cpp
    bool command(const std::string& command) final;
```

  this ensures that the function is kept in sync with base class

- spaceship operator (c++20)
  see presentation.h or block-lines.h

```cpp
  auto operator<=>(const block_lines& r) const
  {
    return m_start <=> r.m_start - 1;
  }
```
