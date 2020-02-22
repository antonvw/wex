# feature/fixes
- test macro

# feature/accelerator
- add wex::accelerators as wxAcceleratorTable

# feature/align
- improve align_text (see e.g. blame annotations)

# feature/aui
- the wxAuiManager should be private for managed_frame

# feature/calc
- command_calc commnd_reg can be combined
- ex_command should have a prefix, instead of textctrl in managedframe

# feature/c++20
- use c++2a, fixed warnings and use ends_with
  (use CMAKE_CXX_STANDARD)
- use clang-9 (llvm@9, first on MacBook) (after macOS 10.15 deployed on travis)
- use gcc-9 (check on linux) and c++20
  - enable lcov
  - use :1 bit for several bools
  - wxDateTime replace by c++20 methods date and time utilities

# backlog
- add use namespace wex
- start up with recent project, close project
  -> windows appear
- each process separate stc
  - process shell and aui
- invest using RFW spaced keywords
