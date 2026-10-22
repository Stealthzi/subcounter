// The pets that unlock as you level up: a dog (level 10), a hamster (20) and
// a dragon that hatches from an egg (30). At level 40 every pet evolves: it
// grows a little, gets a sparkling aura, and the dragon spreads its wings.
// Also the tricks they learn (dance, backflip, juggling, magic).

// ---------------------------------------------------------------- sprites (16x16)
// . clear  X body  Y ears/belly  W wings  H horns  k black  w white  p pink

const char* DOG_SIT[16] = {
  "................", "..YY........YY..", ".YYYY......YYYY.", ".YYXXXXXXXXXXYY.",
  ".YYXXXXXXXXXXYY.", "..YXkwXXXXkwXY..", "...XkkXXXXkkX...", "...XXXXkkXXXX...",
  "...XXXXkkXXXX...", "....XXXppXXX....", "....XXXXXXXX....", "...XXXXXXXXXX...",
  "..XXXXXXXXXXXX.X", "..XXXXXXXXXXXXX.", "..XXX.XXXX.XXX..", "................"};
const char* DOG_SIT2[16] = {
  "................", "..YY........YY..", ".YYYY......YYYY.", ".YYXXXXXXXXXXYY.",
  ".YYXXXXXXXXXXYY.", "..YXkwXXXXkwXY..", "...XkkXXXXkkX...", "...XXXXkkXXXX...",
  "...XXXXkkXXXX...", "....XXXppXXX....", "....XXXXXXXX....", "...XXXXXXXXXX...",
  "X.XXXXXXXXXXXX..", ".XXXXXXXXXXXXX..", "..XXX.XXXX.XXX..", "................"};
const char* DOG_LIE[16] = {
  "................", "................", "................", "................",
  "................", "................", "................", "................",
  "................", "..YY............", ".YYYXXXXX.......", ".YXkkXXXXXXXXXX.",
  ".XXXXpXXXXXXXXXX", "..XXXXXXXXXXXXXX", "..XXXXXXXXXXXXX.", "................"};
const char* DOG_WALK1[16] = {
  "................", "................", "................", "................",
  "...........YY...", "..........YYXX..", "X.........XXkXX.", ".X........XXXXXp",
  "..XXXXXXXXXXXX..", "..XXXXXXXXXXXX..", "..XXXXXXXXXXX...", "..XX.X....XX.X..",
  "..X..X....X..X..", "................", "................", "................"};
const char* DOG_WALK2[16] = {
  "................", "................", "................", "................",
  "...........YY...", "..........YYXX..", "X.........XXkXX.", ".X........XXXXXp",
  "..XXXXXXXXXXXX..", "..XXXXXXXXXXXX..", "..XXXXXXXXXXX...", "...XX.X..XX.X...",
  "....X.X...X.X...", "................", "................", "................"};

const char* HAM_SIT[16] = {
  "................", "................", "................", "....YY....YY....",
  "...YpY....YpY...", "...XXXXXXXXXX...", "..XXXXXXXXXXXX..", "..XXkwXXXXkwXX..",
  ".XXXkkXXXXkkXXX.", ".XwwXXXppXXXwwX.", ".XwwXXXXXXXXwwX.", ".XXXXwwwwwwXXXX.",
  ".XXXwwwwwwwwXXX.", "..XXwwwwwwwwXX..", "...pp......pp...", "................"};
const char* HAM_SIT2[16] = {
  "................", "................", "................", "....YY....YY....",
  "...YpY....YpY...", "...XXXXXXXXXX...", "..XXXXXXXXXXXX..", "..XXkwXXXXkwXX..",
  ".XXXkkXXXXkkXXX.", "XwwwXXXppXXXwwwX", "XwwwXXXXXXXXwwwX", ".XXXXwwwwwwXXXX.",
  ".XXXwwwwwwwwXXX.", "..XXwwwwwwwwXX..", "...pp......pp...", "................"};
const char* HAM_BALL[16] = {
  "................", "................", "................", "................",
  "................", "................", "................", "................",
  "......XXXX......", "....XXXXXXXX....", "...XXkkXXXXXX...", "..XXXXXXXXXXXX..",
  "..XXXXwwwwXXXX..", "..XXXwwwwwwXXX..", "...XXXXXXXXXX...", "................"};
const char* HAM_WALK1[16] = {
  "................", "................", "................", "................",
  "................", "................", "................", ".........YY.....",
  "......XXXXXX....", "....XXXXXXXkX...", "...XXXXXXXXXXp..", "...XXXXwwwwXX...",
  "....XXXXXXXX....", "....p.p..p.p....", "................", "................"};
const char* HAM_WALK2[16] = {
  "................", "................", "................", "................",
  "................", "................", "................", ".........YY.....",
  "......XXXXXX....", "....XXXXXXXkX...", "...XXXXXXXXXXp..", "...XXXXwwwwXX...",
  "....XXXXXXXX....", ".....p.p.p.p....", "................", "................"};

const char* DRG_SIT[16] = {
  "................", "....H......H....", "....HX....XH....", "...XXXXXXXXXX...",
  "..XXXXXXXXXXXX..", "..XXkwXXXXkwXX..", "..XXkkXXXXkkXX..", "..XXXXXXXXXXXX..",
  "...XXXpXXpXXX...", "WW..XXXXXXXX..WW", "WWW.XXYYYYXX.WWW", ".WWXXYYYYYYXXWW.",
  "...XXYYYYYYXX...", "...XXXYYYYXXX.XX", "...XX.XXXX.XXXX.", "................"};
const char* DRG_SIT2[16] = {
  "................", "....H......H....", "....HX....XH....", "...XXXXXXXXXX...",
  "..XXXXXXXXXXXX..", "..XXkwXXXXkwXX..", "..XXkkXXXXkkXX..", "W.XXXXXXXXXXXX.W",
  "WW.XXXpXXpXXX.WW", "WWW.XXXXXXXX.WWW", ".W..XXYYYYXX..W.", "...XXYYYYYYXX...",
  "...XXYYYYYYXX...", "...XXXYYYYXXX.XX", "...XX.XXXX.XXXX.", "................"};
const char* DRG_SLEEP[16] = {
  "................", "................", "................", "................",
  "................", "................", "................", "................",
  ".....HH.........", "...XXXXXXX......", "..XkkXXXXXXXXX..", "..XXXXpXXXXXXXX.",
  ".WWXXXXXXXXXXXXX", "WWWXXYYYYYXXXXX.", "..XXXXXXXXXXXX..", "................"};
const char* DRG_WALK1[16] = {
  "................", "................", "................", "...........HH...",
  "..........XXX...", "..........XkXX..", "......WW..XXXXXp", ".....WWW.XXXX...",
  "XX..WWWXXXXXX...", ".XXXXXXXXXXXXX..", "...XXYYYYYYXX...", "...XXXXXXXXXX...",
  "...XX.X...XX.X..", "...X..X...X..X..", "................", "................"};
const char* DRG_WALK2[16] = {
  "................", "................", "................", "...........HH...",
  "..........XXX...", "..........XkXX..", "......WW..XXXXXp", ".....WWW.XXXX...",
  "XX..WWWXXXXXX...", ".XXXXXXXXXXXXX..", "...XXYYYYYYXX...", "...XXXXXXXXXX...",
  "....XX.X.XX.X...", ".....X.X..X.X...", "................", "................"};


const char** speciesPose(int sp, int pose) {
  static const char** const table[3][5] = {
    {DOG_SIT, DOG_SIT2, DOG_LIE, DOG_WALK1, DOG_WALK2},
    {HAM_SIT, HAM_SIT2, HAM_BALL, HAM_WALK1, HAM_WALK2},
    {DRG_SIT, DRG_SIT2, DRG_SLEEP, DRG_WALK1, DRG_WALK2},
  };
  if (sp == SP_CAT) {
    const char** cat[5] = {CAT_SIT, CAT_SIT2, CAT_LOAF, CAT_WALK1, CAT_WALK2};
    return cat[pose];
  }
  if (sp < SP_DOG || sp > SP_DRAGON) return SPR_ADULT;
  return table[sp - SP_DOG][pose];
}

const char* speciesName(int sp) {
  const char* n[] = {"Classic", "Cat", "Dog", "Hamster", "Dragon"};
  return sp >= 0 && sp < SP_COUNT ? n[sp] : "";
}

const char* speciesSound(int sp) {
  const char* dog[] = {"Woof!", "Arf!", "*wags*"};
  const char* ham[] = {"Squeak!", "*nibble nibble*", "Eep!"};
  const char* drg[] = {"Rawr!", "*puff of smoke*", "Grrr...?"};
  int i = esp_random() % 3;
  return sp == SP_DOG ? dog[i] : sp == SP_HAMSTER ? ham[i] : drg[i];
}

// ---------------------------------------------------------------- colours

uint32_t speciesColour(int sp, char part, int r, int c) {
  int coat = rpg.coat[sp];
  if (sp == SP_DOG) {
    switch (coat % 5) {
      case 0: return part == 'Y' ? 0x9C6B30 : 0xD9A55B;   // golden
      case 1: return part == 'Y' ? 0x4A2A14 : 0x7A4A2A;   // chocolate
      case 2: return part == 'Y' ? 0x5A4030 : 0x2A2420;   // black
      case 3: return part == 'Y' ? 0x2A2A2A : ((r * 7 + c * 3) % 9 == 0 ? 0x2A2A2A : 0xF2F2F2);  // dalmatian
      default: return part == 'Y' ? 0x4A505A : (r <= 8 ? 0x7A808A : 0xF2F2F5);                   // husky
    }
  }
  if (sp == SP_HAMSTER) {
    switch (coat % 4) {
      case 0: return part == 'Y' ? 0xD89A50 : 0xE8B060;
      case 1: return part == 'Y' ? 0xE8C8C8 : 0xF2F2F2;
      case 2: return part == 'Y' ? 0x707078 : 0xA0A0A8;
      default: return part == 'Y' ? 0x3A3A3A : (c < 8 ? 0xF2F2F2 : 0x3A3A3A);
    }
  }
  // Dragon: emerald, ruby, sapphire, amethyst
  const uint32_t body[] = {0x2EB872, 0xD83A3A, 0x3A6AD8, 0x9B5CFF};
  const uint32_t belly[] = {0xB8F2C8, 0xFFC8A0, 0xBFD8FF, 0xE0C8FF};
  const uint32_t wing[] = {0x1E7A50, 0x8A1E1E, 0x1E3A8A, 0x5A2A9A};
  int k = coat % 4;
  if (part == 'Y') return belly[k];
  if (part == 'W') return wing[k];
  if (part == 'H') return k == 3 ? 0xFFD27A : 0xF2E6B8;
  return body[k];
}

const char* coatName(int sp) {
  int coat = rpg.coat[sp];
  if (sp == SP_CAT) return COAT_NAMES[pet.spare[0] % 6];
  if (sp == SP_DOG) {
    const char* n[] = {"golden", "chocolate", "black", "dalmatian", "husky"};
    return n[coat % 5];
  }
  if (sp == SP_HAMSTER) {
    const char* n[] = {"golden", "white", "grey", "panda"};
    return n[coat % 4];
  }
  const char* n[] = {"emerald", "ruby", "sapphire", "amethyst"};
  return n[coat % 4];
}

// Draws a dog, hamster or dragon sprite into the canvas
void drawSpeciesSprite(int sp, const char** spr, int x0, int y0, int s, bool flip, bool eyesShut, float dim, bool upsideDown) {
  for (int r = 0; r < 16; r++) {
    int sr = upsideDown ? 15 - r : r;
    for (int c = 0; c < 16; c++) {
      int sc = flip ? 15 - c : c;
      char ch = spr[sr][sc];
      if (ch == '.') continue;
      uint32_t col;
      switch (ch) {
        case 'k': {
          char below = sr < 15 ? spr[sr + 1][sc] : '.';
          col = eyesShut && (below == 'k' || below == 'w') ? speciesColour(sp, 'X', sr, sc) : 0x101018;
          break;
        }
        case 'w': col = eyesShut ? 0x101018 : 0xFFFFFF; break;
        case 'p': col = sp == SP_DRAGON ? 0x101018 : 0xFF8FB3; break;
        default:  col = speciesColour(sp, ch, sr, sc); break;
      }
      canvas.fillRect(x0 + c * s, y0 + r * s, s, s, mix(col, 0x000000, dim));
    }
  }
}

// Any pet, any pose - used by tricks, battles and the pets screen
void drawPetPose(int sp, int pose, int x0, int y0, int s, bool flip, bool shut, float dim, bool upsideDown) {
  if (sp == SP_CLASSIC) {
    uint8_t stage = petIsAlive() ? pet.stage : PET_ADULT;
    drawSprite(petSprite(stage), x0, y0, s, petBodyColour(stage, pet.form), flip, shut || pose == POSE_SLEEP, dim);
    return;
  }
  if (sp == SP_CAT) {
    if (upsideDown) {  // the cat drawer has no upside-down mode, so borrow the generic one with the cat's coat
      const char** spr = speciesPose(SP_CAT, pose);
      for (int r = 0; r < 16; r++)
        for (int c = 0; c < 16; c++) {
          char ch = spr[15 - r][flip ? 15 - c : c];
          if (ch == '.') continue;
          uint32_t col = ch == 'k' ? 0x101018 : ch == 'w' ? 0xFFFFFF : ch == 'p' ? 0xFF8FB3 : coatColour(15 - r, c);
          canvas.fillRect(x0 + c * s, y0 + r * s, s, s, mix(col, 0x000000, dim));
        }
      return;
    }
    drawCatSprite(speciesPose(SP_CAT, pose), x0, y0, s, flip, shut, dim);
    return;
  }
  drawSpeciesSprite(sp, speciesPose(sp, pose), x0, y0, s, flip, shut, dim, upsideDown);
}

// The pet's pixel size for its age, a little bigger once evolved
int petPixelSize() {
  int s = pet.stage == PET_BABY ? 4 : pet.stage == PET_CHILD ? 5 : 6;
  if (petEvolved() && s == 6) s = 7;
  return s;
}

// A soft glow and twinkling sparkles around an evolved pet
void drawEvolveAura(int cx, int cy, int r) {
  uint32_t glow = theme().glow;
  canvas.fillEllipse(cx, cy, r, r * 3 / 4, mix(glow, to888(bgAt(PET_SCENE_Y + cy)), 0.82f));
  for (int i = 0; i < 5; i++) {
    float a = (petFrameNo * 0.35f) + i * 1.2566f;
    int sx = cx + (int)(cosf(a) * r), sy = cy + (int)(sinf(a) * r * 0.6f);
    int len = 2 + ((petFrameNo + i) % 3);
    canvas.fillRect(sx - len, sy, len * 2 + 1, 1, C_YELLOW);
    canvas.fillRect(sx, sy - len, 1, len * 2 + 1, C_YELLOW);
  }
}

// ---------------------------------------------------------------- the dragon egg

void checkDragonHatch() {
  if (rpg.dragonHatched || !speciesUnlocked(SP_DRAGON) || rpg.dragonEggAt == 0) return;
  if ((uint32_t)time(nullptr) >= rpg.dragonEggAt + 3600) {
    rpg.dragonHatched = 1;
    rpgNotice(String("Your dragon hatched! It's ") + coatName(SP_DRAGON) + "!");
    awardBadge(B_DRAGON);
    saveRpg();
  }
}

// Switching pet type (from the pets screen)
String switchSpecies(int sp) {
  if (!speciesUnlocked(sp)) return String(speciesName(sp)) + " unlocks at level " + SPECIES_LEVEL[sp];
  cfg.petType = sp;
  prefs.putUChar("pettype", sp);
  catHabit = H_SIT;
  catHabitUntil = 0;
  if (sp == SP_DRAGON && !rpg.dragonHatched && rpg.dragonEggAt == 0) {
    rpg.dragonEggAt = (uint32_t)time(nullptr);
    saveRpg();
    return "The dragon egg is warming up... (about an hour)";
  }
  if (sp == SP_CAT) ensureCatCoat();
  return String("Hello, ") + coatName(sp) + " " + speciesName(sp) + "!";
}

// ---------------------------------------------------------------- tricks

int           trickId = -1;
unsigned long trickUntil = 0;

const char* trickName(int i) {
  const char* n[] = {"Dance", "Backflip", "Juggle", "Magic"};
  return i >= 0 && i < 4 ? n[i] : "";
}

String startTrick(int i) {
  if (i >= tricksUnlocked()) return String(trickName(i)) + " - learn it at level " + TRICK_LEVEL[i];
  if (!petIsAlive() || pet.asleep) return "Too sleepy for tricks";
  trickId = i;
  trickUntil = millis() + 3200;
  catHabit = H_TRICK;
  catHabitUntil = trickUntil;
  struct tm t;
  if (getLocalTime(&t, 0) && rpg.trickDay[i] != t.tm_mday) {
    rpg.trickDay[i] = t.tm_mday;
    careXp(15);
  }
  questEvent(Q_TRICK, 1);
  const char* says[] = {"Watch this!", "Hup!", "Ta-daaa!", "Abracadabra!"};
  return says[i];
}

bool trickActive() {
  return trickId >= 0 && millis() < trickUntil;
}

// One frame of the current trick, with the pet centred on cx, standing on floorY
void drawTrickFrame(int cx, int floorY, float dim) {
  int sp = cfg.petType, s = petPixelSize();
  unsigned long left = trickUntil - millis();
  int f = petFrameNo;
  int x0 = cx - 8 * s, y0 = floorY - 15 * s + 2;
  switch (trickId) {
    case 0: {  // dance: hop and turn, music notes
      int hop = (f % 2) ? -10 : 0;
      drawPetPose(sp, (f % 2) ? POSE_SIT2 : POSE_SIT, x0, y0 + hop, s, (f / 2) % 2, false, dim, false);
      canvas.setFont(F_BOLD);
      canvas.setTextDatum(D_MC);
      canvas.setTextColor(f % 2 ? C_ACCENT : C_PURPLE);
      canvas.drawString("~", cx - 9 * s, y0 + 10 - (f % 3) * 4);
      canvas.drawString("~", cx + 9 * s, y0 + 20 - (f % 3) * 4);
      break;
    }
    case 1: {  // backflip: up, upside down at the top, back down
      int phase = (3200 - (int)left) * 6 / 3200;  // 0-5
      int lift[] = {0, 18, 34, 34, 18, 0};
      drawPetPose(sp, POSE_SIT, x0, y0 - lift[phase], s, false, false, dim, phase == 2 || phase == 3);
      break;
    }
    case 2: {  // juggling three balls
      drawPetPose(sp, POSE_SIT2, x0, y0, s, false, false, dim, false);
      const uint16_t cols[] = {C_RED, C_YELLOW, C_ACCENT};
      for (int i = 0; i < 3; i++) {
        float a = f * 0.9f + i * 2.094f;
        canvas.fillCircle(cx + (int)(cosf(a) * 4 * s), y0 - 6 - (int)(fabsf(sinf(a)) * 5 * s), s, cols[i]);
      }
      break;
    }
    default: {  // magic: vanish in sparkles, reappear
      int phase = (3200 - (int)left) * 4 / 3200;
      float fade = phase == 1 ? 0.75f : phase == 2 ? 1.0f : 0;
      if (fade < 1.0f) drawPetPose(sp, POSE_SIT, x0, y0, s, false, false, max(dim, fade), false);
      for (int i = 0; i < 10; i++) {
        float a = i * 0.628f + f * 0.2f;
        int r = 10 + ((f * 7 + i * 13) % 50);
        canvas.fillCircle(cx + (int)(cosf(a) * r), y0 + 8 * s + (int)(sinf(a) * r * 0.6f), 2, i % 2 ? C_YELLOW : C_PURPLE);
      }
      break;
    }
  }
}

// ---------------------------------------------------------------- dog, hamster and dragon in the room

void critterPickHabit() {
  CatHabit idle[] = {H_SIT, H_SIT, H_WALK, H_WALK, H_NAP, H_BLINK, H_YAWN, H_SPECIAL, H_TOY};
  CatHabit h = idle[esp_random() % 9];
  if (h == H_TOY && rpg.room[5] == NO_ITEM) h = H_WALK;
  unsigned long ms = h == H_NAP ? 9000 : h == H_SPECIAL ? 5000 : 4000 + esp_random() % 4000;
  catStartHabit(h, ms);
  if (h == H_TOY) catTargetX = 220;
  if (h == H_SPECIAL) petSay(cfg.petType == SP_DOG ? "Zoomies!" : cfg.petType == SP_HAMSTER ? "Wheel time!" : "Rawr!", 2000);
}

void drawCritterScene(int floorY, bool dark) {
  int sp = cfg.petType;
  unsigned long now = millis();
  float dim = dark ? 0.6f : 0;
  int s = petPixelSize();

  // Eggs: a puppy-to-be... well, eggs for everyone, it's a game
  if (pet.stage == PET_EGG || (sp == SP_DRAGON && !rpg.dragonHatched)) {
    int wob = (petFrameNo % 6 == 0) ? 3 : (petFrameNo % 6 == 1) ? -3 : 0;
    uint32_t col = sp == SP_DRAGON ? speciesColour(SP_DRAGON, 'X', 0, 0) : 0x2EE6A8;
    drawSprite(SPR_EGG, W / 2 - 8 * 6 + wob, floorY - 14 * 6, 6, col, false, false, dim);
    return;
  }

  if (trickActive()) {
    drawTrickFrame((int)catX, floorY, dim);
    return;
  }
  if (pet.asleep) catHabit = H_NAP;
  else if (catHabit == H_TRICK || now > catHabitUntil) critterPickHabit();

  // Movement
  bool zoom = catHabit == H_SPECIAL && sp == SP_DOG;
  float speed = zoom ? 8 : 1.8f;
  bool moving = false;
  if (catHabit == H_WALK || catHabit == H_TOY || zoom) {
    float target = zoom ? (catDir > 0 ? W - 40 : 40) : catTargetX;
    if (fabs(target - catX) > speed) {
      catDir = target > catX ? 1 : -1;
      catX += catDir * speed;
      moving = true;
    } else if (zoom) {
      catDir = -catDir;
      moving = true;
    } else if (catHabit == H_WALK) {
      catHabitUntil = 0;
    }
  }
  if (catX < 40) catX = 40;
  if (catX > W - 40) catX = W - 40;
  int x0 = (int)catX - 8 * s, sitY = floorY - 15 * s + 2, walkY = floorY - 13 * s + 2;
  bool flip = catDir < 0;
  int f = petFrameNo;

  if (petEvolved() && !pet.asleep) drawEvolveAura((int)catX, sitY + 8 * s, 10 * s);
  if (!moving && !pet.asleep) drawGear(x0, sitY, s, true);

  // Specials
  if (catHabit == H_SPECIAL && sp == SP_HAMSTER) {  // running in the wheel
    int wx = (int)catX, wy = floorY - 9 * s;
    canvas.fillCircle(wx, wy, 9 * s, C_GREY);
    canvas.fillCircle(wx, wy, 9 * s - 3, mix(theme().card, 0x000000, dim));
    for (int i = 0; i < 6; i++) {
      float a = f * 0.6f + i * 1.047f;
      canvas.drawLine(wx, wy, wx + (int)(cosf(a) * (9 * s - 3)), wy + (int)(sinf(a) * (9 * s - 3)), C_DIM);
    }
    drawSpeciesSprite(sp, (f % 2) ? HAM_WALK1 : HAM_WALK2, x0, wy + 9 * s - 13 * s - 2, s, false, false, dim, false);
    return;
  }

  if (moving) {
    drawPetPose(sp, (f % 2) ? POSE_WALK1 : POSE_WALK2, x0, walkY, s, flip, false, dim, false);
  } else if (catHabit == H_NAP) {
    drawPetPose(sp, POSE_SLEEP, x0, floorY - 15 * s + 2, s, flip, true, dim, false);
  } else {
    bool blink = catHabit == H_BLINK ? (f % 6) < 3 : (f % 20) == 0;
    bool play = catHabit == H_TOY;
    drawPetPose(sp, ((f / 4) % 2 || play) ? POSE_SIT2 : POSE_SIT, x0, sitY - (play && (f % 2) ? 6 : 0), s, false,
                blink || catHabit == H_PURR, dim, false);
    if (catHabit == H_YAWN) canvas.fillRect(x0 + 7 * s, sitY + 9 * s, 2 * s, 2 * s, RGB565(200, 60, 90));
    if (catHabit == H_SPECIAL && sp == SP_DRAGON) {  // a little burst of fire
      int fx = x0 + 15 * s, fy = sitY + 8 * s;
      for (int i = 0; i < 4; i++) {
        int len = 10 + ((f + i) % 3) * 8;
        canvas.fillTriangle(fx, fy - 4 + i * 2, fx + len + i * 6, fy - 8 + i * 4, fx + len + i * 6, fy + i * 4, i % 2 ? C_AMBER : C_RED);
      }
    }
    drawGear(x0, sitY, s, false);
  }

  if (catHabit == H_PURR) {  // happy hearts when stroked
    heartsT++;
    for (int i = 0; i < 3; i++) {
      int hy = sitY - ((heartsT * 3 + i * 25) % 70), hx = (int)catX - 30 + i * 30;
      uint16_t hc = RGB565(255, 90, 140);
      canvas.fillCircle(hx - 3, hy, 4, hc);
      canvas.fillCircle(hx + 3, hy, 4, hc);
      canvas.fillTriangle(hx - 7, hy + 1, hx + 7, hy + 1, hx, hy + 9, hc);
    }
  }
}
