# YouTube Counter, version 2.3

A desk screen for your YouTube stats and much more. It runs on two boards, and the program works out which one it's on by itself:

- **Elecrow CrowPanel Advance 7"** (ESP32-S3 with 16MB storage, 800 × 480 IPS, the recommended one). Everything is drawn twice the size, with big sharp fonts. Sold on Amazon UK as "ELECROW ESP32 Display 800×480, 7" HMI **Advanced**". The cheaper CrowPanel **Basic** 7" is wired differently and isn't supported.
- **ESP32 "Cheap Yellow Display" 2.8"** (ESP32-2432S028R), the small original.

You set it up from your phone, so there's no code to edit. It updates itself over the internet, so a counter you've given away keeps getting new versions. It comes with colour themes: **Stealth glow** (navy with teal-to-blue gradients and purple glows), **Classic dark**, **Purple haze**, **Synthwave**, **Ocean**, **Forest** and **Sunset**, plus **Galaxy** and **Gold rush**, which your pet unlocks at levels 25 and 50.

## What it shows

It starts on the **Overview**, with all the main YouTube numbers on one page, and stays on whatever page you pick (even after a restart). Only you change the page:

- **Arrows** either side (7"), or **swipe** left or right.
- **Home** (the button at the bottom left on the 7", or tap the title bar on either screen) shows every page as a tile. Tap one to go straight there.
- On the 2.8" screen, a **tap** on an empty part of the page also moves to the next one.

If you'd like the pages to change by themselves, pick **Every 10 s** or **Every 30 s** in Settings. Pages that need setting up only appear once they're set up.

| Section | Pages |
|---|---|
| **YouTube** | **Overview** (subscribers, gained today, next milestone, total views, views today, this week, videos, and your latest upload's views, likes, comments, views per hour and vs your usual) · Subscribers with a milestone bar · Channel stats · Latest upload, with views per hour and **how it's doing against your usual** · Recent uploads · Most viewed video · Newest comment · 7-day chart · **Weekly report card** · Head to head · Live and premieres · **Exact stats** (gained and lost subscribers, watch time, revenue) |
| **Home** | Clock · Weather · **Alerts** (frost, Met Office warnings, MOT/tax due) · **Air quality and pollen** · **Energy** (Octopus usage and cost) · **Cheapest petrol** with a price trend · **Google Calendar** · **Countdowns** |
| **You and your brother** | **Messages** (quick replies, typing with the stylus, or sending from your phone) · **Friendship lamp** · **Desk pet**, which visits his pet |
| **Pet adventures** | Levels 1-50 · Quests and daily streaks · Mystery boxes and 91 items to collect · Missions and battles · Dog, hamster and dragon to unlock · Tricks · Mini-games · Badges · A room to decorate |
| **Ask ChatGPT** | Ask about your channel, energy, petrol and more from your phone, or brainstorm video ideas |
| **Fun** | **Spotify now playing** · **Free Epic games** · **Joke or fact of the day** · **Magic 8-ball** · **Stopwatch** · **Screensaver** (aquarium or fireplace) · **Mood lamp** · Crypto prices |

**Seasonal decorations** appear at Christmas, Halloween, New Year, Valentine's and Easter, and the pet dresses up for them too.

**On the screen:**
- **Arrows (7") or swipe:** the page before or after. **Tap (2.8"):** the next page. Buttons on a page work as normal.
- **Home:** every page as a tile, plus **Sleep**.
- **Settings** (the button at the bottom right on the 7", or the tile on Home):
  - change pages by themselves (**Never**, every 10 or 30 seconds)
  - **brightness**
  - **at night**: stay on, dim, or screen off (night hours are set on your phone)
  - **choose screens**: tick the pages you want
  - a **QR code** for the phone settings page
- **Sleep** (on Home or Settings): the screen goes off until you touch it. The touch that wakes it doesn't press anything. At night with "Dim" or "Screen off", a tap lights it up for 30 seconds.
- **Mood lamp:** tap to change the colour (on the 2.8", tap the middle).
- **Hold for 2 seconds:** info screen, showing the version, WiFi signal, any problems and the **settings page address**.
- **Hold for 8 seconds:** setup mode. On the 2.8" board, holding the **BOOT** button on the back for 3 seconds does the same.
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

`-Fresh` wipes the demo software the board came with. Leave it off when re-uploading to a board that's already set up, so it keeps its settings. For the small 2.8" board add `-Board 2.8`.

<details><summary>Doing it with the Arduino IDE instead</summary>

Install the libraries **LovyanGFX** and **ArduinoJson**. Then pick the board settings below and open `YouTubeCounter.ino` and click Upload. The partition setting is important for remote updates.

- **7" CrowPanel Advance:** use `flash.ps1` rather than the IDE. Its storage layout (**ESP SR 16M**) has a space for speech models, which `flash.ps1` puts on along with the program. The settings it uses are board **ESP32S3 Dev Module**, **PSRAM: OPI PSRAM**, **Flash Size: 16MB**, **Partition Scheme: ESP SR 16M (3MB APP/6MB SPIFFS/3.9MB MODEL)** and **USB CDC On Boot: Disabled**.
- **2.8" Cheap Yellow Display:** board **ESP32 Dev Module**, **Partition Scheme: Minimal SPIFFS (1.9MB APP with OTA/128KB SPIFFS)**.
</details>

## 3. Set it up from your phone

1. The screen says **Let's set up** and shows a QR code.
2. On your phone, join the WiFi network **SubCounter-Setup**, or scan the QR code.
3. The setup page opens by itself. If it doesn't, open a browser and go to **192.168.4.1**.
4. Fill in your WiFi, your channel's @handle and the API key, then press **Save and restart**.
5. The first time it starts, it checks the touch screen. On the 7" board you **tap one dot** in the top corner. On the 2.8" board you **tap four arrows**, ideally with the stylus from the box. If you don't tap anything, it skips this after 30 seconds.

**To change settings later:** open **Settings > Phone settings** on the counter and scan the QR code. Or hold a finger on the screen for 2 seconds: the info screen shows the counter's address (something like `http://192.168.1.50`). Open that on any phone or PC on the same WiFi.

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

### Pet adventures
The pet page's buttons are **Feed**, **Play**, **Care** (clean, medicine, light), **Adventure** and **Bag**.

**Levels and XP.** Your pet levels up from 1 to 50 (Newborn to Legend). The bar at the top of its room shows the XP, with your coins, energy and daily streak beside it. XP comes from:
- **Looking after it:** feeding, playing, stroking, cleaning, tricks and mini-games. This is capped each day, so it can't be farmed.
- **Your channel:** new subscribers, a new upload, and a video that beats your usual.
- **Real life:** a day when your electricity use is lower than usual, and filling up when petrol is cheap (tap **I filled up** on the Quests screen).
- **Visiting every day:** a streak bonus. Every 7 days in a row also brings a mystery box.

**Quests.** Three small quests each day and two bigger ones each week. Finishing both weekly quests earns a mystery box.

**Unlocks on the way:**

| Level | Unlock |
|---|---|
| 3, 12, 22 | Mini-games: Catch the treats, Whack-a-mouse, Memory match |
| 5, 12, 18, 25, 32, 40 | New places to explore: Park, Woods, Beach, Mountains, Haunted house, Volcano |
| 10, 20 | A dog, then a hamster (switch pets on the **Pets** screen) |
| 30 | A dragon egg, which hatches an hour after you first pick it |
| 15, 25, 35, 45 | Tricks: dance, backflip, juggling, magic (**Play → Tricks**) |
| 25, 50 | The Galaxy and Gold rush themes |
| 40 | Every pet evolves: bigger, with a sparkling aura |
| Every 5 levels | A mystery box |

**Items.** There are 91 to find, in four rarities: common, rare, epic and legendary. Some only turn up at Christmas, Halloween, New Year, Valentine's or Easter.
- **Kinds:** food, toys, wallpaper, wall pictures, rugs, beds, corner pieces, hats, collars and capes, and treasures.
- **Where they come from:** mystery boxes, missions, battles and the **Shop**, which has five things each day plus mystery boxes.
- **Using them:** tap an item in the **Bag** to see it, and tap again to eat it, wear it or put it in the room. Gear makes the pet stronger in battles.
- **The Book** shows every item you've found, with **?** for the rest.

**Missions.** Send your pet to a place for 1 hour, 4 hours or overnight. It costs some energy, and it doesn't get hungry while it's away. When it gets back it tells you the story, with any loot, coins and XP. Longer trips bring more and better loot.

**Battles.** Cartoon scraps with the critter that lives in each place (squirrels, foxes, crabs, ghosts...) or that place's boss cat.
- **How they go:** they play out by themselves. When the bar under your pet's health fills up, press **Special move** for a big hit.
- **Energy:** each fight costs 20. It comes back 10 an hour, or with food.
- **Rewards:** wins bring coins, XP and sometimes items. Beating a boss always brings an item.

There are also 32 **Badges** to earn. Your progress is kept even if the classic pet's life starts again.

### Ask ChatGPT about your data
On your phone, open the counter's settings page and tap **Ask ChatGPT about your data**. Type a question or use one of the quick ones, for example "How did my last video do?" or "When should I fill up?". You can also tap **Brainstorm video ideas**. The answer shows on your phone, and on the counter if you like.

It needs your own **OpenAI API key**. A ChatGPT subscription doesn't include one.
1. Sign in at <https://platform.openai.com>.
2. Add a little credit under **Billing**. $5 lasts a long time, as each question costs a fraction of a penny.
3. Create a key under **API keys**.
4. Paste it into Settings, in the **Ask ChatGPT** section.

You choose what can be sent with a question: channel stats, energy, petrol, weather, calendar (off unless you tick it) and the pet. Nothing is sent until you ask something, and OpenAI is asked not to store the conversation.

### How your latest video is doing, and the weekly report
- **VS USUAL** on the Latest upload page compares the video's views with your other videos at the same age. 2.0x means twice your usual. The counter notes each new video's views at 6 hours, 1 day, 2 days and a week, so this gets more accurate with every upload. Until then it shows an estimate with a **~**.
- The **Weekly report card** appears after your counter has been running for a week or so. It shows last week's views and subscribers, how many videos you uploaded, and a grade from A+ to D against the week before.

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
1. Claude changes the program, increases `FIRMWARE_VERSION` in `app.h` (for example to `2.2.1`) and runs `.\build.ps1`. That creates `release\firmware-7inch.bin` (7" board) and `release\firmware.bin` (2.8" board).
2. On GitHub, open the repository and go to **Releases → Draft a new release**, and tag it `v2.2.1`. Attach both files, which must keep those names. Each counter downloads the one for its own board.
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
| Ask ChatGPT (only when you ask) | OpenAI | Your question, your API key, and only the data you ticked |
| Updates | GitHub | Nothing personal |

---

## Troubleshooting

| What you see | Fix |
|---|---|
| "Can't load your channel: API key not valid" | Copy the key again into Settings. |
| "API not enabled for this key (403)" | Enable **YouTube Data API v3** in the same Google project as the key. |
| "YouTube daily limit used up" | It carries on after the limit resets at 8am UK time. Choose a slower **Check for new numbers** setting. |
| "Channel not found" | Check the @handle spelling, or paste the channel ID that starts with `UC`. |
| 7" screen stays dark, or touch doesn't work | Read the version printed on the back of the board (e.g. `V1.2`) and set **Board version** in Settings to match. If the screen is dark, join the **SubCounter-Setup** WiFi from your phone to reach Settings. |
| Colours look like a photo negative, or the screen stays blank (2.8" board) | Change **Screen type** in Settings. |
| Buttons react in the wrong place | Tick **Check the touch screen again** (7") or **Line up the touch screen again** (2.8") in Settings and save. |
| It went into setup mode by itself | It couldn't join the WiFi, which must be 2.4GHz. If the router was just off, it goes back to normal by itself after 5 minutes. |
| Messages don't arrive | Both counters need exactly the same pairing code. |
| "Sign-in expired" on Spotify or Exact stats | Run `connect-accounts.ps1` again. For YouTube, make sure the Google app is **published**, not in testing mode. |
| Red dot in the top-right corner | The last update failed. Hold the screen for 2 seconds to see why. It keeps showing the last numbers. |
| The upload stops at `Connecting...` | Hold the **BOOT** button on the board while it uploads. On the 7" board, hold **BOOT**, tap **RESET**, then let go of **BOOT**. |

## Good to know

- **Views today** counts from the counter's first look of the day. If it wasn't on just after midnight, it says **SINCE START** instead. YouTube only updates a channel's total views every few hours, so it can sit at 0 for a while.
- **YouTube rounds public subscriber counts.** Above 1,000 you get the first 3 digits, so at 800,000 it moves in steps of 1,000. The **Exact stats** page uses your own YouTube Analytics data, which is exact but a day or two behind.
- **Today, this week, the chart and the petrol trend fill in over time.** The counter keeps a snapshot each day and remembers it through restarts and updates.
- **Prices:** petrol prices come from checkfuelprices.co.uk, weather and air quality from Open-Meteo, and crypto from CoinGecko. All are free, with no account needed.
- **The 7" screen and its memory.** The board's picture memory (PSRAM) can only just keep up with feeding an 800x480 screen, so busy drawing can make the picture slip for a frame. To keep it steady, animations (the pet, screensaver and 8-ball) are drawn at half size in the chip's fast memory and doubled up on screen, which makes text inside them a little chunkier. For anyone working on the code, `screen_watch.h` counts lost frames; type `s` in the serial monitor at 115200 baud to see them.
