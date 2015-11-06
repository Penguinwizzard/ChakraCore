
param (
    [string]$directory
)

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
    $host.ui.WriteErrorLine("ERROR: {0} prefast warning detected" -f $count)
} elseif ($filecount -ne 0) {
    Write-Output "No prefast warning detected"
} else {
    Write-Output "No prefast result found"
}
Exit($count)
