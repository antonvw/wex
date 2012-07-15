@echo off
rem Name:      deploy.bat
rem Purpose:   Deploy file (for syncped)
rem Author:    Anton van Wezenbeek
rem Copyright: (c) 2012 Anton van Wezenbeek

rem Run this file in the build folder

mkdir syncped
mkdir syncped\fr
mkdir syncped\nl_NL

rem Copy application.
copy vcmswu\syncped.exe syncped
copy syncped.exe.manifest syncped

rem Copy msvc DLL's
copy c:\windows\syswow64\msvcp100.dll syncped
copy c:\windows\syswow64\msvcr100.dll syncped

rem Copy xml data.
copy ..\extension\data\*.xml syncped

rem Copy locale files.
copy c:\wxWidgets-2.9.4\locale\fr.mo syncped\fr\
copy c:\wxWidgets-2.9.4\locale\nl.mo syncped\nl_NL\
copy ..\locale\*fr.mo syncped\fr\
copy ..\locale\*nl.mo syncped\nl_NL\
 
7z a syncped.zip syncped\

move syncped.zip ..\..\syncped\bin

rmdir /S /Q syncped
