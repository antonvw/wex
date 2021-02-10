# wex library is written in the c++ language

It benefits from the following c++ features:

## c++ libraries

- Filesystem library (c++17)
```cpp
  std::filesystem
  std::filesystem::directory_iterator
  std::filesystem::recursive_directory_iterator
```

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
   std::vector<std::string> v;
   std::string              line(trim(cmdline));
   bool                     found = false;

   while (!line.empty())
   {
     ...
     // [option[=[value]] ...]
     else if (match("([a-z0-9]+)(=[a-z0-9]+)?(.*)", line, v) > 0)
```

  and match:
```cpp
    int wex::match(
      const std::string&        reg,
      const std::string&        text,
      std::vector<std::string>& v)
    {
      if (reg.empty())
        return -1;

      try
      {
        if (std::match_results<std::string::const_iterator> m;
            !std::regex_search(text, m, std::regex(reg)))
        {
          return -1;
        }
        else if (m.size() > 1)
        {
          v.clear();
          std::copy(++m.begin(), m.end(), std::back_inserter(v));
        }

        return v.size();
      }
      catch (std::regex_error& e)
      {
        log(e) << reg << "code:" << (int)e.code();
        return -1;
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

## c++ language

- init_statement in if and case statements (c++17)
  vi/command.cpp:
    ui/textctrl-input.cpp:
```cpp
      switch (const int page = 10; key)
```

- initializer_list (c++11)

- lambda expressions (c++11)

- nested namespaces (c++17)
