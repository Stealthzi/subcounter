# YouTube Counter, version 2

A desk screen for your YouTube stats, built on the ESP32 "Cheap Yellow Display" (ESP32-2432S028R).

You set it up from your phone, so there's no code to edit. It can update itself over the internet, so a counter you've given away keeps getting new versions.

## What it shows

The pages change every 10 seconds. **Tap the screen** to skip to the next one. You can switch any page on or off in Settings.

| Page | What's on it |
|---|---|
| **Subscribers** | Big count, gained today, a progress bar to your next milestone (100K Silver, 1M Gold...) and roughly how many days until you reach it |
| **Channel stats** | Total views, views today, video count, and subscribers or views gained this week |
| **Latest upload** | Title, views, likes, comments and **views per hour** |
| **Recent uploads** | Your last 4 videos and their views |
| **Most viewed video** | Your top video of all time |
| **Newest comment** | The latest comment on your newest video |
| **7-day chart** | Views for each of the last 7 days. It fills in over the first few days |
| **Head to head** | You against another channel you choose |
| **Live and premieres** | Live viewers while you're streaming, or a countdown before a stream or premiere. While you're live it shows between every other page |
| **Clock** | Time and date, with the weather underneath |
| **Weather** | Today and tomorrow for your town |
| **Crypto prices** | Up to 3 coins, with the change over 24 hours |

When your subscriber count goes up, the number turns green and the light on the back flashes.

**Buttons:**
- **Tap:** next page. At night, a tap lights the screen up for 30 seconds.
- **Hold for 2 seconds:** info screen, showing the version, WiFi signal, any problems and the **settings page address**.
- **Hold for 8 seconds,** or hold the **BOOT** button on the back for 3 seconds: setup mode.
- **Hold a finger on the screen while plugging it in:** setup mode.

---

## 1. Get a YouTube API key (free, about 5 minutes)

1. Go to <https://console.cloud.google.com> and sign in with your Google account.
2. Click the project dropdown at the top, then **New Project**. Name it `Sub Counter` and click **Create**.
3. In the search bar, type **YouTube Data API v3**, open it and click **Enable**.
4. In the left menu, go to **APIs & Services → Credentials**, then **+ Create credentials → API key**.
5. Copy the key. It starts with `AIza...`.
6. Click the key, then under *API restrictions* choose **Restrict key**, tick **YouTube Data API v3** and click **Save**.

**One key is enough for both counters.** The setup page tells you how much of the free daily allowance (10,000 units) each counter uses. At the default settings it's about 2,500 a day each.

## 2. Put the program on a board (once, over USB)

Plug the board into the PC with its USB cable and ask Claude to upload it, or run this in PowerShell from this folder:

```
.\flash.ps1 -Fresh
```

`-Fresh` wipes the demo software the board came with. Leave it off when re-uploading to a board that's already set up, so it keeps its settings.

<details><summary>Doing it with the Arduino IDE instead</summary>

Install the libraries **LovyanGFX** and **ArduinoJson**. Choose the board **ESP32 Dev Module** and set **Partition Scheme** to **Minimal SPIFFS (1.9MB APP with OTA/128KB SPIFFS)**. That partition setting is important for remote updates. Then open `YouTubeCounter.ino` and click Upload.
</details>

## 3. Set it up from your phone

1. The screen says **Let's set up** and shows a QR code.
2. On your phone, join the WiFi network **SubCounter-Setup**, or scan the QR code.
3. The setup page opens by itself. If it doesn't, open a browser and go to **192.168.4.1**.
4. Fill in your WiFi, your channel's @handle and the API key, tick the pages you want, then press **Save and restart**.

The counter restarts, joins your WiFi and starts showing your stats.

**To change settings later:** hold a finger on the screen for 2 seconds. The info screen shows the counter's address (something like `http://192.168.1.50`). Open that on any phone or PC on the same WiFi.

---

## Your brother's counter

1. **Upload the program at your house** with `.\flash.ps1 -Fresh`. That way it's checked before you post it.
2. You can either set it up on your WiFi to test it, or leave it in setup mode. Either way, when it reaches his house and can't find your WiFi, **it opens setup mode by itself** after 45 seconds.
3. Post it with the USB cable. He needs any USB phone charger.
4. He joins **SubCounter-Setup** on his phone, picks his WiFi and saves. If you'd already filled in the channel and API key, they're kept, and he only has to choose his WiFi.

If his WiFi ever changes, the counter opens setup mode by itself again. He can also hold a finger on the screen while plugging it in.

---

## Remote updates (GitHub)

Once a day each counter checks a GitHub page for a newer version, installs it and restarts. Nobody has to do anything at your brother's end.

### One-time setup

1. Make a free account at <https://github.com>.
2. Click **+ → New repository**. Name it `subcounter`, choose **Public** and click **Create repository**.
3. Tell Claude your GitHub username. Claude puts it into `UPDATE_REPO` in `app.h`, for example `"yourname/subcounter"`. **Do this before uploading the program to your brother's board.** A board can only update from the address it was given.

The update file contains no WiFi passwords or API keys, because those are only stored on each board, so it's safe for it to be public.

### Releasing a new version

1. Claude changes the program, increases `FIRMWARE_VERSION` in `app.h` (for example to `2.0.1`) and runs `.\build.ps1`. That creates `release\firmware.bin`.
2. On GitHub, open your `subcounter` repository and go to **Releases → Draft a new release**.
3. Under **Choose a tag**, type `v2.0.1` (a **v** plus the version) and pick **Create new tag**.
4. Drag in `release\firmware.bin`. The name must stay `firmware.bin`.
5. **Test it on your counter first:** tick **Set as a pre-release** and click **Publish release**.
   - Your counter has **Get test versions first** ticked in Settings, so it picks this up. To install it straight away, open its settings page and press **Check for updates now**.
   - Your brother's counter ignores pre-releases.
6. When you're happy with it, edit the release, untick **pre-release**, tick **Set as the latest release** and save. Your brother's counter installs it within a day.

**It's safe if something goes wrong:**
- If a download fails, the counter keeps its current version.
- If a new version crashes or can't load stats when it starts, the counter **automatically goes back to the previous version** the next time it restarts.

---

## Troubleshooting

| What you see | Fix |
|---|---|
| "Can't load your channel: API key not valid" | Copy the key again into Settings. |
| "API not enabled for this key (403)" | Enable **YouTube Data API v3** in the same Google project as the key. |
| "YouTube daily limit used up" | The counter has used its 10,000 units for the day. It carries on after the limit resets, which is 8am UK time. Choose a slower **Check for new numbers** setting. |
| "Channel not found" | Check the @handle spelling. You can also paste the channel ID that starts with `UC`. |
| Colours look like a photo negative, or the screen stays blank | In Settings, change **Screen type**. Boards with a USB-C port often need one of the other options. |
| It went into setup mode by itself | It couldn't join the WiFi. Check the name and password; it must be a 2.4GHz network. If the router was just off, it goes back to normal by itself after 5 minutes. |
| Red dot in the top-right corner | The last update failed. Hold the screen for 2 seconds to see why. It keeps showing the last numbers. |
| Amber dot in the top-right corner | The YouTube daily limit is used up. It carries on later by itself. |
| The upload stops at `Connecting...` | Hold the **BOOT** button on the board while it uploads. |

## Good to know

- **Subscriber counts are rounded by YouTube.** Up to 1,000 they're exact. Above that, YouTube only gives out the first 3 digits, so at 800,000 the count moves in steps of 1,000. Every counter that uses YouTube's public data has this limit. Views aren't rounded, which is why views today and views per hour move a lot more.
- **Today, this week and the chart fill in over time.** The counter takes a snapshot each morning and keeps 35 days of them, even through restarts and updates.
- **Weather** comes from Open-Meteo and **crypto** from CoinGecko. Both are free and need no account.
