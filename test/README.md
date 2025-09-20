# lib tests

This directory contains the tests for the wex libraries.

## Purpose

- Achieve a high test coverage, when adding functionality,
  add a test for it. syncped can help in checking
  coverage by using the @Coverage@ macro by opening the cpp and h
  file, and running the macro on the h file.

## Organization

- The subdirectories reflect the wex libraries

- except for app, that executes integration tests
  using robotframework on the wex-sample

some functions are not tested:

- print()
  waits for input

- print_preview()
  for grid, listview, stc
  this shows a dialog, not correctly destroyed when application exits.

## Each component (except app)

- has a main.cpp, that either directly calls wex::test::app
  or an instance of a class derived from it

- has a separate binary that executes unit tests using catch2
