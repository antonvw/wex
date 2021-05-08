# roadmap

## backlog
- improve path, use less std::string
  - add const char* constructor
  - e.g. path::current
  - get_path -> get_parent
- more cpplint
  - config_items should not copy
  - change wxWindowID into window_id if possible
  - add more explicit constructors
- clang asan build using LeakSanitizer

- use spaceship operator
  and modules (wait for gcc-11)
- start up with recent project, close project
  -> windows appear
- update po files
- allow text entry validator for stc_entry_dialog, see macro mode
- del listview, stream when using queue, test case fails, as
  not all events are processed (move the queue from stream to del frame)
  use wxEvtHandler
- use boost JSON (requires boost 1.76)
