// Getting stats from YouTube (YouTube Data API v3, with an API key)

const char* YT_API = "https://www.googleapis.com/youtube/v3/";

// Plain HTTPS GET. Returns the HTTP status (or a negative number if the
// connection failed) and fills in body.
int httpGet(const String& url, String& body) {
  body = "";
  if (WiFi.status() != WL_CONNECTED) return -1;
  tls.setInsecure();  // public data only, so certificates aren't checked
  HTTPClient http;
  http.setTimeout(12000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setUserAgent("SubCounter/" FIRMWARE_VERSION);
  if (!http.begin(tls, url)) return -2;
  int code = http.GET();
  if (code > 0) body = http.getString();
  http.end();
  return code;
}

// GET from YouTube and parse the JSON. Sets lastError on failure.
bool youtubeGet(const String& url, JsonDocument& doc) {
  String body;
  int code = httpGet(url, body);
  if (code == 200) {
    if (deserializeJson(doc, body)) {
      lastError = "Bad reply from YouTube";
      return false;
    }
    return true;
  }
  if (body.indexOf("quotaExceeded") >= 0 || body.indexOf("dailyLimitExceeded") >= 0) {
    lastError = "YouTube daily limit used up - resumes later";
    youtubeBlockedUntil = time(nullptr) + 30 * 60;
  } else if (code == 400) {
    lastError = "API key not valid (error 400)";
  } else if (code == 403) {
    lastError = "API not enabled for this key (403)";
  } else if (code < 0) {
    lastError = "Can't reach YouTube - check WiFi";
  } else {
    lastError = "YouTube error " + String(code);
  }
  Serial.println(lastError);
  return false;
}

String urlEncode(const String& s) {
  String out;
  const char* hex = "0123456789ABCDEF";
  for (size_t i = 0; i < s.length(); i++) {
    uint8_t c = s[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      out += (char)c;
    } else {
      out += '%';
      out += hex[c >> 4];
      out += hex[c & 15];
    }
  }
  return out;
}

// "&id=UC..." or "&forHandle=name" for a handle or channel ID
String channelQuery(String who) {
  who.trim();
  if (who.startsWith("https://")) who = who.substring(who.lastIndexOf('/') + 1);
  if (who.startsWith("UC") && who.length() == 24) return "&id=" + who;
  if (who.startsWith("@")) who = who.substring(1);
  return "&forHandle=" + urlEncode(who);
}

// YouTube sends counts as text, e.g. "12345"
uint64_t toNumber(JsonVariant v) {
  return strtoull(v | "0", nullptr, 10);
}

int64_t daysFromCivil(int y, int m, int d) {
  y -= m <= 2;
  int64_t era = (y >= 0 ? y : y - 399) / 400;
  int yoe = (int)(y - era * 400);
  int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + doe - 719468;
}

// "2026-10-21T14:03:12Z" -> seconds since 1970 (UTC)
time_t parseIsoTime(const char* s) {
  int y, mo, d, h, mi, sec;
  if (!s || sscanf(s, "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi, &sec) != 6) return 0;
  return (time_t)(daysFromCivil(y, mo, d) * 86400LL + h * 3600 + mi * 60 + sec);
}

// ---------------------------------------------------------------- channel

bool updateChannel() {
  String url = String(YT_API) + "channels?part=snippet,statistics,contentDetails"
             + "&fields=items(id,snippet/title,statistics,contentDetails/relatedPlaylists/uploads)"
             + channelQuery(cfg.channel) + "&key=" + cfg.apiKey;
  JsonDocument doc;
  if (!youtubeGet(url, doc)) return false;

  JsonVariant item = doc["items"][0];
  if (item.isNull()) {
    lastError = "Channel not found - check the channel name";
    return false;
  }
  JsonVariant stats = item["statistics"];
  channel.id      = item["id"] | "";
  channel.title   = cleanText(item["snippet"]["title"] | "");
  channel.hidden  = stats["hiddenSubscriberCount"] | false;
  channel.subs    = toNumber(stats["subscriberCount"]);
  channel.views   = toNumber(stats["viewCount"]);
  channel.videos  = toNumber(stats["videoCount"]);
  channel.uploads = item["contentDetails"]["relatedPlaylists"]["uploads"] | "";
  if (channel.title.length() == 0) channel.title = cfg.channel;
  channel.loaded = true;
  lastError = "";
  return true;
}

bool updateCompare() {
  String url = String(YT_API) + "channels?part=snippet,statistics"
             + "&fields=items(snippet/title,statistics)"
             + channelQuery(cfg.compare) + "&key=" + cfg.apiKey;
  JsonDocument doc;
  if (!youtubeGet(url, doc)) return false;
  JsonVariant item = doc["items"][0];
  if (item.isNull()) {
    rival.loaded = false;
    return true;  // not found - just leave the page out
  }
  rival.title  = cleanText(item["snippet"]["title"] | "");
  rival.hidden = item["statistics"]["hiddenSubscriberCount"] | false;
  rival.subs   = toNumber(item["statistics"]["subscriberCount"]);
  rival.views  = toNumber(item["statistics"]["viewCount"]);
  if (rival.title.length() == 0) rival.title = cfg.compare;
  rival.loaded = true;
  return true;
}

// ---------------------------------------------------------------- videos

// Fills in a VideoInfo from a videos.list item
void readVideo(JsonVariant item, VideoInfo& v) {
  JsonVariant stats = item["statistics"];
  v.id          = item["id"] | "";
  v.title       = cleanText(item["snippet"]["title"] | "");
  v.published   = parseIsoTime(item["snippet"]["publishedAt"] | "");
  v.views       = toNumber(stats["viewCount"]);
  v.hasLikes    = !stats["likeCount"].isNull();
  v.likes       = toNumber(stats["likeCount"]);
  v.hasComments = !stats["commentCount"].isNull();
  v.comments    = toNumber(stats["commentCount"]);

  String state = item["snippet"]["liveBroadcastContent"] | "none";
  JsonVariant live = item["liveStreamingDetails"];
  v.live = state == "live" ? LIVE_NOW : state == "upcoming" ? LIVE_UPCOMING : LIVE_NONE;
  v.liveViewers = toNumber(live["concurrentViewers"]);
  const char* start = live["actualStartTime"] | (const char*)nullptr;
  if (!start) start = live["scheduledStartTime"] | "";
  v.liveStart = parseIsoTime(start);
  if (v.title.length() == 0) v.title = "(untitled)";
}

bool updateUploads() {
  if (channel.uploads.length() == 0) return false;

  JsonDocument doc;
  String url = String(YT_API) + "playlistItems?part=contentDetails&maxResults=" + String(RECENT_COUNT)
             + "&fields=items/contentDetails/videoId"
             + "&playlistId=" + channel.uploads + "&key=" + cfg.apiKey;
  if (!youtubeGet(url, doc)) return false;

  String ids[RECENT_COUNT];
  int n = 0;
  for (JsonVariant it : doc["items"].as<JsonArray>()) {
    String id = it["contentDetails"]["videoId"] | "";
    if (id.length() && n < RECENT_COUNT) ids[n++] = id;
  }
  if (n == 0) {
    uploads.loaded = true;
    uploads.count = 0;
    return true;
  }

  String idList;
  for (int i = 0; i < n; i++) idList += (i ? "," : "") + ids[i];
  url = String(YT_API) + "videos?part=snippet,statistics,liveStreamingDetails"
      + "&fields=items(id,snippet(title,publishedAt,liveBroadcastContent),statistics,"
      + "liveStreamingDetails(actualStartTime,scheduledStartTime,concurrentViewers))"
      + "&id=" + idList + "&key=" + cfg.apiKey;
  if (!youtubeGet(url, doc)) return false;

  // Keep the uploads list's newest-first order
  int count = 0;
  for (int i = 0; i < n; i++) {
    for (JsonVariant it : doc["items"].as<JsonArray>()) {
      if (ids[i] == (it["id"] | "")) {
        readVideo(it, uploads.v[count++]);
        break;
      }
    }
  }
  uploads.count = count;
  uploads.loaded = true;
  recordViewSample();
  return true;
}

// The newest upload that's out (not a scheduled premiere/stream)
int latestIndex() {
  for (int i = 0; i < uploads.count; i++) {
    if (uploads.v[i].live != LIVE_UPCOMING) return i;
  }
  return -1;
}

int liveIndex() {
  for (int i = 0; i < uploads.count; i++) {
    if (uploads.v[i].live == LIVE_NOW) return i;
  }
  return -1;
}

// The soonest scheduled stream or premiere in the next 2 days
int upcomingIndex() {
  int best = -1;
  time_t now = time(nullptr);
  for (int i = 0; i < uploads.count; i++) {
    const VideoInfo& v = uploads.v[i];
    if (v.live != LIVE_UPCOMING || v.liveStart == 0) continue;
    if (v.liveStart < now - 3600 || v.liveStart > now + 2 * 86400) continue;
    if (best < 0 || v.liveStart < uploads.v[best].liveStart) best = i;
  }
  return best;
}

int publishedCount() {
  int n = 0;
  for (int i = 0; i < uploads.count; i++) {
    if (uploads.v[i].live != LIVE_UPCOMING) n++;
  }
  return n;
}

// Keeps the latest video's views over the last hour, for "views per hour"
void recordViewSample() {
  int li = latestIndex();
  if (li < 0) return;
  const VideoInfo& v = uploads.v[li];
  if (v.id != sampledVideo) {
    sampledVideo = v.id;
    sampleCount = 0;
  }
  if (sampleCount == SAMPLE_COUNT) {
    for (int i = 1; i < SAMPLE_COUNT; i++) samples[i - 1] = samples[i];
    sampleCount--;
  }
  samples[sampleCount++] = { time(nullptr), v.views };
}

// Views gained per hour on the latest video, or -1 if it can't be worked out yet
long viewsPerHour() {
  int li = latestIndex();
  if (li < 0) return -1;
  const VideoInfo& v = uploads.v[li];
  time_t now = time(nullptr);
  if (sampleCount >= 2 && sampledVideo == v.id) {
    // compare with the oldest sample from up to an hour ago
    int oldest = sampleCount - 1;
    for (int i = sampleCount - 2; i >= 0; i--) {
      if (now - samples[i].t <= 3700) oldest = i;
    }
    time_t span = samples[sampleCount - 1].t - samples[oldest].t;
    if (span >= 240) {
      uint64_t gained = samples[sampleCount - 1].views - samples[oldest].views;
      return (long)(gained * 3600 / span);
    }
  }
  // Not enough samples yet: average since it went up (only meaningful for new videos)
  if (v.published > 0 && now - v.published < 48 * 3600 && now - v.published > 600) {
    return (long)(v.views * 3600 / (uint64_t)(now - v.published));
  }
  return -1;
}

bool updateTopVideo() {
  if (channel.id.length() == 0) return false;
  JsonDocument doc;
  String url = String(YT_API) + "search?part=id&order=viewCount&type=video&maxResults=1"
             + "&fields=items/id/videoId&channelId=" + channel.id + "&key=" + cfg.apiKey;
  if (!youtubeGet(url, doc)) return false;
  String id = doc["items"][0]["id"]["videoId"] | "";
  if (id.length() == 0) {
    topLoaded = false;
    return true;
  }
  url = String(YT_API) + "videos?part=snippet,statistics"
      + "&fields=items(id,snippet(title,publishedAt,liveBroadcastContent),statistics)"
      + "&id=" + id + "&key=" + cfg.apiKey;
  if (!youtubeGet(url, doc)) return false;
  JsonVariant item = doc["items"][0];
  if (item.isNull()) return false;
  readVideo(item, topVideo);
  topLoaded = true;
  return true;
}

bool updateComment() {
  int li = latestIndex();
  if (li < 0) return false;
  const VideoInfo& v = uploads.v[li];
  String url = String(YT_API) + "commentThreads?part=snippet&order=time&maxResults=1&textFormat=plainText"
             + "&fields=items/snippet/topLevelComment/snippet(authorDisplayName,textDisplay,publishedAt)"
             + "&videoId=" + v.id + "&key=" + cfg.apiKey;
  String body;
  int code = httpGet(url, body);
  if (code == 403 && body.indexOf("commentsDisabled") >= 0) {
    comment.loaded = false;  // comments are turned off on this video
    return true;
  }
  if (code != 200) {
    Serial.println("Comment fetch failed: " + String(code));
    return false;
  }
  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;
  JsonVariant c = doc["items"][0]["snippet"]["topLevelComment"]["snippet"];
  if (c.isNull()) {
    comment.loaded = false;
    return true;
  }
  comment.author     = cleanText(c["authorDisplayName"] | "");
  comment.text       = cleanText(c["textDisplay"] | "");
  comment.published  = parseIsoTime(c["publishedAt"] | "");
  comment.videoTitle = v.title;
  comment.loaded     = comment.text.length() > 0;
  return true;
}
