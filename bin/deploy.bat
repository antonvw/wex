@echo off
rem Name:      deploy.bat
rem Purpose:   Deploy file (for syncped)
rem Author:    Anton van Wezenbeek
rem Copyright: (c) 2015 Anton van Wezenbeek

for /f %%g in ('dir c:\wxWidgets-3.1.* /b') do set WXWIN=c:\%%g

mkdir app
mkdir app\fr_FR
mkdir app\nl_NL

rem Copy application.
copy syncped.exe app
copy syncped.exe.manifest app

rem Copy msvc DLL's
copy %WINDIR%\syswow64\msvcp120.dll app
copy %WINDIR%\syswow64\msvcr120.dll app

rem Copy templates and xml data.
copy ..\..\extension\data\*.txt app
copy ..\..\extension\data\*.xml app
copy ..\..\extension\data\*.xsl app

rem Copy locale files.
"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" %WXWIN%\locale\nl.po -o app\nl_NL\nl.mo
"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" %WXWIN%\locale\fr.po -o app\fr_FR\fr.mo

"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" ..\..\locale\wxextension-nl.po -o app\nl_NL\wxextension-nl.mo
"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" ..\..\locale\wxextension-fr.po -o app\fr_FR\wxextension-fr.mo
"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" ..\..\locale\syncped-nl.po -o app\nl_NL\syncped-nl.mo
"%PROGRAMFILES(X86)%\GnuWin32\bin\msgfmt.exe" ..\..\locale\syncped-fr.po -o app\fr_FR\syncped-fr.mo
  
7z a syncped-v%WXWIN:~-5%.zip .\app\*

rmdir /S /Q app
