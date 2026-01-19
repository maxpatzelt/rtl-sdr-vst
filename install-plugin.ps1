Write-Host 'Installing RTL-SDR Radio VST3 Plugin...' -ForegroundColor Cyan
$source = 'D:\SDR-VST\build\RTLSDRPlugin_artefacts\Release\VST3\RTL-SDR Radio.vst3'

# Try VSTPlugins folder first
Copy-Item -Path $source -Destination 'C:\Program Files\VSTPlugins\' -Recurse -Force
Write-Host '√ Installed to C:\Program Files\VSTPlugins\' -ForegroundColor Green

# Also copy to standard VST3 location
Copy-Item -Path $source -Destination 'C:\Program Files\Common Files\VST3\' -Recurse -Force
Write-Host '√ Installed to C:\Program Files\Common Files\VST3\' -ForegroundColor Green

Write-Host '
Plugin installed! Restart Ableton Live and rescan plugins.' -ForegroundColor Yellow
pause
