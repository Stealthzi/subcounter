# Builds the counter program for both boards and puts the update files in
# release\, ready to attach to a GitHub release:
#   release\firmware-7inch.bin  for the 7" CrowPanel
#   release\firmware.bin        for the 2.8" Cheap Yellow Display
$ErrorActionPreference = 'Stop'
$cli  = "$env:LOCALAPPDATA\Programs\arduino-cli\arduino-cli.exe"
$root = $PSScriptRoot
$boards = @(
  @{ Name = '7';   File = 'firmware-7inch.bin'; Fqbn = 'esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,FlashMode=qio,PartitionScheme=esp_sr_16,CDCOnBoot=default' },
  @{ Name = '2.8'; File = 'firmware.bin';       Fqbn = 'esp32:esp32:esp32:PartitionScheme=min_spiffs' }
)

New-Item -ItemType Directory -Force "$root\release" | Out-Null
foreach ($b in $boards) {
  $out = "$env:LOCALAPPDATA\Temp\subcounter-build-$($b.Name)"
  Write-Output "Building for the $($b.Name)-inch board..."
  # Keeps this PC's folder names (which include your user name) out of the update files
  $hide = "-fmacro-prefix-map=$env:USERPROFILE=~"
  & $cli compile --fqbn $b.Fqbn --build-path $out --build-property "compiler.c.extra_flags=$hide" `
    --build-property "compiler.cpp.extra_flags=$hide" "$root\YouTubeCounter"
  if ($LASTEXITCODE -ne 0) { throw "Build failed for the $($b.Name)-inch board" }
  Copy-Item "$out\YouTubeCounter.ino.bin" "$root\release\$($b.File)" -Force
}

$version = (Select-String -Path "$root\YouTubeCounter\app.h" -Pattern '#define FIRMWARE_VERSION "(.+)"').Matches[0].Groups[1].Value
Write-Output "Built version $version. Update files: release\firmware-7inch.bin and release\firmware.bin (tag the GitHub release v$version)"
