# feature/c++20

- c++latest gives error on visual studio
- fix [=](wxCommandEvent& event) should be [=, this](wxCommandEvent& event)
  (use CMAKE_CXX_STANDARD)
- after new merge, update Standard in .clang-format
- use clang-9 (llvm@9, first on MacBook) (after macOS 10.15 deployed on travis)
- use gcc-9 (check on linux) and c++20
  - enable lcov
  - use :1 bit for several bools
  - wxDateTime replace by c++20 methods date and time utilities
