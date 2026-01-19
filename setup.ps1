# Quick Setup Script for Windows
# Run this in PowerShell to set up the project

Write-Host "RTL-SDR VST3 Plugin - Quick Setup" -ForegroundColor Cyan
Write-Host "====================================`n" -ForegroundColor Cyan

# Check if Git is installed
Write-Host "[1/5] Checking Git..." -ForegroundColor Yellow
if (Get-Command git -ErrorAction SilentlyContinue) {
    Write-Host "✓ Git found" -ForegroundColor Green
} else {
    Write-Host "✗ Git not found. Please install from https://git-scm.com/" -ForegroundColor Red
    exit 1
}

# Check if CMake is installed
Write-Host "`n[2/5] Checking CMake..." -ForegroundColor Yellow
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    Write-Host "✓ CMake found" -ForegroundColor Green
} else {
    Write-Host "✗ CMake not found. Please install from https://cmake.org/" -ForegroundColor Red
    exit 1
}

# Initialize JUCE submodule
Write-Host "`n[3/5] Setting up JUCE..." -ForegroundColor Yellow
if (Test-Path "JUCE/.git") {
    Write-Host "✓ JUCE already initialized" -ForegroundColor Green
} else {
    Write-Host "Cloning JUCE framework..." -ForegroundColor White
    git submodule add https://github.com/juce-framework/JUCE.git JUCE
    git submodule update --init --recursive

    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ JUCE cloned successfully" -ForegroundColor Green
    } else {
        Write-Host "✗ Failed to clone JUCE" -ForegroundColor Red
        exit 1
    }
}

# Check for RTL-SDR
Write-Host "`n[4/5] Checking RTL-SDR library..." -ForegroundColor Yellow
$rtlsdrPaths = @(
    "C:\Program Files\rtl-sdr",
    "C:\rtl-sdr",
    "$env:USERPROFILE\rtl-sdr"
)

$rtlsdrFound = $false
foreach ($path in $rtlsdrPaths) {
    if (Test-Path $path) {
        Write-Host "✓ RTL-SDR found at: $path" -ForegroundColor Green
        $env:RTLSDR_ROOT = $path
        $rtlsdrFound = $true
        break
    }
}

if (-not $rtlsdrFound) {
    Write-Host "⚠ RTL-SDR library not found" -ForegroundColor Yellow
    Write-Host "  You can either:" -ForegroundColor White
    Write-Host "  1. Install RTL-SDR from https://github.com/osmocom/rtl-sdr/releases" -ForegroundColor White
    Write-Host "  2. Build in simulation mode (no hardware required)" -ForegroundColor White

    $response = Read-Host "`nBuild in simulation mode? (Y/N)"
    if ($response -eq "Y" -or $response -eq "y") {
        $simulationMode = $true
        Write-Host "✓ Will build in simulation mode" -ForegroundColor Green
    } else {
        Write-Host "✗ Cannot continue without RTL-SDR or simulation mode" -ForegroundColor Red
        exit 1
    }
}

# Configure and build
Write-Host "`n[5/5] Configuring project..." -ForegroundColor Yellow

if (Test-Path "build") {
    Write-Host "Removing old build directory..." -ForegroundColor White
    Remove-Item -Recurse -Force build
}

New-Item -ItemType Directory -Path build | Out-Null
Set-Location build

Write-Host "Running CMake configuration..." -ForegroundColor White

if ($simulationMode) {
    cmake .. -G "Visual Studio 17 2022" -A x64 -DHAS_RTLSDR=0
} else {
    cmake .. -G "Visual Studio 17 2022" -A x64
}

if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Configuration successful" -ForegroundColor Green
} else {
    Write-Host "✗ Configuration failed" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Write-Host "`nBuilding project..." -ForegroundColor White
cmake --build . --config Release

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n✓ Build successful!" -ForegroundColor Green

    # Find the VST3
    $vst3Path = Get-ChildItem -Recurse -Filter "*.vst3" | Select-Object -First 1

    if ($vst3Path) {
        Write-Host "`n✓ VST3 plugin built at:" -ForegroundColor Green
        Write-Host "  $($vst3Path.FullName)" -ForegroundColor Cyan

        $installPath = "$env:CommonProgramFiles\VST3"
        Write-Host "`nTo install, copy to:" -ForegroundColor White
        Write-Host "  $installPath" -ForegroundColor Cyan

        $install = Read-Host "`nInstall plugin now? (Y/N)"
        if ($install -eq "Y" -or $install -eq "y") {
            if (-not (Test-Path $installPath)) {
                New-Item -ItemType Directory -Path $installPath -Force | Out-Null
            }
            Copy-Item -Path $vst3Path.FullName -Destination $installPath -Recurse -Force
            Write-Host "✓ Plugin installed!" -ForegroundColor Green
        }
    }
} else {
    Write-Host "`n✗ Build failed" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Set-Location ..

Write-Host "`n====================================`n" -ForegroundColor Cyan
Write-Host "Setup complete!" -ForegroundColor Green
Write-Host "`nNext steps:" -ForegroundColor White
Write-Host "  1. Open Ableton Live" -ForegroundColor White
Write-Host "  2. Add RTL-SDR Radio plugin to a track" -ForegroundColor White
Write-Host "  3. Select your RTL-SDR device and click 'Start SDR'" -ForegroundColor White
Write-Host "  4. Tune to a frequency and enjoy!" -ForegroundColor White
Write-Host "`nDocumentation: README.md" -ForegroundColor White
Write-Host "Architecture details: ARCHITECTURE.md" -ForegroundColor White
Write-Host "Build guide: BUILD.md`n" -ForegroundColor White
