#-------------------------------------------------------------------------------------------------------
# Copyright (C) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
#-------------------------------------------------------------------------------------------------------

# Build Summary Script
#
# For convenience, display the build errors and test logs summaries.
# If these files are non-empty then it indicates a failure in the build and it is helpful to view
# a summary of these logs in the build output.

param (
    [Parameter(Mandatory=$True)]
    [ValidateSet("x86", "x64", "arm")]
    [string]$arch,

    [Parameter(Mandatory=$True)]
    [ValidateSet("debug", "release", "test", "codecoverage")]
    [string]$flavor,

    [ValidateSet("default", "codecoverage", "pogo")]
    [string]$subtype = "default",
    
    [Parameter(Mandatory=$True)]
    [string]$dropPath
)

$global:exitcode = 0

$OuterScriptRoot = $PSScriptRoot
. "$PSScriptRoot\pre_post_util.ps1"

$buildName = ConstructBuildName -arch $arch -flavor $flavor -subtype $subtype

$buildErrFile = "${dropPath}\buildlogs\build.${buildName}.err"
$testSummaryFile = "${dropPath}\testlogs\summary.${arch}${flavor}.log"

$buildFailed = (Test-Path $buildErrFile) -and ((Get-Content $buildErrFile).Length -ne 0)
$testsFailed = (Test-Path $testSummaryFile) -and ((Get-Content $testSummaryFile).Length -ne 0)

Write-Output "------------------------------------------------------------"
Write-Output "Displaying build failure summary:"
if ($buildFailed) {
    Get-Content $buildErrFile | % { $host.ui.WriteErrorLine($_) }
}

Write-Output "------------------------------------------------------------"
Write-Output "Displaying test failure summary:"
if ($testsFailed) {
    Get-Content $testSummaryFile | % { $host.ui.WriteErrorLine($_) }
}

Write-Output "------------------------------------------------------------"
if ($buildFailed -or $testsFailed) {
    $host.ui.WriteErrorLine("Build failed. See logs for more info.")
    $global:exitcode = 1
}

exit $global:exitcode
