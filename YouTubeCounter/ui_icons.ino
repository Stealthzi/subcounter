// Little pictures for the Home tiles, drawn from simple shapes about 20
// points across, centred on (cx, cy). `c` is the picture's colour and `bg`
// the tile behind it (used to cut holes, like the slot in an envelope).

void iconText(const char* text, int cx, int cy, uint16_t colour) {
  tft.setFont(F_BOLD);
  tft.setTextSize(1);
  tft.setTextDatum(D_MC);
  tft.setTextColor(colour);
  tft.drawString(text, cx, cy + 1);
}

void drawPageIcon(int page, int cx, int cy, uint16_t c, uint16_t bg) {
  switch (page) {
    case PAGE_OVERVIEW:  // four panels
      for (int i = 0; i < 4; i++) tft.fillSmoothRoundRect(cx - 9 + (i % 2) * 10, cy - 9 + (i / 2) * 10, 8, 8, 2, c);
      break;
    case PAGE_SUBS:  // a person
      tft.fillSmoothCircle(cx, cy - 4, 5, c);
      tft.fillSmoothRoundRect(cx - 8, cy + 2, 16, 8, 4, c);
      tft.fillRect(cx - 8, cy + 6, 16, 4, c);
      break;
    case PAGE_CHANNEL:  // bar chart
      tft.fillRoundRect(cx - 9, cy + 1, 5, 8, 1, c);
      tft.fillRoundRect(cx - 2, cy - 4, 5, 13, 1, c);
      tft.fillRoundRect(cx + 5, cy - 9, 5, 18, 1, c);
      break;
    case PAGE_LATEST:  // a video's play button
      tft.fillSmoothRoundRect(cx - 10, cy - 7, 20, 14, 4, c);
      tft.fillTriangle(cx - 3, cy - 4, cx - 3, cy + 4, cx + 4, cy, bg);
      break;
    case PAGE_RECENT:  // a list of videos
      for (int i = 0; i < 3; i++) {
        int y = cy - 8 + i * 6;
        tft.fillRoundRect(cx - 10, y, 6, 4, 1, c);
        tft.fillRoundRect(cx - 2, y + 1, 12, 2, 1, c);
      }
      break;
    case PAGE_TOP:  // a trophy
      tft.fillSmoothRoundRect(cx - 6, cy - 9, 12, 10, 4, c);
      tft.fillArc(cx - 6, cy - 5, 3, 4, 90, 270, c);
      tft.fillArc(cx + 6, cy - 5, 3, 4, 270, 360, c);
      tft.fillArc(cx + 6, cy - 5, 3, 4, 0, 90, c);
      tft.fillRect(cx - 1, cy, 3, 5, c);
      tft.fillRoundRect(cx - 5, cy + 5, 11, 4, 1, c);
      break;
    case PAGE_COMMENT:  // a speech bubble
      tft.fillSmoothRoundRect(cx - 10, cy - 8, 20, 14, 4, c);
      tft.fillTriangle(cx - 6, cy + 4, cx - 6, cy + 10, cx, cy + 4, c);
      for (int i = -1; i <= 1; i++) tft.fillSmoothCircle(cx + i * 5, cy - 1, 1, bg);
      break;
    case PAGE_GROWTH:  // a line going up
      tft.drawWideLine(cx - 10, cy - 9, cx - 10, cy + 8, 0.8f, c);
      tft.drawWideLine(cx - 10, cy + 8, cx + 10, cy + 8, 0.8f, c);
      tft.drawWideLine(cx - 7, cy + 4, cx - 2, cy - 1, 1.1f, c);
      tft.drawWideLine(cx - 2, cy - 1, cx + 2, cy + 2, 1.1f, c);
      tft.drawWideLine(cx + 2, cy + 2, cx + 9, cy - 6, 1.1f, c);
      break;
    case PAGE_COMPARE:
      iconText("VS", cx, cy, c);
      break;
    case PAGE_LIVE:  // a dot sending out waves
      tft.fillSmoothCircle(cx, cy, 3, C_RED);
      tft.fillArc(cx, cy, 6, 7, 320, 360, c);
      tft.fillArc(cx, cy, 6, 7, 0, 40, c);
      tft.fillArc(cx, cy, 6, 7, 140, 220, c);
      tft.fillArc(cx, cy, 10, 11, 320, 360, c);
      tft.fillArc(cx, cy, 10, 11, 0, 40, c);
      tft.fillArc(cx, cy, 10, 11, 140, 220, c);
      break;
    case PAGE_CLOCK:
      tft.fillArc(cx, cy, 8, 10, 0, 360, c);
      tft.drawWideLine(cx, cy, cx, cy - 5, 1.0f, c);
      tft.drawWideLine(cx, cy, cx + 4, cy + 1, 1.0f, c);
      break;
    case PAGE_WEATHER:  // sun behind a cloud
      tft.fillSmoothCircle(cx + 3, cy - 4, 6, C_AMBER);
      tft.fillSmoothCircle(cx - 3, cy + 2, 5, C_WHITE);
      tft.fillSmoothCircle(cx + 3, cy + 1, 6, C_WHITE);
      tft.fillSmoothRoundRect(cx - 10, cy + 2, 20, 7, 3, C_WHITE);
      break;
    case PAGE_CRYPTO:  // a coin
      tft.fillSmoothCircle(cx, cy, 10, c);
      iconText("B", cx, cy, bg);
      break;
    case PAGE_ENERGY:  // lightning
      tft.fillTriangle(cx + 3, cy - 10, cx - 6, cy + 1, cx + 1, cy + 1, c);
      tft.fillTriangle(cx - 1, cy - 1, cx + 6, cy - 1, cx - 3, cy + 10, c);
      break;
    case PAGE_FUEL:  // a petrol pump
      tft.fillSmoothRoundRect(cx - 9, cy - 9, 11, 18, 2, c);
      tft.fillRect(cx - 7, cy - 7, 7, 5, bg);
      tft.drawWideLine(cx + 2, cy - 5, cx + 7, cy - 1, 1.0f, c);
      tft.drawWideLine(cx + 7, cy - 1, cx + 7, cy + 6, 1.0f, c);
      tft.drawWideLine(cx + 7, cy + 6, cx + 4, cy + 6, 1.0f, c);
      break;
    case PAGE_CALENDAR:
      tft.fillSmoothRoundRect(cx - 10, cy - 7, 20, 17, 3, c);
      tft.fillRect(cx - 8, cy - 2, 16, 10, bg);
      tft.fillRect(cx - 6, cy - 10, 3, 5, c);
      tft.fillRect(cx + 3, cy - 10, 3, 5, c);
      for (int i = 0; i < 5; i++) tft.fillRect(cx - 6 + (i % 3) * 5, cy + (i / 3) * 4, 3, 2, c);
      break;
    case PAGE_MESSAGES:  // an envelope
      tft.fillSmoothRoundRect(cx - 10, cy - 7, 20, 14, 2, c);
      tft.drawWideLine(cx - 8, cy - 5, cx, cy + 1, 1.0f, bg);
      tft.drawWideLine(cx, cy + 1, cx + 8, cy - 5, 1.0f, bg);
      break;
    case PAGE_PET:  // a paw print
      tft.fillEllipse(cx, cy + 4, 6, 5, c);
      tft.fillSmoothCircle(cx - 7, cy - 2, 2, c);
      tft.fillSmoothCircle(cx - 3, cy - 7, 2, c);
      tft.fillSmoothCircle(cx + 3, cy - 7, 2, c);
      tft.fillSmoothCircle(cx + 7, cy - 2, 2, c);
      break;
    case PAGE_ALERTS:  // a warning triangle
      tft.fillTriangle(cx, cy - 10, cx - 11, cy + 9, cx + 11, cy + 9, c);
      tft.fillRect(cx - 1, cy - 3, 3, 7, bg);
      tft.fillRect(cx - 1, cy + 5, 3, 2, bg);
      break;
    case PAGE_GAMES:  // a game controller
      tft.fillSmoothRoundRect(cx - 11, cy - 6, 22, 13, 6, c);
      tft.fillRect(cx - 8, cy - 1, 6, 2, bg);
      tft.fillRect(cx - 6, cy - 3, 2, 6, bg);
      tft.fillSmoothCircle(cx + 4, cy - 2, 1, bg);
      tft.fillSmoothCircle(cx + 7, cy + 1, 1, bg);
      break;
    case PAGE_SCREENSAVER:  // a fish
      tft.fillEllipse(cx - 2, cy, 7, 5, c);
      tft.fillTriangle(cx + 4, cy, cx + 10, cy - 6, cx + 10, cy + 6, c);
      tft.fillSmoothCircle(cx - 5, cy - 1, 1, bg);
      break;
    case PAGE_EIGHTBALL:
      tft.fillSmoothCircle(cx, cy, 10, c);
      tft.fillSmoothCircle(cx, cy, 5, bg);
      iconText("8", cx, cy, c);
      break;
    case PAGE_STOPWATCH:
      tft.fillArc(cx, cy + 1, 7, 9, 0, 360, c);
      tft.fillRect(cx - 2, cy - 11, 5, 3, c);
      tft.drawWideLine(cx, cy + 1, cx + 3, cy - 3, 1.0f, c);
      break;
    case PAGE_AIR:  // breezy lines
      tft.drawWideLine(cx - 10, cy - 6, cx + 5, cy - 6, 1.1f, c);
      tft.drawWideLine(cx - 10, cy, cx + 9, cy, 1.1f, c);
      tft.drawWideLine(cx - 10, cy + 6, cx + 3, cy + 6, 1.1f, c);
      break;
    case PAGE_COUNTDOWN:  // an hourglass
      tft.fillRect(cx - 8, cy - 10, 17, 2, c);
      tft.fillRect(cx - 8, cy + 9, 17, 2, c);
      tft.fillTriangle(cx - 7, cy - 8, cx + 7, cy - 8, cx, cy, c);
      tft.fillTriangle(cx - 7, cy + 9, cx + 7, cy + 9, cx, cy + 1, c);
      break;
    case PAGE_MOODLAMP:  // a light bulb
      tft.fillSmoothCircle(cx, cy - 3, 7, c);
      tft.fillRect(cx - 3, cy + 3, 7, 4, c);
      tft.fillRect(cx - 2, cy + 8, 5, 2, c);
      break;
    case PAGE_JOKE:  // a smiley
      tft.fillSmoothCircle(cx, cy, 10, c);
      tft.fillSmoothCircle(cx - 4, cy - 3, 1, bg);
      tft.fillSmoothCircle(cx + 4, cy - 3, 1, bg);
      tft.fillArc(cx, cy, 5, 6, 20, 160, bg);
      break;
    case PAGE_SPOTIFY:  // two music notes
      tft.fillRect(cx - 4, cy - 7, 2, 13, c);
      tft.fillRect(cx + 6, cy - 9, 2, 13, c);
      tft.drawWideLine(cx - 3, cy - 7, cx + 7, cy - 9, 1.3f, c);
      tft.fillEllipse(cx - 6, cy + 6, 3, 2, c);
      tft.fillEllipse(cx + 4, cy + 4, 3, 2, c);
      break;
    case PAGE_ANALYTICS:  // a magnifying glass
      tft.fillArc(cx - 2, cy - 2, 5, 7, 0, 360, c);
      tft.drawWideLine(cx + 3, cy + 3, cx + 9, cy + 9, 1.5f, c);
      break;
    case PAGE_REPORT:  // a report card
      tft.fillSmoothRoundRect(cx - 8, cy - 10, 16, 20, 2, c);
      iconText("A", cx, cy, bg);
      break;
    case ICON_SETTINGS:  // a cog
      for (int i = 0; i < 8; i++) {
        float a = i * PI / 4;
        tft.drawWideLine(cx + cosf(a) * 6, cy + sinf(a) * 6, cx + cosf(a) * 9, cy + sinf(a) * 9, 1.5f, c);
      }
      tft.fillSmoothCircle(cx, cy, 7, c);
      tft.fillSmoothCircle(cx, cy, 3, bg);
      break;
    case ICON_MORE:  // three dots
      for (int i = -1; i <= 1; i++) tft.fillSmoothCircle(cx + i * 7, cy, 2, c);
      break;
  }
}
