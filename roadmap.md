# feature/state-ex
- in ex mode left and right do not work initially (MSW, GTK3)
  /bin/git grep -n xxx -- "*.adb" does not work (GTK3)
  this is related to editing in text field, not ok
- set ts ,etc. ? should return value
- for boolean options:
  - set <>    sets option
  - set no <> 
- super linter
- visual block -> insert -> escape -> not visual block??

# feature/vcs
- improve blame size calculation, now only uses size of first blame line,
  should be more, or all
  improve align_text (see e.g. blame annotations)
- git blame margin default not wide enough (MSW)
- allow checkout by showing other branches, and remove from menu

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
- use abstract factory design pattern
- reenable ubuntu test on travis (after ubuntu 20.04)
- less platform dependant code
- invest use wxTextCompleter
- use configurable toolbar from xml
- start up with recent project, close project
  -> windows appear
- each process separate stc
  - process shell and aui
- invest using RFW spaced keywords, update rfw FOR keywords
