rem Name:      deploy.bat
rem Purpose:   Deploy file (for syncped)
rem Author:    Anton van Wezenbeek
rem Copyright: (c) 2011 Anton van Wezenbeek

rem Run this file in the build folder

mkdir syncped
mkdir syncped\nl-NL

rem Copy application.
copy vcmswu\syncped.exe syncped

rem Copy msvc DLL's
copy c:\windows\syswow64\msvcp100.dll syncped
copy c:\windows\syswow64\msvcr100.dll syncped

rem Copy data.
copy ..\extension\data\lexers.xml syncped
copy ..\extension\data\vcs.xml syncped

rem Copy locale files.
copy c:\wxWidgets-2.9.2\locale\nl.mo syncped\nl-NL\
copy ..\locale\wxextension-nl.mo syncped\nl-NL\
copy ..\locale\wxstd-xxx-nl.mo syncped\nl-NL\
copy ..\locale\syncped-nl.mo syncped\nl-NL\
 
7z a syncped.7z syncped\

move syncped.7z ..\..\syncped\bin

rmdir /S /Q syncped
