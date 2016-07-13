#-------------------------------------------------------------------------------------------------------
# Copyright (C) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
#-------------------------------------------------------------------------------------------------------

function UseValueOrDefault($value, $defaultvalue, $defaultvalue2) {
    if ($value -and ($value -ne "")) {
        return $value
    } elseif ($defaultvalue -and ($defaultvalue -ne "")) {
        return $defaultvalue
    } elseif ($defaultvalue2 -and ($defaultvalue2 -ne "")) {
        return $defaultvalue2
    } else {
        return ""
    }
}

function GetGitPath() {
    $gitExe = "git.exe"

    if (!(Get-Command $gitExe -ErrorAction SilentlyContinue)) {
        $gitExe = "C:\1image\Git\bin\git.exe"
        if (!(Test-Path $gitExe)) {
            throw "git.exe not found in path -- aborting."
        }
    }

    return $gitExe
}

function GetRepoRoot() {
    $gitExe = GetGitPath
    return iex "$gitExe rev-parse --show-toplevel"
}

function WriteMessage($str) {
    Write-Output $str
    if ($logFile -ne "") {
        Write-Output $str | Out-File $logFile -Append
    }
}

function WriteErrorMessage($str) {
    $host.ui.WriteErrorLine($str)
    if ($logFile -ne "") {
        Write-Output $str | Out-File $logFile -Append
    }
}

function ExecuteCommand($cmd) {
    if ($cmd -eq "") {
        return
    }
    WriteMessage "-------------------------------------"
    WriteMessage "Running $cmd"
    if ($noaction) {
        return
    }
    Invoke-Expression $cmd
    if ($lastexitcode -ne 0) {
        WriteErrorMessage "ERROR: Command failed: exit code $LastExitCode"
        $global:exitcode = $LastExitCode
    }
    WriteMessage ""
}
