@echo off
:: Batch file for building/testing wex
:: we are in the build directory
:: Should use msbuild

set BOOST=83
set YEAR=2022
set VS=C:\Program Files\Microsoft Visual Studio

call "%VS%\%YEAR%\Community\VC\Auxiliary\Build\vcvars32.bat"

:: cmake it
cmake ^
  -DBOOST_ROOT=C:\Libraries\boost_1_%BOOST%_0 ^
  -DCMAKE_BUILD_TYPE=%configuration% ^
  -DwexBUILD_TESTS=ON ^
  -DwexENABLE_GETTEXT=ON ..

:: build it
echo %configuration%
devenv wex.sln /build %configuration%

:: test it
::ctest -C %configuration% -VV -R wex-test-core

:: install it
:: does not work with Debug, but Release gives invalid return after ui test
:: ..\ci\pack.bat
:: As administrator:
:: cmake.exe -P cmake_install.cmake

:: build syncped
:: and in syncped do a build
