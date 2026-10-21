# YouTube Counter, version 2.1

A desk screen for your YouTube stats and much more, built on the ESP32 "Cheap Yellow Display" (ESP32-2432S028R).

You set it up from your phone, so there's no code to edit. It updates itself over the internet, so a counter you've given away keeps getting new versions. It also comes with three colour themes: **Stealth glow**, which is navy with teal-to-blue gradients and purple glows, **Classic dark** and **Purple haze**.

## What it shows

The pages change every 10 seconds. **Tap the screen** to skip to the next one, and use the buttons on the interactive pages. You can switch every page on or off in Settings, and pages that need setting up only appear once they're set up.

| Section | Pages |
|---|---|
| **YouTube** | Subscribers with a milestone bar · Channel stats · Latest upload, with views per hour · Recent uploads · Most viewed video · Newest comment · 7-day chart · Head to head · Live and premieres · **Exact stats** (gained and lost subscribers, watch time, revenue) |
| **Home** | Clock · Weather · **Alerts** (frost, Met Office warnings, MOT/tax due) · **Air quality and pollen** · **Energy** (Octopus usage and cost) · **Cheapest petrol** with a price trend · **Google Calendar** · **Countdowns** |
| **You and your brother** | **Messages** (quick replies, typing with the stylus, or sending from your phone) · **Friendship lamp** · **Desk pet**, which visits his pet |
| **Fun** | **Spotify now playing** · **Free Epic games** · **Joke or fact of the day** · **Magic 8-ball** · **Stopwatch** · **Screensaver** (aquarium or fireplace) · **Mood lamp** · Crypto prices |

**Seasonal decorations** appear at Christmas, Halloween, New Year, Valentine's and Easter, and the pet dresses up for them too.

**Buttons:**
- **Tap:** next page, or press a button on the page. At night, a tap lights the screen up for 30 seconds.
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

**One key is enough for both counters.** The settings page shows how much of the free daily allowance (10,000 units) each counter uses. At the default settings it's about 2,500 a day each.

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
4. Fill in your WiFi, your channel's @handle and the API key, then press **Save and restart**.
5. The first time it starts, it asks you to **tap four arrows** to line up the touch screen. Use the stylus from the box. If you don't tap anything, it skips this after 30 seconds.

**To change settings later:** hold a finger on the screen for 2 seconds. The info screen shows the counter's address (something like `http://192.168.1.50`). Open that on any phone or PC on the same WiFi.

---

## Setting up the extras

Each of these is optional. Fill them in on the settings page whenever you like.

### Your town (weather, air quality, petrol)
Type your town in **Your area and time**. The counter looks up its rough position once and remembers it. Pick your **Met Office warnings** region from the list.

### Octopus Energy (usage and cost)
1. On the Octopus website, go to **Account → Personal details → API access**. Your API key starts with `sk_live_`.
2. Your account number (`A-...`) is at the top of your Octopus account page.
3. Put both into **Energy and petrol**. Smart meter readings usually arrive a day late, so the page shows yesterday's cost and the last week's.

### Google Calendar
This uses a tiny script that runs in your own Google account. It only gives the counter the next week's event titles and times.
1. Go to <https://script.google.com>, click **New project**, delete what's there and paste this:
   ```javascript
   function doGet() {
     const now = new Date();
     const end = new Date(now.getTime() + 7 * 24 * 3600 * 1000);
     const events = [];
     CalendarApp.getAllCalendars().forEach(cal => {
       if (!cal.isSelected()) return;
       cal.getEvents(now, end).forEach(e => events.push({
         t: e.getTitle(), s: Math.floor(e.getStartTime() / 1000),
         e: Math.floor(e.getEndTime() / 1000), a: e.isAllDayEvent()
       }));
     });
     events.sort((x, y) => x.s - y.s);
     return ContentService.createTextOutput(JSON.stringify(events.slice(0, 8)))
       .setMimeType(ContentService.MimeType.JSON);
   }
   ```
2. Click **Deploy → New deployment**. Choose the type **Web app**, set *Execute as* to **Me** and *Who has access* to **Anyone**, then click **Deploy** and allow access.
3. Copy the **Web app URL** (`https://script.google.com/macros/s/.../exec`) into **Google Calendar link** in Settings.

Anyone who has that link can see your next week's event titles, so treat it like a password. It's only stored on the counter.

### MOT, car tax and countdowns
Type the due dates into **Calendar and countdowns**. They're never looked up from your number plate. Add your own countdowns one per line, like `Holiday=2026-12-20`. Write just month-day, like `Mum's birthday=03-02`, for something that comes round every year. MOT and tax also show up on the **Alerts** page in the last two weeks before they're due.

### Messages and friendship lamp (with your brother)
1. Each counter makes its own random **pairing code** (like `K7QM-2XPA-9RTD-4WCE`). It's under **Messages and friendship lamp**.
2. Copy **one** counter's code into the other counter's settings, so both have the **same** code.
3. Put your name in **Your name**, so messages are signed.

Messages are **encrypted on the board** (AES-256) with a key made from that code, then passed through the free ntfy.sh relay. The relay only ever sees scrambled text. **Glow** lights his screen softly in your lamp colour for 10 minutes. He taps it to glow back.

### Desk pet: classic or cat
Under **Pet type**, pick a **Cat** or the **Classic** pixel pet. Either way it hatches (or wakes up in its basket) 3 minutes after you turn it on, then grows up over 3 days. Feed it, play with it, clean up after it, give it medicine when it's ill, and turn the light off at bedtime. Choose **Gentle** rules (it never dies) or **Old-school** rules. Once a day, a happy pet visits your brother's pet and brings a present.

**The cat** gets a random coat when the kitten first wakes up: ginger, tabby, black, white, tuxedo or calico. It also does cat things:
- **Slide your finger or the stylus across it** to stroke it. It purrs and hearts float up.
- **Play → Laser:** tap around the screen and it chases the red dot.
- **Tap it** and it says "Mrrp?"
- On its own, it grooms, stretches, has cat naps, gets the zoomies, sits in a box, chases its tail, kneads, yawns, slow-blinks at you, bats a ball of yarn, and knocks your mug off the table when it's being ignored.
- Every so often it **strolls across the bottom of whatever page is showing**, sometimes stopping to meow. Now and then it **naps on the title bar**.

### Spotify now playing and exact YouTube stats
These need a one-time sign-in on your PC with `connect-accounts.ps1`. The sign-in pass it gives you is only stored on the counter.

**Spotify:**
1. Go to <https://developer.spotify.com/dashboard>, click **Create app**, name it `SubCounter`, add the redirect URI `http://127.0.0.1:8888/callback`, and tick **Web API**.
2. In PowerShell, from this folder, run `.\connect-accounts.ps1 -Spotify` and paste the app's **Client ID** when asked.
3. Copy the two lines it prints into Settings.

**Exact YouTube stats:**
1. In the same Google Cloud project as your API key, enable **YouTube Analytics API**.
2. Go to **APIs & Services → OAuth consent screen**. Choose **External**, fill in the app name, then add yourself under *Test users*. Afterwards, click **Publish app**: in testing mode the sign-in only lasts 7 days.
3. Go to **Credentials → Create credentials → OAuth client ID**, choose **Desktop app** and copy the ID and secret.
4. Run `.\connect-accounts.ps1 -YouTube` and sign in with the Google account that owns the channel. Google will warn that the app isn't verified, because it's your own app; click **Advanced → Go to SubCounter**.
5. Copy the three lines it prints into Settings.

---

## Your brother's counter

1. **Upload the program at your house** with `.\flash.ps1 -Fresh`. That way it's checked before you post it.
2. Put the **same pairing code** on both counters before posting, if you want messages and the pet visits to work.
3. Post it with the USB cable. He needs any USB phone charger.
4. When it can't find your WiFi at his house, **it opens setup mode by itself**. He joins **SubCounter-Setup** on his phone, picks his WiFi and saves.

---

## Remote updates (GitHub)

Once a day each counter checks the GitHub releases in `UPDATE_REPO` (set in `app.h`) and installs newer versions by itself.

### Releasing a new version
1. Claude changes the program, increases `FIRMWARE_VERSION` in `app.h` (for example to `2.1.1`) and runs `.\build.ps1`. That creates `release\firmware.bin`.
2. On GitHub, open the repository and go to **Releases → Draft a new release**, and tag it `v2.1.1`. Attach `release\firmware.bin`, which must keep that name.
3. **Test it on your counter first:** tick **Set as a pre-release** and publish. Your counter has **Get test versions first** ticked, so it picks it up. Press **Check for updates now** on its settings page to install it straight away.
4. When you're happy with it, edit the release, untick pre-release and tick **Set as the latest release**. Your brother's counter installs it within a day.

**It's safe if something goes wrong:**
- If a download fails, the counter keeps its current version.
- If a new version crashes or can't load stats when it starts, the counter **automatically goes back to the previous version**.

---

## What each feature sends, and where

Your WiFi password, API keys, sign-in passes and pairing code are only stored on the board, never in the program or on GitHub. The table below covers everything the counter sends out.

| Feature | Goes to | What it sends |
|---|---|---|
| YouTube pages | Google (YouTube API) | Your channel name and API key |
| Exact stats | Google (YouTube Analytics) | Your sign-in pass |
| Weather, air quality | Open-Meteo | Your town's position |
| Petrol | checkfuelprices.co.uk | Your town's position, rounded to about 1 km |
| Met Office warnings | Met Office | The region you picked |
| Energy | Octopus Energy | Your account number and API key |
| Calendar | Your own Google script | Nothing; it only reads |
| Messages, lamp, pet visits | ntfy.sh | Encrypted text the relay can't read |
| Spotify | Spotify | Your sign-in pass |
| Free games, jokes, facts, crypto | Epic, icanhazdadjoke, uselessfacts, CoinGecko | Nothing personal |
| Updates | GitHub | Nothing personal |

---

## Troubleshooting

| What you see | Fix |
|---|---|
| "Can't load your channel: API key not valid" | Copy the key again into Settings. |
| "API not enabled for this key (403)" | Enable **YouTube Data API v3** in the same Google project as the key. |
| "YouTube daily limit used up" | It carries on after the limit resets at 8am UK time. Choose a slower **Check for new numbers** setting. |
| "Channel not found" | Check the @handle spelling, or paste the channel ID that starts with `UC`. |
| Colours look like a photo negative, or the screen stays blank | Change **Screen type** in Settings. |
| Buttons react in the wrong place | Tick **Line up the touch screen again** in Settings and save. |
| It went into setup mode by itself | It couldn't join the WiFi, which must be 2.4GHz. If the router was just off, it goes back to normal by itself after 5 minutes. |
| Messages don't arrive | Both counters need exactly the same pairing code. |
| "Sign-in expired" on Spotify or Exact stats | Run `connect-accounts.ps1` again. For YouTube, make sure the Google app is **published**, not in testing mode. |
| Red dot in the top-right corner | The last update failed. Hold the screen for 2 seconds to see why. It keeps showing the last numbers. |
| The upload stops at `Connecting...` | Hold the **BOOT** button on the board while it uploads. |

## Good to know

- **YouTube rounds public subscriber counts.** Above 1,000 you get the first 3 digits, so at 800,000 it moves in steps of 1,000. The **Exact stats** page uses your own YouTube Analytics data, which is exact but a day or two behind.
- **Today, this week, the chart and the petrol trend fill in over time.** The counter keeps a snapshot each day and remembers it through restarts and updates.
- **Prices:** petrol prices come from checkfuelprices.co.uk, weather and air quality from Open-Meteo, and crypto from CoinGecko. All are free, with no account needed.
