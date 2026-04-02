param(
    [ValidateSet("msvc", "gcc")]
    [string]$compiler = "gcc",
    
    [int]$iterations = 5,
    
    [switch]$PPL
)

$buildDir = "build"

# Build
& "$PSScriptRoot\build.ps1" -compiler $compiler $(if ($PPL) { "-PPL" })

if ($LASTEXITCODE -ne 0) {
    exit 1
}

Write-Host "`nStarting benchmark (iterations: $iterations)..." -ForegroundColor Cyan

# Change to build directory for execution
Push-Location $buildDir
try { 

    # Run benchmark iterations
    Write-Host "`nRunning $iterations iterations..." -ForegroundColor Yellow
    $times = @()

    for ($i = 1; $i -le $iterations; $i++) {
        Write-Host "  Iteration $i/$iterations..." -NoNewline
        
        $output = & ".\main.exe" 2>&1
        $match = $output | Select-String "Elapsed (\d{2}):(\d{2}):(\d{2})"
        if ($match -and $match.Matches.Count -gt 0) {
            $hours = [int]$match.Matches[0].Groups[1].Value
            $minutes = [int]$match.Matches[0].Groups[2].Value
            $seconds = [int]$match.Matches[0].Groups[3].Value
            $timeSeconds = $hours * 3600 + $minutes * 60 + $seconds
            $times += $timeSeconds
            
            Write-Host " ${timeSeconds}s" -ForegroundColor Green
        }
        else {
            Write-Host " Failed to parse time" -ForegroundColor Red
            # Add a default time or skip this iteration
            continue
        }
    }

    # Verify we got all iterations
    if ($times.Count -ne $iterations) {
        Write-Host "Warning: Expected $iterations iterations but got $($times.Count)" -ForegroundColor Yellow
    }
}
finally {
    Pop-Location
}

# Calculate statistics
$mean = ($times | Measure-Object -Average).Average
$min = ($times | Measure-Object -Minimum).Minimum
$max = ($times | Measure-Object -Maximum).Maximum
$sum = ($times | Measure-Object -Sum).Sum

# Calculate standard deviation
if ($times.Count -gt 1) {
    $variance = ($times | ForEach-Object { [math]::Pow($_ - $mean, 2) } | Measure-Object -Average).Average
    $stdDev = [math]::Sqrt($variance)
}
else {
    $stdDev = 0
}

# Calculate confidence interval (95%)
$stdErr = $stdDev / [math]::Sqrt($times.Count)
$marginOfError = 1.96 * $stdErr
$ci_lower = $mean - $marginOfError
$ci_upper = $mean + $marginOfError

# Convert seconds to human-readable format
function Format-Time {
    param([double]$seconds)
    if ($seconds -ge 60) {
        $mins = [Math]::Floor($seconds / 60)
        $secs = $seconds % 60
        return "{0}m {1:F1}s" -f $mins, $secs
    }
    else {
        return "{0:F1}s" -f $seconds
    }
}

# Generate results content
$timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$resultFile = "benchmark_results_$timestamp.txt"
$resultPath = Join-Path $buildDir $resultFile

$resultContent = @"
============================================================
BENCHMARK RESULTS
============================================================

Configuration:
  Compiler: $compiler
  Iterations: $iterations
  Date: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

Timing Results:
  Total Time:      $(Format-Time $sum)
  Mean:            $(Format-Time $mean)
  Std Dev:         $(Format-Time $stdDev)
  Min:             $(Format-Time $min)
  Max:             $(Format-Time $max)
  95% CI:          $(Format-Time $ci_lower) - $(Format-Time $ci_upper)

Individual Times:
"@

for ($i = 0; $i -lt $times.Count; $i++) {
    $variance_pct = (($times[$i] - $mean) / $mean * 100)
    $variance_str = if ($variance_pct -ge 0) { "+$([math]::Round($variance_pct, 1))%" } else { "$([math]::Round($variance_pct, 1))%" }
    $resultContent += "`n  Iter $($i+1): $(Format-Time $times[$i])  ($variance_str)"
}

$resultContent += "`n`n============================================================`n"

# Print to console
Write-Host "`n" + $resultContent -ForegroundColor Cyan

# Save to file
$resultContent | Out-File -FilePath $resultPath -Encoding UTF8

Write-Host "Results saved to: build\$resultFile" -ForegroundColor Green
Start-Process $resultPath
