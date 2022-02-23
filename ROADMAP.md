# roadmap
- linux print setup asserts

## backlog
- version 22.10
- add undo_action class, to implement Begin and EndUndoAction
- set_indicator does not need parameter 2, 3
- del::frame reorder API
- improve doc for is_address
- add spaceship compare to wex::tool, use in del::test-stream.cpp
- stream.h API is not consistent, do not use get_
- add a blame for previous version, as in github
- process-imp should test whether event handler is ok, see test-process.
- is the define BOOST_ASIO_HAS_STD_INVOKE_RESULT ON necessary?
- menus is a template class, is this really necessary
- REPORT_FIRST, REPORT_LAST no longer necessary
- add wex::accumulate
- codiga improvements
- use boost python for scripting
- current stc uses old scintilla
  - does not support json bool, see config.cpp
- c++ modules: fix gcc (gcc-12), test msvc
- c++23 
  - use std::to_underlying instead of static_cast
  - use string::contains at various places instead of find != npos
