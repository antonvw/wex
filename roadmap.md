# feature/platform
- enlarge font button
- in ex mode left and right do not work initially (GTK3)
- /bin/git grep -n xxx -- "*.adb" does not work (GTK3)
  this is related to editing in text field, not ok
- git blame margin default not wide enough (MSW)
- add manifest file (MSW)
- add missing dll vcruntime140_1.dll (appveyor) (MSW)
- enable test on appveyor
- upgrade and fix wxWidgets

# feature/vcs
- improve blame size calculation, now only uses size of first blame line,
  should be more, or all
  improve align_text (see e.g. blame annotations)
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
- invest use wxTextCompleter
- bug :prev or :n from stdin
- use configurable toobar from xml
- start up with recent project, close project
  -> windows appear
- each process separate stc
  - process shell and aui
- invest using RFW spaced keywords, update rfw FOR keywords
