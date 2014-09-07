echo off
rem Name:      make.bat
rem Purpose:   Batch file to run nmake
rem Author:    Anton van Wezenbeek
rem Copyright: (c) 2014 Anton van Wezenbeek

call config.cmd

nmake -f makefile.vc
