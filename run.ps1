param(
    [ValidateSet("msvc", "gcc")]
    [string]$compiler = "gcc",

    [switch]$PPL
)

$buildDir = "build"

# Build
& "$PSScriptRoot\build.ps1" -compiler $compiler $(if ($PPL) { "-PPL" })

if ($LASTEXITCODE -ne 0) {
    exit 1
}

# Run in build directory
Write-Host "Running..."
Push-Location $buildDir
try {
    & ".\main.exe"
    $runExitCode = $LASTEXITCODE
    Pop-Location

    if ($runExitCode -ne 0) {
        Write-Error "Program execution failed"
        exit 1
    }

    # Open output if it exists
    $bmpFiles = Get-ChildItem -Path $buildDir -Filter *.bmp | Sort-Object Name
    if ($bmpFiles.Count -gt 0) {
        Start-Process $bmpFiles[0].FullName
    }
    else {
        Write-Warning "No BMP file found in $buildDir"
    }
}
finally {
    Pop-Location    
}
