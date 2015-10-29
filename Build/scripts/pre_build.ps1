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
    $sourcesDir = $Env:TF_BUILD_SOURCESDIRECTORY

    $outputDir = $Env:TF_BUILD_DROPLOCATION
    if (-not(Test-Path -Path $outputDir)) {
        New-Item -Path $outputDir -ItemType Directory -Force
    }

    Push-Location $sourcesDir
    $outputFile = Join-Path -Path $outputDir -ChildPath "change.txt"

    Write-Output "TF_BUILD_BUILDDEFINITIONNAME = $Env:TF_BUILD_BUILDDEFINITIONNAME" | Out-File $outputFile -Append
    Write-Output "TF_BUILD_BUILDNUMBER = $Env:TF_BUILD_BUILDNUMBER" | Out-File $outputFile -Append
    Write-Output "TF_BUILD_SOURCEGETVERSION = $Env:TF_BUILD_SOURCEGETVERSION" | Out-File $outputFile -Append
    Write-Output "TF_BUILD_BUILDURI = $Env:TF_BUILD_BUILDURI" | Out-File $outputFile -Append

    iex $command | Out-File $outputFile -Append
    Pop-Location
}
