$src = "src/main.cpp"
$out = "build/main.exe"
$include = "external"
$output = "output.bmp"

# Create build directory
if (-not (Test-Path build)) {
    New-Item -ItemType Directory build | Out-Null
}

# Remove old output file and wait
if (Test-Path $output) {
    Remove-Item -Force $output
    #Start-Sleep -Milliseconds 100  # Give filesystem time
}

# Compile
Write-Host "Compiling..."
$compileResult = & g++ -O3 -march=native -std=c++23 "-I$include" "$src" -o "$out" 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Error "Compilation failed: $compileResult"
    exit 1
}

# Run
Write-Host "Running..."
& ".\$out"
if ($LASTEXITCODE -ne 0) {
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