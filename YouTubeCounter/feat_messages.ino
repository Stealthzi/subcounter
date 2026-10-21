// Messages and the friendship lamp between two counters.
//
// Both counters share a pairing code (Settings > Messages). Every message is
// encrypted on the board with AES-256-GCM using a key made from that code,
// then passed through the free ntfy.sh relay. ntfy.sh only ever sees
// scrambled text, on a channel name that's also derived from the code.

const char* RELAY = "https://ntfy.sh/";
String lastMsgId;

// "k7qm 2xpa-9rtd" -> "K7QM2XPA9RTD"
String normalisedCode() {
  String c;
  for (size_t i = 0; i < cfg.msgCode.length(); i++) {
    char ch = toupper(cfg.msgCode[i]);
    if (isalnum(ch)) c += ch;
  }
  return c;
}

void sha256Of(const String& s, uint8_t out[32]) {
  mbedtls_sha256((const unsigned char*)s.c_str(), s.length(), out, 0);
}

String relayTopic() {
  uint8_t h[32];
  sha256Of("subcounter-topic:" + normalisedCode(), h);
  String t = "sc";
  const char* hex = "0123456789abcdef";
  for (int i = 0; i < 12; i++) {
    t += hex[h[i] >> 4];
    t += hex[h[i] & 15];
  }
  return t;
}

// Encrypts text -> base64 of (12-byte nonce | ciphertext | 16-byte tag)
String sealMessage(const String& plain) {
  uint8_t key[32];
  sha256Of("subcounter-key:" + normalisedCode(), key);
  size_t n = plain.length();
  std::vector<uint8_t> buf(12 + n + 16);
  esp_fill_random(buf.data(), 12);
  mbedtls_gcm_context g;
  mbedtls_gcm_init(&g);
  mbedtls_gcm_setkey(&g, MBEDTLS_CIPHER_ID_AES, key, 256);
  int err = mbedtls_gcm_crypt_and_tag(&g, MBEDTLS_GCM_ENCRYPT, n, buf.data(), 12, nullptr, 0,
                                      (const uint8_t*)plain.c_str(), buf.data() + 12, 16, buf.data() + 12 + n);
  mbedtls_gcm_free(&g);
  if (err) return "";
  size_t outLen = 0;
  std::vector<uint8_t> out(4 * (buf.size() + 2) / 3 + 4);
  mbedtls_base64_encode(out.data(), out.size(), &outLen, buf.data(), buf.size());
  return String((const char*)out.data(), outLen);
}

bool openMessage(const String& sealed, String& plain) {
  std::vector<uint8_t> buf(sealed.length());
  size_t len = 0;
  if (mbedtls_base64_decode(buf.data(), buf.size(), &len, (const uint8_t*)sealed.c_str(), sealed.length())) return false;
  if (len < 12 + 16 + 1) return false;
  uint8_t key[32];
  sha256Of("subcounter-key:" + normalisedCode(), key);
  size_t n = len - 12 - 16;
  std::vector<uint8_t> out(n + 1);
  mbedtls_gcm_context g;
  mbedtls_gcm_init(&g);
  mbedtls_gcm_setkey(&g, MBEDTLS_CIPHER_ID_AES, key, 256);
  int err = mbedtls_gcm_auth_decrypt(&g, n, buf.data(), 12, nullptr, 0, buf.data() + 12 + n, 16,
                                     buf.data() + 12, out.data());
  mbedtls_gcm_free(&g);
  if (err) return false;  // not ours, or tampered with
  out[n] = 0;
  plain = String((const char*)out.data());
  return true;
}

String myName() {
  return cfg.msgName.length() ? cfg.msgName : String("Your brother");
}

// kind: "msg" (text), "glow" (friendship lamp), "visit" / "gift" (pets)
bool sendMessage(const String& kind, const String& text) {
  if (WiFi.status() != WL_CONNECTED || !normalisedCode().length()) return false;
  JsonDocument doc;
  doc["k"] = kind;
  doc["f"] = myName();
  doc["d"] = hostName;
  doc["m"] = text;
  String plain;
  serializeJson(doc, plain);
  String sealed = sealMessage(plain);
  if (!sealed.length()) return false;

  tls.setInsecure();
  HTTPClient http;
  http.setTimeout(10000);
  http.setUserAgent("SubCounter/" FIRMWARE_VERSION);
  if (!http.begin(tls, String(RELAY) + relayTopic())) return false;
  http.addHeader("Content-Type", "text/plain");
  int code = http.POST(sealed);
  http.end();
  if (code != 200) return false;

  if (kind == "msg") {
    addToInbox("You", text, time(nullptr), true);
    if (cfg.ledFlash) flashLed(255, 0, 255, 1);
  }
  return true;
}

void addToInbox(const String& from, const String& text, time_t t, bool mine) {
  for (int i = INBOX_MAX - 1; i > 0; i--) inbox[i] = inbox[i - 1];
  inbox[0].from = from;
  inbox[0].text = text;
  inbox[0].t = t;
  inbox[0].mine = mine;
  if (inboxCount < INBOX_MAX) inboxCount++;
  saveInbox();
}

void saveInbox() {
  JsonDocument doc;
  for (int i = 0; i < inboxCount; i++) {
    JsonObject m = doc.add<JsonObject>();
    m["f"] = inbox[i].from;
    m["m"] = inbox[i].text;
    m["t"] = (long)inbox[i].t;
    m["y"] = inbox[i].mine;
  }
  String s;
  serializeJson(doc, s);
  prefs.putString("inbox", s);
}

void loadInbox() {
  lastMsgId = prefs.getString("msglast", "");
  JsonDocument doc;
  if (deserializeJson(doc, prefs.getString("inbox", "[]"))) return;
  inboxCount = 0;
  for (JsonVariant m : doc.as<JsonArray>()) {
    if (inboxCount >= INBOX_MAX) break;
    Message& x = inbox[inboxCount++];
    x.from = m["f"] | "";
    x.text = m["m"] | "";
    x.t = (time_t)(m["t"] | 0L);
    x.mine = m["y"] | false;
  }
}

void markMessagesRead() {
  if (unreadCount) {
    unreadCount = 0;
    drawHeader();
  }
}

// Checks the relay for anything new from the other counter
void pollMessages() {
  if (!normalisedCode().length()) return;
  String url = String(RELAY) + relayTopic() + "/json?poll=1&since=" + (lastMsgId.length() ? lastMsgId : String("1h"));
  String body;
  if (httpGet(url, body) != 200) return;

  int pos = 0;
  bool gotMessage = false;
  while (pos < (int)body.length()) {
    int end = body.indexOf('\n', pos);
    if (end < 0) end = body.length();
    String line = body.substring(pos, end);
    pos = end + 1;
    JsonDocument ev;
    if (deserializeJson(ev, line)) continue;
    if (String(ev["event"] | "") != "message") continue;
    lastMsgId = ev["id"] | "";
    String plain;
    if (!openMessage(ev["message"] | "", plain)) continue;
    JsonDocument msg;
    if (deserializeJson(msg, plain)) continue;
    if (String(msg["d"] | "") == hostName) continue;  // our own message coming back

    String kind = msg["k"] | "";
    String from = cleanText(msg["f"] | "");
    String text = cleanText(msg["m"] | "");
    time_t when = (time_t)(ev["time"] | 0L);
    if (kind == "msg") {
      addToInbox(from, text, when, false);
      unreadCount++;
      gotMessage = true;
    } else if (kind == "glow") {
      startLampGlow(from);
    } else if (kind == "visit" || kind == "gift") {
      petOnMessage(kind, from, text);
    }
  }
  if (lastMsgId.length()) prefs.putString("msglast", lastMsgId);

  if (gotMessage) {
    if (cfg.ledFlash) flashLed(255, 0, 255, 3);
    if (!userBusy() && !lampGlowing()) showPage(PAGE_MESSAGES);
    else drawHeader();
  }
}

// ---------------------------------------------------------------- messages page

const char* QUICK_REPLIES[] = {"Hi!", "Lol", "Call me"};

void drawMessagesPage() {
  if (inboxCount == 0) {
    tft.setFont(F_MED);
    gradientTextCentred("No messages yet", W / 2, 54, false);
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TC);
    tft.setTextColor(C_GREY);
    tft.drawString("Put the same pairing code on both counters", W / 2, 90);
    tft.drawString("(Settings > Messages). This counter's code:", W / 2, 108);
    tft.setFont(F_BOLD);
    tft.setTextColor(C_PURPLE);
    tft.drawString(cfg.msgCode.c_str(), W / 2, 132);
  }
  for (int i = 0; i < inboxCount && i < 3; i++) {
    const Message& m = inbox[i];
    int y = 40 + i * 44;
    int x = m.mine ? 48 : 8;
    tft.fillSmoothRoundRect(x, y, W - 56, 40, 10, m.mine ? mix(theme().card, 0x9B5CFF, 0.35f) : C_CARD);
    tft.setTextSize(1);
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TL);
    tft.setTextColor(m.mine ? C_CLOUD : C_PURPLE);
    tft.drawString(m.from.c_str(), x + 10, y + 4);
    tft.setTextDatum(D_TR);
    tft.setTextColor(C_DIM);
    tft.drawString(timeAgo(m.t).c_str(), x + W - 66, y + 4);
    tft.setFont(F_BOLD);
    tft.setTextDatum(D_TL);
    tft.setTextColor(C_WHITE);
    tft.drawString(fitText(m.text, W - 76).c_str(), x + 10, y + 20);
  }
  // Buttons: quick replies, type, glow
  for (int i = 0; i < 3; i++) drawButton(4 + i * 63, 178, 59, 40, QUICK_REPLIES[i], C_CARD, C_WHITE);
  drawButton(193, 178, 59, 40, "Type", C_LINE, C_WHITE);
  drawButton(256, 178, 60, 40, "Glow", mix(0x9B5CFF, cfg.lampColour, 0.5f), C_WHITE);
}

void showToast(const String& text, uint16_t colour) {
  tft.fillSmoothRoundRect(W / 2 - 80, 140, 160, 30, 15, colour);
  tft.setFont(F_BOLD);
  tft.setTextDatum(D_MC);
  tft.setTextColor(C_WHITE);
  tft.drawString(text.c_str(), W / 2, 155);
  delay(900);
}

bool messagesTap(int x, int y) {
  if (y < 170) return false;  // taps above the buttons flip the page
  for (int i = 0; i < 3; i++) {
    if (hitBox(x, y, 4 + i * 63, 178, 59, 40)) {
      bool ok = sendMessage("msg", QUICK_REPLIES[i]);
      showToast(ok ? "Sent!" : "Couldn't send", ok ? C_PURPLE : C_RED);
      drawCurrentPage();
      return true;
    }
  }
  if (hitBox(x, y, 193, 178, 59, 40)) {
    openKeyboard();
    return true;
  }
  if (hitBox(x, y, 256, 178, 60, 40)) {
    bool ok = sendMessage("glow", "");
    showToast(ok ? "Glow sent" : "Couldn't send", ok ? C_PURPLE : C_RED);
    drawCurrentPage();
    return true;
  }
  return false;
}

// ---------------------------------------------------------------- friendship lamp

bool lampGlowing() {
  return lampGlowUntil != 0 && !due(lampGlowUntil);
}

void startLampGlow(const String& from) {
  lampFrom = from;
  lampGlowUntil = millis() + 10UL * 60 * 1000;  // glows for 10 minutes
  uint32_t c = cfg.lampColour;
  setLed((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
  drawLampGlow();
}

void endLampGlow() {
  lampGlowUntil = 0;
  setLed(0, 0, 0);
  tft.setBrightness(shownBrightness);
  if (currentPage >= 0) drawCurrentPage();
}

// A soft glow from the middle of the screen, in the lamp colour
void drawLampGlow() {
  uint32_t c = cfg.lampColour;
  fillScreenBg();
  for (int r = 150; r > 0; r -= 6) {
    float t = 1.0f - r / 150.0f;
    tft.fillSmoothCircle(W / 2, H / 2 - 10, r, mix(to888(bgAt(H / 2)), c, t * t));
  }
  tft.setTextSize(1);
  tft.setFont(F_MED);
  tft.setTextDatum(D_MC);
  tft.setTextColor(C_WHITE);
  tft.drawString(fitText(lampFrom, W - 40).c_str(), W / 2, H / 2 - 18);
  tft.setFont(F_BOLD);
  tft.drawString("is thinking of you", W / 2, H / 2 + 8);
  tft.setFont(F_SMALL);
  tft.setTextColor(C_CLOUD);
  tft.drawString("Tap to glow back", W / 2, H - 24);
}

// Breathes by gently raising and lowering the backlight
void lampGlowFrame() {
  float phase = (millis() % 4000) / 4000.0f * 2 * PI;
  float level = 0.45f + 0.55f * (0.5f + 0.5f * sin(phase));
  tft.setBrightness((uint8_t)(shownBrightness * level));
}

void lampTap(int x, int y) {
  sendMessage("glow", "");
  endLampGlow();
}
