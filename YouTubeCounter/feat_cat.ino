// The cat companion: the desk pet as a pixel cat (Settings > Pet type).
//
// Same care rules as the classic pet, plus cat things: it purrs when you
// stroke it, chases a laser dot, grooms, stretches, naps, gets the zoomies,
// sits in boxes, chases its tail, kneads, yawns, slow-blinks at you, bats a
// ball of yarn, knocks your mug off the table when ignored, and now and then
// strolls across whatever page is showing or naps on the title bar.
// Its coat is a random surprise when it wakes up for the first time.

// ---------------------------------------------------------------- sprites (16x16)
// . clear  X fur  k black  w white  p pink  b/B basket

const char* CAT_SIT[16] = {
  "................", "..X..........X..", "..XX........XX..", "..XpX......XpX..",
  "..XXXXXXXXXXXX..", ".XXXXXXXXXXXXXX.", ".XXXkwXXXXkwXXX.", ".XXXkkXXXXkkXXX.",
  "wwXXXXXppXXXXXww", ".XXXXXXkkXXXXXX.", "..XXXXXXXXXXXX..", "...XXXXXXXXXX...",
  "...XXXXXXXXXX.XX", "...XXXXXXXXXXXX.", "...XX.XXXX.XX...", "................"};
const char* CAT_SIT2[16] = {
  "................", "..X..........X..", "..XX........XX..", "..XpX......XpX..",
  "..XXXXXXXXXXXX..", ".XXXXXXXXXXXXXX.", ".XXXkwXXXXkwXXX.", ".XXXkkXXXXkkXXX.",
  "wwXXXXXppXXXXXww", ".XXXXXXkkXXXXXX.", "..XXXXXXXXXXXX..", "...XXXXXXXXXX...",
  "XX.XXXXXXXXXX...", ".XXXXXXXXXXXX...", "...XX.XXXX.XX...", "................"};
const char* CAT_LOAF[16] = {
  "................", "................", "................", "................",
  "................", "................", "................", "...X..X.........",
  "...XXXX.........", "..XXXXXXXXXXX...", ".XXkkXXXXXXXXXX.", ".XXXXpXXXXXXXXX.",
  ".XXXXXXXXXXXXXXX", "..XXXXXXXXXXXXXX", "...XXXXXXXXXXXX.", "................"};
const char* CAT_WALK1[16] = {
  "................", "................", "................", "................",
  "............X.X.", "............XXX.", "XX.........XkXX.", ".X........XXXXpX",
  ".XXXXXXXXXXXXX..", "..XXXXXXXXXXXX..", "..XXXXXXXXXXX...", "..XX.X....XX.X..",
  "..X..X....X..X..", "................", "................", "................"};
const char* CAT_WALK2[16] = {
  "................", "................", "................", "................",
  "............X.X.", "............XXX.", "XX.........XkXX.", ".X........XXXXpX",
  ".XXXXXXXXXXXXX..", "..XXXXXXXXXXXX..", "..XXXXXXXXXXX...", "...XX.X..XX.X...",
  "....X.X...X.X...", "................", "................", "................"};
const char* CAT_STRETCH[16] = {
  "................", "................", "................", "................",
  "................", "XX..............", ".X..............", ".XXXXXXX........",
  "..XXXXXXXXX.X.X.", "..XX....XXXXXXX.", "..X.......XXkXX.", "..X.......XXXXpX",
  "..X......XXXXX..", "..X.....X.X.....", "................", "................"};
const char* CAT_BASKET[16] = {
  "................", "................", "................", "................",
  "................", "................", "....X......X....", "....XX....XX....",
  "...XXXXXXXXXX...", "..bbbbbbbbbbbb..", ".bBbBbBbBbBbBbB.", ".BbBbBbBbBbBbBb.",
  ".bBbBbBbBbBbBbB.", "..bbbbbbbbbbbb..", "................", "................"};

// Coats: ginger, tabby, black, white, tuxedo, calico
const char* COAT_NAMES[] = {"ginger", "tabby", "black", "white", "tuxedo", "calico"};

int coatOverride = -1;  // used to draw a visiting cat in its own coat

uint8_t catCoat() {
  if (coatOverride >= 0) return coatOverride % 6;
  return pet.spare[0] % 6;
}

// Picks a random coat the first time the cat is seen
void ensureCatCoat() {
  if (pet.spare[1]) return;
  pet.spare[0] = esp_random() % 6;
  pet.spare[1] = 1;
  savePet();
}

uint32_t coatColour(int r, int c) {
  switch (catCoat()) {
    case 0: return (r + c / 2) % 3 == 0 ? 0xC96A1F : 0xF2994A;              // ginger stripes
    case 1: return (r + c / 2) % 3 == 0 ? 0x5C636C : 0x9AA0A8;              // grey tabby
    case 2: return 0x2A2A34;                                                // black
    case 3: return (r == 1 || r == 2) ? 0xD8DCE3 : 0xF2F2F5;                // white
    case 4: return (r >= 9 && c >= 5 && c <= 10) || r >= 14 ? 0xF2F2F5 : 0x24242C;  // tuxedo
    default: {                                                              // calico patches
      int patch = ((r / 4) * 3 + (c / 5)) % 3;
      return patch == 0 ? 0xF4F1EA : patch == 1 ? 0xE8893A : 0x2E2A2A;
    }
  }
}

void drawCatSprite(const char** spr, int x0, int y0, int s, bool flip, bool eyesShut, float dim) {
  bool darkCoat = catCoat() == 2 || catCoat() == 4;
  for (int r = 0; r < 16; r++) {
    for (int c = 0; c < 16; c++) {
      int sc = flip ? 15 - c : c;
      char ch = spr[r][sc];
      if (ch == '.') continue;
      uint32_t col;
      switch (ch) {
        case 'k': {
          char below = r < 15 ? spr[r + 1][sc] : '.';
          if (eyesShut && (below == 'k' || below == 'w')) col = coatColour(r, sc);
          else col = darkCoat && !eyesShut ? 0xC6F04A : 0x101018;  // green eyes on dark cats
          break;
        }
        case 'w': col = eyesShut ? 0x101018 : 0xFFFFFF; break;
        case 'p': col = 0xFF8FB3; break;
        case 'b': col = 0x8B5A2B; break;
        case 'B': col = 0xC08A4B; break;
        default:  col = coatColour(r, sc); break;
      }
      canvas.fillRect(x0 + c * s, y0 + r * s, s, s, mix(col, 0x000000, dim));
    }
  }
}

// ---------------------------------------------------------------- behaviour

CatHabit      catHabit = H_SIT;
unsigned long catHabitUntil = 0;
float         catX = W / 2, catTargetX = W / 2, yarnX = 60, yarnV = 0;
int           catDir = 1;
float         mugX = 0;              // how far the mug has been pushed
int           laserX = -1, laserY = -1;
unsigned long laserUntil = 0, lastStrokeBonus = 0;
int           heartsT = 0;

void catStartHabit(CatHabit h, unsigned long ms) {
  catHabit = h;
  catHabitUntil = millis() + ms;
  if (h == H_WALK) catTargetX = 50 + esp_random() % (W - 100);
  if (h == H_YARN) {
    yarnX = catX + (catDir > 0 ? 30 : -30);
    yarnV = catDir * 3.0f;
  }
  if (h == H_MUG) mugX = 0;
}

// Picks something to do when it's idle
void catPickHabit() {
  if (pet.calledAt && (esp_random() % 3 == 0)) {
    catStartHabit(H_MUG, 5000);  // ignored? knock the mug off the table
    petSay("Meow!!", 2500);
    return;
  }
  const CatHabit idle[] = {H_SIT, H_SIT, H_GROOM, H_STRETCH, H_WALK, H_WALK, H_ZOOMIES, H_NAP,
                           H_BOX, H_CHASETAIL, H_KNEAD, H_YAWN, H_BLINK, H_YARN};
  CatHabit h = idle[esp_random() % (sizeof(idle) / sizeof(idle[0]))];
  unsigned long ms = h == H_NAP ? 20000 : h == H_ZOOMIES ? 4000 : h == H_BOX ? 9000 : 5000 + esp_random() % 4000;
  catStartHabit(h, ms);
  if (h == H_BOX) petSay("If it fits, I sits", 2500);
  if (h == H_BLINK) petSay("(slow blink = I love you)", 2500);
}

void catOnTap() {
  const char* sounds[] = {"Mrrp?", "Meow!", "Mew", "Prrrt?"};
  petSay(sounds[esp_random() % 4], 1500);
  if (catHabit == H_NAP) catStartHabit(H_YAWN, 2500);
}

// Stroking (sliding a finger across it) makes it purr
void catStroke() {
  if (pet.asleep || !petIsAlive()) return;
  if (catHabit != H_PURR) petSay("Purrrrr...", 2500);
  catStartHabit(H_PURR, 3000);
  if (millis() - lastStrokeBonus > 120000 && pet.happy < 4) {
    pet.happy++;
    lastStrokeBonus = millis();
    savePet();
  }
}

// ---------------------------------------------------------------- drawing in the pet scene

// Draws the cat and its props for one frame. floorY is the top of the floor strip.
void drawCatScene(int floorY, bool dark) {
  unsigned long now = millis();
  int s = pet.stage == PET_BABY ? 4 : pet.stage == PET_CHILD ? 5 : 6;
  bool shut = pet.asleep;

  if (petMenu == MENU_LASER && millis() > laserUntil) finishLaser();
  if (pet.stage == PET_EGG) {  // a kitten asleep in its basket
    drawCatSprite(CAT_BASKET, W / 2 - 8 * 6, floorY - 15 * 6, 6, false, true, dark ? 0.6f : 0);
    return;
  }

  if (pet.asleep) {
    catHabit = H_NAP;
  } else if (petMenu == MENU_LASER) {
    catHabit = H_LASER;
  } else if (now > catHabitUntil && catHabit != H_PURR) {
    catPickHabit();
  } else if (now > catHabitUntil) {
    catHabit = H_SIT;
  }

  // Movement
  float speed = catHabit == H_ZOOMIES ? 9 : catHabit == H_LASER ? 6 : 1.6f;
  if (catHabit == H_WALK || catHabit == H_LASER || catHabit == H_ZOOMIES) {
    float target = catHabit == H_ZOOMIES ? (catDir > 0 ? W - 40 : 40) : catHabit == H_LASER ? laserX : catTargetX;
    if (catHabit == H_LASER && laserX < 0) target = catX;
    if (fabs(target - catX) > speed) {
      catDir = target > catX ? 1 : -1;
      catX += catDir * speed;
    } else if (catHabit == H_ZOOMIES) {
      catDir = -catDir;
    } else if (catHabit == H_WALK) {
      catHabitUntil = 0;
    }
  }
  if (catX < 40) catX = 40;
  if (catX > W - 40) catX = W - 40;
  bool moving = (catHabit == H_WALK || catHabit == H_ZOOMIES || catHabit == H_LASER) &&
                fabs((catHabit == H_LASER ? laserX : catTargetX) - catX) > speed;
  if (catHabit == H_ZOOMIES) moving = true;

  int x0 = (int)catX - 8 * s;
  int sitY = floorY - 15 * s + 2, walkY = floorY - 13 * s + 2, loafY = floorY - 15 * s + 2;
  bool flip = catDir < 0;
  float dim = dark ? 0.6f : 0;
  int frame = petFrameNo;

  // Props behind/around the cat
  if (catHabit == H_BOX) {
    int bx = (int)catX - 9 * s;
    canvas.fillRect(bx, floorY - 7 * s, 18 * s, 7 * s, RGB565(196, 150, 96));
    canvas.fillTriangle(bx, floorY - 7 * s, bx - 3 * s, floorY - 10 * s, bx + 4 * s, floorY - 7 * s, RGB565(170, 125, 76));
    canvas.fillTriangle(bx + 18 * s, floorY - 7 * s, bx + 21 * s, floorY - 10 * s, bx + 14 * s, floorY - 7 * s, RGB565(170, 125, 76));
  }
  if (catHabit == H_MUG || pet.calledAt) {
    // a little table with your mug on it
    int tx = W - 70;
    canvas.fillRect(tx, floorY - 40, 56, 5, RGB565(120, 80, 50));
    canvas.fillRect(tx + 6, floorY - 35, 4, 35, RGB565(100, 66, 40));
    canvas.fillRect(tx + 46, floorY - 35, 4, 35, RGB565(100, 66, 40));
    if (catHabit == H_MUG) {
      mugX += 1.2f;
      float fallen = mugX > 34 ? (mugX - 34) * 3 : 0;
      int mx = tx + 20 + (int)min(mugX, 40.0f), my = floorY - 52 + (int)min(fallen, 40.0f);
      canvas.fillRect(mx, my, 12, 12, C_WHITE);
      canvas.drawCircle(mx + 14, my + 6, 3, C_WHITE);
      if (fallen >= 40) {
        canvas.setFont(F_BOLD);
        canvas.setTextDatum(D_TC);
        canvas.setTextColor(C_AMBER);
        canvas.drawString("CRASH!", mx, my - 20);
      }
      catX = tx - 10;
      catDir = 1;
    } else {
      canvas.fillRect(tx + 20, floorY - 52, 12, 12, C_WHITE);
      canvas.drawCircle(tx + 34, floorY - 46, 3, C_WHITE);
    }
  }
  if (catHabit == H_YARN) {
    yarnX += yarnV;
    yarnV *= 0.97f;
    if (yarnX < 20 || yarnX > W - 20) yarnV = -yarnV;
    if (fabs(yarnX - catX) < 30 && fabs(yarnV) < 0.5f) yarnV = (yarnX > catX ? 1 : -1) * 4.0f;  // bat it
    catDir = yarnX > catX ? 1 : -1;
    catX += catDir * 0.8f;
    int yy = floorY - 8;
    canvas.fillCircle((int)yarnX, yy, 8, RGB565(230, 70, 110));
    canvas.drawLine((int)yarnX - 6, yy - 3, (int)yarnX + 6, yy + 3, RGB565(255, 160, 190));
    canvas.drawLine((int)yarnX - 4, yy + 5, (int)yarnX + 5, yy - 5, RGB565(255, 160, 190));
  }
  if (catHabit == H_LASER && laserX >= 0) {
    canvas.fillCircle(laserX, laserY, 6, RGB565(120, 0, 0));
    canvas.fillCircle(laserX, laserY, 3, RGB565(255, 40, 40));
  }

  // The cat itself
  switch (catHabit) {
    case H_NAP:
    case H_KNEAD:
      drawCatSprite(CAT_LOAF, x0, loafY, s, flip, catHabit == H_NAP, dim);
      if (catHabit == H_KNEAD) {  // making biscuits
        int px = flip ? x0 + 2 * s : x0 + 11 * s;
        canvas.fillRect(px + ((frame % 2) ? 0 : 2 * s), loafY + 14 * s, 2 * s, s, mix(coatColour(14, 3), 0, dim));
      }
      break;
    case H_STRETCH:
      drawCatSprite(CAT_STRETCH, x0, floorY - 14 * s + 2, s, flip, false, dim);
      break;
    case H_CHASETAIL:
      drawCatSprite((frame % 2) ? CAT_WALK1 : CAT_WALK2, x0, walkY, s, (frame / 2) % 2, false, dim);
      break;
    case H_BOX:
      // only the head pokes out of the box
      canvas.setClipRect(0, 0, W, floorY - 7 * s);
      drawCatSprite(CAT_SIT, x0, sitY, s, flip, false, dim);
      canvas.clearClipRect();
      break;
    case H_MUG:
      drawCatSprite(CAT_STRETCH, x0, floorY - 14 * s + 2, s, false, false, dim);
      break;
    default:
      if (moving || catHabit == H_YARN) {
        drawCatSprite((frame % 2) ? CAT_WALK1 : CAT_WALK2, x0, walkY - ((catHabit == H_LASER && laserY < floorY - 60 && fabs(laserX - catX) < 20) ? 20 : 0),
                      s, flip, false, dim);
      } else {
        bool blink = catHabit == H_BLINK ? (frame % 6) < 3 : (frame % 20) == 0;
        drawCatSprite((frame / 4) % 2 ? CAT_SIT : CAT_SIT2, x0, sitY, s, false, blink || catHabit == H_PURR, dim);
        if (catHabit == H_GROOM) {  // paw up to its mouth
          canvas.fillRect(x0 + 9 * s, sitY + (9 + (frame % 2)) * s, 3 * s, 2 * s, mix(coatColour(9, 9), 0, dim));
          canvas.fillRect(x0 + 9 * s, sitY + 11 * s, 2 * s, 3 * s, mix(coatColour(11, 9), 0, dim));
        }
        if (catHabit == H_YAWN) canvas.fillRect(x0 + 7 * s, sitY + 9 * s, 2 * s, 2 * s, RGB565(200, 60, 90));
      }
  }

  // Floating hearts when purring
  if (catHabit == H_PURR) {
    heartsT++;
    for (int i = 0; i < 3; i++) {
      int hy = sitY - ((heartsT * 3 + i * 25) % 70);
      int hx = (int)catX - 30 + i * 30;
      uint16_t hc = RGB565(255, 90, 140);
      canvas.fillCircle(hx - 3, hy, 4, hc);
      canvas.fillCircle(hx + 3, hy, 4, hc);
      canvas.fillTriangle(hx - 7, hy + 1, hx + 7, hy + 1, hx, hy + 9, hc);
    }
  }
}

// ---------------------------------------------------------------- strolling across other pages

const int WANDER_Y = 176, WANDER_H = 50;
LGFX_Sprite   wanderBg(&tft);
bool          wandering = false;
float         wanderX = 0;
int           wanderDir = 1;
unsigned long wanderPauseUntil = 0;
bool          wanderPaused = false;
unsigned long catNapUntil = 0;      // napping on the title bar

bool catWandering() {
  return wandering;
}

bool catOnHeader() {
  return cfg.petType == 1 && millis() < catNapUntil;
}

bool canWanderHere() {
  if (currentPage < 0 || infoShowing || keyboardOpen || lampGlowing() || userBusy()) return false;
  switch (currentPage) {
    case PAGE_PET: case PAGE_SCREENSAVER: case PAGE_MOODLAMP: case PAGE_MESSAGES:
    case PAGE_STOPWATCH: case PAGE_EIGHTBALL:
      return false;
  }
  return true;
}

// Called once a minute: every so often the cat comes for a stroll or a nap
void maybeCatVisit() {
  if (cfg.petType != 1 || !petIsAlive() || pet.asleep || !pageEnabled(PAGE_PET) || wandering) return;
  if (!canWanderHere() || esp_random() % 12 != 0) return;
  if (esp_random() % 3 == 0) {
    catNapUntil = millis() + 2 * 60 * 1000UL;
    drawHeader();
    return;
  }
  startWander();
}

void startWander() {
  // Copy the strip of screen it walks across, so it can be put back behind the cat
  wanderBg.setColorDepth(16);
  canvas.deleteSprite();
  canvas.setColorDepth(16);
  if (!wanderBg.createSprite(W, WANDER_H)) return;
  if (!canvas.createSprite(W, WANDER_H)) {
    wanderBg.deleteSprite();
    return;
  }
  tft.readRect(0, WANDER_Y, W, WANDER_H, (lgfx::swap565_t*)wanderBg.getBuffer());
  wanderDir = (esp_random() & 1) ? 1 : -1;
  wanderX = wanderDir > 0 ? -40 : W + 40;
  wanderPaused = false;
  wanderPauseUntil = 0;
  wanderDirty = false;
  wandering = true;
}

void endWander() {
  wandering = false;
  wanderBg.pushSprite(0, WANDER_Y);
  wanderBg.deleteSprite();
  canvas.deleteSprite();
  if (wanderDirty) drawCurrentPage();
}

void wanderFrame() {
  if (!wandering) return;
  unsigned long now = millis();
  // Halfway across it sometimes stops to say hello
  if (!wanderPaused && fabs(wanderX - W / 2) < 3 && esp_random() % 2 == 0) {
    wanderPaused = true;
    wanderPauseUntil = now + 2500;
  }
  bool sitting = wanderPaused && now < wanderPauseUntil;
  if (!sitting) wanderX += wanderDir * 3.2f;

  memcpy(canvas.getBuffer(), wanderBg.getBuffer(), W * WANDER_H * 2);
  const int s = 3;
  int x0 = (int)wanderX - 8 * s;
  if (sitting) {
    drawCatSprite(CAT_SIT, x0, WANDER_H - 15 * s - 1, s, false, false, 0);
    canvas.fillRoundRect(x0 + 44, 2, 46, 18, 6, C_WHITE);
    canvas.setFont(F_SMALL);
    canvas.setTextDatum(D_MC);
    canvas.setTextColor(RGB565(20, 20, 30));
    canvas.drawString("Meow!", x0 + 67, 11);
  } else {
    drawCatSprite((petFrameNo++ % 2) ? CAT_WALK1 : CAT_WALK2, x0, WANDER_H - 13 * s - 1, s, wanderDir < 0, false, 0);
  }
  canvas.pushSprite(0, WANDER_Y);
  if (wanderX < -50 || wanderX > W + 50) endWander();
}

// A tiny cat curled up asleep on the title bar
void drawHeaderCat() {
  if (!catOnHeader() || !petIsAlive()) return;
  const int s = 2;
  int x0 = W - 128, y0 = 34 - 15 * s;
  bool darkCoat = catCoat() == 2 || catCoat() == 4;
  for (int r = 7; r < 15; r++) {
    for (int c = 0; c < 16; c++) {
      char ch = CAT_LOAF[r][c];
      if (ch == '.') continue;
      uint32_t col = ch == 'k' ? 0x101018 : ch == 'p' ? 0xFF8FB3 : coatColour(r, c);
      if (darkCoat && ch == 'X') col = coatColour(r, c);
      tft.fillRect(x0 + c * s, y0 + r * s, s, s, to565(col));
    }
  }
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_CLOUD);
  tft.drawString("z", x0 + 34, y0 + 8);
}
