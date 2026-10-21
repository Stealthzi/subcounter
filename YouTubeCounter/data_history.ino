// Daily snapshots of subscribers and views, for "today", "this week",
// the 7-day chart and the milestone estimate. Kept for 35 days, and saved
// so a restart or update doesn't lose them.

void loadHistory() {
  memset(history, 0, sizeof(history));
  // Snapshots belong to one channel - start fresh if the channel changed
  if (prefs.getString("histch", "") != cfg.channel) return;
  if (prefs.getBytesLength("hist") == sizeof(history)) {
    prefs.getBytes("hist", history, sizeof(history));
  }
}

void saveHistory() {
  prefs.putString("histch", cfg.channel);
  prefs.putBytes("hist", history, sizeof(history));
}

// Today's date (local time) as days since 1970, or 0 if the clock isn't set yet
uint32_t localDay() {
  struct tm t;
  if (!getLocalTime(&t, 0)) return 0;
  return (uint32_t)daysFromCivil(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
}

const DaySnap* snapFor(uint32_t day) {
  if (day == 0) return nullptr;
  const DaySnap& s = history[day % HISTORY_DAYS];
  return s.day == day ? &s : nullptr;
}

// Takes today's snapshot the first time stats load each day
void recordHistory() {
  if (!channel.loaded) return;
  uint32_t today = localDay();
  if (today == 0 || snapFor(today)) return;
  DaySnap& s = history[today % HISTORY_DAYS];
  s.day   = today;
  s.taken = (uint32_t)time(nullptr);
  s.subs  = (uint32_t)channel.subs;
  s.views = channel.views;
  saveHistory();
}

// Gained since the start of today, or -1 if unknown
int64_t subsToday() {
  const DaySnap* s = snapFor(localDay());
  return s ? (int64_t)channel.subs - (int64_t)s->subs : -1;
}

int64_t viewsToday() {
  const DaySnap* s = snapFor(localDay());
  return s ? (int64_t)channel.views - (int64_t)s->views : -1;
}

// The oldest snapshot from up to `days` days ago (but not today's), or null
const DaySnap* oldestWithin(int days) {
  uint32_t today = localDay();
  if (today == 0) return nullptr;
  for (int back = days; back >= 1; back--) {
    const DaySnap* s = snapFor(today - back);
    if (s) return s;
  }
  return nullptr;
}

int64_t subsThisWeek() {
  const DaySnap* s = oldestWithin(7);
  return s ? (int64_t)channel.subs - (int64_t)s->subs : -1;
}

int64_t viewsThisWeek() {
  const DaySnap* s = oldestWithin(7);
  return s ? (int64_t)channel.views - (int64_t)s->views : -1;
}

// Views each day for the last 7 days (index 6 = today so far), -1 where unknown
void dailyViews(int64_t out[7]) {
  uint32_t today = localDay();
  for (int i = 0; i < 7; i++) {
    uint32_t day = today - (6 - i);
    const DaySnap* start = snapFor(day);
    const DaySnap* end = snapFor(day + 1);
    if (!start) out[i] = -1;
    else if (day == today) out[i] = (int64_t)channel.views - (int64_t)start->views;
    else if (end) out[i] = (int64_t)end->views - (int64_t)start->views;
    else out[i] = -1;
  }
}

// The chart needs at least one complete day
bool growthAvailable() {
  uint32_t today = localDay();
  return today != 0 && snapFor(today) && snapFor(today - 1);
}

// ---------------------------------------------------------------- milestones

const uint64_t MILESTONES[] = {
  100, 500, 1000, 5000, 10000, 25000, 50000, 100000, 250000, 500000,
  1000000, 2000000, 5000000, 10000000, 25000000, 50000000, 100000000, 250000000
};
const int MILESTONE_COUNT = sizeof(MILESTONES) / sizeof(MILESTONES[0]);

uint64_t nextMilestone(uint64_t subs) {
  for (int i = 0; i < MILESTONE_COUNT; i++) {
    if (MILESTONES[i] > subs) return MILESTONES[i];
  }
  return 0;
}

uint64_t previousMilestone(uint64_t subs) {
  uint64_t prev = 0;
  for (int i = 0; i < MILESTONE_COUNT; i++) {
    if (MILESTONES[i] <= subs) prev = MILESTONES[i];
  }
  return prev;
}

const char* milestoneAward(uint64_t m) {
  if (m == 100000) return "Silver Play Button";
  if (m == 1000000) return "Gold Play Button";
  if (m == 10000000) return "Diamond Play Button";
  if (m == 100000000) return "Red Diamond Award";
  return "";
}

// Estimated days to the next milestone at the recent rate, or -1 if unknown
long daysToMilestone() {
  uint64_t target = nextMilestone(channel.subs);
  const DaySnap* s = oldestWithin(30);
  if (!target || !s) return -1;
  double days = (time(nullptr) - (time_t)s->taken) / 86400.0;
  double gained = (double)channel.subs - (double)s->subs;
  if (days < 1 || gained <= 0) return -1;
  double perDay = gained / days;
  return (long)ceil((double)(target - channel.subs) / perDay);
}
