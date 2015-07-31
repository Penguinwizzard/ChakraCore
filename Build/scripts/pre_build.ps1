# 
# Post-Build script
#
# This script is fairly simple. It checks if it's running
# in a VSO. If it is, it uses the VSO environment variables
# to figure out the commit that triggered the build, and if
# such a commit exists, it saves it's description to the build
# output to make it easy to inspect builds.
#

if (Test-Path Env:\TF_BUILD_SOURCEGETVERSION)
{
    $commitHash = ($Env:TF_BUILD_SOURCEGETVERSION).split()[2]
    $gitExe = "git.exe"

    if (!(Get-Command $gitExe -ErrorAction SilentlyContinue)) {
        $gitExe = "C:\1image\Git\bin\git.exe"
        if (!(Test-Path $gitExe)) {
            throw "git.exe not found in path- aborting."
        }
    }

    $command = "$gitExe log -1 --name-status -p $commitHash"
    $outputDir = $Env:TF_BUILD_SOURCESDIRECTORY
    if (!(Test-Path $outputDir)) {
        mdkir $outputDir
    }

    Push-Location $outputDir
    $outputFile = Join-Path -Path $Env:TF_BUILD_DROPLOCATION  -ChildPath "change.txt"
    iex $command | Out-File $outputFile
    Pop-Location
}
