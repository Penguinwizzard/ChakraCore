
param (
    [string]$directory,
    [string]$logFile = ""
)

function WriteMessage($str)
{
    Write-Output $str
    if ($logFile -ne "" )
    {
        Write-Output $str | Out-File $logFile
    }
}

function WriteErrorMessage($str)
{
    $host.ui.WriteErrorLine($str);
    if ($logFile -ne "" )
    {
        Write-Output $str | Out-File $logFile
    }
}


if ((Test-Path $directory) -eq 0) {
    Write-Error ("ERROR: Directory {0} does not exist.  Cannot scan for prefast defects" -f $directory);
    Exit(-1)
}

$files = Get-ChildItem -recurse ("{0}\vc.nativecodeanalysis.all.xml" -f $directory)

$filecount = 0;
$count = 0;
foreach ($file in $files) {
    $filecount++;
    [xml]$x = Get-Content $file
    foreach ($d in $x.DEFECTS.DEFECT) {
        if ($d.CATEGORY.RULECATEGORY -eq 'mspft') {
            $count++;
        }
    } 
}

if ($count -ne 0) {
    WriteErrorMessage ("ERROR: {0} prefast warning detected" -f $count)
} elseif ($filecount -ne 0) {
    WriteMessage "No prefast warning detected" 
} else {
    WriteMessage "No prefast result found"
}
Exit($count)
