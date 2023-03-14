[string[]] $include = '*.h', '*.hh', '*.hpp', '*.c', '*.cc', '*.cpp'
[string[]] $exclude = '*.g.*', '*.refactor'

$path_root   = git rev-parse --show-toplevel
$path_code   = Join-Path $path_root code
$path_header = Join-Path $path_code header
$path_source = Join-Path $path_code source
$path_misc   = Join-Path $path_root misc

$file_spec = Join-Path $path_misc zpl.refactor
$refactor  = Join-Path $path_misc refactor.exe

# Gather the files to be formatted.
$targetFiles = @()
$targetFiles += Get-ChildItem -Recurse -Path $path_header -Include $include -Exclude $exclude | Select-Object -ExpandProperty FullName
$targetFiles += Get-ChildItem -Recurse -Path $path_source -Include $include -Exclude $exclude | Select-Object -ExpandProperty FullName
$targetFiles += Join-Path $path_code zpl.h
$targetFiles += Join-Path $path_code zpl_hedley.h

# Format the files.
$formatParams = @(
    '-i'          # In-place
    '-style=file' # Search for a .clang-format file in the parent directory of the source file.
    '-verbose'
)


write-host "Beginning refactor...`n"

$refactors = @(@())

foreach ( $file in $targetFiles )
{
    $refactorParams = @(
        "-src=$($file)",
        "-spec=$($file_spec)"
    )

    $refactors += (Start-Process $refactor $refactorParams -NoNewWindow -PassThru)
}

foreach ( $process in $refactors )
{
    if ( $process )
    {
        $process.WaitForExit()
    }
}

Write-Host "`nRefactoring completen`n`n"


Write-Host "Beginning format...`n"

clang-format $formatParams $targetFiles

Write-Host "`nFormatting complete"
