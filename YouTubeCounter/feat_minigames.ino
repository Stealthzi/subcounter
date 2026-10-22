// Pet mini-games, unlocked at levels 3, 12 and 22:
//   Catch the treats - tap where the bowl should go to catch falling treats
//   Whack-a-mouse    - tap the mice as they pop out of their holes
//   Memory match     - find the pairs of items in as few goes as you can
// They play in the pet's scene and pay out coins and XP.

const unsigned long GAME_MS = 30000;

struct MiniGame {
  int           which = -1;
  bool          over = false;
  unsigned long startedAt = 0;
  int           score = 0;
  // catch
  float         bowlX = 160, bowlTarget = 160;
  float         tx[5], ty[5], tv[5];
  int           treatItem[5];
  // whack
  int           mouseHole = -1;
  unsigned long mouseUntil = 0, whackedAt = 0;
  // memory
  int           cards[12];
  bool          shown[12], matched[12];
  int           firstPick = -1, secondPick = -1;
  unsigned long hideAt = 0;
  int           moves = 0, pairs = 0;
  int           rewardCoins = 0, rewardXp = 0;
} mini;

const char* miniGameName(int i) {
  const char* n[] = {"Catch the treats", "Whack-a-mouse", "Memory match"};
  return i >= 0 && i < 3 ? n[i] : "";
}

void resetTreat(int i, bool top) {
  mini.tx[i] = 20 + esp_random() % (W - 40);
  mini.ty[i] = top ? -20 - (int)(esp_random() % 60) : -(int)(esp_random() % 140);
  mini.tv[i] = 1.6f + (esp_random() % 100) / 60.0f;
  const int treats[] = {0, 1, 2, 3, 4};
  mini.treatItem[i] = treats[esp_random() % 5];
}

String startMiniGame(int which) {
  if (!miniGameUnlocked(which)) return String(miniGameName(which)) + " unlocks at level " + MINIGAME_LEVEL[which];
  mini = MiniGame();
  mini.which = which;
  mini.startedAt = millis();
  if (which == 0)
    for (int i = 0; i < 5; i++) resetTreat(i, false);
  if (which == 2) {
    int pool[6];
    uint32_t seed = esp_random();
    for (int i = 0; i < 6; i++) {
      seed = seed * 1103515245UL + 12345UL;
      pool[i] = (seed >> 8) % itemCount();
      for (int j = 0; j < i; j++)
        if (pool[j] == pool[i]) {
          pool[i] = (pool[i] + 7) % itemCount();
          j = -1;
        }
    }
    for (int i = 0; i < 12; i++) mini.cards[i] = pool[i / 2];
    for (int i = 11; i > 0; i--) {  // shuffle
      int j = esp_random() % (i + 1);
      int t = mini.cards[i];
      mini.cards[i] = mini.cards[j];
      mini.cards[j] = t;
    }
  }
  const char* says[] = {"Catch them all!", "Get those mice!", "Find the pairs!"};
  return says[which];
}

void endMiniGame() {
  if (mini.over) return;
  mini.over = true;
  int score = mini.score;
  if (mini.which == 2) score = max(0, 30 - max(0, mini.moves - 6) * 2) * mini.pairs / 6;
  mini.score = score;
  mini.rewardCoins = min(score * 2, 80);
  mini.rewardXp = min(score * 3, 60);
  addCoins(mini.rewardCoins);
  careXp(mini.rewardXp);
  rpg.gamesPlayed++;
  if (score > rpg.bestGameScore) rpg.bestGameScore = score;
  if (pet.happy < 4) pet.happy++;
  questEvent(Q_MINIGAME, 1);
  checkBadges();
  savePet();
  saveRpg();
}

bool miniRunning() {
  return mini.which >= 0 && !mini.over;
}

int miniSecondsLeft() {
  long left = (long)GAME_MS - (long)(millis() - mini.startedAt);
  return left > 0 ? (int)(left / 1000) + 1 : 0;
}

// Taps on the scene (y is relative to the top of the scene)
void miniTap(int x, int y) {
  if (mini.over) return;
  if (mini.which == 0) {
    mini.bowlTarget = constrain(x, 30, W - 30);
  } else if (mini.which == 1) {
    int col = x / (W / 3), row = (y - 40) / 50;
    int hole = row * 3 + col;
    if (mini.mouseHole >= 0 && hole == mini.mouseHole && millis() < mini.mouseUntil) {
      mini.score++;
      mini.whackedAt = millis();
      mini.mouseUntil = millis();
    }
  } else if (mini.which == 2) {
    if (mini.secondPick >= 0) return;  // wait for the last pair to turn back
    int col = x / (W / 6), row = (y - 12) / 64;
    if (row < 0 || row > 1) return;
    int i = row * 6 + constrain(col, 0, 5);
    if (mini.matched[i] || mini.shown[i]) return;
    mini.shown[i] = true;
    if (mini.firstPick < 0) {
      mini.firstPick = i;
    } else {
      mini.secondPick = i;
      mini.moves++;
      if (mini.cards[i] == mini.cards[mini.firstPick]) {
        mini.matched[i] = mini.matched[mini.firstPick] = true;
        mini.pairs++;
        mini.firstPick = mini.secondPick = -1;
        if (mini.pairs == 6) endMiniGame();
      } else {
        mini.hideAt = millis() + 700;
      }
    }
  }
}

// One frame, drawn into the pet scene
void miniGameFrame() {
  unsigned long now = millis();
  if (!mini.over && now - mini.startedAt >= GAME_MS) endMiniGame();
  for (int y = 0; y < PET_SCENE_H; y++) canvas.drawFastHLine(0, y, W, bgAt(PET_SCENE_Y + y));

  if (mini.which == 0) {  // catch the treats
    mini.bowlX += (mini.bowlTarget - mini.bowlX) * 0.35f;
    int by = PET_SCENE_H - 18;
    for (int i = 0; i < 5; i++) {
      if (!mini.over) mini.ty[i] += mini.tv[i] * (1 + (now - mini.startedAt) / 15000.0f) * 2;
      if (mini.ty[i] > by - 6 && mini.ty[i] < by + 8 && fabs(mini.tx[i] - mini.bowlX) < 30) {
        mini.score++;
        resetTreat(i, true);
      } else if (mini.ty[i] > PET_SCENE_H + 10) {
        resetTreat(i, true);
      }
      if (mini.ty[i] > -14) drawItemIcon(canvas, mini.treatItem[i], (int)mini.tx[i], (int)mini.ty[i], 20);
    }
    int bx = (int)mini.bowlX;
    canvas.fillRoundRect(bx - 30, by, 60, 12, 6, C_ACCENT2);
    canvas.fillRect(bx - 26, by, 52, 3, mix(theme().accent2, 0xFFFFFF, 0.3f));
  } else if (mini.which == 1) {  // whack-a-mouse
    if (!mini.over && now > mini.mouseUntil + 250) {
      mini.mouseHole = esp_random() % 6;
      mini.mouseUntil = now + 550 + esp_random() % 650 - min(300UL, (now - mini.startedAt) / 100);
    }
    for (int h = 0; h < 6; h++) {
      int cx = (h % 3) * (W / 3) + W / 6, cy = 58 + (h / 3) * 50;
      canvas.fillEllipse(cx, cy + 14, 30, 9, RGB565(40, 26, 18));
      bool up = h == mini.mouseHole && now < mini.mouseUntil;
      bool bonked = h == mini.mouseHole && now - mini.whackedAt < 300;
      if (up || bonked) {
        drawShape(canvas, S_MOUSE, cx, cy + (bonked ? 8 : 0), 40, 0x9AA0A8, 0xFF8FB3);
        if (bonked) {
          canvas.setFont(F_BOLD);
          canvas.setTextDatum(D_MC);
          canvas.setTextColor(C_YELLOW);
          canvas.drawString("Bonk!", cx, cy - 18);
        }
      }
    }
  } else {  // memory match
    if (mini.secondPick >= 0 && mini.hideAt && now > mini.hideAt) {
      mini.shown[mini.firstPick] = mini.shown[mini.secondPick] = false;
      mini.firstPick = mini.secondPick = -1;
      mini.hideAt = 0;
    }
    for (int i = 0; i < 12; i++) {
      int x = (i % 6) * (W / 6) + 4, y = 12 + (i / 6) * 64, w = W / 6 - 8, h = 58;
      if (mini.matched[i] || mini.shown[i]) {
        drawItemTile(canvas, mini.cards[i], x, y, w, h, true, 0);
      } else {
        canvas.fillSmoothRoundRect(x, y, w, h, 6, C_PURPLE);
        canvas.fillSmoothRoundRect(x + 4, y + 4, w - 8, h - 8, 4, mix(0x9B5CFF, 0x000000, 0.35f));
      }
    }
  }

  // Score and time
  canvas.setFont(F_BOLD);
  canvas.setTextSize(1);
  canvas.setTextColor(C_WHITE);
  canvas.setTextDatum(D_TL);
  canvas.drawString(mini.which == 2 ? String(mini.pairs) + "/6 pairs" : "Score " + String(mini.score), 8, 2);
  canvas.setTextDatum(D_TR);
  canvas.drawString(mini.over ? String("Done!") : String(miniSecondsLeft()) + "s", W - 8, 2);
  if (mini.over) {
    canvas.fillRoundRect(W / 2 - 100, 44, 200, 50, 10, mix(theme().card, 0x000000, 0.1f));
    canvas.setTextDatum(D_TC);
    canvas.setTextColor(C_GREEN);
    canvas.drawString("Score " + String(mini.score), W / 2, 50);
    canvas.setFont(F_SMALL);
    canvas.setTextColor(C_WHITE);
    canvas.drawString("+" + String(mini.rewardCoins) + " coins  +" + String(mini.rewardXp) + " XP", W / 2, 74);
  }
}
