$src = "src/main.cpp"
$buildDir = "build"
$out = "$buildDir/main.exe"
$include = "external"
$output = "$buildDir/output.bmp"  # Output now in build directory

# Clean build directory (more aggressive)
if (Test-Path $buildDir) {
    Remove-Item -Path $buildDir -Recurse -Force
    Start-Sleep -Milliseconds 100  # Ensure filesystem sync
}
New-Item -ItemType Directory $buildDir -Force | Out-Null

# Compile
Write-Host "Compiling..."
#$compileResult = & g++ -O3 -march=native -std=c++23 -ltbb12 -fopenmp "-I$include" "$src" -o "$out" 2>&1
$compileResult = g++ -std=c++23 -O3 -I"C:/msys64/ucrt64/include" -I"external" src/main.cpp -L"C:/msys64/ucrt64/lib" -ltbb12 -o build/main.exe

if ($LASTEXITCODE -ne 0) {
    Write-Error "Compilation failed: $compileResult"
    exit 1
}

# Run in build directory
Write-Host "Running..."
Push-Location $buildDir
& ".\main.exe"
$runExitCode = $LASTEXITCODE
Pop-Location

if ($runExitCode -ne 0) {
    Write-Error "Program execution failed"
    exit 1
}

# Open output if it exists
if (Test-Path $output) {
    Start-Process $output
}
else {
    Write-Warning "Output file $output was not created"
}