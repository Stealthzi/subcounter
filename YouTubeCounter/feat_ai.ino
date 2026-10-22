// Ask ChatGPT about your data, from your phone.
//
// Open the counter's address on your phone and go to "Ask ChatGPT". Type a
// question ("How did my last video do?", "When should I fill up?") or ask for
// video ideas. The counter sends your question with a short summary of its own
// data to OpenAI, using your own API key, and shows the answer on your phone
// (and on the counter too, if you like).
//
// Privacy: nothing is sent until you ask something. Only the kinds of data you
// tick in Settings go with a question (the calendar is off unless you tick it),
// and "store: false" asks OpenAI not to keep the conversation.

struct AiTurn {
  String question, answer;
};
AiTurn        aiHistory[3];
int           aiHistoryCount = 0;

void aiRemember(const String& q, const String& a) {
  for (int i = 2; i > 0; i--) aiHistory[i] = aiHistory[i - 1];
  aiHistory[0] = {q, a};
  if (aiHistoryCount < 3) aiHistoryCount++;
}

// ---------------------------------------------------------------- what it knows

String aiLine(const String& label, const String& value) {
  return label + ": " + value + "\n";
}

String aiContext() {
  String c;
  c.reserve(2500);
  struct tm t;
  if (getLocalTime(&t, 0)) {
    char buf[40];
    strftime(buf, sizeof(buf), "%A %d %B %Y, %H:%M", &t);
    c += aiLine("Now", buf);
  }

  if ((cfg.aiShare & AI_CHANNEL) && channel.loaded) {
    c += "\n[YouTube channel]\n";
    c += aiLine("Channel", channel.title);
    c += aiLine("Subscribers (YouTube rounds this)", withCommas(channel.subs));
    c += aiLine("Total views", withCommas(channel.views));
    c += aiLine("Videos", withCommas(channel.videos));
    if (subsToday() >= 0) c += aiLine("Subscribers gained today", String((long)subsToday()));
    if (subsThisWeek() >= 0) c += aiLine("Subscribers gained this week", String((long)subsThisWeek()));
    if (viewsToday() >= 0) c += aiLine("Views today", String((long)viewsToday()));
    if (viewsThisWeek() >= 0) c += aiLine("Views this week", String((long)viewsThisWeek()));
    c += aiLine("Next milestone", withCommas(nextMilestone(channel.subs)) +
                (daysToMilestone() > 0 ? " (about " + String(daysToMilestone()) + " days away)" : String("")));
    int li = latestIndex();
    if (li >= 0) {
      const VideoInfo& v = uploads.v[li];
      c += aiLine("Latest video", "\"" + v.title + "\", " + timeAgo(v.published) + ", " + withCommas(v.views) + " views, " +
                  (v.hasLikes ? withCommas(v.likes) : String("hidden")) + " likes, " +
                  (v.hasComments ? withCommas(v.comments) : String("no")) + " comments");
      long ph = viewsPerHour();
      if (ph >= 0) c += aiLine("Latest video views per hour now", String(ph));
      String vs = vsUsualText();
      if (vs != "-") c += aiLine("Latest video vs the channel's usual at the same age", vs + (vs[0] == '~' ? " (estimate)" : ""));
    }
    for (int i = 0; i < uploads.count; i++) {
      if (i == li) continue;
      const VideoInfo& v = uploads.v[i];
      c += aiLine("Recent video", "\"" + v.title + "\", " + timeAgo(v.published) + ", " + withCommas(v.views) + " views");
    }
    if (topLoaded) c += aiLine("Most viewed video ever", "\"" + topVideo.title + "\", " + withCommas(topVideo.views) + " views");
    if (analytics.loaded) {
      c += aiLine("Exact stats for " + analytics.dayLabel, "+" + String(analytics.gained) + " / -" + analytics.lost +
                  " subscribers, " + analytics.views + " views, " + analytics.minutes + " minutes watched");
      c += aiLine("Exact stats, last 28 days", "+" + String(analytics.gained28) + " / -" + analytics.lost28 + " subscribers, " +
                  analytics.views28 + " views, " + analytics.minutes28 + " minutes watched");
      if (analytics.revenue28 >= 0) c += aiLine("Estimated revenue, last 28 days", String(analytics.revenue28, 2));
    }
  }

  if ((cfg.aiShare & AI_ENERGY) && energy.loaded) {
    c += "\n[Home energy, Octopus]\n";
    c += aiLine("Electricity", String(energy.elecRate, 2) + "p per kWh, standing charge " + String(energy.elecStanding, 2) + "p a day");
    if (energy.hasGas) c += aiLine("Gas", String(energy.gasRate, 2) + "p per kWh, standing charge " + String(energy.gasStanding, 2) + "p a day");
    for (int i = max(0, energy.days - 7); i < energy.days; i++) {
      int y, m, d;
      dayToDate(energy.day[i], y, m, d);
      String use = String(energy.elecKwh[i], 1) + " kWh electricity";
      if (energy.hasGas) use += ", " + String(energy.gasKwh[i], 1) + " kWh gas";
      c += aiLine(String(d) + "/" + m, use + ", cost about " + String(dayCost(i) / 100.0f, 2) + " pounds");
    }
  }

  if ((cfg.aiShare & AI_PETROL) && fuel.loaded) {
    c += "\n[Petrol nearby, " + cfg.fuelType + "]\n";
    for (int i = 0; i < fuel.count; i++)
      c += aiLine(i == 0 ? "Cheapest" : "Also", fuel.st[i].brand + ", " + fuel.st[i].place + ": " + String(fuel.st[i].price, 1) +
                  "p, " + String(fuel.st[i].miles, 1) + " miles away");
    c += aiLine("Area average", String(fuel.avg, 1) + "p");
    int days;
    float change = fuelTrend(days);
    if (!isnan(change)) c += aiLine("Cheapest price change over " + String(days) + " days", String(change, 1) + "p");
  }

  if ((cfg.aiShare & AI_WEATHER) && weather.loaded) {
    c += "\n[Weather in " + cfg.placeName + "]\n";
    String unit = cfg.fahrenheit ? "F" : "C";
    c += aiLine("Now", String(weather.temp, 0) + unit + ", " + weatherText(weather.code) + ", wind " + String(weather.wind, 0));
    c += aiLine("Today", String(weather.lo[0], 0) + " to " + String(weather.hi[0], 0) + unit + ", " + weatherText(weather.dayCode[0]) +
                ", " + weather.rain[0] + "% chance of rain");
    c += aiLine("Tomorrow", String(weather.lo[1], 0) + " to " + String(weather.hi[1], 0) + unit + ", " + weatherText(weather.dayCode[1]) +
                ", " + weather.rain[1] + "% chance of rain");
  }

  if ((cfg.aiShare & AI_CALENDAR) && calendar.loaded) {
    c += "\n[Calendar]\n";
    for (int i = 0; i < calendar.count; i++) {
      struct tm et;
      time_t st = calendar.ev[i].start;
      localtime_r(&st, &et);
      char buf[32];
      strftime(buf, sizeof(buf), calendar.ev[i].allDay ? "%a %d %b" : "%a %d %b %H:%M", &et);
      c += aiLine(buf, calendar.ev[i].title);
    }
  }

  if (cfg.aiShare & AI_PET) {
    c += "\n[Desk pet]\n";
    c += aiLine("Pet", cfg.petName + ", a " + speciesName(cfg.petType) + ", level " + rpg.level + " (" + petTitle(rpg.level) + "), " +
                rpg.coins + " coins, " + rpg.streak + "-day streak");
  }
  return c;
}

// ---------------------------------------------------------------- talking to OpenAI

// Returns the answer, or an empty string with `error` set
String aiAsk(const String& question, bool ideas, String& error) {
  error = "";
  if (!cfg.openaiKey.length()) {
    error = "Add your OpenAI API key in Settings first";
    return "";
  }
  if (WiFi.status() != WL_CONNECTED) {
    error = "No WiFi";
    return "";
  }
  const char* plain =
    "You are the assistant built into the user's desk counter, a small screen showing their YouTube channel stats and home "
    "information. Answer using the data below. Be brief: under 80 words, plain text, no markdown, British English. If the "
    "data doesn't cover the question, say so, then help in general if you can.";
  const char* brainstorm =
    "You are a YouTube ideas partner built into the user's desk counter. Using the channel data below (recent and most "
    "viewed videos), suggest 5 fresh video ideas. For each: a catchy title and one short line on why it could do well. "
    "Plain text, numbered, no markdown, British English.";

  JsonDocument req;
  req["model"] = cfg.aiModel.length() ? cfg.aiModel : String("gpt-6-luna");
  req["instructions"] = ideas ? brainstorm : plain;
  String input = aiContext();
  if (ideas && !(cfg.aiShare & AI_CHANNEL)) input += "\n(Sharing channel data is switched off in Settings.)\n";
  input += ideas ? "\nPlease suggest video ideas." : "\nQuestion: " + question;
  req["input"] = input;
  req["max_output_tokens"] = 1200;
  req["store"] = false;
  String body;
  serializeJson(req, body);

  HTTPClient http;
  tls.setInsecure();
  http.setTimeout(30000);
  if (!http.begin(tls, "https://api.openai.com/v1/responses")) {
    error = "Couldn't reach OpenAI";
    return "";
  }
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + cfg.openaiKey);
  int code = http.POST(body);
  String reply = code > 0 ? http.getString() : "";
  http.end();

  JsonDocument filter;
  filter["output"][0]["type"] = true;
  filter["output"][0]["content"][0]["type"] = true;
  filter["output"][0]["content"][0]["text"] = true;
  filter["error"]["message"] = true;
  JsonDocument doc;
  bool parsed = !deserializeJson(doc, reply, DeserializationOption::Filter(filter));

  if (code != 200) {
    String msg = parsed ? String(doc["error"]["message"] | "") : String("");
    if (code == 401) error = "OpenAI didn't accept the API key - check it in Settings";
    else if (code == 429) error = "OpenAI says: out of credit, or too many questions at once";
    else if (code == 404 || code == 400) error = "OpenAI problem: " + (msg.length() ? msg : String(code)) + " (check the model name in Settings)";
    else error = "Couldn't get an answer (" + String(code) + ")";
    return "";
  }
  String text;
  for (JsonVariant item : doc["output"].as<JsonArray>()) {
    if (String(item["type"] | "") != "message") continue;
    for (JsonVariant part : item["content"].as<JsonArray>()) {
      if (String(part["type"] | "") == "output_text") text += part["text"].as<String>();
    }
  }
  text.trim();
  if (!text.length()) error = "The answer came back empty - try again";
  return text;
}

// Shows the answer on the counter for a while (tap to close)
void showAiAnswer(const String& question, const String& answer) {
  fillScreenBg();
  tft.setTextSize(1);
  tft.setFont(F_BOLD);
  gradientText("ChatGPT", 12, 10, false, 0xFFFFFF, theme().accent);
  tft.setFont(F_SMALL);
  tft.setTextColor(C_GREY);
  tft.setTextDatum(D_TL);
  tft.drawString(fitText(question, W - 24).c_str(), 12, 34);
  tft.drawFastHLine(12, 54, W - 24, C_LINE);
  tft.setFont(F_BODY);
  tft.setTextColor(C_WHITE);
  drawWrapped(answer, 12, 62, W - 24, 8, 21);
  infoShowing = true;
  infoShownAt = millis() + 30000;  // stays up 45 seconds, or until tapped
}

// ---------------------------------------------------------------- the phone page

void sendAskPage(const String& note) {
  lastPortalActivity = millis();
  String p;
  p.reserve(6000);
  p += PAGE_HEAD;
  p += "<h1><span class=logo></span>Ask ChatGPT</h1>";
  p += "<p class=sub>About your channel, energy, petrol and more</p>";
  if (note.length()) p += "<div class=msg>" + htmlEscape(note) + "</div>";
  if (!cfg.openaiKey.length()) {
    p += "<p>Add your OpenAI API key in the <a href=\"/\">settings</a> (section \"Ask ChatGPT\") to use this.</p>";
  }
  p += "<form method=post action=/ask><fieldset><legend>Your question</legend>"
       "<label><textarea name=q rows=3 maxlength=300 placeholder=\"e.g. How did my last video do?\" "
       "style=\"width:100%;box-sizing:border-box\"></textarea></label><button>Ask</button></fieldset></form>";
  p += "<form method=post action=/ask><fieldset><legend>Quick questions</legend>";
  const char* quick[] = {"How did my last video do?", "How many subscribers this week?", "When should I fill up?",
                         "How much energy did I use yesterday?", "What's the weather doing tomorrow?"};
  for (int i = 0; i < 5; i++) p += "<button name=q value=\"" + String(quick[i]) + "\" class=alt>" + quick[i] + "</button> ";
  p += "<button name=ideas value=1>Brainstorm video ideas</button></fieldset></form>";
  for (int i = 0; i < aiHistoryCount; i++) {
    p += "<fieldset><legend>" + htmlEscape(aiHistory[i].question) + "</legend><p style=\"white-space:pre-wrap\">" +
         htmlEscape(aiHistory[i].answer) + "</p></fieldset>";
  }
  p += "<p class=note>Only the data you've allowed in Settings is sent with a question, and OpenAI is asked not to store it.</p>";
  p += "<p><a href=\"/\">Back to settings</a></p>";
  p += PAGE_TAIL;
  server.send(200, "text/html", p);
}

void handleAsk() {
  lastPortalActivity = millis();
  bool ideas = server.hasArg("ideas");
  String q = server.arg("q");
  q.trim();
  if (!ideas && !q.length()) {
    sendAskPage("Type a question first");
    return;
  }
  String shown = ideas ? String("Video ideas, please!") : q;
  if (cfg.aiOnScreen && !setupMode) showMessage("Asking ChatGPT...", fitText(shown, W - 40), "");
  String error;
  String answer = aiAsk(q, ideas, error);
  if (error.length()) {
    if (cfg.aiOnScreen && !setupMode) {
      if (currentPage >= 0) drawCurrentPage();
    }
    sendAskPage(error);
    return;
  }
  aiRemember(shown, answer);
  if (cfg.aiOnScreen && !setupMode) showAiAnswer(shown, answer);
  sendAskPage("");
}
