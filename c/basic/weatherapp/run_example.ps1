if (-not $env:OPENWEATHER_API_KEY) {
    Write-Host "Please set OPENWEATHER_API_KEY environment variable first." -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path -Path .\weather)) {
    Write-Host "Executable 'weather' not found; run 'mingw32-make' or 'make' first." -ForegroundColor Yellow
    exit 2
}

.\weather "London"
