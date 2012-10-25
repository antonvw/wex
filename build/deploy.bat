@echo off
rem Name:      deploy.bat
rem Purpose:   Deploy file (for syncped)
rem Author:    Anton van Wezenbeek
rem Copyright: (c) 2012 Anton van Wezenbeek

rem Run this file in the build folder

mkdir syncped
mkdir syncped\fr_FR
mkdir syncped\nl_NL

rem Copy application.
copy vcmswu\syncped.exe syncped
copy syncped.exe.manifest syncped

rem Copy msvc DLL's
copy c:\windows\syswow64\msvcp110.dll syncped
copy c:\windows\syswow64\msvcr110.dll syncped

rem Copy templates and xml data.
copy ..\extension\data\*.txt syncped
copy ..\extension\data\*.xml syncped

rem Copy locale files.
msgftm c:\wxWidgets-2.9.5\locale\fr.mo -o syncped\fr_FR\fr.po
msgftm c:\wxWidgets-2.9.5\locale\nl.mo -o syncped\nl_NL\nl.po

for %%X in (..\locale\*fr.mo) do (
  msgfmt %%X syncped\fr_FR\%%X.po)
  
for %%X in (..\locale\*nl.mo) do (
  msgfmt %%X syncped\fr_NL\%%X.po)
  
7z a syncped.zip syncped\

move syncped.zip ..\..\syncped\bin

rmdir /S /Q syncped
