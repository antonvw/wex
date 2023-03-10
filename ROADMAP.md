# roadmap

- linux print setup asserts
- c++20
  - use modules, add flag -fmodules
- c++23
  - use std::to_underlying instead of static_cast
  - use std::format (clang 15 not yet, gcc 12 not yet)
    in ex/util.cpp:
      if (flags.contains("#"))
      {
        std::format(text, "{:6} ", i + 1);
      }
