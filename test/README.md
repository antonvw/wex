# lib tests

This directory contains the tests for the wex libraries.

## Organization

- the test directories reflect the wex libraries

some functions are not tested:

- print()
  waits for input

- print_preview()
  for grid, listview, stc
  this shows a dialog, not correctly destroyed when application exits.

## Components

- Each lib test component in a subdir

  - has a main.cpp, that either directly calls wex::test::app
    or an instance of a class derived from it

  - has a separate binary that executes unit tests using doctest
  
  - except for app, that executes integration tests
    using robotframework on the wex-sample
