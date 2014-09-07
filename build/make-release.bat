echo off
rem Name:      make-release.bat
rem Purpose:   Batch file to run nmake to make a release version
rem Author:    Anton van Wezenbeek
rem Copyright: (c) 2014 Anton van Wezenbeek

call config.cmd

nmake -f makefile-release.vc
