@echo off
rem Name:      deploy.bat
rem Purpose:   Deploy file (for syncped)
rem Author:    Anton van Wezenbeek
rem Copyright: (c) 2014 Anton van Wezenbeek

rem Run this file in the build folder

call config.cmd

mkdir syncped
mkdir syncped\fr_FR
mkdir syncped\nl_NL

rem Copy application.
copy vcmswu\syncped.exe syncped
copy vcmswu\syncped.exe.manifest syncped

rem Copy msvc DLL's
copy %WINDIR%\syswow64\msvcp120.dll syncped
copy %WINDIR%\syswow64\msvcr120.dll syncped

rem Copy templates and xml data.
copy ..\extension\data\*.txt syncped
copy ..\extension\data\*.xml syncped

rem Copy locale files.
"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" %WXWIN%\locale\nl.po -o syncped\nl_NL\nl.mo
"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" %WXWIN%\locale\fr.po -o syncped\fr_FR\fr.mo

"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" ..\locale\wxextension-nl.po -o syncped\nl_NL\wxextension-nl.mo
"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" ..\locale\wxextension-fr.po -o syncped\fr_FR\wxextension-fr.mo
"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" ..\locale\syncped-nl.po -o syncped\nl_NL\syncped-nl.mo
"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" ..\locale\syncped-fr.po -o syncped\fr_FR\syncped-fr.mo
  
7z a syncped-v%WXWIN:~-5%.zip syncped\

rmdir /S /Q syncped
