# feature/stc
- improve align_text (see e.g. blame annotations)
- add stc get_text, and use it instead of GetText (159 times)

# feature/blame
- improve blame size calculation, now only uses size of first blame line,
  should be more, or all
  
# feature/git
- allow checkout by showing other branches, and remove from menu

# feature/tests
- reenable test on travis (first osx)
- enable test on appveyor

# feature/c++20
- no longer use StartsWidth (after new develop merge) or find() == 0
  as starts_with
- c++latest gives error on visual studio
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
