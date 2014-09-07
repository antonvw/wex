echo off
rem Name:      config.cmd
rem Purpose:   Batch file to set variables
rem Author:    Anton van Wezenbeek
rem Copyright: (c) 2014 Anton van Wezenbeek

for /f %%g in ('dir c:\wxWidgets-3.0.* /b') do set WXWIN=c:\%%g
