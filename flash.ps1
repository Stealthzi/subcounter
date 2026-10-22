# Puts the counter program on a board plugged in by USB.
#   .\flash.ps1 -Fresh       first time: wipes the board's old software first
#   .\flash.ps1              later: keeps the board's settings
#   .\flash.ps1 -Port COM5   if more than one board is plugged in
#   .\flash.ps1 -Board 2.8   for the small 2.8" Cheap Yellow Display (default is the 7" CrowPanel)
param([string]$Port = '', [switch]$Fresh, [ValidateSet('7', '2.8')][string]$Board = '7')
$ErrorActionPreference = 'Stop'
$cli  = "$env:LOCALAPPDATA\Programs\arduino-cli\arduino-cli.exe"
$fqbn = if ($Board -eq '7') { 'esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,FlashMode=qio,PartitionScheme=esp_sr_16,CDCOnBoot=default' }
        else { 'esp32:esp32:esp32:PartitionScheme=min_spiffs' }
if ($Fresh) { $fqbn += ',EraseFlash=all' }
$root = $PSScriptRoot

if (-not $Port) {
  $found = (& $cli board list --format json | ConvertFrom-Json).detected_ports |
    Where-Object { $_.port.protocol -eq 'serial' -and $_.port.properties.vid } |
    ForEach-Object { $_.port.address }
  $found = @($found)
  if ($found.Count -eq 0) { throw 'No board found. Check the USB cable is plugged in (and carries data).' }
  if ($found.Count -gt 1) { throw "More than one board found ($($found -join ', ')). Run again with -Port." }
  $Port = $found[0]
}

$out = "$env:LOCALAPPDATA\Temp\subcounter-$Board"
Write-Output "Building for the $Board-inch board..."
$hide = "-fmacro-prefix-map=$env:USERPROFILE=~"  # keeps your user name out of the program
& $cli compile --fqbn $fqbn --build-path $out --build-property "compiler.c.extra_flags=$hide" `
  --build-property "compiler.cpp.extra_flags=$hide" "$root\YouTubeCounter"
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }

# The 7" board's storage has a space for speech models (for a voice assistant
# later). They go on over USB with the program, as remote updates can't add them.
if ($Board -eq '7') {
  $models = Get-ChildItem "$env:LOCALAPPDATA\Arduino15\packages\esp32\tools" -Recurse -Filter srmodels.bin -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -match 'esp32s3' } | Select-Object -First 1
  if (-not $models) { throw 'Speech model file (srmodels.bin) not found in the ESP32 tools' }
  Copy-Item $models.FullName "$out\srmodels.bin" -Force
}

Write-Output "Uploading to the board on $Port..."
& $cli upload --port $Port --fqbn $fqbn --input-dir $out
if ($LASTEXITCODE -ne 0) { throw 'Upload failed. If it stopped at "Connecting...", hold the BOOT button, tap RESET, let go of BOOT and try again.' }
Write-Output 'Done. The counter restarts by itself.'
