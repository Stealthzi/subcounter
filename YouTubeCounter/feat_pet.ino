// The desk pet: an old-school virtual pet that lives on the counter.
//
// It hatches from an egg and grows from baby to child, teen and adult over
// three days. Its adult look depends on how well it was looked after (care
// mistakes). It gets hungry and bored, poos, falls ill and goes to bed at
// night. Tap the buttons to feed it, play, clean up, give medicine and turn
// the light off. Strict ("old-school") rules: neglect it for long enough and
// it dies. Gentle rules: it just gets sad.
// Extras: weather outfits, a dance for new subscribers, and visits (with
// presents) to the pet on your brother's counter.



int           gameRound = 0, gameWins = 0, petFacing = 0;  // facing: -1 left, 1 right
unsigned long gameResultUntil = 0;
bool          gameLastWon = false;
unsigned long petCelebrateUntil = 0;
String        petSpeech;
unsigned long petSpeechUntil = 0;
String        visitorName, visitorPet;
uint8_t       visitorStage = 0, visitorForm = 0, visitorType = 0, visitorCoat = 0;
unsigned long visitorUntil = 0;
unsigned long lastPetSave = 0;

// ---------------------------------------------------------------- sprites (16x16)
// . clear   X body   k black   w white   p pink

const char* SPR_EGG[16] = {
  "................", "......wwww......", "....wwwwwwww....", "...wwwwwwwwww...",
  "...wwXwwwwwwww..", "..wwXXwwwwXwww..", "..wwwwwwwXXwww..", "..wwwwwwwwwwww..",
  "..wwwXwwwwwwww..", "..wwXXwwwwwXww..", "...wwwwwwwwww...", "...wwwwwwwwww...",
  "....wwwwwwww....", "......wwww......", "................", "................"};
const char* SPR_BABY[16] = {
  "................", "................", "................", "................",
  "................", "......XXXX......", "....XXXXXXXX....", "...XXXXXXXXXX...",
  "..XXXkXXXXkXXX..", "..XXXkXXXXkXXX..", "..XXpXXXXXXpXX..", "..XXXXXppXXXXX..",
  "...XXXXXXXXXX...", "....XXXXXXXX....", "................", "................"};
const char* SPR_CHILD[16] = {
  "................", "................", ".....XXXXXX.....", "....XXXXXXXX....",
  "...XXXXXXXXXX...", "..XXXkkXXkkXXX..", "..XXXkwXXkwXXX..", "..XXpXXXXXXpXX..",
  "..XXXXXkkXXXXX..", "..XXXXXXXXXXXX..", "...XXXXXXXXXX...", "....XXXXXXXX....",
  "....XX....XX....", "...XXX....XXX...", "................", "................"};
const char* SPR_TEEN[16] = {
  "................", "..XX........XX..", "..XXX......XXX..", "..XXXXXXXXXXXX..",
  ".XXXXXXXXXXXXXX.", ".XXXkkXXXXkkXXX.", ".XXXkwXXXXkwXXX.", ".XXpXXXXXXXXpXX.",
  ".XXXXXXkkXXXXXX.", ".XXXXXkXXkXXXXX.", "..XXXXXXXXXXXX..", "...XXXXXXXXXX...",
  "...XX......XX...", "..XXX......XXX..", "................", "................"};
const char* SPR_ADULT[16] = {
  "................", "...XX......XX...", "...XXX....XXX...", "..XXXXXXXXXXXX..",
  ".XXXXXXXXXXXXXX.", ".XXXkkXXXXkkXXX.", "XXXXkwXXXXkwXXXX", "XXXpXXXXXXXXpXXX",
  ".XXXXXXkkXXXXXX.", ".XXXXXXXXXXXXXX.", "..XXXXXXXXXXXX..", "..XXXXXXXXXXXX..",
  "...XXXXXXXXXX...", "...XX......XX...", "..XXX......XXX..", "................"};
const char* SPR_GHOST[16] = {
  "................", "................", ".....wwwwww.....", "....wwwwwwww....",
  "...wwkwwwwkwww..", "...wwkwwwwkwww..", "...wwwwwwwwwww..", "...wwwwkkwwwww..",
  "...wwwwwwwwwww..", "...wwwwwwwwwww..", "...wwwwwwwwwww..", "...ww.ww.ww.ww..",
  "................", "................", "................", "................"};
const char* SPR_POOP[8] = {
  "...X....", "..XX....", "..XXX...", ".XXXXX..", ".XXXXXX.", "XXXXXXXX", "........", "........"};

const char** petSprite(uint8_t stage) {
  switch (stage) {
    case PET_EGG:   return SPR_EGG;
    case PET_BABY:  return SPR_BABY;
    case PET_CHILD: return SPR_CHILD;
    case PET_TEEN:  return SPR_TEEN;
    case PET_ADULT: return SPR_ADULT;
  }
  return SPR_GHOST;
}

uint32_t petBodyColour(uint8_t stage, uint8_t form) {
  switch (stage) {
    case PET_EGG:   return 0x2EE6A8;
    case PET_BABY:  return 0x7FE7FF;
    case PET_CHILD: return 0x2EE6A8;
    case PET_TEEN:  return 0x3DA5FF;
    case PET_ADULT: return form == 0 ? 0xB57BFF : form == 1 ? 0x4FD1C5 : 0xFF8A3D;
  }
  return 0xE6ECFF;
}

// Draws a 16x16 sprite into the canvas, scale s, optionally mirrored, eyes shut when asleep
void drawSprite(const char** spr, int x0, int y0, int s, uint32_t body, bool flip, bool asleep, float dim) {
  uint16_t cBody = mix(body, 0x000000, dim);
  uint16_t cBlack = mix(0x101018, 0x000000, dim);
  uint16_t cWhite = mix(0xFFFFFF, 0x000000, dim);
  uint16_t cPink = mix(0xFF7AA8, 0x000000, dim);
  for (int r = 0; r < 16; r++) {
    for (int c = 0; c < 16; c++) {
      char ch = spr[r][flip ? 15 - c : c];
      if (ch == '.') continue;
      uint16_t col = cBody;
      if (ch == 'k' || ch == 'w') {
        col = ch == 'k' ? cBlack : cWhite;
        if (asleep) {
          // shut eyes: only the bottom row of each eye shows, as a line
          char below = r < 15 ? spr[r + 1][flip ? 15 - c : c] : '.';
          col = (below == 'k' || below == 'w') ? cBody : cBlack;
        }
      } else if (ch == 'p') {
        col = cPink;
      }
      canvas.fillRect(x0 + c * s, y0 + r * s, s, s, col);
    }
  }
}

// ---------------------------------------------------------------- saving

void savePet() {
  prefs.putBytes("pet", &pet, sizeof(pet));
  lastPetSave = millis();
}

void newEgg() {
  memset(&pet, 0, sizeof(pet));
  uint32_t now = (uint32_t)time(nullptr);
  pet.stage = PET_EGG;
  pet.born = now;
  pet.lastTick = now;
  savePet();
}

void loadPet() {
  if (prefs.getBytesLength("pet") == sizeof(pet)) prefs.getBytes("pet", &pet, sizeof(pet));
  else memset(&pet, 0, sizeof(pet));
  if (pet.stage == PET_NONE && pageEnabled(PAGE_PET) && time(nullptr) > 1700000000) newEgg();
}

// ---------------------------------------------------------------- life simulation

uint32_t hungerInterval() {
  switch (pet.stage) {
    case PET_BABY:  return 30 * 60;
    case PET_CHILD: return 60 * 60;
    case PET_TEEN:  return 80 * 60;
  }
  return 100 * 60;
}

int bedHour() {
  return pet.stage == PET_CHILD ? 20 : pet.stage == PET_TEEN ? 21 : 22;
}

bool petIsAlive() {
  return pet.stage >= PET_BABY && pet.stage <= PET_ADULT;
}

void hatch(uint32_t t) {
  pet.stage = PET_BABY;
  pet.hunger = 2;
  pet.happy = 2;
  pet.weight = 5;
  pet.nextHunger = t + hungerInterval();
  pet.nextHappy = t + hungerInterval() * 6 / 5;
  pet.nextPoop = t + 90 * 60;
  if (cfg.petType == 1) {
    // The surprise: what colour is the kitten?
    pet.spare[0] = esp_random() % 6;
    pet.spare[1] = 1;
    petSay("It's a " + String(COAT_NAMES[pet.spare[0]]) + " kitten!", 6000);
  } else {
    petSay("Hello!", 4000);
  }
}

// One simulated minute at time t
void petStep(uint32_t t) {
  if (pet.stage == PET_NONE || pet.stage == PET_GONE) return;
  uint32_t age = t - pet.born;
  if (pet.stage == PET_EGG) {
    if (age >= 180) hatch(t);
    return;
  }

  // Growing up
  uint8_t want = age < 3600 ? PET_BABY : age < 86400 ? PET_CHILD : age < 3 * 86400 ? PET_TEEN : PET_ADULT;
  if (want > pet.stage) {
    pet.stage = want;
    if (want == PET_ADULT) pet.form = pet.mistakes <= 3 ? 0 : pet.mistakes <= 8 ? 1 : 2;
    pet.weight += 3;
  }

  // Bedtime (babies just nap when they like)
  time_t tt = t;
  struct tm lt;
  localtime_r(&tt, &lt);
  bool night = pet.stage >= PET_CHILD && (lt.tm_hour >= bedHour() || lt.tm_hour < 8);
  if (night && !pet.asleep) {
    pet.asleep = 1;
    if (!cfg.petStrict) pet.lightsOff = 1;  // gentle rules turn the light off for you
  }
  if (!night && pet.asleep) {
    pet.asleep = 0;
    pet.lightsOff = 0;
  }

  if (pet.asleep) {
    // Nothing wears off overnight
    pet.nextHunger += 60;
    pet.nextHappy += 60;
    pet.nextPoop += 60;
  } else {
    if (t >= pet.nextHunger) {
      if (pet.hunger > 0) pet.hunger--;
      pet.nextHunger = t + hungerInterval();
    }
    if (t >= pet.nextHappy) {
      if (pet.happy > 0) pet.happy--;
      pet.nextHappy = t + hungerInterval() * 6 / 5;
    }
    if (t >= pet.nextPoop) {
      if (pet.poops < 4) pet.poops++;
      pet.nextPoop = t + 3 * 3600 + (t % 3600);
    }
  }
  if (pet.hunger == 0 && !pet.hungryAt) pet.hungryAt = t;
  if (pet.hunger > 0) pet.hungryAt = 0;
  if (pet.happy == 0 && !pet.sadAt) pet.sadAt = t;
  if (pet.happy > 0) pet.sadAt = 0;

  // Falling ill: a filthy home, or going hungry for hours
  if (!pet.sick && (pet.poops >= 4 || (pet.hungryAt && t - pet.hungryAt > 3 * 3600))) {
    pet.sick = 2;
    pet.sickAt = t;
  }

  // Calling for you. Ignoring a call for 15 minutes counts as a care mistake.
  bool needs = pet.hunger == 0 || pet.happy == 0 || pet.sick || pet.poops >= 3 || (pet.asleep && !pet.lightsOff);
  if (needs) {
    if (!pet.calledAt) {
      pet.calledAt = t;
      pet.callCounted = 0;
    } else if (!pet.callCounted && t - pet.calledAt >= 15 * 60) {
      if (pet.mistakes < 255) pet.mistakes++;
      pet.callCounted = 1;
    }
  } else {
    pet.calledAt = 0;
    pet.callCounted = 0;
  }

  // Old-school rules: long neglect is fatal
  if (cfg.petStrict && ((pet.hungryAt && t - pet.hungryAt >= 12 * 3600) ||
                        (pet.sick && t - pet.sickAt >= 24 * 3600) ||
                        (pet.sadAt && t - pet.sadAt >= 24 * 3600))) {
    pet.stage = PET_GONE;
  }
}

// Called every minute: catches the pet's life up to now (even after being unplugged)
void petMinute() {
  if (pet.stage == PET_NONE) {
    if (pageEnabled(PAGE_PET)) newEgg();
    return;
  }
  uint32_t now = (uint32_t)time(nullptr);
  if (now < 1700000000 || now <= pet.lastTick) return;
  uint32_t from = pet.lastTick;
  if (now - from > 7UL * 86400) from = now - 7UL * 86400;
  uint8_t stageBefore = pet.stage;
  for (uint32_t t = from + 60; t <= now; t += 60) petStep(t);
  pet.lastTick = now;
  if (pet.stage != stageBefore || millis() - lastPetSave > 5UL * 60 * 1000) savePet();
  if (pet.stage != stageBefore && pet.stage != PET_GONE) petSay("I grew up!", 5000);

  // Once a day, a happy pet pops round to your brother's pet with a present
  if (petIsAlive() && !pet.asleep && pet.happy >= 3 && pageEnabled(PAGE_MESSAGES) && cfg.msgCode.length()) {
    uint32_t today = localDay();
    if (prefs.getUInt("petvisit", 0) != today && localHour() >= 12 && (now % 3600) < 60) {
      prefs.putUInt("petvisit", today);
      sendPetVisit();
    }
  }
}

bool petNeedsYou() {
  return petIsAlive() && pet.calledAt != 0;
}

bool petBusy() {
  return petMenu == MENU_GAME || petMenu == MENU_LASER;
}

void petSay(const String& text, unsigned long ms) {
  petSpeech = text;
  petSpeechUntil = millis() + ms;
}

void petCelebrate() {
  if (!petIsAlive()) return;
  petCelebrateUntil = millis() + 20000;
  petSay("New subscriber!", 5000);
}

void petLeave() {
  petMenu = MENU_MAIN;
}

// ---------------------------------------------------------------- playdates

const char* GIFTS[] = {"a cookie", "a flower", "a shiny star", "a ball", "a cupcake", "a leaf", "a pebble", "a drawing"};

void sendPetVisit() {
  JsonDocument d;
  d["n"] = cfg.petName;
  d["s"] = pet.stage;
  d["f"] = pet.form;
  d["t"] = cfg.petType;
  d["c"] = pet.spare[0];
  d["g"] = GIFTS[esp_random() % 8];
  String s;
  serializeJson(d, s);
  if (sendMessage("visit", s)) petSay("Off to visit!", 4000);
}

void petOnMessage(const String& kind, const String& from, const String& text) {
  JsonDocument d;
  if (deserializeJson(d, text)) return;
  visitorName = from;
  visitorPet = cleanText(d["n"] | "Friend");
  visitorStage = d["s"] | (int)PET_CHILD;
  visitorForm = d["f"] | 0;
  visitorType = d["t"] | 0;
  visitorCoat = d["c"] | 0;
  visitorUntil = millis() + 30UL * 60 * 1000;
  String gift = cleanText(d["g"] | "");
  if (petIsAlive()) {
    if (pet.happy < 4) pet.happy++;
    savePet();
  }
  petSay(visitorPet + " brought " + (gift.length() ? gift : String("a present")) + "!", 15000);
  if (pageEnabled(PAGE_PET) && !userBusy() && !lampGlowing()) showPage(PAGE_PET);
}

// ---------------------------------------------------------------- drawing

bool ensureCanvas(int w, int h) {
  if (canvas.width() == w && canvas.height() == h) return true;
  canvas.deleteSprite();
  canvas.setColorDepth(8);
  return canvas.createSprite(w, h) != nullptr;
}

void drawHeart(int x, int y, bool full) {
  uint16_t c = full ? RGB565(255, 90, 140) : C_LINE;
  canvas.fillCircle(x + 3, y + 3, 3, c);
  canvas.fillCircle(x + 9, y + 3, 3, c);
  canvas.fillTriangle(x, y + 4, x + 12, y + 4, x + 6, y + 11, c);
}

void drawPetStatus() {
  canvas.setTextSize(1);
  canvas.setFont(F_SMALL);
  canvas.setTextDatum(D_TL);
  canvas.setTextColor(C_GREY);
  canvas.drawString("Food", 8, 4);
  for (int i = 0; i < 4; i++) drawHeart(44 + i * 15, 6, i < pet.hunger);
  canvas.drawString("Fun", 112, 4);
  for (int i = 0; i < 4; i++) drawHeart(140 + i * 15, 6, i < pet.happy);
  uint32_t days = ((uint32_t)time(nullptr) - pet.born) / 86400 + 1;
  canvas.setTextDatum(D_TR);
  canvas.drawString(("Day " + String(days) + "  " + String(pet.weight) + "g").c_str(), W - 8, 4);
}

// Weather outfits: umbrella in the rain, scarf when cold, shades when sunny
void drawOutfit(int px, int py, int s, bool flip) {
  if (!weather.loaded || pet.stage < PET_BABY || pet.stage > PET_ADULT) return;
  int c = weather.code;
  bool rain = (c >= 51 && c <= 67) || (c >= 80 && c <= 82) || c >= 95;
  bool snow = (c >= 71 && c <= 77) || c == 85 || c == 86;
  float tempC = cfg.fahrenheit ? (weather.temp - 32) * 5 / 9 : weather.temp;
  int cx = px + 8 * s;
  if (rain) {
    canvas.fillCircle(cx, py - 2, 9 * s, RGB565(155, 92, 255));
    canvas.fillRect(cx - 9 * s, py - 2, 18 * s + 1, 9 * s + 2, mix(theme().bgTop, theme().bgBottom, 0.3f));
    canvas.fillRect(cx - 1, py - 2, 3, 6 * s, C_CLOUD);
  } else if (snow || tempC <= 5) {
    int row = pet.stage == PET_BABY ? 12 : 10;
    canvas.fillRect(px + 3 * s, py + row * s, 10 * s, s + 2, C_RED);
    canvas.fillRect(px + (flip ? 4 : 10) * s, py + row * s, 2 * s, 3 * s, C_RED);
    if (snow) {
      canvas.fillTriangle(cx - 4 * s, py + 2 * s, cx + 4 * s, py + 2 * s, cx, py - 3 * s, C_RED);
      canvas.fillCircle(cx, py - 3 * s, s, C_WHITE);
    }
  } else if (weather.isDay && c <= 1 && tempC >= 18) {
    int row = pet.stage == PET_BABY ? 8 : pet.stage == PET_CHILD ? 5 : 5;
    canvas.fillRect(px + 3 * s, py + row * s, 10 * s, 2 * s, RGB565(10, 10, 14));
  }
}

void drawSpeech(const String& text, int x, int y) {
  canvas.setFont(F_SMALL);
  int w = canvas.textWidth(text.c_str()) + 16;
  if (x + w > W - 4) x = W - 4 - w;
  canvas.fillRoundRect(x, y, w, 22, 8, C_WHITE);
  canvas.setTextDatum(D_ML);
  canvas.setTextColor(RGB565(20, 20, 30));
  canvas.drawString(text.c_str(), x + 8, y + 11);
}

void petFrame() {
  if (!ensureCanvas(W, PET_SCENE_H)) return;
  petFrameNo++;
  unsigned long now = millis();

  // Scene background: night sky with the light off, otherwise the theme
  bool dark = pet.lightsOff && pet.asleep;
  for (int y = 0; y < PET_SCENE_H; y++) {
    uint16_t c = dark ? mix(0x02030A, 0x0A0E24, (float)y / PET_SCENE_H) : bgAt(PET_SCENE_Y + y);
    canvas.drawFastHLine(0, y, W, c);
  }
  canvas.fillRect(0, PET_SCENE_H - 14, W, 14, dark ? RGB565(8, 10, 20) : C_CARD);
  drawPetStatus();

  const int s = 6;
  bool celebrating = now < petCelebrateUntil;
  int bounce = pet.asleep ? 0 : celebrating ? ((petFrameNo % 2) ? -14 : 0) : ((petFrameNo % 4) < 2 ? 0 : -4);
  int px = W / 2 - 8 * s + (celebrating ? ((petFrameNo % 4) - 2) * 6 : 0);
  int py = PET_SCENE_H - 14 - 16 * s + bounce + 8;
  bool flip = petMenu == MENU_GAME ? petFacing > 0 : (petFrameNo / 8) % 2;

  // Seasonal touches and the visiting pet
  if (now < visitorUntil) {
    if (visitorType == 1) {
      coatOverride = visitorCoat;
      drawCatSprite(CAT_SIT, 18, PET_SCENE_H - 14 - 15 * 4 + 2, 4, false, false, 0);
      coatOverride = -1;
    } else {
      drawSprite(petSprite(visitorStage), 18, PET_SCENE_H - 14 - 16 * 4 + 6, 4,
                 petBodyColour(visitorStage, visitorForm), true, false, 0);
    }
    canvas.setFont(F_SMALL);
    canvas.setTextDatum(D_TC);
    canvas.setTextColor(C_CLOUD);
    canvas.drawString(fitText(visitorPet, 90).c_str(), 50, 26);
  }

  if (pet.stage == PET_GONE) {
    drawSprite(SPR_GHOST, px, py - 10 - (petFrameNo % 4 < 2 ? 0 : 4), s, 0xFFFFFF, false, false, 0.2f);
    canvas.setFont(F_BOLD);
    canvas.setTextDatum(D_TC);
    canvas.setTextColor(C_CLOUD);
    canvas.drawString((cfg.petName + " has gone to pet heaven").c_str(), W / 2, 30);
  } else if (cfg.petType == 1) {
    ensureCatCoat();
    drawCatScene(PET_SCENE_H - 14, dark);
    // signs and speech follow the cat around
    int cs = pet.stage == PET_BABY ? 4 : pet.stage == PET_CHILD ? 5 : 6;
    px = (int)catX - 8 * cs;
    py = PET_SCENE_H - 14 - 15 * cs;
    if (pet.stage == PET_ADULT && pet.form == 0) {  // star cat: a gold collar and bell
      canvas.fillRect(px + 4 * cs, py + 10 * cs, 8 * cs, cs, C_YELLOW);
      canvas.fillCircle(px + 8 * cs, py + 11 * cs + 2, cs, C_YELLOW);
    }
    if (!dark && (catHabit == H_SIT || catHabit == H_GROOM || catHabit == H_BLINK || catHabit == H_PURR)) {
      drawOutfit(px, py, cs, false);
      drawSeasonalPet(px, py, cs);
    }
  } else {
    drawSprite(petSprite(pet.stage), px, py, s, petBodyColour(pet.stage, pet.form), flip, pet.asleep, dark ? 0.6f : 0);
    if (pet.stage == PET_ADULT && pet.form == 0) {  // star form wears a crown
      int cx = px + 8 * s;
      canvas.fillTriangle(cx - 18, py + 8, cx - 12, py - 4, cx - 6, py + 8, C_YELLOW);
      canvas.fillTriangle(cx - 6, py + 8, cx, py - 8, cx + 6, py + 8, C_YELLOW);
      canvas.fillTriangle(cx + 6, py + 8, cx + 12, py - 4, cx + 18, py + 8, C_YELLOW);
    }
    if (pet.stage == PET_ADULT && pet.form == 2) {  // grumpy brows
      canvas.drawWideLine(px + 3 * s, py + 4 * s, px + 6 * s, py + 5 * s, 2, RGB565(20, 10, 10));
      canvas.drawWideLine(px + 13 * s, py + 4 * s, px + 10 * s, py + 5 * s, 2, RGB565(20, 10, 10));
    }
    if (!dark) drawOutfit(px, py, s, flip);
    drawSeasonalPet(px, py, s);
  }

  // Poos on the floor
  for (int i = 0; i < pet.poops; i++) {
    int x0 = W - 34 - i * 28, y0 = PET_SCENE_H - 14 - 24;
    for (int r = 0; r < 8; r++)
      for (int c = 0; c < 8; c++)
        if (SPR_POOP[r][c] != '.') canvas.fillRect(x0 + c * 3, y0 + r * 3, 3, 3, RGB565(140, 90, 40));
  }

  // Little signs: Zzz, ill, needs you, music notes when dancing
  int signX = px + 16 * s + 4, signY = py + 6;
  canvas.setFont(F_BOLD);
  canvas.setTextDatum(D_TL);
  if (pet.asleep && pet.stage != PET_GONE) {
    canvas.setTextColor(C_CLOUD);
    canvas.drawString(petFrameNo % 4 < 2 ? "z" : "Z z", signX, signY - (petFrameNo % 4) * 2);
  } else if (pet.sick) {
    canvas.fillCircle(signX + 10, signY + 10, 10, C_RED);
    canvas.fillRect(signX + 8, signY + 3, 4, 14, C_WHITE);
    canvas.fillRect(signX + 3, signY + 8, 14, 4, C_WHITE);
  } else if (pet.calledAt && petIsAlive()) {
    canvas.fillCircle(signX + 10, signY + 10, 11, C_AMBER);
    canvas.setTextColor(RGB565(30, 20, 0));
    canvas.setTextDatum(D_MC);
    canvas.drawString("!", signX + 10, signY + 11);
  }
  if (celebrating) {
    canvas.setTextColor(petFrameNo % 2 ? C_ACCENT : C_PURPLE);
    canvas.setTextDatum(D_TL);
    canvas.drawString("~ * ~", px - 50, py + 10 + (petFrameNo % 2) * 6);
  }

  if (now < petSpeechUntil) drawSpeech(petSpeech, px + 8 * s, max(24, py - 30));
  if (petMenu == MENU_GAME && now < gameResultUntil) {
    drawSpeech(gameLastWon ? "Yes! You guessed it" : "Nope!", W / 2 - 60, 30);
  }

  canvas.pushSprite(0, PET_SCENE_Y);
}

void drawPetButtons() {
  fillBg(0, PET_BTN_Y - 4, W, PET_BTN_H + 6);
  if (pet.stage == PET_GONE) {
    drawButton(80, PET_BTN_Y, 160, PET_BTN_H, "New egg", C_GREEN, RGB565(5, 30, 20));
    return;
  }
  if (petMenu == MENU_FEED) {
    drawButton(4, PET_BTN_Y, 100, PET_BTN_H, "Meal", C_CARD, C_WHITE);
    drawButton(110, PET_BTN_Y, 100, PET_BTN_H, "Snack", C_CARD, C_WHITE);
    drawButton(216, PET_BTN_Y, 100, PET_BTN_H, "Back", C_LINE, C_WHITE);
  } else if (petMenu == MENU_LASER) {
    drawButton(80, PET_BTN_Y, 160, PET_BTN_H, "Done", C_LINE, C_WHITE);
  } else if (petMenu == MENU_PLAY) {
    drawButton(4, PET_BTN_Y, 100, PET_BTN_H, cfg.petType == 1 ? "Laser" : "Game", C_CARD, C_WHITE);
    drawButton(110, PET_BTN_Y, 100, PET_BTN_H, "Visit", C_PURPLE, C_WHITE);
    drawButton(216, PET_BTN_Y, 100, PET_BTN_H, "Back", C_LINE, C_WHITE);
  } else if (petMenu == MENU_GAME) {
    drawButton(4, PET_BTN_Y, 150, PET_BTN_H, "< Left", C_CARD, C_WHITE);
    drawButton(166, PET_BTN_Y, 150, PET_BTN_H, "Right >", C_CARD, C_WHITE);
  } else {
    const char* labels[] = {"Feed", "Play", "Clean", "Meds", pet.lightsOff ? "Light on" : "Light"};
    for (int i = 0; i < 5; i++) drawButton(4 + i * 63, PET_BTN_Y, 59, PET_BTN_H, labels[i], C_CARD, C_WHITE);
  }
}

void drawPetPage() {
  petFrame();
  drawPetButtons();
}

// ---------------------------------------------------------------- taps

void petAfterAction() {
  petMinute();  // make the change show up straight away
  pet.calledAt = 0;
  savePet();
  drawPetButtons();
}

void finishLaser() {
  petMenu = MENU_MAIN;
  laserX = laserY = -1;
  if (pet.happy < 4) pet.happy++;
  if (pet.weight > 5) pet.weight--;
  petSay("That was fun!", 2500);
  petAfterAction();
}

bool petTap(int x, int y) {
  if (y < PET_BTN_Y - 8) {
    bool cat = cfg.petType == 1;
    // Laser pointer: the red dot goes where you tap
    if (cat && petMenu == MENU_LASER) {
      laserX = x;
      laserY = y - PET_SCENE_Y;
      return true;
    }
    // A tap on the pet itself: a little pat
    bool onPet = cat ? fabs(x - catX) < 50 : (x > 100 && x < 220);
    if (petIsAlive() && !pet.asleep && onPet) {
      if (cat) catOnTap();
      else petSay("Hehe!", 1500);
      return true;
    }
    return false;  // elsewhere flips the page
  }
  if (petMenu == MENU_LASER) {
    finishLaser();
    return true;
  }
  if (pet.stage == PET_GONE) {
    if (hitBox(x, y, 80, PET_BTN_Y, 160, PET_BTN_H)) {
      newEgg();
      petSay("A new egg!", 3000);
      drawPetButtons();
    }
    return true;
  }
  if (pet.stage == PET_EGG) {
    petSay(cfg.petType == 1 ? "Shh... still asleep in the basket" : "It's still an egg...", 2000);
    return true;
  }

  int col = x * 5 / W;           // which of 5 buttons
  int third = x < 107 ? 0 : x < 213 ? 1 : 2;  // which of 3 buttons
  if (petMenu == MENU_MAIN) {
    if (col == 0) petMenu = MENU_FEED;
    if (col == 1) petMenu = MENU_PLAY;
    if (col == 2) {
      if (pet.poops) petSay("All clean!", 2000);
      else petSay("Already clean", 1500);
      pet.poops = 0;
    }
    if (col == 3) {
      if (pet.sick) {
        pet.sick--;
        petSay(pet.sick ? "One more dose" : "Feeling better!", 2000);
      } else {
        petSay("I'm not ill!", 1500);
      }
    }
    if (col == 4) {
      pet.lightsOff = !pet.lightsOff;
      if (!pet.asleep && pet.lightsOff) petSay("I'm not sleepy!", 1500);
    }
    petAfterAction();
    return true;
  }
  if (petMenu == MENU_FEED) {
    if (third == 2) {
      petMenu = MENU_MAIN;
    } else if (pet.asleep) {
      petSay("Shh... sleeping", 1500);
    } else if (third == 0) {
      if (pet.hunger >= 4) petSay("I'm full!", 1500);
      else {
        pet.hunger++;
        pet.weight++;
        petSay("Yum!", 1500);
      }
    } else {
      if (pet.happy < 4) pet.happy++;
      pet.weight += 2;
      petSay("Sweet!", 1500);
    }
    petAfterAction();
    return true;
  }
  if (petMenu == MENU_PLAY) {
    if (third == 2) {
      petMenu = MENU_MAIN;
    } else if (pet.asleep) {
      petSay("Shh... sleeping", 1500);
    } else if (third == 0 && cfg.petType == 1) {
      petMenu = MENU_LASER;
      laserX = laserY = -1;
      laserUntil = millis() + 30000;
      petSay("Tap anywhere for the laser!", 3000);
    } else if (third == 0) {
      petMenu = MENU_GAME;
      gameRound = gameWins = 0;
      petFacing = 0;
      petSay("Which way will I look?", 3000);
    } else {
      if (!cfg.msgCode.length()) petSay("Pair with your brother first", 2500);
      else sendPetVisit();
    }
    petAfterAction();
    return true;
  }
  if (petMenu == MENU_GAME) {
    if (millis() < gameResultUntil) return true;
    int guess = x < W / 2 ? -1 : 1;
    petFacing = (esp_random() & 1) ? 1 : -1;
    gameLastWon = guess == petFacing;
    if (gameLastWon) gameWins++;
    gameRound++;
    gameResultUntil = millis() + 900;
    if (gameRound >= 5) {
      petMenu = MENU_MAIN;
      if (gameWins >= 3 && pet.happy < 4) pet.happy++;
      if (gameWins == 5 && pet.happy < 4) pet.happy++;
      if (pet.weight > 5) pet.weight--;
      petSay(String(gameWins) + " out of 5! " + (gameWins >= 3 ? "Fun!" : "Again?"), 3000);
      petAfterAction();
    }
    return true;
  }
  return true;
}
