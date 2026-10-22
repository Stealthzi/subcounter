// Two channel extras:
//
// "vs your usual" - how your latest video is doing compared with your others
// at the same age. The counter notes each new video's views at 6 hours, 1 day,
// 2 days and a week, so the comparison gets more accurate with every upload.
// Until it has seen a few, it estimates from your recent videos' totals.
//
// The weekly report card - last week's views and subscribers, against the
// week before, with a grade. It needs a week or two of daily snapshots.

const uint16_t CK_HOURS[4] = {6, 24, 48, 168};

void loadCheckpoints() {
  memset(checkpoints, 0, sizeof(checkpoints));
  if (prefs.getBytesLength("vidck") == sizeof(checkpoints)) prefs.getBytes("vidck", checkpoints, sizeof(checkpoints));
}

uint32_t& ckValue(VideoCheckpoint& c, int i) {
  return i == 0 ? c.v6 : i == 1 ? c.v24 : i == 2 ? c.v48 : c.v168;
}

// Called after the uploads list refreshes: records views at each checkpoint age
void updateCheckpoints() {
  uint32_t now = (uint32_t)time(nullptr);
  if (now < 1700000000) return;
  bool changed = false;
  for (int u = 0; u < uploads.count; u++) {
    const VideoInfo& v = uploads.v[u];
    if (v.live != LIVE_NONE || v.published == 0) continue;
    uint32_t h = textHash(v.id);
    int slot = -1, oldest = 0;
    for (int i = 0; i < CHECKPOINT_VIDEOS; i++) {
      if (checkpoints[i].idHash == h) slot = i;
      if (checkpoints[i].published < checkpoints[oldest].published) oldest = i;
    }
    float ageH = (now - (uint32_t)v.published) / 3600.0f;
    if (slot < 0) {
      if (ageH > 200) continue;  // too old to be useful now
      slot = oldest;
      memset(&checkpoints[slot], 0, sizeof(VideoCheckpoint));
      checkpoints[slot].idHash = h;
      checkpoints[slot].published = (uint32_t)v.published;
      changed = true;
    }
    for (int i = 0; i < 4; i++) {
      uint32_t& val = ckValue(checkpoints[slot], i);
      // only within a short window after the age, so it's a fair comparison
      if (val == 0 && ageH >= CK_HOURS[i] && ageH < CK_HOURS[i] * 1.25f + 2) {
        val = max((uint32_t)1, (uint32_t)v.views);
        changed = true;
      }
    }
  }
  if (changed) prefs.putBytes("vidck", checkpoints, sizeof(checkpoints));

  // XP once the latest video has beaten your usual at a day old
  int li = latestIndex();
  if (li >= 0) {
    float ageH = (now - (uint32_t)uploads.v[li].published) / 3600.0f;
    bool real = false;
    float r = vsUsual(real);
    if (ageH >= 24 && ageH < 48 && r >= 1.0f) rpgOnBeatAverage(textHash(uploads.v[li].id));
  }
}

// Middle value of a checkpoint across your other videos, or -1 if too few
float usualAt(int i, uint32_t skipHash) {
  uint32_t vals[CHECKPOINT_VIDEOS];
  int n = 0;
  for (int k = 0; k < CHECKPOINT_VIDEOS; k++) {
    VideoCheckpoint& c = checkpoints[k];
    if (!c.idHash || c.idHash == skipHash) continue;
    uint32_t v = ckValue(c, i);
    if (v) vals[n++] = v;
  }
  if (n < 2) return -1;
  for (int a = 1; a < n; a++)  // sort (tiny list)
    for (int b = a; b > 0 && vals[b] < vals[b - 1]; b--) {
      uint32_t t = vals[b];
      vals[b] = vals[b - 1];
      vals[b - 1] = t;
    }
  return n % 2 ? vals[n / 2] : (vals[n / 2 - 1] + vals[n / 2]) / 2.0f;
}

// The latest video's views divided by what's usual at its age (0 = unknown).
// `real` says whether it's from your own history rather than an estimate.
float vsUsual(bool& real) {
  real = false;
  int li = latestIndex();
  if (li < 0) return 0;
  const VideoInfo& v = uploads.v[li];
  uint32_t now = (uint32_t)time(nullptr);
  if (v.published == 0 || now < (uint32_t)v.published) return 0;
  float ageH = (now - (uint32_t)v.published) / 3600.0f;
  if (ageH < 1) return 0;
  uint32_t h = textHash(v.id);

  // From your own checkpoints: interpolate between the ages it has seen
  float px = 0, py = 0;
  bool have = true;
  for (int i = 0; i < 4; i++) {
    float u = usualAt(i, h);
    if (u < 0) {
      have = false;
      break;
    }
    if (ageH <= CK_HOURS[i]) {
      float expected = py + (u - py) * (ageH - px) / (CK_HOURS[i] - px);
      real = true;
      return expected > 0 ? v.views / expected : 0;
    }
    px = CK_HOURS[i];
    py = u;
  }
  if (have && py > 0) {
    real = true;
    return v.views / py;
  }

  // Estimate: your other recent videos' views, scaled for this one's age
  uint64_t others[RECENT_COUNT];
  int n = 0;
  for (int i = 0; i < uploads.count; i++) {
    if (i == li || uploads.v[i].live != LIVE_NONE) continue;
    float a = (now - (uint32_t)uploads.v[i].published) / 3600.0f;
    if (a >= 72) others[n++] = uploads.v[i].views;
  }
  if (n == 0) return 0;
  uint64_t sum = 0;
  for (int i = 0; i < n; i++) sum += others[i];
  float typical = (float)sum / n;
  float expected = typical * (1.0f - expf(-ageH / 60.0f));
  return expected > 1 ? v.views / expected : 0;
}

String vsUsualText() {
  bool real;
  float r = vsUsual(real);
  if (r <= 0) return "-";
  char buf[12];
  snprintf(buf, sizeof(buf), r >= 10 ? "%.0fx" : "%.1fx", r);
  if (!real) return "~" + String(buf);  // an estimate until it knows your usual
  return String(buf);
}

// ---------------------------------------------------------------- weekly report card

void dayToDate(uint32_t day, int& y, int& m, int& d) {
  long z = (long)day + 719468;
  long era = (z >= 0 ? z : z - 146096) / 146097;
  unsigned doe = (unsigned)(z - era * 146097);
  unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  long yy = (long)yoe + era * 400;
  unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  unsigned mp = (5 * doy + 2) / 153;
  d = doy - (153 * mp + 2) / 5 + 1;
  m = mp < 10 ? mp + 3 : mp - 9;
  y = yy + (m <= 2);
}

// A snapshot for the day, or the day either side
const DaySnap* snapNear(uint32_t day) {
  const DaySnap* s = snapFor(day);
  if (!s) s = snapFor(day + 1);
  if (!s) s = snapFor(day - 1);
  return s;
}

uint32_t thisMonday() {
  uint32_t today = localDay();
  return today ? today - (today + 3) % 7 : 0;
}

bool reportAvailable() {
  uint32_t mon = thisMonday();
  return mon && snapNear(mon) && snapNear(mon - 7);
}

String shortDay(uint32_t day) {
  int y, m, d;
  dayToDate(day, y, m, d);
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  return String(d) + " " + months[(m - 1) % 12];
}

void drawReportPage() {
  uint32_t mon = thisMonday();
  const DaySnap* end = snapNear(mon);
  const DaySnap* start = snapNear(mon - 7);
  const DaySnap* before = snapNear(mon - 14);
  if (!end || !start) return;
  int64_t views = (int64_t)end->views - (int64_t)start->views;
  int64_t subs = (int64_t)end->subs - (int64_t)start->subs;
  int64_t prevViews = before ? (int64_t)start->views - (int64_t)before->views : -1;

  drawLabel(12, 40, "WEEKLY REPORT CARD", C_ACCENT, shortDay(mon - 7) + " - " + shortDay(mon - 1));

  // The grade, against the week before
  String grade = "New";
  uint16_t gradeCol = C_ACCENT2;
  String change = "First week - next time there's a comparison";
  if (prevViews > 0) {
    float r = (float)views / prevViews;
    grade = r >= 1.25f ? "A+" : r >= 1.05f ? "A" : r >= 0.95f ? "B" : r >= 0.8f ? "C" : "D";
    gradeCol = r >= 1.05f ? C_GREEN : r >= 0.95f ? C_ACCENT2 : r >= 0.8f ? C_AMBER : C_RED;
    char buf[48];
    snprintf(buf, sizeof(buf), "%+.0f%% views on the week before", (r - 1) * 100);
    change = buf;
  }
  tft.fillSmoothCircle(268, 92, 32, C_CARD);
  tft.fillSmoothCircle(268, 92, 28, gradeCol);
  tft.setTextSize(1);
  tft.setFont(grade.length() > 2 ? F_MED : F_LARGE);
  tft.setTextDatum(D_MC);
  tft.setTextColor(RGB565(10, 14, 30));
  tft.drawString(grade.c_str(), 268, 93);

  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_GREY);
  tft.drawString("Views last week", 12, 60);
  tft.setFont(F_LARGE);
  gradientText(views >= 0 ? "+" + withCommas(views) : String("-"), 12, 76, false, 0xFFFFFF, theme().accent);
  tft.setFont(F_SMALL);
  tft.setTextColor(prevViews > 0 && views >= prevViews ? C_GREEN : C_GREY);
  tft.drawString(fitText(change, 220).c_str(), 12, 112);

  // Uploads last week, and the best of them
  int count = 0, best = -1;
  uint32_t from = (uint32_t)((mon - 7) * 86400LL), to = (uint32_t)(mon * 86400LL);
  for (int i = 0; i < uploads.count; i++) {
    uint32_t p = (uint32_t)uploads.v[i].published;
    if (p >= from && p < to) {
      count++;
      if (best < 0 || uploads.v[i].views > uploads.v[best].views) best = i;
    }
  }
  drawCard(8, 146, 97, 76, "SUBSCRIBERS", signedCompact(subs), subs >= 0 ? C_GREEN : C_RED);
  drawCard(111, 146, 97, 76, "UPLOADS", String(count), C_WHITE);
  drawCard(214, 146, 97, 76, "VS USUAL", vsUsualText(), C_WHITE);
  if (best >= 0) {
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TL);
    tft.setTextColor(C_DIM);
    tft.drawString(fitText("Best: " + uploads.v[best].title, 220).c_str(), 12, 128);
  }
}
