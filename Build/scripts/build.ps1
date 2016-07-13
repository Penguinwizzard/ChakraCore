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

    [string]$core = "", # e.g. "core" indicates path to core (empty means we are in core)
    # [string]$buildConfigPrefix = "", # e.g. "all-"

    [string]$verMajor = "",
    [string]$verMinor = "",

    [string]$solutionFile = "Build\Chakra.Core.sln",
    [switch]$clean,

    [switch]$cleanBinDir,

    [string]$envConfigScript = "ComputedEnvironment.cmd",

    [string[]]$supportedPogoBuildTypes = @("x64_release", "x86_release"),

    [string]$oauth
)

# $Env:BuildPlatform = $arch
# $Env:BuildConfiguration = $flavor
# $Env:BuildSubtype = $subtype

if ($clean) { $cleanSwitch = "-clean" }

. $PSScriptRoot\init_build.ps1 -arch "$arch" -flavor "$flavor" -subtype "$subtype" -buildtype "$buildtype" -verMajor "$verMajor" -verMinor "$verMinor"
# . $PSScriptRoot\pre_build.ps1 -arch $BuildPlatform -flavor $BuildConfiguration -subtype $BuildSubtype
# iex ". $PSScriptRoot\run_build.ps1 -arch $BuildPlatform -flavor $BuildConfiguration -subtype $BuildSubtype -solutionFile $solutionFile $cleanSwitch"
