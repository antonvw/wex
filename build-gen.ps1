################################################################################
# Name:      build-gen.ps1
# Purpose:   Generates build files for windows
#            for building wex itself, or for building apps using it
#            Just run from repo root
# Author:    Anton van Wezenbeek
# Copyright: (c) 2024 Anton van Wezenbeek
################################################################################

<#
.Description
  This is a script file to build wex or apps using it.
.PARAMETER boost_dir
  The base dir where boost libraries should be present.
  The default value is c:\libraries
.PARAMETER configuration
  The kind of build. Valid values are Release and Debug.
.PARAMETER dir
  The directory where cmake files are generated.
.PARAMETER help
  Shows help and exits.
.PARAMETER install
  Installs wex.
.PARAMETER mingw
  Use MinGW makefiles.
.PARAMETER prepare
  Prepares cmake files only, does not build.
.PARAMETER samples
  Builds samples as well.
.PARAMETER shared
  Makes a shared build, using dll libraries.
.PARAMETER tests
  Builds test binaries as well.
#>
param (
    [Parameter()]
    [string]$boost_dir = "c:\libraries",
    [string]$configuration = "Release",
    [string]$dir = "build",
    [switch]$help = $false,
    [switch]$install = $false,
    [switch]$mingw = $false,
    [switch]$prepare = $false,
    [switch]$samples = $false,
    [switch]$shared = $false,
    [switch]$tests = $false
 )

if ($help)
{
  get-help .\build-gen.ps1 -full
  Exit
}

if ( -not (Test-Path -Path $boost_dir -PathType Container) )
{
  Write-Host "Boost dir:" $boost_dir "does not exist"
  Exit
}

$boost_names = @((Get-ChildItem -Path $boost_dir -Filter "*85*" -Directory).Fullname)

if ( -not ($boost_names))
{
  Write-Host "No boost libraries found in:" $boost_dir
  Exit
}

Write-Host "found boost libraries:" $boost_names[-1]

$option_mingw=
$option_shared=
$option_tests=

if ($mingw)
{
  $option_mingw="-G MinGW Makefiles"
}

if ($samples)
{
  $option_samples="-DwexBUILD_SAMPLES=ON"
}

if ($shared)
{
  $option_shared="-DwexBUILD_SHARED=ON"
}

if ($tests)
{
  $option_tests="-DwexBUILD_TESTS=ON"
}

mkdir -p $dir

cmake `
  -B $dir `
  -DBOOST_ROOT=$boost_names `
  -DCMAKE_BUILD_TYPE=$configuration `
  -DCMAKE_INSTALL_PREFIX="c:\program files (x86)\wex" `
  $option_mingw `
  $option_samples `
  $option_shared `
  $option_tests

if ( -not ($prepare))
{
  cd $dir
  msbuild /noLogo /m /p:Configuration=$configuration ALL_BUILD.vcxproj
}

if ($install)
{
  # As administrator
  cmake -P cmake_install.cmake
}
