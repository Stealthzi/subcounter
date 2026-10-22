// The pet's adventure screens: the hub, missions, battles, shop, bag,
// collection book, badges, quests, pets, mystery boxes and level-ups.
//
// Full-screen menus (hub, shop...) are drawn once and redrawn after a tap.
// Battles, mini-games, boxes and level-ups animate in the pet's scene with
// their own row of buttons underneath.

const int BOTTOM_Y = 196, BOTTOM_H = 26;

int           placeSel = 0, fightPlace = 0, bagTab = 0, bagPage = 0, bagSel = -1;
int           bookPage = 0, bookSel = -1, badgeSel = -1, shopSel = -1;
String        screenMsg;
int           boxItem = -1;
unsigned long boxAt = 0, levelUpAt = 0;
uint8_t       levelShown = 0;

bool petFullScreen() {
  switch (petMenu) {
    case MENU_HUB: case MENU_MISSIONS: case MENU_MISSION_LEN: case MENU_REPORT: case MENU_FIGHT_PICK:
    case MENU_SHOP: case MENU_BAG: case MENU_BOOK: case MENU_BADGES: case MENU_QUESTS: case MENU_PETS:
      return true;
  }
  return false;
}

// Menus that animate in the scene with their own buttons
bool petSpecialScene() {
  return petMenu == MENU_BATTLE || petMenu == MENU_MINIGAME || petMenu == MENU_BOX || petMenu == MENU_LEVELUP;
}

void openPetMenu(int m) {
  petMenu = (PetMenu)m;
  screenMsg = "";
  drawPetPage();
}

// ---------------------------------------------------------------- shared bits

void screenTop(const String& title, const String& right) {
  clearContent();
  tft.setTextSize(1);
  tft.setFont(F_BOLD);
  gradientText(title, 12, 42, false, 0xFFFFFF, theme().accent);
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TR);
  tft.setTextColor(C_GREY);
  tft.drawString(right.c_str(), W - 12, 44);
}

void screenBottom(bool back) {
  if (screenMsg.length()) {
    tft.setFont(F_SMALL);
    tft.setTextSize(1);
    tft.setTextDatum(D_ML);
    tft.setTextColor(C_CLOUD);
    tft.drawString(fitText(screenMsg, back ? W - 110 : W - 20).c_str(), 12, BOTTOM_Y + BOTTOM_H / 2);
  }
  if (back) drawButton(W - 88, BOTTOM_Y, 80, BOTTOM_H, "Back", C_LINE, C_WHITE);
}

bool tappedBack(int x, int y) {
  return hitBox(x, y, W - 88, BOTTOM_Y, 80, BOTTOM_H);
}

void drawTile(int x, int y, int w, int h, const String& label, const String& sub, uint16_t accent, bool dim) {
  tft.fillSmoothRoundRect(x, y, w, h, 8, dim ? mix(theme().card, 0x000000, 0.3f) : C_CARD);
  tft.fillSmoothRoundRect(x, y + 6, 4, h - 12, 2, dim ? C_DIM : accent);
  tft.setTextSize(1);
  tft.setFont(F_BOLD);
  tft.setTextDatum(sub.length() ? D_TL : D_ML);
  tft.setTextColor(dim ? C_DIM : C_WHITE);
  tft.drawString(fitText(label, w - 16).c_str(), x + 11, sub.length() ? y + 3 : y + h / 2);
  if (sub.length()) {
    tft.setFont(F_SMALL);
    tft.setTextColor(dim ? C_DIM : C_GREY);
    tft.drawString(fitText(sub, w - 16).c_str(), x + 11, y + h - 17);
  }
}

String timeLeftText(uint32_t secs) {
  if (secs >= 3600) return String(secs / 3600) + "h " + String((secs % 3600) / 60) + "m";
  return String(max(1UL, (unsigned long)secs / 60)) + "m";
}

// A small pet picture drawn straight onto the screen (for the pets screen)
void drawPetIcon(int sp, int x0, int y0, int s, bool locked) {
  const char** spr = sp == SP_CLASSIC ? SPR_ADULT : speciesPose(sp, POSE_SIT);
  for (int r = 0; r < 16; r++)
    for (int c = 0; c < 16; c++) {
      char ch = spr[r][c];
      if (ch == '.') continue;
      uint32_t col;
      if (ch == 'k') col = 0x101018;
      else if (ch == 'w') col = 0xFFFFFF;
      else if (ch == 'p') col = sp == SP_DRAGON ? 0x101018 : 0xFF8FB3;
      else if (sp == SP_CLASSIC) col = petBodyColour(PET_ADULT, pet.form);
      else if (sp == SP_CAT) col = coatColour(r, c);
      else col = speciesColour(sp, ch, r, c);
      if (locked) col = 0x2A3050;
      tft.fillRect(x0 + c * s, y0 + r * s, s, s, to565(col));
    }
}

// ---------------------------------------------------------------- drawing the full screens

void drawHubScreen() {
  screenTop("Adventure", "Lv " + String(rpg.level) + " " + petTitle(rpg.level) + "  " + rpg.coins + " coins");
  float frac = rpg.level >= MAX_LEVEL ? 1.0f : (float)rpg.xp / xpNeeded(rpg.level);
  drawProgressBar(12, 62, W - 24, 6, frac, C_ACCENT);
  int y = 76;
  if (rpg.boxes > 0) {
    drawButton(8, 74, W - 16, 26, "Open mystery box (" + String(rpg.boxes) + ")", C_AMBER, RGB565(40, 24, 0));
    y = 104;
  }
  int done = 0;
  for (int i = 0; i < 3; i++) done += rpg.daily[i].done;
  for (int i = 0; i < 2; i++) done += rpg.weekly[i].done;
  String missionSub = missionActive() ? "Back in " + timeLeftText(missionSecondsLeft())
                    : rpg.reportPending ? String("It's back!") : String("Explore and find loot");
  int owned = 0;
  for (int i = 0; i < itemCount(); i++) owned += rpg.owned[i] ? 1 : 0;
  String labels[9] = {"Missions", "Battle", "Shop", "Quests", "Bag", "Book", "Badges", "Pets", "Back"};
  String subs[9] = {missionSub, "Energy " + String(rpg.energy), String(rpg.coins) + " coins",
                    String(done) + "/5 done", String(owned) + " kinds", String(foundCount()) + "/" + itemCount(),
                    String(badgeCount()) + "/" + String((int)B_COUNT), speciesName(cfg.petType), ""};
  uint16_t cols[9] = {C_BLUE, C_RED, C_YELLOW, C_GREEN, C_ACCENT2, C_PURPLE, C_AMBER, C_ACCENT, C_LINE};
  for (int i = 0; i < 9; i++) {
    int tx = 8 + (i % 3) * 103, ty = y + (i / 3) * 38;
    drawTile(tx, ty, 98, 34, labels[i], i == 8 ? String("") : subs[i], cols[i], false);
  }
}

void drawMissionsScreen() {
  if (missionActive()) {
    screenTop("Out exploring", "Energy " + String(rpg.energy));
    const Place& pl = PLACES[rpg.missionPlace];
    tft.fillSmoothRoundRect(8, 62, W - 16, 84, 10, C_CARD);
    drawShape(tft, S_MAP, 44, 104, 50, 0xE8D8A8, 0xB5452E);
    tft.setFont(F_MED);
    tft.setTextDatum(D_TL);
    tft.setTextColor(C_WHITE);
    tft.drawString(pl.name, 80, 72);
    tft.setFont(F_SMALL);
    tft.setTextColor(C_GREY);
    tft.drawString((String(MISSION_LEN_NAMES[rpg.missionLen]) + " trip  -  back in " + timeLeftText(missionSecondsLeft())).c_str(), 80, 100);
    float total = MISSION_SECS[rpg.missionLen];
    drawProgressBar(80, 122, W - 104, 8, 1.0f - missionSecondsLeft() / total, C_ACCENT);
    drawButton(8, 156, 200, 30, "Call back (no loot)", C_LINE, C_WHITE);
    screenBottom(true);
    return;
  }
  screenTop("Where to?", "Energy " + String(rpg.energy) + "/100");
  for (int p = 0; p < PLACE_COUNT; p++) {
    int x = 8 + (p % 2) * 154, y = 62 + (p / 2) * 32;
    bool open = placeOpen(p);
    drawTile(x, y, 148, 28, PLACES[p].name, "", open ? to565(PLACES[p].ground) : C_DIM, !open);
    if (!open) {
      tft.setFont(F_SMALL);
      tft.setTextDatum(D_MR);
      tft.setTextColor(C_DIM);
      tft.drawString(("Lv " + String(PLACES[p].level)).c_str(), x + 140, y + 14);
    } else if ((rpg.placesVisited >> p) & 1) {
      tft.fillSmoothCircle(x + 136, y + 14, 4, C_GREEN);
    }
  }
  screenBottom(true);
}

void drawMissionLenScreen() {
  const Place& pl = PLACES[placeSel];
  screenTop(String("Explore the ") + pl.name, "Energy " + String(rpg.energy));
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_GREY);
  tft.drawString((String("Lives here: ") + CRITTERS[pl.critter].name + "s.  Boss: " + pl.boss).c_str(), 12, 62);
  const char* finds[] = {"1 adventure", "3 adventures", "5 adventures, best loot"};
  for (int i = 0; i < 3; i++) {
    int y = 84 + i * 36;
    bool can = rpg.energy >= MISSION_ENERGY[i];
    drawTile(8, y, W - 16, 32, String(MISSION_LEN_NAMES[i]) + "  (" + MISSION_ENERGY[i] + " energy)", finds[i],
             can ? C_ACCENT : C_DIM, !can);
  }
  screenBottom(true);
}

void drawReportScreen() {
  screenTop(String("Back from the ") + placeName(rpg.reportPlace) + "!", "");
  String lines[6];
  int n = missionStory(lines, 5);
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  for (int i = 0; i < n; i++) {
    tft.setTextColor(i == 0 ? C_GREY : C_WHITE);
    tft.drawString(fitText(lines[i], W - 24).c_str(), 12, 62 + i * 17);
  }
  int x = 12;
  for (int i = 0; i < REPORT_ITEMS; i++) {
    if (rpg.reportItems[i] == NO_ITEM) break;
    drawItemTile(tft, rpg.reportItems[i], x, 148, 40, 40, true, 0);
    x += 46;
  }
  tft.setFont(F_BOLD);
  tft.setTextDatum(D_ML);
  tft.setTextColor(C_GREEN);
  tft.drawString(("+" + String(rpg.reportXp) + " XP  +" + rpg.reportCoins + " coins").c_str(), 12, BOTTOM_Y + BOTTOM_H / 2);
  drawButton(W - 88, BOTTOM_Y, 80, BOTTOM_H, "Nice!", C_GREEN, RGB565(5, 30, 20));
}

void drawFightPickScreen() {
  screenTop("Battle", "Energy " + String(rpg.energy) + "/100");
  const Place& pl = PLACES[fightPlace];
  drawButton(8, 62, 40, 28, "<", C_CARD, C_WHITE);
  drawButton(W - 48, 62, 40, 28, ">", C_CARD, C_WHITE);
  tft.setFont(F_BOLD);
  tft.setTextDatum(D_MC);
  tft.setTextColor(placeOpen(fightPlace) ? C_WHITE : C_DIM);
  tft.drawString(placeOpen(fightPlace) ? pl.name : (String(pl.name) + " (Lv " + pl.level + ")").c_str(), W / 2, 76);
  bool open = placeOpen(fightPlace), beaten = (rpg.bossWins >> fightPlace) & 1;
  String critter = CRITTERS[pl.critter].name;
  critter[0] = toupper(critter[0]);
  drawTile(8, 98, 148, 52, critter, "Lv " + String(pl.level) + "-" + (pl.level + 3) + "  - tap to fight", C_GREEN, !open);
  drawTile(164, 98, 148, 52, String("Boss: ") + pl.boss, "Lv " + String(pl.level + 6) + (beaten ? "  - beaten!" : "  - tough!"),
           C_RED, !open);
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_GREY);
  tft.drawString(("Your pet:  HP " + String(petMaxHp()) + "   Power " + petPower() + "   Guard " + petGuard() +
                  "   Speed " + petSpeed()).c_str(), 12, 160);
  tft.drawString("Fights cost 20 energy. Gear makes it stronger.", 12, 177);
  screenBottom(true);
}

void drawShopScreen() {
  screenTop("Shop", String(rpg.coins) + " coins");
  int stock[5];
  shopStock(stock);
  for (int i = 0; i < 6; i++) {
    int x = 8 + (i % 3) * 103, y = 60 + (i / 3) * 66, w = 98, h = 62;
    bool sel = shopSel == i;
    tft.fillSmoothRoundRect(x, y, w, h, 8, sel ? C_ACCENT2 : C_CARD);
    tft.fillSmoothRoundRect(x + 2, y + 2, w - 4, h - 4, 7, C_CARD);
    String name, price;
    if (i < 5) {
      drawItemIcon(tft, stock[i], x + 22, y + 24, 30);
      name = itemDef(stock[i]).name;
      price = String(itemDef(stock[i]).price);
      tft.fillSmoothCircle(x + w - 12, y + 12, 4, rarityColour(itemDef(stock[i]).rarity));
    } else {
      drawShape(tft, S_BOX, x + 22, y + 24, 30, 0xFFC83D, 0xE03050);
      name = "Mystery box";
      price = String(BOX_PRICE);
    }
    tft.setFont(F_BOLD);
    tft.setTextDatum(D_TR);
    tft.setTextColor(rpg.coins >= (uint32_t)price.toInt() ? C_YELLOW : C_DIM);
    tft.drawString(price.c_str(), x + w - 8, y + 24);
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_BL);
    tft.setTextColor(C_WHITE);
    tft.drawString(fitText(name, w - 12).c_str(), x + 6, y + h - 6);
  }
  screenBottom(true);
}

// Items in the bag for the current tab
int bagItems(int out[]) {
  int n = 0;
  for (int i = 0; i < itemCount(); i++) {
    if (!rpg.owned[i]) continue;
    int t = itemDef(i).type;
    bool match = bagTab == 0 ? t == IT_FOOD
               : bagTab == 1 ? gearSlotFor(t) >= 0
               : bagTab == 2 ? roomSlotFor(t) >= 0
                             : t == IT_TREASURE;
    if (match) out[n++] = i;
  }
  return n;
}

void drawBagScreen() {
  int items[ITEM_MAX];
  int n = bagItems(items);
  int pages = max(1, (n + 11) / 12);
  if (bagPage >= pages) bagPage = 0;
  screenTop("Bag", pages > 1 ? "Page " + String(bagPage + 1) + "/" + pages + "  >" : String(n) + " here");
  const char* tabs[] = {"Food", "Gear", "Room", "Treasure"};
  for (int i = 0; i < 4; i++) drawButton(8 + i * 77, 60, 72, 24, tabs[i], i == bagTab ? C_ACCENT2 : C_CARD, C_WHITE);
  if (n == 0) {
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_MC);
    tft.setTextColor(C_DIM);
    tft.drawString("Nothing here yet - try missions, boxes or the shop", W / 2, 136);
  }
  for (int k = 0; k < 12 && bagPage * 12 + k < n; k++) {
    int id = items[bagPage * 12 + k];
    int x = 8 + (k % 6) * 51, y = 90 + (k / 6) * 52;
    drawItemTile(tft, id, x, y, 47, 48, true, rpg.owned[id]);
    if (itemInUse(id)) tft.fillSmoothCircle(x + 8, y + 8, 4, C_GREEN);
    if (bagSel == id) tft.fillSmoothRoundRect(x, y + 45, 47, 3, 1, C_ACCENT);
  }
  screenBottom(true);
}

void drawBookScreen() {
  int pages = (itemCount() + 17) / 18;
  if (bookPage >= pages) bookPage = 0;
  screenTop("Collection", String(foundCount()) + "/" + itemCount() + "  page " + (bookPage + 1) + "/" + pages + " >");
  for (int k = 0; k < 18; k++) {
    int id = bookPage * 18 + k;
    if (id >= itemCount()) break;
    int x = 8 + (k % 6) * 51, y = 60 + (k / 6) * 44;
    drawItemTile(tft, id, x, y, 47, 40, hasFound(id), 0);
  }
  screenBottom(true);
}

void drawBadgesScreen() {
  screenTop("Badges", String(badgeCount()) + "/" + String((int)B_COUNT) + " earned");
  for (int b = 0; b < B_COUNT; b++) {
    int cx = 27 + (b % 8) * 38, cy = 78 + (b / 8) * 32;
    bool got = hasBadge(b);
    uint16_t col = got ? (b < 8 ? C_ACCENT : b < 16 ? C_BLUE : b < 24 ? C_PURPLE : C_AMBER) : C_CARD;
    tft.fillSmoothCircle(cx, cy, 14, col);
    if (got) {
      drawShape(tft, S_STAR, cx, cy, 18, 0xFFFFFF, 0xFFFFFF);
    } else {
      tft.setFont(F_SMALL);
      tft.setTextDatum(D_MC);
      tft.setTextColor(C_DIM);
      tft.drawString("?", cx, cy);
    }
    if (badgeSel == b) tft.drawCircle(cx, cy, 16, C_WHITE);
  }
  screenBottom(true);
}

void drawQuestRow(const Quest& q, int y) {
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  tft.setTextColor(q.done ? C_DIM : C_WHITE);
  tft.drawString(fitText(questText(q.kind), 168).c_str(), 12, y);
  drawProgressBar(184, y + 4, 70, 8, q.goal ? (float)q.progress / q.goal : 0, q.done ? C_GREEN : C_ACCENT);
  tft.setTextDatum(D_TR);
  tft.setTextColor(q.done ? C_GREEN : C_GREY);
  tft.drawString(q.done ? "Done!" : (String(min(q.progress, q.goal)) + "/" + q.goal).c_str(), W - 12, y);
}

void drawQuestsScreen() {
  refreshQuests();
  screenTop("Quests", "Streak " + String(rpg.streak) + " days (best " + rpg.bestStreak + ")");
  drawLabel(12, 58, "TODAY", C_ACCENT, "+50 XP each");
  for (int i = 0; i < 3; i++) drawQuestRow(rpg.daily[i], 76 + i * 20);
  drawLabel(12, 136, "THIS WEEK", C_ACCENT, "+250 XP each, both = a box");
  for (int i = 0; i < 2; i++) drawQuestRow(rpg.weekly[i], 154 + i * 20);
  if (cfg.city.length() && pageEnabled(PAGE_FUEL)) drawButton(8, BOTTOM_Y, 120, BOTTOM_H, "I filled up", C_CARD, C_WHITE);
  if (screenMsg.length()) {
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_ML);
    tft.setTextColor(C_CLOUD);
    tft.drawString(fitText(screenMsg, 96).c_str(), 134, BOTTOM_Y + BOTTOM_H / 2);
  }
  drawButton(W - 88, BOTTOM_Y, 80, BOTTOM_H, "Back", C_LINE, C_WHITE);
}

void drawPetsScreen() {
  screenTop("Pets", "Level " + String(rpg.level));
  for (int sp = 0; sp < SP_COUNT; sp++) {
    int x = 6 + sp * 62, y = 60, w = 58, h = 128;
    bool open = speciesUnlocked(sp), active = cfg.petType == sp;
    tft.fillSmoothRoundRect(x, y, w, h, 8, active ? C_ACCENT2 : C_CARD);
    tft.fillSmoothRoundRect(x + 2, y + 2, w - 4, h - 4, 7, C_CARD);
    if (sp == SP_DRAGON && open && !rpg.dragonHatched) drawShape(tft, S_EGG, x + w / 2, y + 34, 40, speciesColour(SP_DRAGON, 'X', 0, 0), 0xFFFFFF);
    else drawPetIcon(sp, x + 5, y + 10, 3, !open);
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TC);
    tft.setTextColor(open ? C_WHITE : C_DIM);
    tft.drawString(speciesName(sp), x + w / 2, y + 66);
    tft.setTextColor(active ? C_ACCENT : C_GREY);
    String sub = !open ? "Lv " + String(SPECIES_LEVEL[sp]) : active ? String("Active") : sp == SP_CLASSIC ? String("") : String(coatName(sp));
    if (sp == SP_DRAGON && open && !rpg.dragonHatched) sub = rpg.dragonEggAt ? "Hatching" : "Egg";
    tft.drawString(fitText(sub, w - 6).c_str(), x + w / 2, y + 86);
  }
  screenBottom(true);
}

void drawPetScreen() {
  switch (petMenu) {
    case MENU_HUB:         drawHubScreen(); break;
    case MENU_MISSIONS:    drawMissionsScreen(); break;
    case MENU_MISSION_LEN: drawMissionLenScreen(); break;
    case MENU_REPORT:      drawReportScreen(); break;
    case MENU_FIGHT_PICK:  drawFightPickScreen(); break;
    case MENU_SHOP:        drawShopScreen(); break;
    case MENU_BAG:         drawBagScreen(); break;
    case MENU_BOOK:        drawBookScreen(); break;
    case MENU_BADGES:      drawBadgesScreen(); break;
    case MENU_QUESTS:      drawQuestsScreen(); break;
    case MENU_PETS:        drawPetsScreen(); break;
    default: break;
  }
  drawPageDots();  // clearing the screen took them off
}

// ---------------------------------------------------------------- taps on the full screens

void petScreenTap(int x, int y) {
  bool back = tappedBack(x, y);
  switch (petMenu) {
    case MENU_HUB: {
      if (rpg.boxes > 0 && hitBox(x, y, 8, 74, W - 16, 26)) {
        startBox();
        return;
      }
      int top = rpg.boxes > 0 ? 104 : 76;
      if (y < top) return;
      int col = constrain((x - 8) / 103, 0, 2), row = (y - top) / 38;
      if (row > 2) return;
      int i = row * 3 + col;
      const int dest[9] = {MENU_MISSIONS, MENU_FIGHT_PICK, MENU_SHOP, MENU_QUESTS, MENU_BAG, MENU_BOOK, MENU_BADGES, MENU_PETS, MENU_MAIN};
      if (dest[i] == MENU_MISSIONS && rpg.reportPending) {
        openPetMenu(MENU_REPORT);
        return;
      }
      openPetMenu(dest[i]);
      return;
    }
    case MENU_MISSIONS:
      if (back) return openPetMenu(MENU_HUB);
      if (missionActive()) {
        if (hitBox(x, y, 8, 156, 200, 30)) {
          callBackMission();
          screenMsg = "It came straight home";
          drawPetScreen();
        }
        return;
      }
      if (y >= 62 && y < 62 + 4 * 32) {
        int p = ((y - 62) / 32) * 2 + (x >= 158 ? 1 : 0);
        if (p >= PLACE_COUNT) return;
        if (!placeOpen(p)) {
          screenMsg = String(PLACES[p].name) + " opens at level " + PLACES[p].level;
          drawPetScreen();
          return;
        }
        placeSel = p;
        openPetMenu(MENU_MISSION_LEN);
      }
      return;
    case MENU_MISSION_LEN:
      if (back) return openPetMenu(MENU_MISSIONS);
      if (y >= 84 && y < 84 + 3 * 36) {
        String msg = startMission(placeSel, (y - 84) / 36);
        petSay(msg, 3000);
        if (missionActive()) {
          petMenu = MENU_MAIN;
          drawPetPage();
        } else {
          screenMsg = msg;
          drawPetScreen();
        }
      }
      return;
    case MENU_REPORT:
      if (hitBox(x, y, W - 88, BOTTOM_Y, 80, BOTTOM_H)) {
        rpg.reportPending = 0;
        saveRpg();
        openPetMenu(MENU_HUB);
      }
      return;
    case MENU_FIGHT_PICK:
      if (back) return openPetMenu(MENU_HUB);
      if (hitBox(x, y, 8, 62, 40, 28)) fightPlace = (fightPlace + PLACE_COUNT - 1) % PLACE_COUNT;
      else if (hitBox(x, y, W - 48, 62, 40, 28)) fightPlace = (fightPlace + 1) % PLACE_COUNT;
      else if (y >= 98 && y < 150) {
        String msg = startBattle(fightPlace, x >= 160);
        if (battle.on) {
          petMenu = MENU_BATTLE;
          drawPetPage();
          return;
        }
        screenMsg = msg;
      }
      drawPetScreen();
      return;
    case MENU_SHOP: {
      if (back) return openPetMenu(MENU_HUB);
      if (y < 60 || y >= 60 + 2 * 66) return;
      int i = ((y - 60) / 66) * 3 + constrain((x - 8) / 103, 0, 2);
      int stock[5];
      shopStock(stock);
      int id = i < 5 ? stock[i] : -2;
      if (shopSel == i) {
        screenMsg = buyItem(id);
        shopSel = -1;
      } else {
        shopSel = i;
        screenMsg = id == -2 ? "A random item - tap again to buy"
                             : String(itemDef(id).name) + " (" + rarityName(itemDef(id).rarity) + " " +
                                   itemTypeName(itemDef(id).type) + ") - tap again to buy";
      }
      drawPetScreen();
      return;
    }
    case MENU_BAG: {
      if (back) return openPetMenu(MENU_HUB);
      if (y < 58) {  // the page arrow in the title
        bagPage++;
        bagSel = -1;
      } else if (y < 86) {
        bagTab = constrain((x - 8) / 77, 0, 3);
        bagPage = 0;
        bagSel = -1;
        screenMsg = "";
      } else if (y < 194) {
        int items[ITEM_MAX];
        int n = bagItems(items);
        int k = ((y - 90) / 52) * 6 + constrain((x - 8) / 51, 0, 5);
        int idx = bagPage * 12 + k;
        if (k < 0 || k >= 12 || idx >= n) return;
        int id = items[idx];
        if (bagSel == id) {
          screenMsg = useItem(id);
          if (!rpg.owned[id]) bagSel = -1;
        } else {
          bagSel = id;
          const ItemDef& d = itemDef(id);
          String action = d.type == IT_FOOD ? "eat" : gearSlotFor(d.type) >= 0 ? (itemInUse(id) ? "take off" : "wear")
                        : roomSlotFor(d.type) >= 0 ? (itemInUse(id) ? "put away" : "place") : "";
          screenMsg = String(d.name) + " (" + rarityName(d.rarity) + ")" + (action.length() ? " - tap again to " + action : "");
        }
      }
      drawPetScreen();
      return;
    }
    case MENU_BOOK: {
      if (back) return openPetMenu(MENU_HUB);
      if (y < 58) {
        bookPage++;
      } else if (y >= 60 && y < 60 + 3 * 44) {
        int id = bookPage * 18 + ((y - 60) / 44) * 6 + constrain((x - 8) / 51, 0, 5);
        if (id < itemCount()) {
          const ItemDef& d = itemDef(id);
          screenMsg = hasFound(id) ? String(d.name) + " - " + rarityName(d.rarity) + " " + itemTypeName(d.type) +
                                         (d.season ? " (seasonal)" : "")
                                   : String("Not found yet (") + rarityName(d.rarity) + ")";
        }
      }
      drawPetScreen();
      return;
    }
    case MENU_BADGES: {
      if (back) return openPetMenu(MENU_HUB);
      if (y >= 62 && y < 62 + 4 * 32) {
        int b = ((y - 62) / 32) * 8 + constrain((x - 8) / 38, 0, 7);
        if (b < B_COUNT) {
          badgeSel = b;
          screenMsg = String(BADGE_NAMES[b]) + ": " + BADGE_HINTS[b];
        }
      }
      drawPetScreen();
      return;
    }
    case MENU_QUESTS:
      if (back) return openPetMenu(MENU_HUB);
      if (cfg.city.length() && pageEnabled(PAGE_FUEL) && hitBox(x, y, 8, BOTTOM_Y, 120, BOTTOM_H)) {
        screenMsg = rpgLogFillUp();
        drawPetScreen();
      }
      return;
    case MENU_PETS:
      if (back) return openPetMenu(MENU_HUB);
      if (y >= 60 && y < 188) {
        int sp = constrain((x - 6) / 62, 0, SP_COUNT - 1);
        screenMsg = switchSpecies(sp);
        drawPetScreen();
      }
      return;
    default:
      return;
  }
}

// ---------------------------------------------------------------- mystery boxes and level-ups (in the scene)

void startBox() {
  boxItem = openMysteryBox();
  if (boxItem < 0) return;
  boxAt = millis();
  petMenu = MENU_BOX;
  drawPetPage();
}

void boxFrame() {
  unsigned long t = millis() - boxAt;
  for (int y = 0; y < PET_SCENE_H; y++) canvas.drawFastHLine(0, y, W, bgAt(PET_SCENE_Y + y));
  int cx = W / 2, cy = 66;
  if (t < 1300) {  // shake, faster and faster
    int amp = 2 + (int)(t / 200);
    int dx = (petFrameNo % 2) ? amp : -amp;
    drawShape(canvas, S_BOX, cx + dx, cy, 80, 0xFFC83D, 0xE03050);
    return;
  }
  const ItemDef& d = itemDef(boxItem);
  uint16_t rc = rarityColour(d.rarity);
  if (t < 1600) {  // flash
    canvas.fillCircle(cx, cy, (int)((t - 1300) / 3), C_WHITE);
    return;
  }
  for (int i = 0; i < 12; i++) {  // rays in the rarity colour
    float a = i * 0.5236f + (t / 900.0f);
    canvas.fillTriangle(cx, cy, cx + (int)(cosf(a) * 120), cy + (int)(sinf(a) * 120),
                        cx + (int)(cosf(a + 0.18f) * 120), cy + (int)(sinf(a + 0.18f) * 120), mix(to888(rc), to888(bgAt(PET_SCENE_Y + cy)), 0.55f));
  }
  canvas.fillCircle(cx, cy, 40, mix(to888(rc), 0x000000, 0.55f));
  drawItemIcon(canvas, boxItem, cx, cy, 56);
  canvas.setFont(F_BOLD);
  canvas.setTextSize(1);
  canvas.setTextDatum(D_TC);
  canvas.setTextColor(C_WHITE);
  canvas.drawString(d.name, cx, 110);
  canvas.setFont(F_SMALL);
  canvas.setTextColor(rc);
  canvas.drawString(String(rarityName(d.rarity)) + " " + itemTypeName(d.type) + (rpg.owned[boxItem] > 1 ? "  (you have " + String(rpg.owned[boxItem]) + ")" : "  - NEW!"), cx, 128);
}

void levelUpFrame() {
  for (int y = 0; y < PET_SCENE_H; y++) canvas.drawFastHLine(0, y, W, bgAt(PET_SCENE_Y + y));
  for (int i = 0; i < 24; i++) {  // confetti
    int x = (i * 53 + petFrameNo * (3 + i % 4)) % W, y = (i * 29 + petFrameNo * (4 + i % 3)) % PET_SCENE_H;
    const uint16_t cols[] = {C_ACCENT, C_PURPLE, C_YELLOW, C_RED, C_ACCENT2};
    canvas.fillRect(x, y, 4, 6, cols[i % 5]);
  }
  int s = 4, cx = W / 2;
  int hop = (petFrameNo % 2) ? -8 : 0;
  drawPetPose(cfg.petType, (petFrameNo % 2) ? POSE_SIT2 : POSE_SIT, cx - 8 * s, 70 + hop, s, (petFrameNo / 2) % 2, false, 0, false);
  canvas.setTextSize(1);
  canvas.setFont(F_LARGE);
  canvas.setTextDatum(D_TC);
  canvas.setTextColor(C_YELLOW);
  canvas.drawString("Level " + String(levelShown) + "!", cx, 8);
  canvas.setFont(F_SMALL);
  canvas.setTextColor(C_WHITE);
  canvas.drawString(String(petTitle(levelShown)) + " - " + rpg.coins + " coins", cx, 48);
}

// Buttons under the scene for battles, games, boxes and level-ups
void drawSpecialButtons() {
  fillBg(0, PET_BTN_Y - 4, W, PET_BTN_H + 6);
  if (petMenu == MENU_BATTLE) {
    if (battle.over) {
      drawButton(80, PET_BTN_Y, 160, PET_BTN_H, "OK", C_GREEN, RGB565(5, 30, 20));
    } else {
      bool ready = battle.charge >= 100 || battle.special;
      drawButton(4, PET_BTN_Y, 200, PET_BTN_H, battle.special ? "Special ready!" : ready ? "Special move!" : "Special (charging)",
                 ready ? C_YELLOW : C_CARD, ready ? RGB565(40, 30, 0) : C_DIM);
      drawButton(212, PET_BTN_Y, 104, PET_BTN_H, "Run away", C_LINE, C_WHITE);
    }
  } else if (petMenu == MENU_MINIGAME) {
    if (mini.over) {
      drawButton(4, PET_BTN_Y, 150, PET_BTN_H, "Again", C_ACCENT2, C_WHITE);
      drawButton(166, PET_BTN_Y, 150, PET_BTN_H, "Done", C_LINE, C_WHITE);
    } else {
      drawButton(80, PET_BTN_Y, 160, PET_BTN_H, "Stop", C_LINE, C_WHITE);
    }
  } else if (petMenu == MENU_BOX) {
    if (rpg.boxes > 0) {
      drawButton(4, PET_BTN_Y, 190, PET_BTN_H, "Open another (" + String(rpg.boxes) + ")", C_AMBER, RGB565(40, 24, 0));
      drawButton(202, PET_BTN_Y, 114, PET_BTN_H, "Done", C_LINE, C_WHITE);
    } else {
      drawButton(80, PET_BTN_Y, 160, PET_BTN_H, "Done", C_GREEN, RGB565(5, 30, 20));
    }
  } else if (petMenu == MENU_LEVELUP) {
    drawButton(80, PET_BTN_Y, 160, PET_BTN_H, "Yay!", C_GREEN, RGB565(5, 30, 20));
  }
}

// One frame of whichever special scene is up
void specialSceneFrame() {
  if (!ensureCanvas(W, PET_SCENE_H)) return;
  petFrameNo++;
  static bool wasOver = false;
  bool overNow = false;
  if (petMenu == MENU_BATTLE) {
    battleFrame();
    overNow = battle.over;
  } else if (petMenu == MENU_MINIGAME) {
    miniGameFrame();
    overNow = mini.over;
  } else if (petMenu == MENU_BOX) {
    boxFrame();
  } else if (petMenu == MENU_LEVELUP) {
    levelUpFrame();
  }
  canvas.pushSprite(0, PET_SCENE_Y);
  if (overNow != wasOver) drawSpecialButtons();  // the buttons change when it ends
  wasOver = overNow;
}

void specialTap(int x, int y) {
  if (y < PET_BTN_Y - 8) {
    if (petMenu == MENU_MINIGAME) miniTap(x, y - PET_SCENE_Y);
    return;
  }
  if (petMenu == MENU_BATTLE) {
    if (battle.over) {
      battle.on = false;
      openPetMenu(MENU_FIGHT_PICK);
    } else if (x < 208) {
      useSpecial();
      drawSpecialButtons();
    } else {
      battle.on = false;
      petSay("Ran home!", 2000);
      openPetMenu(MENU_FIGHT_PICK);
    }
  } else if (petMenu == MENU_MINIGAME) {
    if (mini.over && x < W / 2) {
      petSay(startMiniGame(mini.which), 2000);
      drawSpecialButtons();
    } else if (mini.over) {
      openPetMenu(MENU_MAIN);
    } else {
      endMiniGame();
      drawSpecialButtons();
    }
  } else if (petMenu == MENU_BOX) {
    if (rpg.boxes > 0 && x < 198) startBox();
    else openPetMenu(MENU_HUB);
  } else if (petMenu == MENU_LEVELUP) {
    openPetMenu(MENU_MAIN);
  }
}

// When the pet page opens: show a level-up or a finished mission first
void petPageOpened() {
  rpgCheckIn();
  if (pendingLevelUp && petIsAlive() && petMenu == MENU_MAIN) {
    levelShown = pendingLevelUp;
    pendingLevelUp = 0;
    levelUpAt = millis();
    petMenu = MENU_LEVELUP;
  }
}

// How often the pet page animates
int petAnimInterval() {
  return petSpecialScene() ? 50 : petFullScreen() ? 0 : 250;
}
