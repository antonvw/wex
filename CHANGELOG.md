# changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## 24.10 - 2024-10-01 [Unreleased]

### Added

- added msw build script build-gen.ps1 for building wex on windows
- gersemi support, and wex cmake code follows these guidelines
- right click on File Type enables or disables showing whitespace
- Go lexer
- a WIN_SINGLE_LINE to data::stc, used in e.g. stc_entry_dialog
- factory::vcs and support exclude find in git submodules

### Changed

- upgrade to lexilla 5.3.1
- updated art default bitmap colour
- hexmode insert or erase keeps position
- clang-tidy updated
- renamed FindWEX into Findwex and supports adding a version
- used wrapline none for rfw lexer
- moved vcs_admin to factory lib
- renamed frame::is_address into vi_is_address and frame::exec_ex_command into
  vi_exec_command

### Fixed

- ex mode find and getting previous line
- any file used for config uses json lexer
- fixed a possible filename too long exception when trying opening links
- show error if no macros present for a lexer
- improved cleaning when history view is closed
- register calculator used in insert mode
- reporting matches when doing replace in files
- handling shift home, end key in command-line
- initializing item combobox with empty value

## 24.04 - 2024-03-29

### Added

- clang-tidy support
- added osx, linux build script build-gen.sh for building wex or
  for building apps using it (installed as wex-build-gen.sh)
- the build tool now supports ninja besides make
- added class function_repeat to offer syncing without idle events
- added class reflection to add reflection to classes
- added wxMaterialDesignArtProvider, and renamed wex::stockart into wex::art.

### Changed

- moved single_choice_dialog to syntax lib, use data::window parameter
- ex options ignorecase and matchword are kept in config
- std::optional is used to return values at several places
- use more enum classes

### Fixed

- sync_close_all now correctly handled
- syntax on now restores previous syntax
- case insensitive find in files
- auto_complete handling
- history clear fixed
- statusbar setup after showing dialog fixed
- vi selecting using arrow keys while shift or control down pressed
- switching between find in stc margin and normal window
- ex mode fixes for printing, adjust_window

## 23.10 - 2023-10-08

### Added

- wex-test lib
- python bindings using swig

### Changed

- boost::regular expression lib used by wex::regex_part
- boost::URL lib used, requiring boost 1.81
- frame::open_file_same_page: argument now takes wex::path
- item::layout: argument now takes wex::data::layout
- stc::vi_command: argument now takes wex::line_data
- removed some not necessary copy constructors
- moved window.h and control.h to factory
- lilypond lexer improved
- added wex::ex_stream::stream argument line size and increased default value

### Fixed

- possible crash in lisview sorting
- ctags::find and empty tag finds next tag
- rectangular paste

## 23.04 - 2023-03-12

### Added

- wex-ctags and wex-syntax library

### Changed

- c++23 standard
- wxWidgets 3.3, and lexilla 5.0.1

### Fixed

- ex mode fixes

## 22.10 - 2022-09-18

### Added

- blame revision
- ex command-line is improved, it is now a factory::stc
- wex-ex and wex-vcs library

### Changed

- wxWidgets 3.2
- ex commands more closely follow
  The Open Group Base Specifications Issue 7, 2018 edition

## 22.04 - 2022-03-06

### Added

- methods to allow testing wex applications
- a few vim g commands
- version info to wex libs

### Changed

- c++20 standard
- boost::json lib instead of nlohmann/json lib, requiring boost 1.75
- Microsoft Visual Studio 2022

## 21.10 - 2021-09-12

### Added

- wex::factory namespace, renamed wex::report namespace into wex::del
- option wexBUILD_SHARED to use dynamic libs

### Changed

- boost::algorithm lib used
- boost::tokenizer lib used
- use std::thread for find and replace in files

## 21.04 - 2021-03-07

### Added

- FindWEX.cmake to assist using wex library using cmake projects
- support of ex mode handling of files

### Changed

- c++2a standard, c++17 available as tag
- boost::log lib used instead of easylogging++ lib
- moved apps folder to gitlab

## 20.10 - 2020-10-02

### Added

- clang-format support, and wex code follows these guidelines
- ctags update to use libreadtags and auto_complete improvements
- bettercodehub improvements, split up of src directories
- gtk3 is used as default widget toolkit
- wex::data namespace

### Changed

- ex append, insert, change, set commands follow
    The Open Group Base Specifications Issue 7, 2018 edition

## 20.04 - 2020-03-15

### Added

- usage of test suites

### Changed

- The Open Group Base Specifications Issue 7, 2018 edition
- scintilla is compiled to use `std::regex` (ECMAScript)
- Microsoft Visual Studio 2019

## 19.10 - 2019-09-14

### Added

- wex::test and wex::report namespace

### Changed

- uses nlohmann/json instead of wxConfig
- uses boost 1.65
  - boost::process lib instead of tiny-process-library
  - boost::program_options lib instead of TCLAP lib
  - boost::spirit lib instead of eval lib
  - boost::statechart lib instead of FSM lib

## 19.04 - 2019-03-09

### Added

- wex namespace
- tiny-process-library lib

### Changed

- uses std::filesystem instead of std::experimental::filesystem

## 18.10 - 2018-10-01

### Changed

- wxWidgets used as submodule

### Fixed

- osx fixes

### Changed

- c++17 standard

## 18.04 - 2018-04-08

### Added

- easylogging++ lib
- FSM lib

## 17.10 - 2017-09-30

### Changed

- c++1z, using std::experimental::filesystem
- Microsoft Visual Studio 2017

## 17.04 - 2017-04-08

### Changed

- git submodules
- ctags lib
- doctest lib instead of catch lib
- pugixml lib
- TCLAP lib

## 16.10 - 2016-09-30

### Added

- eval lib
- osx support

## 3.1.0 - 2016-03-11

### Added

- The Open Group Base Specifications Issue 7, 2013 edition

### Changed

- catch lib instead of cppunit lib
- cmake build tool
- Microsoft Visual Studio 2015

### Changed

- c++14 standard
- wxWidgets 3.1.0

## 3.0.2 - 2014-10-10

### Changed

- c++11 standard
- wxWidgets 3.0.2

## 3.0.1 - 2010-06-19

### Changed

- wxWidgets 3.0.1
- Microsoft Visual Studio 2013

## 3.0.0 - 2013-11-14

### Added

- unittests using cppunit lib
- sourceforge

### Changed

- wxWidgets 3.0.0
- Microsoft Visual Studio 2012

## 2.9.5 - 2013-07-20

### Changed

- wxWidgets 2.9.5

## 2.9.4 - 2012-07-16

### Changed

- wxWidgets 2.9.4

## 2.9.3 - 2012-03-24

### Added

- ex mode derived from vi mode
- rfw lexer

### Changed

- wxWidgets 2.9.3

## 2.9.2 - 2011-12-11

### Added

- vi mode

### Changed

- use git (GitHub)

### Changed

- c++0x standard
- wxWidgets 2.9.2
- Microsoft Visual Studio 2010

## 6.0 - 2008-11-20

### Added

- macro support added
- multi-threaded architecture
- doxygen
- otlv4.0 lib
- use bakefile

### Changed

- use svn (sliksvn, xp-dev)

## 5.1 - 2007-09-10

### Added

- recent project support

## 5.0 - 2007-04-04

### Added

- wxAUI lib
- MDI projects

## 4.4 - 2006-12-14

### Added

- portable version that reads and saves all config
    files from exe directory

## 4.3 - 2006-06-19

## 4.2 - 2006-04-03

### Added

- MDI editors

## 4.1 - 2006-02-06

### Added

- hex mode opening

## 4.0 - 2005-12-23

### Added

- STL container classes
    (ClassBuilder no longer used)
- MDI interface
- binary files
- printing
- added Open Link
- drag/drop
- autocompletion
- folding

## 3.6 - 2003-07-09

### Added

- wxStyledTextCtrl component used

### Changed

- gcc 3.2

## 3.5 - 2002-03-22

## 3.4 - 2002-02-21

## 3.3 - 2001-11-30

## 3.2 - 2001-07-11

## 3.1 - 2001-06-22

### Changed

- wxWidgets 2.2.7

## 3.0 - 2001-06-12

### Added

- recent file support
- ported to Unix

### Changed

- wxWidgets 2.2.1 instead of MFC

## 2.2 - 2002-12-18

### Added

- drag/drop support

### Changed

- XTToolKit lib

## 2.1 - 2000-11-14

### Added

- CodeJock lib v6.09

## 2.0 - 2000-06-19

### Changed

- MFC 6.0
- Visual C++ 6.0

## 1.9 - 2000-04-18

## 1.8 - 2000-01-07

## 1.7 - 1999-09-15

### Added

- ClassBuilder 2.0

## 1.6 - 1999-10-12

### Added

- SDI interface

## 1.5 - 1999-03-23

## 1.4 - 1998-08-08

## 1.3 - 1998-06-18

## 1.2 - 1998-04-23

### Changed

- use CListCtrl instead of CListBox

## 1.1 - 1998-03-16

### Changed

- use CListBox instead of CEdit

## 1.0 - 1998-01-27

### Added

- MFC 5.0
- CEdit is used as base for the output
- Visual C++ 5.0

<!-- markdownlint-configure-file { "MD022": false,
  "MD024": false, "MD030": false, "MD032": false} -->
