# SubCounter

A desk screen for YouTube channel stats, running on the ESP32 "Cheap Yellow Display" (ESP32-2432S028R).

It shows your YouTube stats, including exact analytics, plus home info (energy use, cheapest petrol, calendar, weather, alerts, air quality, countdowns). It also has encrypted messages and a friendship lamp between two counters, an old-school desk pet, and fun pages such as Spotify, free games, screensavers and a magic 8-ball. You set it up from a phone, and it installs new versions from this repository's releases by itself.

**The full guide is in [YouTubeCounter/README.md](YouTubeCounter/README.md).**

| File | What it's for |
|---|---|
| `YouTubeCounter/` | The program (Arduino sketch) |
| `flash.ps1` | Uploads the program to a board over USB (`-Fresh` the first time) |
| `build.ps1` | Builds `release/firmware.bin`, to attach to a new GitHub release |
| `connect-accounts.ps1` | One-time sign-in for Spotify and exact YouTube stats |
