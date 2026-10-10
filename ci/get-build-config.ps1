################################################################################
# Name:      get-build-config.ps1
# Purpose:   Writes the build config according to current git branch
# Author:    Anton van Wezenbeek
# Copyright: (c) 2025-2026 Anton van Wezenbeek
################################################################################

$branch = git rev-parse --abbrev-ref HEAD

if (($branch -eq "main") -or ($branch -eq "develop" ))
{
    Write-Output "Release"
}
else
{
    Write-Output "Debug"
}
