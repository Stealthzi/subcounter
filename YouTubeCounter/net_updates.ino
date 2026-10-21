// Remote updates from GitHub Releases.
//
// Each release is tagged with its version (e.g. "v2.0.1") and has the program
// attached as "firmware.bin". Counters check once a day:
//   - normally they take the newest full release
//   - with "Get test versions first" ticked they also take pre-releases,
//     so a new version can be tried on one counter before the others get it.

// "v2.0.1" -> 20001
long versionCode(const String& version) {
  String s = version;
  if (s.startsWith("v") || s.startsWith("V")) s = s.substring(1);
  int major = 0, minor = 0, patch = 0;
  sscanf(s.c_str(), "%d.%d.%d", &major, &minor, &patch);
  return major * 10000L + minor * 100L + patch;
}

bool updatesConfigured() {
  return String(UPDATE_REPO).indexOf("your-github-name") < 0;
}

String nowText() {
  struct tm t;
  if (!getLocalTime(&t, 0)) return "";
  char buf[24];
  strftime(buf, sizeof(buf), "%d %b %H:%M", &t);
  return String(buf);
}

void checkForUpdate() {
  if (!updatesConfigured()) {
    updateStatus = "Switched off in this version";
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    updateStatus = "No WiFi";
    return;
  }

  String api = String("https://api.github.com/repos/") + UPDATE_REPO +
               (cfg.earlyUpdates ? "/releases?per_page=1" : "/releases/latest");
  String body;
  int code = httpGet(api, body);
  if (code == 404) {
    updateStatus = "No releases yet (" + nowText() + ")";
    return;
  }
  if (code != 200) {
    updateStatus = "Couldn't reach GitHub (" + String(code) + ")";
    return;
  }

  JsonDocument filter;
  if (cfg.earlyUpdates) filter[0]["tag_name"] = true;
  else filter["tag_name"] = true;
  JsonDocument doc;
  if (deserializeJson(doc, body, DeserializationOption::Filter(filter))) {
    updateStatus = "Bad reply from GitHub";
    return;
  }
  const char* tagText = cfg.earlyUpdates ? (doc[0]["tag_name"] | "") : (doc["tag_name"] | "");
  String tag = tagText;
  if (tag.length() == 0) {
    updateStatus = "No releases yet (" + nowText() + ")";
    return;
  }
  if (versionCode(tag) <= versionCode(FIRMWARE_VERSION)) {
    updateStatus = "Up to date (checked " + nowText() + ")";
    return;
  }

  String url = String("https://github.com/") + UPDATE_REPO + "/releases/download/" + tag + "/firmware.bin";
  Serial.println("Updating from " + url);
  drawUpdateScreen(tag, -1);
  httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  httpUpdate.rebootOnUpdate(true);
  httpUpdate.onProgress([tag](int done, int total) {
    if (total > 0) drawUpdateScreen(tag, (int)((int64_t)done * 100 / total));
  });
  tls.setInsecure();
  HTTPUpdateResult result = httpUpdate.update(tls, url);

  // Only gets here if the update didn't happen (a successful one restarts the counter)
  if (result == HTTP_UPDATE_NO_UPDATES) updateStatus = "Up to date (checked " + nowText() + ")";
  else updateStatus = "Update to " + tag + " failed: " + httpUpdate.getLastErrorString();
  Serial.println(updateStatus);
  if (currentPage >= 0) drawCurrentPage();
  else showMessage("Update didn't work", updateStatus, "Carrying on with this version");
}
