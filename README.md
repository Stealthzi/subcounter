# SubCounter

A desk screen for YouTube channel stats, running on the ESP32 "Cheap Yellow Display" (ESP32-2432S028R).

It shows subscribers with a milestone bar, channel stats, your latest upload with views per hour, recent uploads, your most viewed video, your newest comment, a 7-day chart, a head-to-head with another channel, live viewers, a clock, weather and crypto prices. You set it up from a phone, and it installs new versions from this repository's releases by itself.

**The full guide is in [YouTubeCounter/README.md](YouTubeCounter/README.md).**

| File | What it's for |
|---|---|
| `YouTubeCounter/` | The program (Arduino sketch) |
| `flash.ps1` | Uploads the program to a board over USB (`-Fresh` the first time) |
| `build.ps1` | Builds `release/firmware.bin`, to attach to a new GitHub release |
