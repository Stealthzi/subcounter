// Pet adventures: levels, XP, coins, daily streaks, quests and badges.
//
// The pet earns XP when you look after it, when your channel grows (new
// subscribers, uploads, a video beating your usual) and for good real-life
// habits (a low-energy day, filling up when petrol is cheap). Every 5 levels it
// gets a mystery box, and new pets, tricks, mini-games and themes unlock on the
// way to level 50. It's all saved in one block, so it survives restarts and
// updates - even if the classic pet's life starts again.

const uint8_t RPG_VERSION = 1;
const uint16_t CARE_XP_PER_DAY = 300;

bool          rpgDirty = false;
unsigned long rpgSavedAt = 0;
String        rpgNotices[6];
int           rpgNoticeCount = 0;
uint8_t       pendingLevelUp = 0;   // the level-up screen shows next time the pet page opens

// ---------------------------------------------------------------- saving

void saveRpg() {
  prefs.putBytes("rpg", &rpg, sizeof(rpg));
  rpgDirty = false;
  rpgSavedAt = millis();
}

void markRpg() {
  rpgDirty = true;
}

// Saves now and then rather than on every little change, to spare the flash
void rpgMaybeSave() {
  if (rpgDirty && millis() - rpgSavedAt > 30000) saveRpg();
}

void loadRpg() {
  memset(&rpg, 0, sizeof(rpg));
  if (prefs.getBytesLength("rpg") == sizeof(rpg)) prefs.getBytes("rpg", &rpg, sizeof(rpg));
  if (rpg.version != RPG_VERSION) {
    memset(&rpg, 0, sizeof(rpg));
    rpg.version = RPG_VERSION;
    rpg.level = 1;
    rpg.coins = 50;
    rpg.energy = 100;
    rpg.unlocked = (1 << SP_CLASSIC) | (1 << SP_CAT);
    memset(rpg.room, NO_ITEM, sizeof(rpg.room));
    memset(rpg.gear, NO_ITEM, sizeof(rpg.gear));
    rpg.missionPlace = NO_ITEM;
    for (int i = 0; i < SP_COUNT; i++) rpg.coat[i] = esp_random() % 6;
    rpg.coat[SP_CAT] = pet.spare[1] ? pet.spare[0] : rpg.coat[SP_CAT];
    giveStarterItems();
    saveRpg();
  }
  if (cfg.petType >= SP_COUNT || !speciesUnlocked(cfg.petType)) cfg.petType = SP_CAT;
}

// ---------------------------------------------------------------- notices

// Short messages ("Level 12!", "Badge: Night owl") the pet says next time you look
void rpgNotice(const String& text) {
  if (rpgNoticeCount < 6) rpgNotices[rpgNoticeCount++] = text;
}

bool takeRpgNotice(String& out) {
  if (rpgNoticeCount == 0) return false;
  out = rpgNotices[0];
  for (int i = 1; i < rpgNoticeCount; i++) rpgNotices[i - 1] = rpgNotices[i];
  rpgNoticeCount--;
  return true;
}

// ---------------------------------------------------------------- levels

uint32_t xpNeeded(int level) {
  return 150 + 60UL * level;
}

const char* petTitle(int level) {
  const char* titles[] = {"Newborn", "Little one", "Explorer", "Adventurer", "Brave heart", "Hero",
                          "Champion", "Master", "Mythic", "Almost legend", "Legend"};
  return titles[min(level / 5, 10)];
}

bool speciesUnlocked(int sp) {
  return sp >= 0 && sp < SP_COUNT && (rpg.unlocked >> sp) & 1;
}

int tricksUnlocked() {
  int n = 0;
  for (int i = 0; i < 4; i++)
    if (rpg.level >= TRICK_LEVEL[i]) n++;
  return n;
}

bool miniGameUnlocked(int i) {
  return rpg.level >= MINIGAME_LEVEL[i];
}

bool themeUnlocked(int t) {
  return t >= 0 && t < THEME_COUNT && rpg.level >= THEME_LEVEL[t];
}

bool petEvolved() {
  return rpg.level >= EVOLVE_LEVEL;
}

void onLevelUp(int level) {
  addCoins(10 * level);
  pendingLevelUp = level;
  rpgNotice("Level " + String(level) + "! " + petTitle(level));
  if (level % 5 == 0) {
    rpg.boxes++;
    rpgNotice("A mystery box!");
  }
  for (int sp = 0; sp < SP_COUNT; sp++) {
    if (SPECIES_LEVEL[sp] == level && !speciesUnlocked(sp)) {
      rpg.unlocked |= 1 << sp;
      rpgNotice(sp == SP_DRAGON ? String("A dragon egg appeared!") : String("New pet: ") + speciesName(sp) + "!");
    }
  }
  for (int i = 0; i < 4; i++)
    if (TRICK_LEVEL[i] == level) rpgNotice(String("New trick: ") + trickName(i));
  for (int i = 0; i < 3; i++)
    if (MINIGAME_LEVEL[i] == level) rpgNotice(String("New game: ") + miniGameName(i));
  for (int t = 0; t < THEME_COUNT; t++)
    if (THEME_LEVEL[t] == level) rpgNotice(String("New theme: ") + THEMES[t].name);
  for (int p = 0; p < placeCount(); p++)
    if (placeLevel(p) == level && level > 1) rpgNotice(String("New place: ") + placeName(p));
  if (level == EVOLVE_LEVEL) {
    rpgNotice("Your pet evolved!");
    awardBadge(B_EVOLVED);
  }
  if (level >= 5) awardBadge(B_LEVEL5);
  if (level >= 10) awardBadge(B_LEVEL10);
  if (level >= 25) awardBadge(B_LEVEL25);
  if (level >= MAX_LEVEL) awardBadge(B_LEVEL50);
  if (tricksUnlocked() == 4) awardBadge(B_TRICKSTER);
}

void addXp(int amount) {
  if (amount <= 0) return;
  rpg.totalXp += amount;
  if (rpg.level >= MAX_LEVEL) {
    markRpg();
    return;
  }
  rpg.xp += amount;
  while (rpg.level < MAX_LEVEL && rpg.xp >= xpNeeded(rpg.level)) {
    rpg.xp -= xpNeeded(rpg.level);
    rpg.level++;
    onLevelUp(rpg.level);
  }
  if (rpg.level >= MAX_LEVEL) rpg.xp = 0;
  markRpg();
  if (pendingLevelUp) saveRpg();
}

void addCoins(int amount) {
  if (amount < 0 && (uint32_t)(-amount) > rpg.coins) rpg.coins = 0;
  else rpg.coins += amount;
  if (rpg.coins >= 1000) awardBadge(B_RICH);
  markRpg();
}

// Looking after the pet earns XP, up to a daily cap so it can't be farmed
void careXp(int amount) {
  uint32_t today = localDay();
  if (rpg.careDay != today) {
    rpg.careDay = today;
    rpg.careXp = 0;
  }
  if (rpg.careXp >= CARE_XP_PER_DAY) return;
  amount = min(amount, (int)(CARE_XP_PER_DAY - rpg.careXp));
  rpg.careXp += amount;
  addXp(amount);
}

// ---------------------------------------------------------------- daily streak

// The first visit to the pet each day keeps the streak going
void rpgCheckIn() {
  uint32_t today = localDay();
  if (today == 0 || rpg.lastCheckIn == today) return;
  rpg.streak = (rpg.lastCheckIn == today - 1) ? rpg.streak + 1 : 1;
  rpg.lastCheckIn = today;
  if (rpg.streak > rpg.bestStreak) rpg.bestStreak = rpg.streak;
  int bonus = 20 + 10 * min((int)rpg.streak, 10);
  addXp(bonus);
  addCoins(5 + min((int)rpg.streak, 10));
  rpgNotice(rpg.streak > 1 ? String(rpg.streak) + " days in a row! +" + bonus + " XP" : String("Welcome back! +") + bonus + " XP");
  if (rpg.streak % 7 == 0) {
    rpg.boxes++;
    rpgNotice("A week's streak - a mystery box!");
  }
  if (rpg.streak >= 7) awardBadge(B_STREAK7);
  if (rpg.streak >= 30) awardBadge(B_STREAK30);
  questEvent(Q_CHECKIN, 1);
  saveRpg();
}

// ---------------------------------------------------------------- quests

const char* questText(uint8_t kind) {
  switch (kind) {
    case Q_FEED:      return "Feed your pet";
    case Q_STROKE:    return "Stroke or pat your pet";
    case Q_PLAY:      return "Play with your pet";
    case Q_CLEAN:     return "Clean up after it";
    case Q_MINIGAME:  return "Play a mini-game";
    case Q_MISSION:   return "Finish a mission";
    case Q_WIN:       return "Win a battle";
    case Q_TRICK:     return "Do a trick";
    case Q_BUY:       return "Buy something in the shop";
    case Q_SUBS:      return "Gain subscribers";
    case Q_UPLOAD:    return "Upload a video";
    case Q_LOWENERGY: return "Have a low-energy day";
    case Q_FILLUP:    return "Fill up when petrol's cheap";
    case Q_CHECKIN:   return "Visit your pet every day";
  }
  return "";
}

// Can this quest be done at all right now?
bool questPossible(uint8_t kind) {
  switch (kind) {
    case Q_MINIGAME:  return miniGameUnlocked(0);
    case Q_TRICK:     return tricksUnlocked() > 0;
    case Q_WIN:       return rpg.level >= 2;
    case Q_UPLOAD:
    case Q_SUBS:      return channel.loaded;
    case Q_LOWENERGY: return cfg.octoKey.length() > 0;
    case Q_FILLUP:    return cfg.city.length() > 0 && pageEnabled(PAGE_FUEL);
  }
  return true;
}

void pickQuests(Quest* out, int n, const uint8_t* pool, const uint8_t* goals, int poolSize, uint32_t seed) {
  int chosen = 0;
  for (int tries = 0; tries < 40 && chosen < n; tries++) {
    seed = seed * 1103515245UL + 12345UL;
    int i = (seed >> 8) % poolSize;
    uint8_t kind = pool[i];
    bool dup = false;
    for (int j = 0; j < chosen; j++)
      if (out[j].kind == kind) dup = true;
    if (dup || !questPossible(kind)) continue;
    out[chosen].kind = kind;
    out[chosen].goal = goals[i];
    out[chosen].progress = 0;
    out[chosen].done = 0;
    chosen++;
  }
  for (; chosen < n; chosen++) out[chosen] = {Q_FEED, 2, 0, 0};
}

// New daily quests each day, new weekly ones each Monday
void refreshQuests() {
  uint32_t today = localDay();
  if (today == 0) return;
  uint32_t week = (today + 3) / 7;  // weeks start on Monday
  if (rpg.questDay != today) {
    const uint8_t pool[] = {Q_FEED, Q_STROKE, Q_PLAY, Q_CLEAN, Q_MINIGAME, Q_MISSION, Q_WIN, Q_TRICK, Q_BUY};
    const uint8_t goals[] = {2, 3, 1, 1, 1, 1, 1, 1, 1};
    pickQuests(rpg.daily, 3, pool, goals, sizeof(pool), today * 2654435761UL);
    rpg.questDay = today;
    markRpg();
  }
  if (rpg.questWeek != week) {
    const uint8_t pool[] = {Q_MISSION, Q_WIN, Q_MINIGAME, Q_UPLOAD, Q_SUBS, Q_LOWENERGY, Q_FILLUP, Q_CHECKIN};
    const uint8_t goals[] = {5, 8, 5, 1, 3, 3, 1, 7};
    pickQuests(rpg.weekly, 2, pool, goals, sizeof(pool), week * 2246822519UL);
    rpg.questWeek = week;
    markRpg();
  }
}

// Something happened that might count towards a quest
void questEvent(uint8_t kind, int amount) {
  refreshQuests();
  for (int i = 0; i < 5; i++) {
    Quest& q = i < 3 ? rpg.daily[i] : rpg.weekly[i - 3];
    if (q.kind != kind || q.done) continue;
    q.progress = min(255, q.progress + amount);
    if (q.progress >= q.goal) {
      q.done = 1;
      bool weekly = i >= 3;
      addXp(weekly ? 250 : 50);
      addCoins(weekly ? 100 : 20);
      rpgNotice(String(weekly ? "Weekly quest done! " : "Quest done! ") + (weekly ? "+250 XP" : "+50 XP"));
      if (weekly && rpg.weekly[0].done && rpg.weekly[1].done) {
        rpg.boxes++;
        rpgNotice("Both weekly quests - a mystery box!");
      }
    }
    markRpg();
  }
}

// ---------------------------------------------------------------- badges

const char* BADGE_NAMES[B_COUNT] = {
  "First steps", "Explorer", "Hero", "Legend", "Box opener", "Box hoarder", "Lucky!", "Collector",
  "Completionist", "Off we go", "Globetrotter", "Night owl", "First win", "Boss slayer", "Champion", "On a roll",
  "Devoted", "Shopaholic", "Interior designer", "Game on", "High score", "Growing channel", "New video!", "Beat the average",
  "Eco hero", "Smart filler", "Trickster", "Dragon tamer", "Evolution", "Seasonal spirit", "Rich", "Dressed up"
};
const char* BADGE_HINTS[B_COUNT] = {
  "Reach level 5", "Reach level 10", "Reach level 25", "Reach level 50", "Open a mystery box", "Open 10 boxes",
  "Find a legendary item", "Find 25 different items", "Find every item", "Finish a mission", "Visit every place",
  "Finish an overnight mission", "Win a battle", "Beat a boss cat", "Win 50 battles", "7-day streak", "30-day streak",
  "Buy 10 things", "Fill every spot in the room", "Play 10 mini-games", "Score 30 in a mini-game",
  "XP from 10 subscriber gains", "XP from a new upload", "A video beat your usual", "5 low-energy days",
  "3 cheap fill-ups", "Learn every trick", "Hatch the dragon", "Evolve your pet", "Find a seasonal item",
  "Have 1,000 coins", "Wear gear in all 3 spots"
};

bool hasBadge(int b) {
  return (rpg.badges >> b) & 1;
}

int badgeCount() {
  int n = 0;
  for (int b = 0; b < B_COUNT; b++)
    if (hasBadge(b)) n++;
  return n;
}

void awardBadge(int b) {
  if (b < 0 || b >= B_COUNT || hasBadge(b)) return;
  rpg.badges |= 1UL << b;
  rpg.coins += 50;
  rpgNotice(String("Badge: ") + BADGE_NAMES[b] + " (+50 coins)");
  markRpg();
}

// Badges that depend on totals, checked after anything changes them
void checkBadges() {
  if (rpg.boxesOpened >= 1) awardBadge(B_FIRSTBOX);
  if (rpg.boxesOpened >= 10) awardBadge(B_TENBOXES);
  int found = foundCount();
  if (found >= 25) awardBadge(B_COLLECT25);
  if (found >= itemCount()) awardBadge(B_COLLECTALL);
  if (rpg.missionsDone >= 1) awardBadge(B_FIRSTMISSION);
  if (rpg.placesVisited == (1 << placeCount()) - 1) awardBadge(B_ALLPLACES);
  if (rpg.battlesWon >= 1) awardBadge(B_FIRSTWIN);
  if (rpg.bossesBeaten >= 1) awardBadge(B_BOSS);
  if (rpg.battlesWon >= 50) awardBadge(B_FIFTYWINS);
  if (rpg.itemsBought >= 10) awardBadge(B_SHOPPER);
  bool full = true;
  for (int i = 0; i < ROOM_SLOTS; i++)
    if (rpg.room[i] == NO_ITEM) full = false;
  if (full) awardBadge(B_DESIGNER);
  if (rpg.gamesPlayed >= 10) awardBadge(B_GAMER);
  if (rpg.bestGameScore >= 30) awardBadge(B_HIGHSCORE);
  if (rpg.subsXpEvents >= 10) awardBadge(B_SUBS10);
  if (rpg.lowEnergyDays >= 5) awardBadge(B_ECO);
  if (rpg.cheapFills >= 3) awardBadge(B_FILLER);
  if (rpg.gear[0] != NO_ITEM && rpg.gear[1] != NO_ITEM && rpg.gear[2] != NO_ITEM) awardBadge(B_DRESSED);
}

// ---------------------------------------------------------------- XP from your channel and real life

uint32_t textHash(const String& s) {
  uint32_t h = 2166136261UL;
  for (unsigned i = 0; i < s.length(); i++) h = (h ^ (uint8_t)s[i]) * 16777619UL;
  return h ? h : 1;
}

// A new subscriber (the public count went up)
void rpgOnSubsUp() {
  if (!petIsAlive()) return;
  addXp(25);
  rpg.subsXpEvents++;
  questEvent(Q_SUBS, 1);
  checkBadges();
  markRpg();
}

// Called after the uploads list refreshes
void rpgOnUploads() {
  int li = latestIndex();
  if (li < 0) return;
  uint32_t h = textHash(uploads.v[li].id);
  if (rpg.lastUploadHash == h) return;
  bool first = rpg.lastUploadHash == 0;
  rpg.lastUploadHash = h;
  markRpg();
  if (first) return;  // just learning which video is the latest
  addXp(150);
  addCoins(50);
  rpgNotice("New video! +150 XP");
  questEvent(Q_UPLOAD, 1);
  awardBadge(B_UPLOAD);
}

// The latest video beat your usual views at a day old
void rpgOnBeatAverage(uint32_t videoHash) {
  if (rpg.lastBeatHash == videoHash) return;
  rpg.lastBeatHash = videoHash;
  addXp(200);
  addCoins(40);
  rpgNotice("Your video beat your usual! +200 XP");
  awardBadge(B_BEATAVG);
  saveRpg();
}

// Once a day: yesterday's electricity use against the days before it
void rpgCheckEnergy() {
  if (!energy.loaded || energy.days < 4) return;
  uint32_t last = energy.day[energy.days - 1];
  if (rpg.lastEnergyDay == last) return;
  rpg.lastEnergyDay = last;
  float avg = 0;
  for (int i = 0; i < energy.days - 1; i++) avg += energy.elecKwh[i];
  avg /= energy.days - 1;
  if (avg > 0.5f && energy.elecKwh[energy.days - 1] < avg * 0.9f) {
    addXp(60);
    addCoins(15);
    rpg.lowEnergyDays++;
    rpgNotice("Low energy day! +60 XP");
    questEvent(Q_LOWENERGY, 1);
    checkBadges();
  }
  markRpg();
}

// "I filled up" on the quests screen. Cheap if it's within a penny of the
// lowest price seen in the last two weeks.
String rpgLogFillUp() {
  uint32_t today = localDay();
  if (rpg.lastFillDay == today) return "Already logged today";
  rpg.lastFillDay = today;
  if (!fuel.loaded) {
    addXp(10);
    return "Logged! +10 XP";
  }
  float low = fuel.min;
  for (int i = 0; i < FUEL_DAYS; i++)
    if (fuelHistory[i].day && fuelHistory[i].min > 0 && fuelHistory[i].min < low) low = fuelHistory[i].min;
  if (fuel.min <= low + 1.0f) {
    addXp(80);
    addCoins(20);
    rpg.cheapFills++;
    questEvent(Q_FILLUP, 1);
    checkBadges();
    saveRpg();
    return "Cheap fill-up! +80 XP";
  }
  addXp(20);
  saveRpg();
  return "Logged! Prices were higher than usual. +20 XP";
}

// ---------------------------------------------------------------- battle energy and stats

void rpgEnergyTick() {
  uint32_t now = (uint32_t)time(nullptr);
  if (now < 1700000000) return;
  if (rpg.energyAt == 0 || rpg.energy >= 100) {
    rpg.energyAt = now;
    return;
  }
  uint32_t hours = (now - rpg.energyAt) / 3600;
  if (hours == 0) return;
  rpg.energy = min(100, (int)rpg.energy + (int)hours * 10);
  rpg.energyAt += hours * 3600;
  markRpg();
}

int gearBonus(int slot) {
  uint8_t id = rpg.gear[slot];
  if (id == NO_ITEM || id >= itemCount()) return 0;
  const int bonus[] = {2, 4, 7, 12};
  return bonus[itemDef(id).rarity];
}

int petMaxHp() { return 40 + rpg.level * 6 + (petEvolved() ? 20 : 0) + gearBonus(0) * 2; }
int petPower() { return 6 + rpg.level * 2 + gearBonus(2) + (petEvolved() ? 6 : 0); }
int petGuard() { return 2 + rpg.level + gearBonus(0); }
int petSpeed() { return 5 + rpg.level + gearBonus(1) + (cfg.petType == SP_DOG ? 3 : cfg.petType == SP_HAMSTER ? 5 : 0); }

// ---------------------------------------------------------------- once a minute

void rpgMinute() {
  refreshQuests();
  rpgEnergyTick();
  rpgCheckEnergy();
  checkMissionDone();
  checkDragonHatch();
  rpgMaybeSave();
}
