// Full-screen messages: starting up, problems, setup mode and updating

void showMessage(const String& title, const String& line1, const String& line2) {
  tft.fillScreen(C_BG);
  drawLogo(W / 2 - 22, 46, 44, 31);
  tft.setTextSize(1);
  tft.setFont(F_MED);
  tft.setTextDatum(D_TC);
  tft.setTextColor(C_WHITE);
  tft.drawString(fitText(title, W - 20).c_str(), W / 2, 96);
  tft.setFont(F_SMALL);
  tft.setTextColor(C_GREY);
  tft.drawString(fitText(line1, W - 20).c_str(), W / 2, 136);
  tft.drawString(fitText(line2, W - 20).c_str(), W / 2, 156);
  tft.setTextColor(C_DIM);
  tft.drawString("v" FIRMWARE_VERSION, W / 2, 218);
}

void drawSetupScreen() {
  tft.fillScreen(C_BG);
  drawLogo(12, 12, 28, 20);
  tft.setTextSize(1);
  tft.setFont(F_MED);
  tft.setTextDatum(D_ML);
  tft.setTextColor(C_WHITE);
  tft.drawString(setupReason == SETUP_FIRST_TIME ? "Let's set up" : "Setup mode", 50, 22);

  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_GREY);
  tft.drawString("1. On your phone, join WiFi:", 12, 50);
  tft.setFont(F_BOLD);
  tft.setTextColor(C_WHITE);
  tft.drawString(SETUP_WIFI_NAME, 24, 70);
  tft.setFont(F_SMALL);
  tft.setTextColor(C_GREY);
  tft.drawString("   or scan the code", 12, 92);
  tft.drawString("2. The setup page opens.", 12, 118);
  tft.drawString("   If not, go to 192.168.4.1", 12, 136);
  tft.drawString("3. Fill it in, press Save.", 12, 160);

  if (setupReason == SETUP_WIFI_FAILED) {
    tft.setTextColor(C_AMBER);
    tft.drawString(fitText("Couldn't join WiFi \"" + cfg.wifiName + "\"", W - 24).c_str(), 12, 192);
    tft.drawString("Trying it again in 5 minutes.", 12, 210);
  } else if (setupReason == SETUP_ASKED) {
    tft.setTextColor(C_DIM);
    tft.drawString("Goes back to normal in 10 minutes", 12, 192);
    tft.drawString("if nothing is changed.", 12, 210);
  }

  tft.qrcode("WIFI:T:nopass;S:" SETUP_WIFI_NAME ";;", 190, 46, 120, 1, true);
}

void drawUpdateScreen(const String& version, int percent) {
  static int lastPercent = -1;
  if (percent >= 0 && percent == lastPercent) return;
  if (percent < 0 || lastPercent < 0) {
    tft.fillScreen(C_BG);
    drawLogo(W / 2 - 22, 40, 44, 31);
    tft.setTextSize(1);
    tft.setFont(F_MED);
    tft.setTextDatum(D_TC);
    tft.setTextColor(C_WHITE);
    tft.drawString(("Updating to " + version).c_str(), W / 2, 92);
    tft.setFont(F_SMALL);
    tft.setTextColor(C_AMBER);
    tft.drawString("Please don't unplug the counter", W / 2, 180);
    tft.setTextColor(C_DIM);
    tft.drawString("It restarts by itself when it's done", W / 2, 200);
  }
  lastPercent = percent < 0 ? 0 : percent;
  drawProgressBar(40, 136, W - 80, 16, lastPercent / 100.0f, C_GREEN);
}
