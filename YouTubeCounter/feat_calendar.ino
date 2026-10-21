// Google Calendar: your next events. A tiny script you add to your own Google
// account (README.md has the copy-and-paste steps) gives the counter a private
// link that returns just the next week's events. The link stays on the board.

bool updateCalendar() {
  String body;
  if (httpGet(cfg.calendarUrl, body) != 200) return false;
  JsonDocument doc;
  if (deserializeJson(doc, body) || !doc.is<JsonArray>()) return false;
  calendar.count = 0;
  for (JsonVariant e : doc.as<JsonArray>()) {
    if (calendar.count >= CAL_MAX) break;
    CalEvent& ev = calendar.ev[calendar.count++];
    ev.title = cleanText(e["t"] | "");
    ev.start = (time_t)(e["s"] | 0L);
    ev.end = (time_t)(e["e"] | 0L);
    ev.allDay = e["a"] | false;
    if (!ev.title.length()) ev.title = "(no title)";
  }
  calendar.loaded = true;
  return true;
}

String dayHeading(time_t t, bool allDay) {
  struct tm d;
  // All-day events start at midnight UTC in the script's output
  if (allDay) gmtime_r(&t, &d);
  else localtime_r(&t, &d);
  uint32_t day = (uint32_t)daysFromCivil(d.tm_year + 1900, d.tm_mon + 1, d.tm_mday);
  uint32_t today = localDay();
  if (day == today) return "Today";
  if (day == today + 1) return "Tomorrow";
  const char* names[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  return String(names[d.tm_wday]) + " " + String(d.tm_mday) + " " + months[d.tm_mon];
}

void drawCalendarPage() {
  time_t now = time(nullptr);
  int y = 42;
  String lastHeading;
  int shown = 0;
  for (int i = 0; i < calendar.count && y < 212; i++) {
    const CalEvent& ev = calendar.ev[i];
    if (ev.end && ev.end < now) continue;
    String heading = dayHeading(ev.start, ev.allDay);
    if (heading != lastHeading) {
      if (y > 190) break;
      drawLabel(12, y, heading, C_ACCENT, "");
      lastHeading = heading;
      y += 20;
    }
    bool happening = !ev.allDay && ev.start <= now && ev.end > now;
    drawCardBox(8, y, W - 16, 28);
    if (happening) tft.fillSmoothRoundRect(12, y + 6, 4, 16, 2, C_GREEN);
    struct tm st;
    localtime_r(&ev.start, &st);
    String when = ev.allDay ? String("All day") : happening ? String("Now") : clockText(st);
    tft.setTextSize(1);
    tft.setFont(F_BOLD);
    tft.setTextDatum(D_ML);
    tft.setTextColor(happening ? C_GREEN : C_ACCENT2);
    tft.drawString(when.c_str(), 22, y + 14);
    tft.setTextColor(C_WHITE);
    tft.drawString(fitText(ev.title, W - 110).c_str(), 92, y + 14);
    y += 32;
    shown++;
  }
  if (shown == 0) {
    tft.setFont(F_MED);
    tft.setTextDatum(D_MC);
    gradientTextCentred("Nothing planned", W / 2, 110, false);
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TC);
    tft.setTextColor(C_GREY);
    tft.drawString("Your next 7 days are free", W / 2, 146);
  }
}
