# roadmap

## feature/clang
- clang 11
- clang-format only if clang available

## feature/vcs
- improve blame size calculation, now only uses size of first blame line,
  should be more, or all
  improve align_text (see e.g. blame annotations)
- git blame margin default not wide enough (MSW)
- allow checkout by showing other branches, and remove from menu

## backlog
- use abstract factory design pattern
- reenable ubuntu test on travis (after ubuntu 20.04)
- less platform dependant code
- invest use wxTextCompleter
- start up with recent project, close project
  -> windows appear
- each process separate stc
  - process shell and aui
