#-------------------------------------------------------------------------------------------------------
# Copyright (C) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
#-------------------------------------------------------------------------------------------------------

param (
    [Parameter(Mandatory=$True)]
    $commit,

    [Parameter(Mandatory=$True)]
    $branch,

    [switch]
    $core, # indicate whether this is for the core repo

    [string]$oauth = "\\chakrafs\fs\misc\oauth\token.txt"
)

$commitHash = $commit
$branchName = $branch
$arch = "x64"
$flavor = "debug"

. $PSScriptRoot\util.ps1
$gitExe = GetGitPath

$DropRoot = "\\chakrafs\fs\Builds\"
if ($core) {
    $DropRoot = Join-Path $DropRoot "ChakraCore"
} else {
    $DropRoot = Join-Path $DropRoot "ChakraFull"
}

$OuterScriptRoot = $PSScriptRoot # Used in pre_post_util.ps1
. "$PSScriptRoot\pre_post_util.ps1"

$BranchPath = $BranchName.replace('/','\')

#
# Get Build Info
#

$buildPushDate = $null
$buildPushIdString = $null

$info = GetBuildInfo $oauth $CommitHash
$Username = ($info.author.email).split('@')[0]
$_, $_, $_, $buildPushIdString = GetBuildPushId $info
$buildPushDate = [DateTime]$info.push.date

$PushDate = Get-Date $buildPushDate -Format yyMMdd.HHmm

# unless it is a build branch, subdivide the output directory by month
if ($BranchPath.StartsWith("build")) {
    $YearAndMonth = ""
} else {
    $YearAndMonth = (Get-Date $buildPushDate -Format yyMM) + "\"
}

$BuildIdentifier = "${buildPushIdString}_${PushDate}_${Username}_${CommitHash}"
$ComputedDropPathSegment = "${BranchPath}\${YearAndMonth}${BuildIdentifier}"

$DropPath = Join-Path $DropRoot $ComputedDropPathSegment
Write-Output $DropPath
