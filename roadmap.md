# feature/ex
- the :a, :i, :c ex commands do not follow ex opengroup specs
  e.g. :a xxx should give an error, and . on empty line should end append
- so the tests are not ok as well, first fix them

# feature/align
- improve align_text (see e.g. blame annotations)

# feature/calc
- command_calc command_reg can be combined
- ex_command should have a prefix, instead of textctrl in managedframe
- test in vi::calc and ex_command, managed_frame

# feature/blame
- improve blame size calculation, now only uses size of first blame line,
  should be more, or all
  
# feature/link
- find_between be configurable

# feature/git
- allow checkout by showing other branches, and remove from menu

# feature/tests
- fix failing tests

# feature/c++20
- c++latest gives error on visual studio
- wxWidgets submodule is not same as develop
- fix [=](wxCommandEvent& event) should be [=, this](wxCommandEvent& event)
  (use CMAKE_CXX_STANDARD)
- use clang-9 (llvm@9, first on MacBook) (after macOS 10.15 deployed on travis)
- use gcc-9 (check on linux) and c++20
  - enable lcov
  - use :1 bit for several bools
  - wxDateTime replace by c++20 methods date and time utilities

# backlog
- add missing dll vcruntime140_1.dll (appveyor) 
- bug :prev or :n from stdin
- add use namespace wex
- use configurable toobar from xml
- start up with recent project, close project
  -> windows appear
- each process separate stc
  - process shell and aui
- invest using RFW spaced keywords
