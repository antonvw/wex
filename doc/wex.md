# wex library is written in the c++ language

It benefits from the following c++ features:

## c++ libraries

- Algorithms library
```cpp
  std::all_of (c++11)
```

  As wex allows you to search in files or replace in files, this
  functionality is used.

  example:
```cpp
    int wex::dir::find_files()
    {
      int matches = 0;

      if (m_data.type().test(data::dir::RECURSIVE))
      {
        fs::recursive_directory_iterator rdi(
          m_dir.data(),
          fs::directory_options::skip_permission_denied),
          end;

        if (!std::all_of(rdi, end, [&](const fs::directory_entry& p) {
              return traverse(p, this, matches);
            }))
        {
          log("recursive_directory_iterator") << m_dir;
        }
      }
      else
      {
        fs::directory_iterator di(m_dir.data()), end;
        if (!std::all_of(di, end, [&](const fs::directory_entry& p) {
              return traverse(p, this, matches);
            }))
        {
          log("directory_iterator") << m_dir;
        }
      }
```

- Filesystem library (c++17)
```cpp
  std::filesystem
  std::filesystem::directory_iterator
  std::filesystem::recursive_directory_iterator
```

- Input/output library
```cpp
  std::fstream
```
  The base of all io uses a std::fstream class.

- Numerics library
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

- Regular expressions library (c++11)
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

- Strings library
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

  A recent added function.

- Thread support library (c++17)
```cpp
  std::thread
  std::future (together with boost) (c++11)
```
  See next.

- Utilities library
```cpp
  std::any (c++17)
```
  The std::any container is used as general container for
  gui elements.

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
  
## c++ language

- init_statement in if and case statements (c++17)
  vi/command.cpp:
    ui/textctrl-input.cpp:
```cpp
      switch (const int page = 10; key)
```

- initializer_list (c++11)

- lambda expressions (c++11)
  used of lof, e.g. useful to assign result of expression to a constant class
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

## boost c++ libraries
- boost::algorithm lib
  uses replace_all, to_upper, trim

- boost::json lib
  to implement wex::config

- boost::log lib
  to implement wex::log

- boost::process lib
  to implement wex::process

- boost::program_options lib
  to implement wex::cmdline

- boost::spirit lib
  to implement the wex::evaluator

- boost::statechart lib
  to implement the statemachine for vi mode and macro mode

- boost::tokenizer lib
  to tokenize expressions

## wxWidgets libraries
- all gui classes are derived from / use wxWidgets base classes
  wxbase
  wxcore
  wxaui
  wxhtml
  wxscintilla
  wxstc

## wex c++ libraries

wex-core <- wex-factory <- wex-data <- wex-common <- wex-ui <- wex-vi <- wex-stc <- wex-del
