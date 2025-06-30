param(
    [ValidateSet("msvc", "msvc-ppl", "gcc")]
    [string]$compiler = "gcc"
)

$buildDir = "build"

# Clean build directory
if (Test-Path $buildDir) {
    Remove-Item -Path $buildDir -Recurse -Force
    Start-Sleep -Milliseconds 100
}
New-Item -ItemType Directory -Path $buildDir -Force | Out-Null

# Compile
switch ($compiler) {
    "msvc" {
        Write-Host "Compiling with MSVC and TBB for X64..."
        $compilerArgs = @(
            "/O2"
            "/std:c++20"
            "/utf-8"
            "/EHsc"
            "/Iexternal"
            "/Iexternal/oneapi-tbb-2022_1_0/include"
            "src\\main.cpp"
            "/Fe:$buildDir\\main.exe"
            "/link"
            "/LIBPATH:external/oneapi-tbb-2022_1_0/lib/intel64/vc14"
            "tbb12.lib"
        )
        $compileResult = & cl @compilerArgs
        Copy-Item "external/oneapi-tbb-2022_1_0/redist/intel64/vc14/tbb12.dll" "$buildDir"
    }
    "msvc-ppl" {
        Write-Host "Compiling with MSVC and PPL..."
        $compilerArgs = @(
            "/DPPL"
            "/O2"
            "/std:c++20"
            "/utf-8"
            "/EHsc"
            "/Iexternal"
            "src\\main.cpp"
            "/Fe:$buildDir\\main.exe"
        )
        $compileResult = & cl @compilerArgs
    }
    "gcc" {
        Write-Host "Compiling with GCC (MinGW)..."
        $compilerArgs = @(
            "-std=c++23"
            "-O3"
            "-Iexternal"
            "-Iexternal/oneapi-tbb-2022_1_0/include"
            "-Lexternal/oneapi-tbb-2022_1_0/lib/mingw-w64-ucrt-x86_64"
            "src/main.cpp"
            "-ltbb12"
            "-o"
            "$buildDir/main.exe"
        )
        $compileResult = & g++ @compilerArgs
        Copy-Item "external/oneapi-tbb-2022_1_0/redist/mingw-w64-ucrt-x86_64/libtbb12.dll" "$buildDir"
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
    if ((Get-Location).Path -ne (Resolve-Path ".").Path) {
        Pop-Location
    }
}
