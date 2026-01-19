$ErrorActionPreference = "Stop"

Write-Host "Removing old plugin..." -ForegroundColor Yellow
Remove-Item "C:\Program Files\Common Files\VST3\RTL-SDR Radio.vst3" -Recurse -Force -ErrorAction SilentlyContinue

Write-Host "Copying new plugin..." -ForegroundColor Yellow
Copy-Item "D:\SDR-VST\build\RTLSDRPlugin_artefacts\Release\VST3\RTL-SDR Radio.vst3" "C:\Program Files\Common Files\VST3\" -Recurse -Force

Write-Host "`n✓✓✓ Plugin installed successfully!" -ForegroundColor Green

$installedDll = "C:\Program Files\Common Files\VST3\RTL-SDR Radio.vst3\Contents\x86_64-win\RTL-SDR Radio.vst3"
if (Test-Path $installedDll) {
    $date = (Get-Item $installedDll).LastWriteTime
    Write-Host "Installed at: $date" -ForegroundColor Gray
}

Write-Host "`nPress any key to close..." -ForegroundColor Cyan
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
