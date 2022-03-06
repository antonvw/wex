# roadmap
- /blame/,/this/y

- menus is a template class, is this really necessary
- add wex::accumulate
  - see also cmdline for find_if template
- add version to static libs as well
- use boost python for scripting
- current stc uses old scintilla
  - does not support json bool, see config.cpp
- linux print setup asserts
- c++ modules: fix gcc (gcc-12), test msvc
- c++23 
  - use std::to_underlying instead of static_cast
  - use string::contains at various places instead of find != npos
- is the define BOOST_ASIO_HAS_STD_INVOKE_RESULT ON necessary?
  - next boost version (> 1.76)
