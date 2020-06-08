This directory contains the tests for the wex libraries.

Most code is tested, except for:

- print()
  waits for input
  
- print_preview()
  for grid, listview, stc
  this shows a dialog, not correctly destroyed when application exits.
