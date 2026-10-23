// The pages the counter cycles through

void drawCurrentPage() {
  if (currentPage < 0) return;
  if (keyboardOpen) {
    drawKeyboard();
    return;
  }
  if (lampGlowing()) {
    drawLampGlow();
    return;
  }
  // These two fill the whole screen
  if (currentPage == PAGE_SCREENSAVER) {
    tft.fillSides(RGB565(0, 0, 0));
    drawNavArrowsIn(RGB565(70, 70, 80), RGB565(18, 18, 24));
    startScreensaver();
    return;
  }
  if (currentPage == PAGE_MOODLAMP) {
    moodLabelUntil = millis() + 2500;
    moodLampFrame();
    return;
  }
  drawSides();
  clearContent();
  drawHeader();
  switch (currentPage) {
    case PAGE_SUBS:      drawSubsPage(); break;
    case PAGE_CHANNEL:   drawChannelPage(); break;
    case PAGE_LATEST:    drawLatestPage(); break;
    case PAGE_RECENT:    drawRecentPage(); break;
    case PAGE_TOP:       drawTopPage(); break;
    case PAGE_COMMENT:   drawCommentPage(); break;
    case PAGE_GROWTH:    drawGrowthPage(); break;
    case PAGE_COMPARE:   drawComparePage(); break;
    case PAGE_LIVE:      drawLivePage(); break;
    case PAGE_CLOCK:     drawClockPage(); break;
    case PAGE_WEATHER:   drawWeatherPage(); break;
    case PAGE_CRYPTO:    drawCryptoPage(); break;
    case PAGE_ENERGY:    drawEnergyPage(); break;
    case PAGE_FUEL:      drawFuelPage(); break;
    case PAGE_CALENDAR:  drawCalendarPage(); break;
    case PAGE_MESSAGES:  drawMessagesPage(); break;
    case PAGE_PET:       drawPetPage(); break;
    case PAGE_ALERTS:    drawAlertsPage(); break;
    case PAGE_GAMES:     drawGamesPage(); break;
    case PAGE_EIGHTBALL: drawEightBallPage(); break;
    case PAGE_STOPWATCH: drawStopwatchPage(); break;
    case PAGE_AIR:       drawAirPage(); break;
    case PAGE_COUNTDOWN: drawCountdownPage(); break;
    case PAGE_JOKE:      drawJokePage(); break;
    case PAGE_SPOTIFY:   drawSpotifyPage(); break;
    case PAGE_ANALYTICS: drawAnalyticsPage(); break;
    case PAGE_REPORT:    drawReportPage(); break;
  }
  drawPageDots();
}

// ---------------------------------------------------------------- subscribers

void drawSubsPage() {
  String number = channel.hidden ? String("Hidden") : withCommas(channel.subs);
  drawBigNumber(number, W / 2, 86, 296, 1.8f, highlighted ? C_GREEN : C_WHITE);

  // "SUBSCRIBERS   +2K today"
  String label = "SUBSCRIBERS";
  String gain;
  int64_t today = subsToday();
  if (!channel.hidden && today > 0) gain = "   " + signedCompact(today) + " today";
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  int x = (W - tft.textWidth(label.c_str()) - tft.textWidth(gain.c_str())) / 2;
  tft.setTextColor(C_GREY);
  x += tft.drawString(label.c_str(), x, 128);
  tft.setTextColor(C_GREEN);
  tft.drawString(gain.c_str(), x, 128);

  if (channel.hidden) return;
  uint64_t target = nextMilestone(channel.subs);
  if (!target) return;
  uint64_t from = previousMilestone(channel.subs);
  String award = milestoneAward(target);

  // Next milestone, with a progress bar from the last one
  String left = "NEXT: " + compact(target);
  if (award.length()) left += "  " + award;
  drawLabel(14, 156, left, C_WHITE, "");
  tft.setTextDatum(D_TR);
  tft.setTextColor(C_GREY);
  tft.drawString((compact(target - channel.subs) + " to go").c_str(), W - 14, 156);

  float fraction = (float)(channel.subs - from) / (float)(target - from);
  drawProgressBar(14, 176, W - 28, 14, fraction, C_ACCENT);

  tft.setTextColor(C_DIM);
  tft.setTextDatum(D_TL);
  tft.drawString(compact(from).c_str(), 14, 196);
  tft.setTextDatum(D_TR);
  tft.drawString(compact(target).c_str(), W - 14, 196);

  long days = daysToMilestone();
  if (days > 0) {
    String eta = days > 3650 ? String("over 10 years at this rate")
               : "about " + String(days) + (days == 1 ? " day" : " days") + " at this rate";
    tft.setTextDatum(D_TC);
    tft.setTextColor(C_GREY);
    tft.drawString(eta.c_str(), W / 2, 196);
  }
}

// ---------------------------------------------------------------- channel totals

void drawChannelPage() {
  drawLabel(12, 40, "CHANNEL STATS", C_ACCENT, "");
  int64_t vToday = viewsToday();
  int64_t vWeek = viewsThisWeek();
  int64_t sWeek = subsThisWeek();

  drawCard(8, 60, 148, 78, "TOTAL VIEWS", compact(channel.views), C_WHITE);
  drawCard(164, 60, 148, 78, "VIEWS TODAY",
           vToday >= 0 ? signedCompact(vToday) : String("-"), vToday > 0 ? C_GREEN : C_DIM);
  drawCard(8, 146, 148, 78, "VIDEOS", withCommas(channel.videos), C_WHITE);
  if (sWeek >= 0 && sWeek != 0) {
    drawCard(164, 146, 148, 78, "SUBS THIS WEEK", signedCompact(sWeek), sWeek > 0 ? C_GREEN : C_AMBER);
  } else {
    drawCard(164, 146, 148, 78, "VIEWS THIS WEEK",
             vWeek >= 0 ? signedCompact(vWeek) : String("-"), vWeek > 0 ? C_GREEN : C_DIM);
  }
}

// ---------------------------------------------------------------- videos

// Shared layout for a single video: title, big view count and stat cards
void drawVideoDetail(const String& label, const VideoInfo& v, bool showPerHour) {
  String when = v.live == LIVE_NOW ? String("LIVE NOW") : timeAgo(v.published);
  drawLabel(12, 40, label, C_ACCENT, when);

  tft.setFont(F_BOLD);
  tft.setTextColor(C_WHITE);
  drawWrapped(v.title, 12, 60, W - 24, 2, 20);

  tft.setFont(F_LARGE);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_WHITE);
  String views = withCommas(v.views);
  if (tft.textWidth(views.c_str()) > 230) views = compact(v.views);
  int w = tft.drawString(views.c_str(), 12, 104);
  tft.setFont(F_SMALL);
  tft.setTextColor(C_GREY);
  tft.drawString("views", 12 + w + 8, 118);

  String likes = v.hasLikes ? compact(v.likes) : String("hidden");
  String comments = v.hasComments ? compact(v.comments) : String("off");
  if (showPerHour) {
    long perHour = viewsPerHour();
    bool real;
    float vs = vsUsual(real);
    drawCard(8, 146, 73, 76, "LIKES", likes, C_WHITE);
    drawCard(85, 146, 73, 76, "COMMENTS", comments, C_WHITE);
    drawCard(162, 146, 73, 76, "PER HOUR",
             perHour >= 0 ? "+" + compact(perHour) : String("-"), perHour > 0 ? C_GREEN : C_DIM);
    drawCard(239, 146, 73, 76, "VS USUAL", vsUsualText(),
             vs <= 0 ? C_DIM : vs >= 1.0f ? C_GREEN : C_AMBER);
  } else {
    drawCard(8, 146, 148, 76, "LIKES", likes, C_WHITE);
    drawCard(164, 146, 148, 76, "COMMENTS", comments, C_WHITE);
  }
}

void drawLatestPage() {
  int li = latestIndex();
  if (li < 0) return;
  drawVideoDetail("LATEST UPLOAD", uploads.v[li], uploads.v[li].live != LIVE_NOW);
}

void drawTopPage() {
  drawVideoDetail("MOST VIEWED VIDEO", topVideo, false);
}

void drawRecentPage() {
  drawLabel(12, 40, "RECENT UPLOADS", C_ACCENT, "");
  int y = 58, shown = 0;
  for (int i = 0; i < uploads.count && shown < 4; i++) {
    const VideoInfo& v = uploads.v[i];
    if (v.live == LIVE_UPCOMING) continue;
    String views = compact(v.views);
    tft.setFont(F_BOLD);
    int viewsW = tft.textWidth(views.c_str());
    tft.setTextDatum(D_TL);
    tft.setTextColor(C_WHITE);
    tft.drawString(fitText(v.title, W - 36 - viewsW).c_str(), 12, y + 2);
    tft.setTextDatum(D_TR);
    tft.drawString(views.c_str(), W - 12, y + 2);

    tft.setFont(F_SMALL);
    tft.setTextColor(v.live == LIVE_NOW ? C_ACCENT : C_GREY);
    tft.setTextDatum(D_TL);
    tft.drawString(v.live == LIVE_NOW ? "LIVE NOW" : timeAgo(v.published).c_str(), 12, y + 22);
    tft.setTextColor(C_GREY);
    tft.setTextDatum(D_TR);
    tft.drawString("views", W - 12, y + 22);
    if (shown < 3) tft.drawFastHLine(12, y + 40, W - 24, C_LINE);
    y += 42;
    shown++;
  }
}

void drawCommentPage() {
  drawLabel(12, 40, "NEWEST COMMENT", C_ACCENT, timeAgo(comment.published));
  tft.setFont(F_SMALL);
  tft.setTextColor(C_GREY);
  tft.setTextDatum(D_TL);
  tft.drawString(fitText("on: " + comment.videoTitle, W - 24).c_str(), 12, 58);

  tft.fillSmoothRoundRect(8, 78, W - 16, 146, 12, C_CARD);
  tft.setFont(F_BOLD);
  tft.setTextColor(C_WHITE);
  tft.setTextDatum(D_TL);
  tft.drawString(fitText(comment.author, W - 40).c_str(), 20, 88);
  tft.setFont(F_BODY);
  tft.setTextColor(C_CLOUD);
  drawWrapped("\"" + comment.text + "\"", 20, 112, W - 40, 5, 21);
}

// ---------------------------------------------------------------- growth chart

void drawGrowthPage() {
  int64_t daily[7];
  dailyViews(daily);
  int64_t sWeek = subsThisWeek();
  drawLabel(12, 40, "VIEWS PER DAY", C_ACCENT, "last 7 days");
  if (sWeek > 0) {
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TR);
    tft.setTextColor(C_GREEN);
    tft.drawString((signedCompact(sWeek) + " subs").c_str(), W - 12, 40);
  }

  int64_t biggest = 1;
  for (int i = 0; i < 7; i++) if (daily[i] > biggest) biggest = daily[i];

  const char* dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  time_t now = time(nullptr);
  int slot = (W - 24) / 7, barW = slot - 14, top = 78, bottom = 200;
  for (int i = 0; i < 7; i++) {
    int x = 12 + i * slot + (slot - barW) / 2;
    int cx = x + barW / 2;
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TC);
    if (daily[i] >= 0) {
      int h = max(3, (int)((bottom - top) * daily[i] / biggest));
      tft.fillSmoothRoundRect(x, bottom - h, barW, h, 4, i == 6 ? C_ACCENT : C_ACCENT2);
      tft.setTextColor(C_WHITE);
      tft.drawString(compact(daily[i]).c_str(), cx, bottom - h - 16);
    } else {
      tft.setTextColor(C_DIM);
      tft.drawString("-", cx, bottom - 16);
    }
    time_t dayTime = now - (time_t)(6 - i) * 86400;
    struct tm t;
    localtime_r(&dayTime, &t);
    tft.setTextColor(i == 6 ? C_WHITE : C_GREY);
    tft.drawString(i == 6 ? "Today" : dayNames[t.tm_wday], cx, bottom + 6);
  }
}

// ---------------------------------------------------------------- head to head

void drawComparePage() {
  drawLabel(12, 40, "HEAD TO HEAD", C_ACCENT, "");
  int cols[2] = {W / 4, W * 3 / 4};
  const String* names[2] = {&channel.title, &rival.title};
  uint64_t subs[2] = {channel.subs, rival.subs};
  uint64_t views[2] = {channel.views, rival.views};
  for (int i = 0; i < 2; i++) {
    tft.setFont(F_BOLD);
    tft.setTextDatum(D_TC);
    tft.setTextColor(i == 0 ? C_WHITE : C_GREY);
    tft.drawString(fitText(*names[i], W / 2 - 16).c_str(), cols[i], 62);
    tft.setFont(F_LARGE);
    tft.setTextColor(C_WHITE);
    tft.drawString(compact(subs[i]).c_str(), cols[i], 88);
    tft.setFont(F_SMALL);
    tft.setTextColor(C_GREY);
    tft.drawString("subscribers", cols[i], 124);
    tft.drawString((compact(views[i]) + " views").c_str(), cols[i], 142);
  }
  tft.drawFastVLine(W / 2, 64, 96, C_LINE);

  int64_t diff = (int64_t)channel.subs - (int64_t)rival.subs;
  String verdict = diff == 0 ? String("Neck and neck!")
                 : diff > 0 ? "You're " + compact(diff) + " ahead"
                            : "You're " + compact(-diff) + " behind";
  tft.setFont(F_MED);
  tft.setTextDatum(D_TC);
  tft.setTextColor(diff >= 0 ? C_GREEN : C_AMBER);
  tft.drawString(verdict.c_str(), W / 2, 168);

  float share = (channel.subs + rival.subs) ? (float)channel.subs / (float)(channel.subs + rival.subs) : 0.5f;
  tft.fillSmoothRoundRect(14, 204, W - 28, 10, 5, C_DIM);
  int fw = (int)((W - 28) * share);
  if (fw >= 10) tft.fillSmoothRoundRect(14, 204, fw, 10, 5, C_ACCENT);
}

// ---------------------------------------------------------------- live

void drawLivePage() {
  int li = liveIndex();
  if (li >= 0) {
    const VideoInfo& v = uploads.v[li];
    tft.fillSmoothRoundRect(W / 2 - 46, 42, 92, 26, 13, C_RED);
    tft.fillSmoothCircle(W / 2 - 28, 55, 5, C_WHITE);
    tft.setFont(F_BOLD);
    tft.setTextDatum(D_ML);
    tft.setTextColor(C_WHITE);
    tft.drawString("LIVE", W / 2 - 16, 55);
    drawBigNumber(withCommas(v.liveViewers), W / 2, 104, 280, 1.4f, C_WHITE);
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TC);
    tft.setTextColor(C_GREY);
    tft.drawString(("watching now  -  started " + timeAgo(v.liveStart)).c_str(), W / 2, 136);
    tft.setFont(F_BOLD);
    tft.setTextColor(C_WHITE);
    drawWrapped(v.title, 16, 164, W - 32, 2, 21);
    return;
  }
  int ui = upcomingIndex();
  if (ui < 0) return;
  const VideoInfo& v = uploads.v[ui];
  tft.fillSmoothRoundRect(W / 2 - 58, 42, 116, 26, 13, C_AMBER);
  tft.setFont(F_BOLD);
  tft.setTextDatum(D_MC);
  tft.setTextColor(C_BG);
  tft.drawString("COMING UP", W / 2, 55);
  long secs = (long)(v.liveStart - time(nullptr));
  drawBigNumber(secs > 0 ? durationText(secs) : String("Any minute"), W / 2, 104, 280, 1.4f, C_WHITE);
  struct tm t;
  localtime_r(&v.liveStart, &t);
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TC);
  tft.setTextColor(C_GREY);
  tft.drawString(("starts at " + clockText(t)).c_str(), W / 2, 136);
  tft.setFont(F_BOLD);
  tft.setTextColor(C_WHITE);
  drawWrapped(v.title, 16, 164, W - 32, 2, 21);
}

// ---------------------------------------------------------------- clock

void drawClockPage() {
  struct tm t;
  if (!getLocalTime(&t, 0)) return;
  drawBigNumber(clockText(t), W / 2, 100, 280, 2.2f, C_WHITE);
  if (!cfg.clock24) {
    tft.setFont(F_BOLD);
    tft.setTextDatum(D_TR);
    tft.setTextColor(C_GREY);
    tft.drawString(t.tm_hour < 12 ? "AM" : "PM", W - 10, 52);
  }
  const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  const char* months[] = {"January", "February", "March", "April", "May", "June", "July",
                          "August", "September", "October", "November", "December"};
  String date = String(days[t.tm_wday]) + " " + String(t.tm_mday) + " " + months[t.tm_mon];
  tft.setFont(F_MED);
  tft.setTextDatum(D_TC);
  tft.setTextColor(C_GREY);
  tft.drawString(date.c_str(), W / 2, 158);

  if (weather.loaded) {
    tft.setFont(F_BOLD);
    String line = tempText(weather.temp) + "  " + weatherText(weather.code);
    int w = degreeTextWidth(line);
    drawDegreeText(line, (W - w) / 2, 196, C_CLOUD);
  }
}

// ---------------------------------------------------------------- weather

// Simple weather pictures, drawn in a 90x90 box
void drawSun(int cx, int cy, int r) {
  for (int i = 0; i < 8; i++) {
    float a = i * PI / 4;
    tft.drawWideLine(cx + cos(a) * (r + 6), cy + sin(a) * (r + 6),
                     cx + cos(a) * (r + 14), cy + sin(a) * (r + 14), 2.0f, C_YELLOW);
  }
  tft.fillSmoothCircle(cx, cy, r, C_YELLOW);
}

void drawCloud(int x, int y, uint16_t colour) {
  tft.fillSmoothCircle(x + 26, y + 46, 15, colour);
  tft.fillSmoothCircle(x + 46, y + 38, 20, colour);
  tft.fillSmoothCircle(x + 64, y + 48, 13, colour);
  tft.fillSmoothRoundRect(x + 12, y + 44, 66, 18, 9, colour);
}

void drawWeatherIcon(int x, int y, int code, bool isDay) {
  bool clear = code <= 1, partly = code == 2;
  bool fog = code == 45 || code == 48;
  bool snow = (code >= 71 && code <= 77) || code == 85 || code == 86;
  bool storm = code >= 95;
  bool rain = !snow && !storm && code >= 51;

  if (clear || partly) {
    if (isDay) {
      drawSun(clear ? x + 45 : x + 32, clear ? y + 45 : y + 30, clear ? 20 : 14);
    } else {
      int cx = clear ? x + 45 : x + 32, cy = clear ? y + 42 : y + 28, r = clear ? 22 : 15;
      tft.fillSmoothCircle(cx, cy, r, C_CLOUD);
      tft.fillSmoothCircle(cx + r / 2, cy - r / 3, r * 4 / 5, bgAt(cy));
    }
    if (clear) return;
  }
  if (fog) {
    for (int i = 0; i < 4; i++) tft.fillSmoothRoundRect(x + 10 + (i % 2) * 8, y + 22 + i * 14, 62, 8, 4, C_CLOUD);
    return;
  }
  drawCloud(x, y + (partly ? 8 : 0), storm ? C_GREY : C_CLOUD);
  if (rain) {
    for (int i = 0; i < 4; i++) {
      int dx = x + 24 + i * 14;
      tft.drawWideLine(dx, y + 70, dx - 5, y + 84, 2.0f, C_BLUE);
    }
  }
  if (snow) {
    for (int i = 0; i < 4; i++) tft.fillSmoothCircle(x + 22 + i * 15, y + 76 + (i % 2) * 6, 3, C_WHITE);
  }
  if (storm) {
    tft.fillTriangle(x + 46, y + 58, x + 34, y + 78, x + 46, y + 76, C_YELLOW);
    tft.fillTriangle(x + 44, y + 74, x + 54, y + 74, x + 40, y + 92, C_YELLOW);
  }
}

void drawWeatherPage() {
  drawLabel(12, 40, cfg.placeName.length() ? cfg.placeName : cfg.city, C_ACCENT, "");
  drawWeatherIcon(14, 60, weather.code, weather.isDay);

  tft.setFont(F_HUGE);
  tft.setTextSize(1.3f);
  drawDegreeText(tempText(weather.temp), 130, 58, C_WHITE);
  tft.setTextSize(1);

  tft.setFont(F_BOLD);
  tft.setTextColor(C_WHITE);
  tft.setTextDatum(D_TL);
  tft.drawString(fitText(weatherText(weather.code), W - 140).c_str(), 132, 116);

  tft.setFont(F_SMALL);
  drawDegreeText("Feels " + tempText(weather.feels) + "   Wind " + String((int)round(weather.wind)) + " mph",
                 132, 140, C_GREY);
  drawDegreeText("High " + tempText(weather.hi[0]) + "  Low " + tempText(weather.lo[0]) +
                 "   Rain " + String(weather.rain[0]) + "%", 132, 158, C_GREY);

  // Tomorrow
  tft.fillSmoothRoundRect(8, 186, W - 16, 36, 10, C_CARD);
  tft.setFont(F_SMALL);
  String tomorrow = "Tomorrow:  " + String(weatherText(weather.dayCode[1])) + "   " +
                    tempText(weather.hi[1]) + " / " + tempText(weather.lo[1]) +
                    "   Rain " + String(weather.rain[1]) + "%";
  int w = degreeTextWidth(tomorrow);
  drawDegreeText(tomorrow, max(16, (W - w) / 2), 196, C_CLOUD);
}

// ---------------------------------------------------------------- crypto

String priceText(double p) {
  char buf[24];
  if (p >= 1000) return withCommas((uint64_t)(p + 0.5));
  if (p >= 1) snprintf(buf, sizeof(buf), "%.2f", p);
  else snprintf(buf, sizeof(buf), "%.4f", p);
  return String(buf);
}

void drawCryptoPage() {
  String cur = cfg.currency;
  cur.toUpperCase();
  drawLabel(12, 40, "CRYPTO", C_ACCENT, "prices in " + cur + ", change over 24h");
  for (int i = 0; i < crypto.count; i++) {
    int y = 60 + i * 56;
    tft.fillSmoothRoundRect(8, y, W - 16, 50, 10, C_CARD);
    tft.setFont(F_BOLD);
    tft.setTextDatum(D_TL);
    tft.setTextColor(C_WHITE);
    tft.drawString(fitText(coinName(crypto.id[i]), 130).c_str(), 20, y + 8);
    tft.setFont(F_SMALL);
    tft.setTextColor(C_GREY);
    tft.drawString(coinSymbol(crypto.id[i]).c_str(), 20, y + 28);

    tft.setFont(F_MED);
    tft.setTextDatum(D_TR);
    tft.setTextColor(C_WHITE);
    tft.drawString(priceText(crypto.price[i]).c_str(), W - 20, y + 6);
    char change[16];
    snprintf(change, sizeof(change), "%+.2f%%", crypto.change[i]);
    tft.setFont(F_SMALL);
    tft.setTextColor(crypto.change[i] >= 0 ? C_GREEN : C_ACCENT);
    tft.drawString(change, W - 20, y + 30);
  }
}

// ---------------------------------------------------------------- info screen

void showInfoScreen() {
  infoShowing = true;
  infoShownAt = millis();
  fillScreenBg();
  drawLogo(12, 12, 28, 20);
  tft.setFont(F_MED);
  tft.setTextDatum(D_ML);
  tft.setTextColor(C_WHITE);
  tft.drawString("Counter info", 50, 22);

  String ip = WiFi.localIP().toString();
  String lines[7] = {
    "Version " FIRMWARE_VERSION,
    "WiFi: " + cfg.wifiName + " (signal " + String((int)WiFi.RSSI()) + " dBm)",
    "Settings page: http://" + ip,
    "YouTube: about " + String(dailyUnits()) + " units a day",
    "Updates: " + updateStatus,
    lastError.length() ? "Problem: " + lastError : String("No problems"),
    "Keep holding to open setup mode",
  };
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  for (int i = 0; i < 7; i++) {
    tft.setTextColor(i == 2 ? C_WHITE : i == 6 ? C_AMBER : C_GREY);
    tft.drawString(fitText(lines[i], W - 24).c_str(), 12, 50 + i * 22);
  }
  tft.setTextColor(C_DIM);
  tft.drawString("Tap to go back", 12, 214);
}
