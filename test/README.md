# lib tests

This directory contains the tests for the wex libraries.

## Organization

- the directories reflect the wex libraries tests

some functions are not tested:

- print()
  waits for input

- print_preview()
  for grid, listview, stc
  this shows a dialog, not correctly destroyed when application exits.

## Each component in a subdir wil have get a separate binary, except for data.
