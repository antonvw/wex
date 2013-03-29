@echo off
rem Name:      deploy.bat
rem Purpose:   Deploy file (for syncped)
rem Author:    Anton van Wezenbeek
rem Copyright: (c) 2013 Anton van Wezenbeek

rem Run this file in the build folder

mkdir syncped
mkdir syncped\fr-FR
mkdir syncped\nl-NL

rem Copy application.
copy vcmswu\syncped.exe syncped
copy vcmswu\syncped.exe.manifest syncped

rem Copy msvc DLL's
copy c:\windows\syswow64\msvcp110.dll syncped
copy c:\windows\syswow64\msvcr110.dll syncped

rem Copy templates and xml data.
copy ..\extension\data\*.txt syncped
copy ..\extension\data\*.xml syncped

rem Copy locale files.
"C:\Program Files (x86)\Poedit\bin\msgfmt.exe" c:\wxWidgets-2.9.5\misc\locale\nl.po -o syncped\nl-NL\nl.mo
"C:\Program Files (x86)\Poedit\bin\msgfmt.exe" c:\wxWidgets-2.9.5\misc\locale\fr.po -o syncped\fr-FR\fr.mo

"C:\Program Files (x86)\Poedit\bin\msgfmt.exe" ..\locale\wxextension-nl.po -o syncped\nl-NL\wxextension-nl.mo
"C:\Program Files (x86)\Poedit\bin\msgfmt.exe" ..\locale\wxextension-fr.po -o syncped\fr-FR\wxextension-fr.mo
"C:\Program Files (x86)\Poedit\bin\msgfmt.exe" ..\locale\syncped-nl.po -o syncped\nl-NL\syncped-nl.mo
"C:\Program Files (x86)\Poedit\bin\msgfmt.exe" ..\locale\syncped-fr.po -o syncped\fr-FR\syncped-fr.mo
  
7z a syncped.zip syncped\

move syncped.zip ..\..\syncped\bin

rmdir /S /Q syncped
