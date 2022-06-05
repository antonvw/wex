# changes

**v22.10**
- added blame revision
- ex commands more closely follow
  The Open Group Base Specifications Issue 7, 2018 edition
- ex command-line is improved, it is now a factory::stc

**v22.04** *March 6, 2022*
- Microsoft Visual Studio 2022
- boost::json lib instead of nlohmann/json lib, requiring boost 1.75
- added methods to allow testing wex applications
- c++20 standard
- added a few vim g commands
- added version info to wex libs

**v21.10** *September 12, 2021*
- boost::algorithm lib used
- boost::tokenizer lib used
- added wex::factory namespace, renamed wex::report namespace into wex::del
- added option wexBUILD_SHARED to use dynamic libs
- use std::thread for find and replace in files

**v21.04** *March 7, 2021*
- added FindWEX.cmake to assist using wex library using cmake projects
- boost::log lib instead of easylogging++ lib
- c++2a standard, c++17 available as tag
- support of ex mode handling of files
- moved apps folder to gitlab

**v20.10** *October 2, 2020*
- clang-format support, and wex code follows these guidelines
- ex append, insert, change, set commands follow
    The Open Group Base Specifications Issue 7, 2018 edition
- ctags update to use libreadtags and auto_complete improvements
- bettercodehub improvements, split up of src directories
- gtk3 is used as default widget toolkit
- added wex::data namespace

**v20.04** *March 15, 2020*
- scintilla is compiled to use `std::regex` (ECMAScript)
- The Open Group Base Specifications Issue 7, 2018 edition
- Microsoft Visual Studio 2019
- usage of test suites

**v19.10** *September 14, 2019*
- uses boost 1.65
  - boost::process lib instead of tiny-process-library
  - boost::program_options lib instead of TCLAP lib
  - boost::spirit lib instead of eval lib
  - boost::statechart lib instead of FSM lib
- uses nlohmann/json instead of wxConfig
- added wex::test and wex::report namespace

**v19.04** *March 9, 2019*
- added wex namespace
- uses std::filesystem instead of std::experimental::filesystem
- tiny-process-library lib

**v18.10** *October 1, 2018*
- c++17 standard
- wxWidgets used as submodule
- osx fixes

**v18.04** *April 8, 2018*
- easylogging++ lib
- FSM lib

**v17.10** *September 30, 2017*
- c++1z, using std::experimental::filesystem
- Microsoft Visual Studio 2017

**v17.04** *April 8, 2017*
- git submodules
- ctags lib
- doctest lib instead of catch lib
- pugixml lib
- TCLAP lib

**v16.10** *September 30, 2016*
- eval lib
- osx support

**v3.1.0** *March 11, 2016*
- c++14 standard
- wxWidgets 3.1.0
- catch lib instead of cppunit lib
- cmake build tool
- Microsoft Visual Studio 2015
- The Open Group Base Specifications Issue 7, 2013 edition

**v3.0.2** *October 10, 2014*
- c++11 standard
- wxWidgets 3.0.2

**v3.0.1** *June 19, 2014*
- wxWidgets 3.0.1
- Microsoft Visual Studio 2013

**v3.0.0** *November 14, 2013*
- cppunit lib
- wxWidgets 3.0.0
- Microsoft Visual Studio 2012
- sourceforge

**v2.9.5** *July 20, 2013*
- wxWidgets 2.9.5

**v2.9.4** *July 16, 2012*
- wxWidgets 2.9.4

**v2.9.3** *March 24, 2012*
- ex mode derived from vi mode
- wxWidgets 2.9.3
- rfw lexer

**v2.9.2** *December 16, 2011*
- use git (github)
- vi mode
- c++0x
- wxWidgets 2.9.2
- Microsoft Visual Studio 2010

**v6.0** *November 20, 2008*
- use svn (sliksvn, xp-dev)
- macro support added
- multi-threaded architecture
- doxygen
- otlv4.0 lib
- use bakefile

**v5.1** *September 10, 2007*
- recent project support

**v5.0** *April 4, 2007*
- wxAUI lib
- MDI projects

**v4.4** *December 14, 2006*
- portable version that reads and saves all config
    files from exe directory

**v4.3** *June 19, 2006*

**v4.2** *April 3, 2006*
- MDI editors

**v4.1** *February 6, 2006*
- hex mode opening

**v4.0** *December 23, 2005*
- STL container classes
    (ClassBuilder no longer used)
- MDI interface
- binary files
- printing
- added Open Link
- drag/drop
- autocompletion
- folding

**v3.6** *July 9, 2003*
- wxStyledTextCtrl component used
- gcc 3.2

**v3.5** *March 22, 2002*

**v3.4** *February 21, 2002*

**v3.3** *November 30, 2001*

**v3.2** *July 11, 2001*

**v3.1** *June 22, 2001*
- wxWidgets 2.2.7

**v3.0** *June 12, 2001*
- wxWidgets 2.2.1
- recent file support
- ported to Unix

**v2.2** *December 18, 2000*
- XTToolKit lib
- drag/drop support

**v2.1** *November 14, 2000*
- CodeJock lib v6.09

**v2.0** *June 19, 2000*
- Visual C++ 6.0 and MFC 6.0

**v1.9** *April 18, 2000*

**v1.8** *January 7, 2000*

**v1.7** *December 15, 1999*
- ClassBuilder 2.0

**v1.6** *October 12, 1999*
- SDI interface

**v1.5** *March 23, 1999*

**v1.4** *August 8, 1998*

**v1.3** *June 18, 1998*

**v1.2** *April 23, 1998*
- use CListCtrl instead of CListBox

**v1.1** *March 16, 1998*
- use CListBox instead of CEdit

**v1.0** *January 27, 1998*
- Visual C++ 5.0 and MFC 5.0
- CEdit is used as base for the output
