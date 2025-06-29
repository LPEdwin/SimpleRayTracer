param(
    [ValidateSet("msvc", "msvc-ppl", "gcc")]
    [string]$compiler = "gcc"
)

$buildDir = "build"

# Clean build directory
if (Test-Path $buildDir) {
    Remove-Item -Path $buildDir -Recurse -Force
    Start-Sleep -Milliseconds 100  # Ensure filesystem sync
}
New-Item -ItemType Directory $buildDir -Force | Out-Null

# Compile
switch ($compiler) {
    "msvc" {
        Write-Host "Compiling with MSVC and TBB for X64..."
        # Assumes running inside Developer Command Prompt with env set
        $compileResult = & cl /O2 /std:c++20 /utf-8 /EHsc /Iexternal /Iexternal/tbb/include src\main.cpp /Fe:build\main.exe /link /LIBPATH:"external\tbb\lib\intel64\vc14" tbb12.lib
        Copy-Item "external\tbb\redist\tbb12.dll" "$buildDir"
    }
    "msvc-ppl" {
        Write-Host "Compiling with MSVC and PPL..."
        # Assumes running inside Developer Command Prompt with env set
        $compileResult = & cl /DPPL /O2 /std:c++20 /utf-8 /EHsc /Iexternal src\main.cpp /Fe:build\main.exe
    }    
    "gcc" {
        Write-Host "Compiling with GCC (MinGW)..." 
        #$compileResult = & g++ -std=c++23 -O3 -I"C:/msys64/ucrt64/include" -I"external" src/main.cpp -L"C:/msys64/ucrt64/lib" -ltbb12 -o build/main.exe
        $compileResult = & g++ -std=c++23 -O3 -I"external" src/main.cpp -ltbb12 -o build/main.exe
    }
}

if ($LASTEXITCODE -ne 0) {
    Write-Error "Compilation failed: $compileResult"
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