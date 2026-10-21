# Builds the counter program and puts the update file in release\firmware.bin,
# ready to attach to a GitHub release.
$ErrorActionPreference = 'Stop'
$cli  = "$env:LOCALAPPDATA\Programs\arduino-cli\arduino-cli.exe"
$fqbn = 'esp32:esp32:esp32:PartitionScheme=min_spiffs'
$root = $PSScriptRoot

& $cli compile --fqbn $fqbn --output-dir "$root\build" "$root\YouTubeCounter"
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }

New-Item -ItemType Directory -Force "$root\release" | Out-Null
Copy-Item "$root\build\YouTubeCounter.ino.bin" "$root\release\firmware.bin" -Force

$version = (Select-String -Path "$root\YouTubeCounter\app.h" -Pattern '#define FIRMWARE_VERSION "(.+)"').Matches[0].Groups[1].Value
Write-Output "Built version $version. Update file: release\firmware.bin (tag the GitHub release v$version)"
