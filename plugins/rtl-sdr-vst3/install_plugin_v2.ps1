$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.IO.Compression.FileSystem

Write-Host "Installing RTL-SDR Radio VST3..." -ForegroundColor Cyan
Write-Host ""

$source = "D:\SDR-VST\build\RTLSDRPlugin_artefacts\Release\VST3\RTL-SDR Radio.vst3"
$dest = "C:\Program Files\Common Files\VST3\RTL-SDR Radio.vst3"

try {
    # Remove old version
    if (Test-Path $dest) {
        Write-Host "Removing old version..." -ForegroundColor Yellow
        cmd /c "rmdir /S /Q `"$dest`"" 2>$null
        Start-Sleep -Milliseconds 500
    }

    # Copy new version using xcopy
    Write-Host "Installing new version..." -ForegroundColor Yellow
    $result = cmd /c "xcopy `"$source`" `"$dest\`" /E /I /H /Y" 2>&1

    Start-Sleep -Milliseconds 500

    # Verify
    $dll = Join-Path $dest "Contents\x86_64-win\RTL-SDR Radio.vst3"
    if (Test-Path $dll) {
        Write-Host ""
        Write-Host "✓✓✓ SUCCESS! Plugin installed!" -ForegroundColor Green
        Write-Host ""
        Write-Host "File: $dll" -ForegroundColor Gray
        Write-Host "Date: $((Get-Item $dll).LastWriteTime)" -ForegroundColor Gray
        Write-Host "Size: $((Get-Item $dll).Length) bytes" -ForegroundColor Gray
    } else {
        Write-Host ""
        Write-Host "✗ Installation failed - DLL not found" -ForegroundColor Red
    }

} catch {
    Write-Host ""
    Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""
Write-Host "Press any key to close..." -ForegroundColor Cyan
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
