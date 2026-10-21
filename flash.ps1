# Puts the counter program on a board plugged in by USB.
#   .\flash.ps1 -Fresh     first time: wipes the board's old software first
#   .\flash.ps1            later: keeps the board's settings
#   .\flash.ps1 -Port COM5 if more than one board is plugged in
param([string]$Port = '', [switch]$Fresh)
$ErrorActionPreference = 'Stop'
$cli  = "$env:LOCALAPPDATA\Programs\arduino-cli\arduino-cli.exe"
$fqbn = 'esp32:esp32:esp32:PartitionScheme=min_spiffs'
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

Write-Output "Uploading to the board on $Port..."
& $cli compile --upload --port $Port --fqbn $fqbn --output-dir "$root\build" "$root\YouTubeCounter"
if ($LASTEXITCODE -ne 0) { throw 'Upload failed. If it stopped at "Connecting...", hold the BOOT button and try again.' }
Write-Output 'Done. The counter restarts by itself.'
