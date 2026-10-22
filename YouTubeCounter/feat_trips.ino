// Missions and battles.
//
// Send the pet off for an hour, four hours or overnight. While it's away it
// doesn't get hungry, and when it gets back it tells you what happened: things
// it found, critters it scrapped with, coins it dug up. Longer trips and
// tougher places bring better loot. New places open up as it levels up.
//
// Battles are cartoon scraps (paws and bonks) with the critters that live in
// each place and the boss cat who rules it. They cost energy, which comes back
// over time or with food. Gear makes the pet stronger.

struct Place {
  const char* name;
  uint8_t     level;      // pet level needed
  uint8_t     critter;    // who lives there
  const char* boss;       // the boss cat
  uint8_t     bossCoat;   // cat coat (see COAT_NAMES)
  uint8_t     bossLook;   // 0 none, 1 eyepatch, 2 crown, 3 pirate hat, 4 scarf, 5 shades
  uint32_t    sky, ground;
};
const Place PLACES[] = {
  {"Garden", 1, 0, "Ginger Tom", 0, 0, 0x7FC8F8, 0x4FA84F},
  {"Park", 5, 1, "Alley Cat", 1, 4, 0x9AD4F5, 0x5DB85D},
  {"Woods", 12, 2, "Wildcat", 1, 1, 0x3A6A5A, 0x2E5A30},
  {"Beach", 18, 3, "Captain Whiskers", 3, 3, 0x6AD4F0, 0xF2D8A0},
  {"Mountains", 25, 4, "Snow Leopard", 5, 4, 0xB8D8F0, 0xE8ECF2},
  {"Haunted house", 32, 5, "Shadow", 2, 5, 0x1A1430, 0x2A2438},
  {"Volcano", 40, 6, "Lava King", 0, 2, 0x3A0A0A, 0x2A1A1A},
};
const int PLACE_COUNT = sizeof(PLACES) / sizeof(PLACES[0]);

int placeCount() { return PLACE_COUNT; }
int placeLevel(int p) { return PLACES[p].level; }
const char* placeName(int p) { return p >= 0 && p < PLACE_COUNT ? PLACES[p].name : ""; }
bool placeOpen(int p) { return p >= 0 && p < PLACE_COUNT && rpg.level >= PLACES[p].level; }

// ---------------------------------------------------------------- critters (16x16)
// . clear  X body  T/Y/C/H accent  G/W shade  k black  w light  p nose

const char* CR_SQUIRREL[16] = {
  "................", "..........TT....", ".........TTTT...", "...XX...TTTTTT..",
  "..XXXX..TTTTTT..", "..XkXX..TTTTT...", ".XXXXXp.TTTTT...", "..XXXX..TTTT....",
  "..XXXXX.TTT.....", "..XwwXXXTT......", "..XwwXXXX.......", "..XXXXXX........",
  "..XX.XX.........", "................", "................", "................"};
const char* CR_PIGEON[16] = {
  "................", "................", ".....XXX........", "....XXkXX.......",
  "....XXXXYY......", ".....XXX........", "....GGGXXX......", "...XXXXXXXXX....",
  "..XXXXXXXXXXXX..", "..XXXXWWWXXXXXX.", "...XXXWWWWXXXXXX", "....XXXXXXXXX...",
  ".....YY..YY.....", "................", "................", "................"};
const char* CR_FOX[16] = {
  "................", "..X.........X...", "..XX.......XX...", "..XXX.....XXX...",
  "..XXXXXXXXXXX...", ".XXXXXXXXXXXXX..", ".XXkwXXXXXkwXX..", ".XXkkXXXXXkkXX..",
  "..wwXXXXXXXww...", "...wwwXkXwww....", "....wwwwwww.....", "...XXXXXXXXX..TT",
  "..XXXXXXXXXXXTTw", "..XXXXXXXXXXXT..", "..XX..XXX..XX...", "................"};
const char* CR_CRAB[16] = {
  "................", "..C..........C..", ".CC..........CC.", ".CCC........CCC.",
  "..CC..w..w..CC..", "...C..k..k..C...", "....XXXXXXXX....", "...XXXXXXXXXX...",
  "..XXXXXXXXXXXX..", "..XXXXXXXXXXXX..", "...XXXXXXXXXX...", "..X.X.X..X.X.X..",
  ".X..X..X.X..X..X", "................", "................", "................"};
const char* CR_GOAT[16] = {
  "................", "..HH......HH....", "...HH....HH.....", "....XXXXXX......",
  "...XXXXXXXX.....", "..XXkwXXkwXX....", "..XXXXXXXXXX....", "...XXXppXXX.....",
  "....XXXXXX......", ".....wwww.......", "...XXXXXXXXXX...", "..XXXXXXXXXXXX..",
  "..XXXXXXXXXXXX..", "..XX.XX..XX.XX..", "..kk.kk..kk.kk..", "................"};
const char* CR_GHOST[16] = {
  "................", ".....XXXXXX.....", "....XXXXXXXX....", "...XXXXXXXXXX...",
  "...XXkkXXkkXX...", "...XXkkXXkkXX...", "...XXXXXXXXXX...", "...XXXXkkXXXX...",
  "...XXXXkkXXXX...", "...XXXXXXXXXX...", "...XXXXXXXXXX...", "...XXXXXXXXXX...",
  "...XX.XXX.XXX...", "...X...X...X....", "................", "................"};
const char* CR_NEWT[16] = {
  "................", "................", "................", "..........XXX...",
  ".........XkXXX..", ".........XXXXXp.", "..XX....XXXX....", ".XXXXXXXXXXX....",
  "XXXYXYXYXXX.....", ".XXXXXXXXXXX....", "..X.X....X.X....", ".X...X..X...X...",
  "................", "................", "................", "................"};

struct Critter {
  const char*  name;
  const char** spr;
  uint32_t     main, accent, shade, light;
};
const Critter CRITTERS[] = {
  {"squirrel", CR_SQUIRREL, 0xB5651D, 0xD98B3A, 0x8A4A14, 0xF2D8B0},
  {"pigeon", CR_PIGEON, 0x8A90A0, 0xF28C28, 0x5A606A, 0xFFFFFF},
  {"fox", CR_FOX, 0xE8722A, 0xE8722A, 0xB0501A, 0xFFFFFF},
  {"crab", CR_CRAB, 0xE04030, 0xE85040, 0xA02A20, 0xFFFFFF},
  {"mountain goat", CR_GOAT, 0xE8E4DA, 0x8A7A60, 0xB0AA9A, 0xC8C4BA},
  {"ghost", CR_GHOST, 0xE8ECF8, 0xE8ECF8, 0xB8C0D8, 0xFFFFFF},
  {"lava newt", CR_NEWT, 0x3A2A2A, 0xFF8A3D, 0x2A1A1A, 0xFFFFFF},
};

void drawCritter(int critter, int x0, int y0, int s, bool flip, float dim) {
  const Critter& cr = CRITTERS[critter];
  for (int r = 0; r < 16; r++) {
    for (int c = 0; c < 16; c++) {
      char ch = cr.spr[r][flip ? 15 - c : c];
      if (ch == '.') continue;
      uint32_t col;
      switch (ch) {
        case 'X': col = cr.main; break;
        case 'T': case 'Y': case 'C': case 'H': col = cr.accent; break;
        case 'G': col = 0x6A4A9A; break;
        case 'W': col = cr.shade; break;
        case 'w': col = cr.light; break;
        case 'p': col = critter == 6 ? cr.accent : 0xFF8FB3; break;
        default:  col = 0x101018; break;
      }
      canvas.fillRect(x0 + c * s, y0 + r * s, s, s, mix(col, 0x000000, dim));
    }
  }
}

// A boss cat: a cat in its own coat with something to show who's boss
void drawBossCat(int place, int x0, int y0, int s, bool flip, float dim) {
  const Place& pl = PLACES[place];
  coatOverride = pl.bossCoat;
  drawCatSprite(CAT_SIT, x0, y0, s, flip, false, dim);
  coatOverride = -1;
  int cx = x0 + 8 * s, eyeL = x0 + (flip ? 10 : 4) * s, top = y0 + 3 * s;
  switch (pl.bossLook) {
    case 1:  // eyepatch
      canvas.fillRect(eyeL, y0 + 6 * s, 2 * s + 2, 2 * s, RGB565(10, 10, 14));
      canvas.drawLine(x0 + 2 * s, y0 + 5 * s, x0 + 14 * s, y0 + 7 * s, RGB565(10, 10, 14));
      break;
    case 2:  // crown
      canvas.fillRect(cx - 5 * s, top - 2 * s, 10 * s, 2 * s, C_YELLOW);
      canvas.fillTriangle(cx - 5 * s, top - 2 * s, cx - 3 * s, top - 5 * s, cx - s, top - 2 * s, C_YELLOW);
      canvas.fillTriangle(cx - 2 * s, top - 2 * s, cx, top - 6 * s, cx + 2 * s, top - 2 * s, C_YELLOW);
      canvas.fillTriangle(cx + s, top - 2 * s, cx + 3 * s, top - 5 * s, cx + 5 * s, top - 2 * s, C_YELLOW);
      break;
    case 3:  // pirate hat
      canvas.fillTriangle(cx - 7 * s, top, cx + 7 * s, top, cx, top - 5 * s, RGB565(20, 16, 24));
      canvas.fillCircle(cx, top - 2 * s, s, C_WHITE);
      break;
    case 4:  // scarf
      canvas.fillRect(x0 + 3 * s, y0 + 10 * s, 10 * s, 2 * s, C_RED);
      canvas.fillRect(x0 + (flip ? 4 : 10) * s, y0 + 10 * s, 2 * s, 4 * s, C_RED);
      break;
    case 5:  // shades
      canvas.fillRect(x0 + 3 * s, y0 + 6 * s, 10 * s, 2 * s, RGB565(10, 10, 14));
      break;
  }
}

// ---------------------------------------------------------------- missions

const uint32_t MISSION_SECS[3] = {3600, 4 * 3600, 8 * 3600};
const uint8_t MISSION_ENERGY[3] = {10, 25, 40};
const char* MISSION_LEN_NAMES[3] = {"1 hour", "4 hours", "Overnight"};

bool missionActive() {
  return rpg.missionPlace != NO_ITEM;
}

uint32_t missionSecondsLeft() {
  uint32_t now = (uint32_t)time(nullptr);
  return missionActive() && rpg.missionEnd > now ? rpg.missionEnd - now : 0;
}

// How far through the trip it is, 0 to 1
float missionProgress() {
  if (!missionActive()) return 0;
  return 1.0f - (float)missionSecondsLeft() / MISSION_SECS[rpg.missionLen];
}

String startMission(int place, int len) {
  if (!placeOpen(place)) return String(placeName(place)) + " opens at level " + PLACES[place].level;
  if (!petIsAlive()) return "Your pet needs to hatch first";
  if (missionActive()) return "Already out exploring";
  if (rpg.energy < MISSION_ENERGY[len]) return "Too tired - feed it or let it rest";
  rpg.energy -= MISSION_ENERGY[len];
  rpg.missionPlace = place;
  rpg.missionLen = len;
  rpg.missionEnd = (uint32_t)time(nullptr) + MISSION_SECS[len];
  rpg.missionSeed = esp_random();
  saveRpg();
  return String("Off to the ") + placeName(place) + "!";
}

void callBackMission() {
  rpg.missionPlace = NO_ITEM;
  saveRpg();
}

// A quick, unseen fight during a mission. True if the pet won.
bool missionScrap(int place, uint32_t& seed) {
  seed = seed * 1103515245UL + 12345UL;
  int foeLevel = PLACES[place].level + (seed >> 8) % 4;
  int chance = 55 + (rpg.level - foeLevel) * 6 + gearBonus(2) + gearBonus(0);
  chance = constrain(chance, 15, 92);
  seed = seed * 1103515245UL + 12345UL;
  return (int)((seed >> 8) % 100) < chance;
}

// Works out what happened on the trip and hands out the loot
void resolveMission() {
  int place = rpg.missionPlace, len = rpg.missionLen;
  uint32_t seed = rpg.missionSeed;
  int events = len == 0 ? 1 : len == 1 ? 3 : 5;
  int luck = len + (place >= 4 ? 1 : 0);
  rpg.reportWins = rpg.reportLosses = 0;
  rpg.reportCoins = 0;
  memset(rpg.reportItems, NO_ITEM, sizeof(rpg.reportItems));
  int nItems = 0;
  for (int e = 0; e < events; e++) {
    seed = seed * 1103515245UL + 12345UL;
    int roll = (seed >> 8) % 100;
    if (roll < 45) {
      int id = rollItem(min(luck, 3), seed);
      if (nItems < REPORT_ITEMS) rpg.reportItems[nItems++] = id;
      giveItem(id);
    } else if (roll < 80) {
      if (missionScrap(place, seed)) {
        rpg.reportWins++;
        rpg.reportCoins += 8 + PLACES[place].level;
      } else {
        rpg.reportLosses++;
      }
    } else {
      rpg.reportCoins += 10 + (seed >> 12) % (15 + PLACES[place].level * 2);
    }
  }
  const uint16_t baseXp[3] = {40, 120, 260};
  rpg.reportXp = baseXp[len] + rpg.reportWins * 20;
  rpg.reportPlace = place;
  rpg.reportSeed = rpg.missionSeed;
  rpg.reportPending = 1;
  rpg.missionPlace = NO_ITEM;
  rpg.missionsDone++;
  rpg.battlesWon += rpg.reportWins;
  rpg.placesVisited |= 1 << place;
  addCoins(rpg.reportCoins);
  addXp(rpg.reportXp);
  questEvent(Q_MISSION, 1);
  if (rpg.reportWins) questEvent(Q_WIN, rpg.reportWins);
  if (len == 2) awardBadge(B_OVERNIGHT);
  checkBadges();
  saveRpg();
  rpgNotice(String(cfg.petName) + " is back from the " + placeName(place) + "!");
}

void checkMissionDone() {
  if (missionActive() && missionSecondsLeft() == 0) resolveMission();
}

// The little story of the last trip, made up again from its seed when shown
int missionStory(String lines[], int maxLines) {
  uint32_t seed = rpg.reportSeed;
  const Place& pl = PLACES[rpg.reportPlace];
  const char* critter = CRITTERS[pl.critter].name;
  const char* things[] = {"an old log", "a pile of leaves", "a big rock", "a flower pot", "a tree root"};
  int n = 0;
  String name = cfg.petName;
  auto next = [&]() { seed = seed * 1103515245UL + 12345UL; return (int)(seed >> 8); };
  lines[n++] = name + " set off for the " + pl.name + ".";
  for (int i = 0; i < REPORT_ITEMS && n < maxLines - 1; i++) {
    if (rpg.reportItems[i] == NO_ITEM) break;
    lines[n++] = "Found a " + String(itemDef(rpg.reportItems[i]).name) + " under " + things[next() % 5] + "!";
  }
  if (rpg.reportWins && n < maxLines - 1)
    lines[n++] = rpg.reportWins == 1 ? "Won a scrap with a " + String(critter) + "." : "Won " + String(rpg.reportWins) + " scraps with the local " + critter + "s.";
  if (rpg.reportLosses && n < maxLines - 1)
    lines[n++] = "Got chased off by a " + String(critter) + ". Never mind!";
  if (n < maxLines) {
    const char* endings[] = {"Came home tired but happy.", "Had a nap on the way back.", "Made friends with a ladybird.",
                             "Smells a bit of the outdoors now.", "Can't wait to go again!"};
    lines[n++] = endings[next() % 5];
  }
  return n;
}

// ---------------------------------------------------------------- battles

struct Battle {
  bool          on = false, over = false, won = false;
  int           place = 0;
  bool          boss = false;
  int           foeLevel = 1;
  int           foeHp = 1, foeMax = 1, foePower = 1, foeGuard = 0, foeSpeed = 1;
  int           petHp = 1, petMax = 1;
  int           charge = 0;          // special move fills up to 100
  bool          special = false;     // use it on the next hit
  bool          petTurn = true;
  unsigned long nextAt = 0;
  int           hitWho = 0;          // 1 pet hit, 2 foe hit (for the animation)
  int           hitDmg = 0;
  bool          hitCrit = false, hitMiss = false;
  unsigned long hitAt = 0;
  int           rewardCoins = 0, rewardXp = 0, rewardItem = -1;
} battle;

String fighterName() {
  const Place& pl = PLACES[battle.place];
  if (battle.boss) return pl.boss;
  String n = CRITTERS[pl.critter].name;
  n[0] = toupper(n[0]);
  return n;
}

String startBattle(int place, bool boss) {
  if (!placeOpen(place)) return String(placeName(place)) + " opens at level " + PLACES[place].level;
  if (!petIsAlive() || pet.asleep) return "Too sleepy to fight";
  if (missionActive()) return "It's out exploring";
  if (rpg.energy < 20) return "Too tired - feed it or let it rest";
  rpg.energy -= 20;
  markRpg();
  battle = Battle();
  battle.on = true;
  battle.place = place;
  battle.boss = boss;
  int L = PLACES[place].level + (boss ? 6 : esp_random() % 4);
  battle.foeLevel = L;
  // A critter is a fair fight at the same level; a boss needs a few levels or good gear
  battle.foeMax = (24 + L * 5) * (boss ? 14 : 10) / 10;
  battle.foeHp = battle.foeMax;
  battle.foePower = (4 + L * 2) * (boss ? 5 : 4) / 4;
  battle.foeGuard = 1 + L / 2;
  battle.foeSpeed = 3 + L;
  battle.petMax = petMaxHp();
  battle.petHp = battle.petMax;
  battle.petTurn = petSpeed() >= battle.foeSpeed;
  battle.nextAt = millis() + 1200;
  return "Fight!";
}

bool battleRunning() {
  return battle.on && !battle.over;
}

void stopBattle() {
  battle.on = false;
}

void useSpecial() {
  if (battle.on && !battle.over && battle.charge >= 100) {
    battle.special = true;
    battle.charge = 0;
  }
}

void finishBattle() {
  battle.over = true;
  const Place& pl = PLACES[battle.place];
  if (battle.won) {
    battle.rewardCoins = (10 + battle.foeLevel * 2) * (battle.boss ? 3 : 1);
    battle.rewardXp = battle.boss ? 120 + battle.foeLevel * 2 : 30 + battle.foeLevel;
    uint32_t seed = esp_random();
    if (battle.boss || (seed % 100) < 25) {
      battle.rewardItem = rollItem(battle.boss ? 2 : 0, seed);
      giveItem(battle.rewardItem);
    }
    if (battle.boss) {
      if (!((rpg.bossWins >> battle.place) & 1)) battle.rewardCoins += 100;  // first time beating this boss
      rpg.bossWins |= 1 << battle.place;
      rpg.bossesBeaten++;
    }
    rpg.battlesWon++;
    questEvent(Q_WIN, 1);
    petSay(battle.boss ? String("Beat ") + pl.boss + "!" : String("Won!"), 3000);
  } else {
    battle.rewardXp = 10;
    petSay("Ouch... next time!", 3000);
  }
  addCoins(battle.rewardCoins);
  addXp(battle.rewardXp);
  checkBadges();
  saveRpg();
}

// Moves the fight on one hit at a time
void battleStep() {
  if (!battle.on || battle.over || millis() < battle.nextAt) return;
  bool petAttacks = battle.petTurn;
  int power = petAttacks ? petPower() : battle.foePower;
  int guard = petAttacks ? battle.foeGuard : petGuard();
  int atkSpd = petAttacks ? petSpeed() : battle.foeSpeed;
  int defSpd = petAttacks ? battle.foeSpeed : petSpeed();
  uint32_t r = esp_random();
  bool special = petAttacks && battle.special;
  int dodge = constrain((defSpd - atkSpd) * 2 + 6, 3, 25);
  battle.hitMiss = !special && (int)(r % 100) < dodge;
  battle.hitCrit = !battle.hitMiss && ((r >> 8) % 100) < 10;
  int dmg = 0;
  if (!battle.hitMiss) {
    dmg = power * (80 + (int)((r >> 16) % 41)) / 100 - guard / 2;
    if (battle.hitCrit) dmg = dmg * 3 / 2;
    if (special) dmg = dmg * 22 / 10;
    dmg = max(1, dmg);
  }
  if (special) battle.special = false;
  if (petAttacks) {
    battle.foeHp = max(0, battle.foeHp - dmg);
    battle.charge = min(100, battle.charge + 25);
  } else {
    battle.petHp = max(0, battle.petHp - dmg);
    battle.charge = min(100, battle.charge + 15);
  }
  battle.hitWho = petAttacks ? 2 : 1;
  battle.hitDmg = dmg;
  battle.hitAt = millis();
  battle.petTurn = !battle.petTurn;
  battle.nextAt = millis() + 900;
  if (battle.foeHp == 0 || battle.petHp == 0) {
    battle.won = battle.foeHp == 0;
    battle.nextAt = millis() + 600;
    finishBattle();
  }
}

void drawHpBar(int x, int y, int w, int hp, int maxHp, uint16_t colour) {
  canvas.fillRoundRect(x, y, w, 8, 4, mix(0x000000, 0xFFFFFF, 0.15f));
  int fw = maxHp > 0 ? (int)((long)w * hp / maxHp) : 0;
  if (fw > 2) canvas.fillRoundRect(x, y, fw, 8, 4, hp * 4 < maxHp ? C_RED : colour);
}

// One frame of the fight, drawn into the pet scene
void battleFrame() {
  battleStep();
  const Place& pl = PLACES[battle.place];
  int floorY = PET_SCENE_H - 18;
  for (int y = 0; y < floorY; y++) canvas.drawFastHLine(0, y, W, mix(pl.sky, 0x000000, 0.25f + 0.4f * y / floorY));
  canvas.fillRect(0, floorY, W, PET_SCENE_H - floorY, to565(pl.ground));

  unsigned long since = millis() - battle.hitAt;
  bool fresh = battle.hitAt && since < 450;
  const int s = 4;
  int petX = 60, foeX = 196;
  if (fresh && battle.hitWho == 2) petX += 14;   // lunge
  if (fresh && battle.hitWho == 1) foeX -= 14;
  int shake = fresh && !battle.hitMiss ? ((petFrameNo % 2) ? 3 : -3) : 0;

  // The pet (facing right) and the foe (facing left)
  bool petDown = battle.over && !battle.won, foeDown = battle.over && battle.won;
  drawPetPose(cfg.petType, POSE_SIT, petX + (battle.hitWho == 1 ? shake : 0), floorY - 15 * s + 2 + (petDown ? 10 : 0), s,
              false, petDown, petDown ? 0.4f : 0, false);
  if (!petDown) drawGear(petX, floorY - 15 * s + 2, s, false);
  int fy = floorY - 16 * s + 2 + (foeDown ? 10 : 0);
  if (battle.boss) drawBossCat(battle.place, foeX + (battle.hitWho == 2 ? shake : 0), floorY - 15 * s + 2 + (foeDown ? 10 : 0), s, true, foeDown ? 0.4f : 0);
  else drawCritter(pl.critter, foeX + (battle.hitWho == 2 ? shake : 0), fy, s, true, foeDown ? 0.4f : 0);

  // Bonk!
  if (fresh) {
    int hx = battle.hitWho == 2 ? foeX + 32 : petX + 32, hy = floorY - 40;
    canvas.setFont(F_BOLD);
    canvas.setTextDatum(D_MC);
    if (battle.hitMiss) {
      canvas.setTextColor(C_CLOUD);
      canvas.drawString("Miss!", hx, hy - (int)(since / 30));
    } else {
      const char* words[] = {"BONK!", "POW!", "BOP!", "WHAM!"};
      canvas.fillCircle(hx, hy + 14, 14, battle.hitCrit ? C_YELLOW : C_WHITE);
      canvas.setTextColor(battle.hitCrit ? C_RED : C_AMBER);
      canvas.drawString(words[(battle.hitDmg + battle.foeHp) % 4], hx, hy - 8);
      canvas.setTextColor(battle.hitWho == 1 ? C_RED : C_GREEN);
      canvas.drawString("-" + String(battle.hitDmg), hx, hy - 26 - (int)(since / 40));
    }
  }

  // Health bars and names
  canvas.setFont(F_SMALL);
  canvas.setTextSize(1);
  canvas.setTextColor(C_WHITE);
  canvas.setTextDatum(D_TL);
  canvas.drawString(fitTextIn(canvas, cfg.petName, 120), 8, 4);
  drawHpBar(8, 22, 130, battle.petHp, battle.petMax, C_GREEN);
  canvas.fillRoundRect(8, 33, 130, 4, 2, mix(0x000000, 0xFFFFFF, 0.15f));
  if (battle.charge > 2) canvas.fillRoundRect(8, 33, 130 * battle.charge / 100, 4, 2, battle.charge >= 100 ? C_YELLOW : C_ACCENT2);
  canvas.setTextDatum(D_TR);
  canvas.drawString(fighterName() + " Lv" + battle.foeLevel, W - 8, 4);
  drawHpBar(W - 138, 22, 130, battle.foeHp, battle.foeMax, C_AMBER);

  if (battle.over) {
    canvas.fillRoundRect(W / 2 - 96, 44, 192, 44, 10, mix(theme().card, 0x000000, 0.1f));
    canvas.setFont(F_BOLD);
    canvas.setTextDatum(D_TC);
    canvas.setTextColor(battle.won ? C_GREEN : C_AMBER);
    canvas.drawString(battle.won ? "You won!" : "You lost", W / 2, 50);
    canvas.setFont(F_SMALL);
    canvas.setTextColor(C_WHITE);
    String reward = "+" + String(battle.rewardXp) + " XP";
    if (battle.rewardCoins) reward += "  +" + String(battle.rewardCoins) + " coins";
    if (battle.rewardItem >= 0) reward += "  +" + String(itemDef(battle.rewardItem).name);
    canvas.drawString(fitTextIn(canvas, reward, 184), W / 2, 70);
  }
}
