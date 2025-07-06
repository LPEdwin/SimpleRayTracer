param(
    [ValidateSet("msvc", "gcc")]
    [string]$compiler = "gcc",

    [switch]$PPL
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
    "gcc" {
        Write-Host "Compiling with GCC (MinGW)..."
        $compilerArgs = @(
            "-std=c++23"
            "-O3"
            "-Isrc"
            "-Iexternal"
            "-Iexternal/oneapi-tbb-2022_1_0/include"
            "src/main.cpp"
            "-Lexternal/oneapi-tbb-2022_1_0/lib/mingw-w64-ucrt-x86_64"        
            "-o"
            "$buildDir/main.exe"
            "-ltbb12"
        )

        $compileResult = & g++ @compilerArgs
        Copy-Item "external/oneapi-tbb-2022_1_0/redist/mingw-w64-ucrt-x86_64/libtbb12.dll" "$buildDir"
    }
    "msvc" {
        Write-Host "Compiling with MSVC and $(if ($PPL) { 'PPL' } else { 'TBB' })..."
        $compilerArgs = @(
            "/O2"
            "/std:c++20"
            "/utf-8"
            "/EHsc"
            "/Isrc"
            "/Iexternal"       
            $(if ($PPL) { "/DPPL" } else { "/Iexternal/oneapi-tbb-2022_1_0/include" })
            "src/main.cpp"
            "/Fe:$buildDir/main.exe"    
        )          
        if (!$PPL) {
            $compilerArgs += @(
                "/link"
                "/LIBPATH:external/oneapi-tbb-2022_1_0/lib/intel64/vc14"
                "tbb12.lib"
            )
        }
        $compileResult = & cl @compilerArgs
        if ( !$PPL ) {
            Copy-Item "external/oneapi-tbb-2022_1_0/redist/intel64/vc14/tbb12.dll" "$buildDir"
        }        
    }  
}

if ($LASTEXITCODE -ne 0) {
    Write-Error "Compilation failed: $compileResult"
    exit 1
}

Copy-Item "assets" "$buildDir" -Recurse -Force

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
