#-------------------------------------------------------------------------------------------------------
# Copyright (C) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
#-------------------------------------------------------------------------------------------------------

# Init Build Script
#
# Run this as the very first step in the build to configure the environment.
# This is distinct from the Pre-Build script as there may be more non-script steps that need to be
# taken before setting up and running the build.
# For example, this script creates a cmd script which should be run to initialize environment variables
# before running the Pre-Build script.

param (
    [ValidateSet("x86", "x64", "arm", "")]
    [string]$arch = "",
    [ValidateSet("debug", "release", "test", "codecoverage", "")]
    [string]$flavor = "",
    [ValidateSet("default", "codecoverage", "pogo")]
    [string]$subtype = "default",
    [string]$buildtype,

    [string]$envConfigScript = "ComputedEnvironment.cmd",

    [string[]]$supportedPogoBuildTypes = @("x64_release", "x86_release"),

    [string]$verMajor = "",
    [string]$verMinor = "",
    [string]$verPatch = "",
    [string]$verSecurity = "",

    [string]$dropRoot,

    [switch]$cleanBinDir,

    [string]$oauth
)

#
# Define values for variables based on parameters and environment variables
# with default values in case the environment variables are not defined.
#

. $PSScriptRoot\util.ps1
$gitExe = GetGitPath

$BuildType = UseValueOrDefault $buildtype $Env:BuildType
$BuildPlatform = UseValueOrDefault $arch $Env:BuildPlatform
$BuildConfiguration = UseValueOrDefault $flavor $Env:BuildConfiguration
$BuildSubtype = UseValueOrDefault $subtype $Env:BuildSubtype

# If $BuildType is specified, extract BuildPlatform and BuildConfiguration
# Otherwise, if $BuildPlatform and $BuildConfiguration are specified, construct $BuildType
# $BuildSubtype will remain as "default" if not already specified, or become e.g. "pogo", "codecoverage"
if ($BuildType) {
    $buildTypeSegments = $BuildType.split("_")
    $BuildPlatform = $buildTypeSegments[0]
    $BuildConfiguration = $buildTypeSegments[1]
    if ($buildTypeSegments[2]) {
        # overwrite with new value if it exists, otherwise keep as "default"
        $BuildSubtype = $buildTypeSegments[2]
    }

    if ($BuildConfiguration -eq "codecoverage") {
        $BuildConfiguration = "test" # codecoverage builds are actually "test" configuration
        $BuildSubtype = "codecoverage" # keep information about codecoverage in the subtype
    }

    if (-not ($BuildSubtype -in @("default", "pogo", "codecoverage"))) {
        Write-Error "Unsupported BuildSubtype: $BuildSubtype"
    }
} elseif ($BuildPlatform -and $BuildConfiguration) {
    $BuildType = "${BuildPlatform}_${BuildConfiguration}"
} else {
    Write-Error (@"

    Not enough information about BuildType:
        BuildType={0}
        BuildPlatform={1}
        BuildConfiguration={2}
        BuildSubtype={3}

"@ -f $BuildType, $BuildPlatform, $BuildConfiguration, $BuildSubtype)

    exit 1
}

$CommitHash = UseValueOrDefault $Env:BUILD_SOURCEVERSION $(iex "${gitExe} rev-parse HEAD")

$branchFullName  = UseValueOrDefault $Env:BUILD_SOURCEBRANCH $(iex "${gitExe} rev-parse --symbolic-full-name HEAD")

$SourcesDirectory = UseValueOrDefault $Env:BUILD_SOURCESDIRECTORY $(GetRepoRoot)
$BinariesDirectory = UseValueOrDefault (Join-Path $SourcesDirectory "Build\VcBuild")
$ObjectDirectory = Join-Path $BinariesDirectory "obj\${BuildPlatform}_${BuildConfiguration}"

$DropRoot = UseValueOrDefault $dropRoot $Env:DROP_ROOT (Join-Path $(GetRepoRoot) "_DROP")

# set up required variables and import pre_post_util.ps1
$arch = $BuildPlatform
$flavor = $BuildConfiguration
$OuterScriptRoot = $PSScriptRoot # Used in pre_post_util.ps1
. "$PSScriptRoot\pre_post_util.ps1"

$BuildName = ConstructBuildName -arch $BuildPlatform -flavor $BuildConfiguration -subtype $BuildSubtype

$BranchName = $branchFullName.split('/',3)[2]
$BranchPath = $BranchName.replace('/','\')

if (-not $CommitHash) {
    $CommitHash = iex "${gitExe} rev-parse HEAD"
}
$CommitShortHash = $(iex "${gitExe} rev-parse --short $CommitHash")

$Username = (iex "${gitExe} log $CommitHash -1 --pretty=%ae").split('@')[0]
$CommitDateTime = [DateTime]$(iex "${gitExe} log $CommitHash -1 --pretty=%aD")
$CommitTime = Get-Date $CommitDateTime -Format yyMMdd.HHmm

#
# Get Build Info
#

$buildPushDate = $null
$buildPushIdString = $null

if (-not $oauth)
{
    $buildPushIdString = "65535.65535"
    $buildPushDate = [DateTime]$CommitDateTime
}
else
{
    $info = GetBuildInfo $oauth $CommitHash
    $_, $_, $_, $buildPushIdString = GetBuildPushId $info
    $buildPushDate = [DateTime]$info.push.date
}

$PushDate = Get-Date $buildPushDate -Format yyMMdd.HHmm

$VersionMajor       = UseValueOrDefault $verMajor       $Env:VERSION_MAJOR  "1"
$VersionMinor       = UseValueOrDefault $verMinor       $Env:VERSION_MINOR  "2"
$VersionPatch       = UseValueOrDefault $verPatch       $Env:VERSION_PATCH  "0"
$VersionSecurity    = UseValueOrDefault $verSecurity    $Env:VERSION_QFE    "0"

$VersionString = "${VersionMajor}.${VersionMinor}.${VersionPatch}" # Only use MAJOR.MINOR.PATCH to align with SemVer

$BranchNameSemVerSanitized = $BranchName -replace ("[^a-zA-Z\d]", ".")
$PreviewVersionString = "${VersionString}-preview-${CommitShortHash}"

# unless it is a build branch, subdivide the output directory by month
if ($BranchPath.StartsWith("build")) {
    $YearAndMonth = ""
} else {
    $YearAndMonth = (Get-Date $buildPushDate -Format yyMM) + "\"
}

$BuildIdentifier = "${buildPushIdString}_${PushDate}_${Username}_${CommitHash}"
$ComputedDropPathSegment = "${BranchPath}\${YearAndMonth}${BuildIdentifier}"
$ObjectDirectory = "${BinariesDirectory}\obj\${BuildPlatform}_${BuildConfiguration}"

# Create a sentinel file for each build flavor to track whether the build is complete.
# * ${arch}_${flavor}.incomplete       # will be deleted when the build of this flavor completes

$buildIncompleteFileContentsString = @"
{0} is incomplete.
This could mean that the build is in progress, or that it was unable to run to completion.
The contents of this directory should not be relied on until the build completes.
"@

if ($DropRoot) {
    $DropPath = Join-Path $DropRoot $ComputedDropPathSegment
    New-Item -ItemType Directory -Force -Path $DropPath
    New-Item -ItemType Directory -Force -Path (Join-Path $SourcesDirectory "test\logs")
    New-Item -ItemType Directory -Force -Path (Join-Path $BinariesDirectory "buildlogs")
    New-Item -ItemType Directory -Force -Path (Join-Path $BinariesDirectory "logs")
}

$FlavorBuildIncompleteFile = Join-Path $DropPath "${BuildType}.incomplete"

if (-not (Test-Path $FlavorBuildIncompleteFile)) {
    ($buildIncompleteFileContentsString -f "Build of ${BuildType}") `
        | Out-File $FlavorBuildIncompleteFile -Encoding utf8
}

$PogoConfig = $supportedPogoBuildTypes -contains "${BuildPlatform}_${BuildConfiguration}"

# Write the $envConfigScript

@"
set BranchName=${BranchName}
set BranchPath=${BranchPath}
set YearAndMonth=${YearAndMonth}
set BuildIdentifier=${BuildIdentifier}

set VersionMajor=${VersionMajor}
set VersionMinor=${VersionMinor}
set BuildPushIdString=${buildPushIdString}
set VersionString=${VersionString}
set PreviewVersionString=${PreviewVersionString}
set PushDate=${PushDate}
set CommitTime=${CommitTime}
set Username=${Username}
set CommitHash=${CommitHash}
set CommitShortHash=${CommitShortHash}

set ComputedDropPathSegment=${ComputedDropPathSegment}
set BinariesDirectory=${BinariesDirectory}
set DropPath=${DropPath}

set BuildType=${BuildType}
set BuildPlatform=${BuildPlatform}
set BuildConfiguration=${BuildConfiguration}
set BuildSubtype=${BuildSubtype}
set BuildName=${BuildName}

set FlavorBuildIncompleteFile=${FlavorBuildIncompleteFile}

set PogoConfig=${PogoConfig}

"@ `
    | Out-File $envConfigScript -Encoding ASCII

# Use the VSTS environment vars to construct a backwards-compatible VSO build environment
# for the sake of reusing the pre-build and post-build scripts as they are.

@"
set TF_BUILD_SOURCEGETVERSION=LG:${branchFullName}:${CommitHash}
set TF_BUILD_DROPLOCATION=${BinariesDirectory}

set TF_BUILD_SOURCESDIRECTORY=${SourcesDirectory}
set TF_BUILD_BUILDDIRECTORY=${ObjectDirectory}
set TF_BUILD_BINARIESDIRECTORY=${BinariesDirectory}

REM The following variables are only used for logging build metadata.
set TF_BUILD_BUILDDEFINITIONNAME=${Env:BUILD_DEFINITIONNAME}
set TF_BUILD_BUILDNUMBER=${Env:BUILD_BUILDNUMBER}
set TF_BUILD_BUILDURI=${Env:BUILD_BUILDURI}
"@ `
    | Out-File $envConfigScript -Encoding ASCII -Append

# Export VSO variables that can be consumed by other VSO tasks where the task
# definition in VSO itself needs to know the value of the variable.
# If the task definition itself doesn't need to know the value of the variables,
# the variables that are added to the environment via the script generated above
# will be interpolated when the tasks run the associated command line with the
# given parameters.
#
# For example, for a Publish Artifacts task, VSO itself needs to know
# the value of DropPath in order to construct links to the artifacts correctly.
# Thus, we export a variable called VSO_DropPath (VSO_ prefix by convention)
# that the VSO build definition, not just the command environment, will know about.
#
# Uses command syntax documented here:
# https://github.com/Microsoft/vso-agent-tasks/blob/master/docs/authoring/commands.md
# Lines written to stdout that match this pattern are interpreted with this command syntax.

Write-Output "Setting VSO variable VSO_DropPath = ${DropPath}"
Write-Output "##vso[task.setvariable variable=VSO_DropPath;]${DropPath}"

Write-Output "Setting VSO variable VSO_VersionString = ${VersionString}"
Write-Output "##vso[task.setvariable variable=VSO_VersionString;]${VersionString}"

#
# Optionally ($cleanBinDir): clean up files that might have been left behind from a previous build.
#

if ($BinariesDirectory -and (Test-Path "$BinariesDirectory") -and $cleanBinDir)
{
    Remove-Item -Verbose "${BinariesDirectory}\*" -Recurse
}
