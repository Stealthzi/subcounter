// Pet items: food, toys, room decorations, gear to wear and treasures to
// collect. They come from mystery boxes, missions and the shop. Each one has a
// rarity (common, rare, epic, legendary), and some only turn up in their season.
// Every item is drawn from simple shapes, so they look sharp at any size.

enum ItemShape : uint8_t {
  S_FISH, S_COOKIE, S_BERRY, S_CARROT, S_CAKE, S_STEAK, S_PIZZA, S_APPLE, S_CANDY, S_EGG,
  S_BALL, S_YARN, S_MOUSE, S_FEATHER, S_BONE, S_WHEEL, S_TEDDY,
  S_PAPER, S_PAPER_STARS, S_PAPER_DOTS, S_PAPER_WAVES,
  S_WINDOW, S_PAINTING, S_CLOCK, S_RECORD, S_PLAQUE, S_LIGHTS,
  S_RUG, S_RUG_STRIPE, S_BASKET, S_CUSHION, S_DONUT, S_THRONE,
  S_PLANT, S_CACTUS, S_LAVA, S_TANK, S_TREE, S_PUMPKIN, S_TOWER, S_ARCADE,
  S_CAP, S_BEANIE, S_FLOWERS, S_WIZARD, S_VIKING, S_CROWN, S_SANTA, S_BUNNY, S_PARTY, S_HALO,
  S_COLLAR, S_BOWTIE, S_SCARF, S_MEDAL,
  S_CAPE, S_BACKPACK, S_WINGS, S_JETPACK, S_BATWINGS,
  S_PEBBLE, S_COIN, S_SHELL, S_FOSSIL, S_MAP, S_CRYSTAL, S_STAR, S_KEY, S_GLOBE, S_CLOVER, S_ROSE, S_SCALE, S_TROPHY,
  S_BOX
};

// Seasons match season(): 1 Christmas, 2 Halloween, 3 New Year, 4 Valentine's, 5 Easter
const ItemDef ITEMS[] = {
  // Food - feed it from the bag for energy and happiness
  {"Fish snack", IT_FOOD, R_COMMON, S_FISH, 0x7FB3D5, 0xF2F2F2, 0, 15},
  {"Cookie", IT_FOOD, R_COMMON, S_COOKIE, 0xD9A55B, 0x6B4423, 0, 10},
  {"Berries", IT_FOOD, R_COMMON, S_BERRY, 0x7B3FA0, 0x2EB872, 0, 12},
  {"Carrot", IT_FOOD, R_COMMON, S_CARROT, 0xF28C28, 0x2EB872, 0, 10},
  {"Cupcake", IT_FOOD, R_RARE, S_CAKE, 0xFF8FB3, 0xF2D6A0, 0, 40},
  {"Steak", IT_FOOD, R_RARE, S_STEAK, 0xB5452E, 0xF2E6D8, 0, 45},
  {"Pizza slice", IT_FOOD, R_RARE, S_PIZZA, 0xF2C94C, 0xD83A3A, 0, 35},
  {"Golden apple", IT_FOOD, R_EPIC, S_APPLE, 0xFFD700, 0x2EB872, 0, 120},
  {"Dragon fruit", IT_FOOD, R_EPIC, S_APPLE, 0xFF4FA0, 0x9BE564, 0, 0},
  {"Candy cane", IT_FOOD, R_COMMON, S_CANDY, 0xFFFFFF, 0xE03030, 1, 0},
  {"Pumpkin pie", IT_FOOD, R_RARE, S_CAKE, 0xE8862A, 0xF2D6A0, 2, 0},
  {"Chocolate egg", IT_FOOD, R_RARE, S_EGG, 0x6B3A1E, 0xD9A55B, 5, 0},
  // Toys - put one in the room and the pet plays with it
  {"Ball", IT_TOY, R_COMMON, S_BALL, 0xE84A4A, 0xFFFFFF, 0, 25},
  {"Ball of yarn", IT_TOY, R_COMMON, S_YARN, 0xE64680, 0xFFA0BE, 0, 25},
  {"Toy mouse", IT_TOY, R_COMMON, S_MOUSE, 0x9AA0A8, 0xFF8FB3, 0, 30},
  {"Feather wand", IT_TOY, R_RARE, S_FEATHER, 0x36E4FF, 0xFF4FD8, 0, 60},
  {"Squeaky bone", IT_TOY, R_RARE, S_BONE, 0xF2E6D8, 0xC8B8A0, 0, 60},
  {"Hamster wheel", IT_TOY, R_RARE, S_WHEEL, 0x9AA4C7, 0x3DA5FF, 0, 80},
  {"Teddy bear", IT_TOY, R_EPIC, S_TEDDY, 0xB07040, 0x6B4423, 0, 0},
  {"Robot mouse", IT_TOY, R_LEGENDARY, S_MOUSE, 0xC0C8D8, 0x3DFFE0, 0, 0},
  // Wallpaper
  {"Cosy cream", IT_WALLPAPER, R_COMMON, S_PAPER, 0x3A3028, 0x4A3E32, 0, 40},
  {"Mint stripes", IT_WALLPAPER, R_COMMON, S_PAPER, 0x123A30, 0x1C4A3E, 0, 40},
  {"Starry night", IT_WALLPAPER, R_RARE, S_PAPER_STARS, 0x0A0E2A, 0xFFFFFF, 0, 90},
  {"Candy pink", IT_WALLPAPER, R_RARE, S_PAPER_DOTS, 0x4A1E36, 0x7A2E56, 0, 90},
  {"Ocean waves", IT_WALLPAPER, R_EPIC, S_PAPER_WAVES, 0x06304A, 0x1A6A8A, 0, 0},
  {"Space station", IT_WALLPAPER, R_LEGENDARY, S_PAPER_STARS, 0x05031A, 0x9B5CFF, 0, 0},
  {"Snowy", IT_WALLPAPER, R_RARE, S_PAPER_DOTS, 0x1A2A44, 0xFFFFFF, 1, 0},
  {"Spooky", IT_WALLPAPER, R_RARE, S_PAPER_DOTS, 0x1A0A24, 0xE8862A, 2, 0},
  // On the wall
  {"Window", IT_WALL, R_COMMON, S_WINDOW, 0x7FB3D5, 0xF2F2F2, 0, 50},
  {"Fish painting", IT_WALL, R_COMMON, S_PAINTING, 0x3D9BFF, 0xD9A55B, 0, 45},
  {"Wall clock", IT_WALL, R_RARE, S_CLOCK, 0xF2F2F2, 0x2A2A34, 0, 70},
  {"Gold record", IT_WALL, R_EPIC, S_RECORD, 0xFFD700, 0x1A1A1A, 0, 0},
  {"Play button plaque", IT_WALL, R_LEGENDARY, S_PLAQUE, 0xFF0033, 0xC0C0C8, 0, 0},
  {"Fairy lights", IT_WALL, R_RARE, S_LIGHTS, 0xFFD27A, 0xFF4FD8, 0, 80},
  {"Heart garland", IT_WALL, R_RARE, S_LIGHTS, 0xFF5E7E, 0xFFB3C6, 4, 0},
  // Rugs
  {"Round rug", IT_RUG, R_COMMON, S_RUG, 0xB5452E, 0xE8B8A8, 0, 40},
  {"Striped rug", IT_RUG, R_COMMON, S_RUG_STRIPE, 0x3D9BFF, 0xF2F2F2, 0, 40},
  {"Fluffy rug", IT_RUG, R_RARE, S_RUG, 0xE8E4DA, 0xFFFFFF, 0, 80},
  {"Royal carpet", IT_RUG, R_EPIC, S_RUG_STRIPE, 0x9B1B30, 0xFFD700, 0, 0},
  // Beds
  {"Basket", IT_BED, R_COMMON, S_BASKET, 0x8B5A2B, 0xC08A4B, 0, 50},
  {"Cushion", IT_BED, R_COMMON, S_CUSHION, 0x9B5CFF, 0xC77DFF, 0, 50},
  {"Donut bed", IT_BED, R_RARE, S_DONUT, 0xF2994A, 0xFF8FB3, 0, 100},
  {"Cloud bed", IT_BED, R_EPIC, S_CUSHION, 0xE6ECFF, 0xFFFFFF, 0, 0},
  {"Throne", IT_BED, R_LEGENDARY, S_THRONE, 0xFFD700, 0x9B1B30, 0, 0},
  // In the corner
  {"Potted plant", IT_CORNER, R_COMMON, S_PLANT, 0x2EB872, 0xB5652D, 0, 35},
  {"Cactus", IT_CORNER, R_COMMON, S_CACTUS, 0x4FA84F, 0xE8862A, 0, 35},
  {"Lava lamp", IT_CORNER, R_RARE, S_LAVA, 0xFF4FD8, 0x36E4FF, 0, 90},
  {"Fish tank", IT_CORNER, R_EPIC, S_TANK, 0x3D9BFF, 0xF28C28, 0, 0},
  {"Christmas tree", IT_CORNER, R_RARE, S_TREE, 0x1E7A40, 0xFFD700, 1, 0},
  {"Jack-o'-lantern", IT_CORNER, R_RARE, S_PUMPKIN, 0xE8862A, 0xFFD27A, 2, 0},
  {"Cat tower", IT_CORNER, R_RARE, S_TOWER, 0xC08A4B, 0xE8E4DA, 0, 110},
  {"Arcade machine", IT_CORNER, R_LEGENDARY, S_ARCADE, 0x3A0A5E, 0x36E4FF, 0, 0},
  // Gear: hats (more guard), neck (more speed), back (more power)
  {"Baseball cap", IT_HEAD, R_COMMON, S_CAP, 0x3D9BFF, 0xFFFFFF, 0, 60},
  {"Beanie", IT_HEAD, R_COMMON, S_BEANIE, 0x2EB872, 0xF2F2F2, 0, 60},
  {"Flower crown", IT_HEAD, R_RARE, S_FLOWERS, 0xFF8FB3, 0x2EB872, 0, 120},
  {"Wizard hat", IT_HEAD, R_EPIC, S_WIZARD, 0x3A2A9A, 0xFFD700, 0, 0},
  {"Viking helmet", IT_HEAD, R_EPIC, S_VIKING, 0xA0A8B8, 0xF2E6B8, 0, 0},
  {"Crown", IT_HEAD, R_LEGENDARY, S_CROWN, 0xFFD700, 0xE03050, 0, 0},
  {"Santa hat", IT_HEAD, R_RARE, S_SANTA, 0xE03030, 0xFFFFFF, 1, 0},
  {"Witch hat", IT_HEAD, R_RARE, S_WIZARD, 0x1A1024, 0x9B5CFF, 2, 0},
  {"Bunny ears", IT_HEAD, R_RARE, S_BUNNY, 0xFFFFFF, 0xFFA0BE, 5, 0},
  {"Party hat", IT_HEAD, R_RARE, S_PARTY, 0xFFD700, 0x36E4FF, 3, 0},
  {"Halo", IT_HEAD, R_LEGENDARY, S_HALO, 0xFFF1A8, 0xFFD700, 0, 0},
  {"Red collar", IT_NECK, R_COMMON, S_COLLAR, 0xE03030, 0xFFD700, 0, 50},
  {"Bow tie", IT_NECK, R_COMMON, S_BOWTIE, 0x1A1A1A, 0x3A3A3A, 0, 50},
  {"Cosy scarf", IT_NECK, R_RARE, S_SCARF, 0x2EB872, 0xF2F2F2, 0, 100},
  {"Gem collar", IT_NECK, R_EPIC, S_COLLAR, 0x9B5CFF, 0x36E4FF, 0, 0},
  {"Gold medal", IT_NECK, R_LEGENDARY, S_MEDAL, 0xFFD700, 0x3D9BFF, 0, 0},
  {"Heart locket", IT_NECK, R_RARE, S_MEDAL, 0xFF5E7E, 0xFFD700, 4, 0},
  {"Red cape", IT_BACK, R_COMMON, S_CAPE, 0xE03030, 0xB01E1E, 0, 70},
  {"Backpack", IT_BACK, R_COMMON, S_BACKPACK, 0x3D9BFF, 0x2A5A9A, 0, 70},
  {"Hero cape", IT_BACK, R_RARE, S_CAPE, 0x9B5CFF, 0x6A2ABA, 0, 140},
  {"Butterfly wings", IT_BACK, R_EPIC, S_WINGS, 0xFF8FB3, 0x36E4FF, 0, 0},
  {"Jetpack", IT_BACK, R_LEGENDARY, S_JETPACK, 0xC0C8D8, 0xFF8A3D, 0, 0},
  {"Bat wings", IT_BACK, R_RARE, S_BATWINGS, 0x2A1A3A, 0x5A3A7A, 2, 0},
  {"Angel wings", IT_BACK, R_LEGENDARY, S_WINGS, 0xFFFFFF, 0xFFF1A8, 0, 0},
  // Treasures - just for the collection book
  {"Shiny pebble", IT_TREASURE, R_COMMON, S_PEBBLE, 0xA0A8B8, 0xE8ECF2, 0, 0},
  {"Old coin", IT_TREASURE, R_COMMON, S_COIN, 0xC8A050, 0x8A6A30, 0, 0},
  {"Seashell", IT_TREASURE, R_COMMON, S_SHELL, 0xFFC8B0, 0xE89880, 0, 0},
  {"Fossil", IT_TREASURE, R_RARE, S_FOSSIL, 0xC8B8A0, 0x8A7A60, 0, 0},
  {"Treasure map", IT_TREASURE, R_RARE, S_MAP, 0xE8D8A8, 0xB5452E, 0, 0},
  {"Crystal", IT_TREASURE, R_EPIC, S_CRYSTAL, 0x7AF0FF, 0x3D9BFF, 0, 0},
  {"Fallen star", IT_TREASURE, R_LEGENDARY, S_STAR, 0xFFD700, 0xFFF1A8, 0, 0},
  {"Golden key", IT_TREASURE, R_EPIC, S_KEY, 0xFFD700, 0xB08A20, 0, 0},
  {"Snow globe", IT_TREASURE, R_RARE, S_GLOBE, 0x9EC8FF, 0xFFFFFF, 1, 0},
  {"Lucky clover", IT_TREASURE, R_RARE, S_CLOVER, 0x2EB872, 0x9BE564, 0, 0},
  {"Rose", IT_TREASURE, R_RARE, S_ROSE, 0xE03050, 0x2EB872, 4, 0},
  {"Painted egg", IT_TREASURE, R_RARE, S_EGG, 0x36E4FF, 0xFF8FB3, 5, 0},
  {"Firework", IT_TREASURE, R_RARE, S_STAR, 0xFF4FD8, 0x36E4FF, 3, 0},
  {"Dragon scale", IT_TREASURE, R_EPIC, S_SCALE, 0x2EB872, 0xFFD700, 0, 0},
  {"Trophy", IT_TREASURE, R_LEGENDARY, S_TROPHY, 0xFFD700, 0xB08A20, 0, 0},
};
const int ITEM_COUNT = sizeof(ITEMS) / sizeof(ITEMS[0]);
static_assert(ITEM_COUNT <= ITEM_MAX, "too many items for the save block");

const uint16_t BOX_PRICE = 300;

int itemCount() {
  return ITEM_COUNT;
}

const ItemDef& itemDef(int id) {
  return ITEMS[id < ITEM_COUNT ? id : 0];
}

int findItem(const char* name) {
  for (int i = 0; i < ITEM_COUNT; i++)
    if (!strcmp(ITEMS[i].name, name)) return i;
  return -1;
}

uint16_t rarityColour(int r) {
  const uint32_t c[] = {0xC8CCD8, 0x3DA5FF, 0xB57BFF, 0xFFC83D};
  return to565(c[r & 3]);
}

const char* rarityName(int r) {
  const char* n[] = {"Common", "Rare", "Epic", "Legendary"};
  return n[r & 3];
}

const char* itemTypeName(int t) {
  const char* n[] = {"Food", "Toy", "Wallpaper", "Wall", "Rug", "Bed", "Corner", "Hat", "Neck", "Back", "Treasure"};
  return t <= IT_TREASURE ? n[t] : "";
}

// Which room slot or gear slot an item goes in (-1 = neither)
int roomSlotFor(int type) {
  switch (type) {
    case IT_WALLPAPER: return 0;
    case IT_WALL:      return 1;
    case IT_RUG:       return 2;
    case IT_BED:       return 3;
    case IT_CORNER:    return 4;
    case IT_TOY:       return 5;
  }
  return -1;
}

int gearSlotFor(int type) {
  return type == IT_HEAD ? 0 : type == IT_NECK ? 1 : type == IT_BACK ? 2 : -1;
}

bool hasFound(int id) {
  return (rpg.found[id / 8] >> (id % 8)) & 1;
}

int foundCount() {
  int n = 0;
  for (int i = 0; i < ITEM_COUNT; i++)
    if (hasFound(i)) n++;
  return n;
}

void giveItem(int id) {
  if (id < 0 || id >= ITEM_COUNT) return;
  if (rpg.owned[id] < 99) rpg.owned[id]++;
  rpg.found[id / 8] |= 1 << (id % 8);
  if (ITEMS[id].rarity == R_LEGENDARY) awardBadge(B_LEGENDARY);
  if (ITEMS[id].season) awardBadge(B_SEASONAL);
  checkBadges();
  markRpg();
}

void giveStarterItems() {
  int cookie = findItem("Cookie"), fish = findItem("Fish snack"), ball = findItem("Ball"), window = findItem("Window");
  for (int i = 0; i < 3; i++) giveItem(cookie);
  giveItem(fish);
  giveItem(ball);
  giveItem(window);
  rpg.room[5] = ball;
  rpg.room[1] = window;
}

bool inSeasonNow(int id) {
  return ITEMS[id].season == 0 || ITEMS[id].season == season();
}

// Random rarity; luck (0-3) makes the better ones likelier
int rollRarity(int luck, uint32_t& seed) {
  seed = seed * 1103515245UL + 12345UL;
  int r = (seed >> 8) % 1000;
  int legendary = 15 + luck * 15, epic = 90 + luck * 40, rare = 300 + luck * 50;
  if (r < legendary) return R_LEGENDARY;
  if (r < legendary + epic) return R_EPIC;
  if (r < legendary + epic + rare) return R_RARE;
  return R_COMMON;
}

// A random item of a given rarity, preferring ones not found yet.
// onlyType = -1 for any type.
int rollItemOf(int rarity, int onlyType, uint32_t& seed) {
  int pool[ITEM_MAX], n = 0, fresh[ITEM_MAX], nf = 0;
  for (int pass = 0; pass < 2 && n == 0; pass++) {
    for (int i = 0; i < ITEM_COUNT; i++) {
      if (!inSeasonNow(i)) continue;
      if (onlyType >= 0 && ITEMS[i].type != onlyType) continue;
      if (pass == 0 && ITEMS[i].rarity != rarity) continue;
      pool[n++] = i;
      if (!hasFound(i)) fresh[nf++] = i;
    }
  }
  if (n == 0) return 0;
  seed = seed * 1103515245UL + 12345UL;
  bool preferNew = nf > 0 && ((seed >> 9) % 10) < 7;
  seed = seed * 1103515245UL + 12345UL;
  return preferNew ? fresh[(seed >> 8) % nf] : pool[(seed >> 8) % n];
}

int rollItem(int luck, uint32_t& seed) {
  int rarity = rollRarity(luck, seed);
  return rollItemOf(rarity, -1, seed);
}

// Opens a mystery box: returns the item (already in the bag)
int openMysteryBox() {
  if (rpg.boxes == 0) return -1;
  rpg.boxes--;
  uint32_t seed = esp_random();
  int id = rollItem(1, seed);
  giveItem(id);
  rpg.boxesOpened++;
  checkBadges();
  saveRpg();
  return id;
}

// ---------------------------------------------------------------- the shop

// Five things for sale today, the same all day
void shopStock(int out[5]) {
  uint32_t seed = localDay() * 2654435761UL + 99;
  const int wanted[5] = {IT_FOOD, IT_FOOD, -2, -3, -1};  // -2 room, -3 gear, -1 anything
  for (int k = 0; k < 5; k++) {
    out[k] = -1;
    for (int tries = 0; tries < 60 && out[k] < 0; tries++) {
      seed = seed * 1103515245UL + 12345UL;
      int i = (seed >> 8) % ITEM_COUNT;
      const ItemDef& d = ITEMS[i];
      if (!d.price || !inSeasonNow(i)) continue;
      bool ok = wanted[k] == -1 || (wanted[k] >= 0 && d.type == wanted[k]) ||
                (wanted[k] == -2 && roomSlotFor(d.type) >= 0) || (wanted[k] == -3 && gearSlotFor(d.type) >= 0);
      for (int j = 0; j < k; j++)
        if (out[j] == i) ok = false;
      if (ok) out[k] = i;
    }
    if (out[k] < 0) out[k] = k;  // fallback: a cheap food
  }
}

// Returns what to say
String buyItem(int id) {
  if (id == -2) {  // a mystery box
    if (rpg.coins < BOX_PRICE) return "Not enough coins";
    addCoins(-(int)BOX_PRICE);
    rpg.boxes++;
  } else {
    const ItemDef& d = itemDef(id);
    if (rpg.coins < d.price) return "Not enough coins";
    addCoins(-(int)d.price);
    giveItem(id);
  }
  rpg.itemsBought++;
  questEvent(Q_BUY, 1);
  checkBadges();
  saveRpg();
  return id == -2 ? String("A mystery box!") : String("Bought: ") + itemDef(id).name;
}

// ---------------------------------------------------------------- using things from the bag

String useItem(int id) {
  if (id < 0 || id >= ITEM_COUNT || rpg.owned[id] == 0) return "";
  const ItemDef& d = ITEMS[id];
  if (d.type == IT_FOOD) {
    if (!petIsAlive()) return "Not now";
    if (pet.asleep) return "Shh... sleeping";
    rpg.owned[id]--;
    pet.hunger = min(4, pet.hunger + (d.rarity >= R_RARE ? 2 : 1));
    if (pet.happy < 4) pet.happy++;
    const int energyBack[] = {20, 40, 70, 100};
    rpg.energy = min(100, (int)rpg.energy + energyBack[d.rarity]);
    careXp(10);
    questEvent(Q_FEED, 1);
    savePet();
    markRpg();
    return String("Yum! ") + d.name;
  }
  int g = gearSlotFor(d.type);
  if (g >= 0) {
    bool off = rpg.gear[g] == id;
    rpg.gear[g] = off ? NO_ITEM : id;
    checkBadges();
    markRpg();
    return off ? String("Took off the ") + d.name : String("Wearing the ") + d.name;
  }
  int r = roomSlotFor(d.type);
  if (r >= 0) {
    bool off = rpg.room[r] == id;
    rpg.room[r] = off ? NO_ITEM : id;
    checkBadges();
    markRpg();
    return off ? String("Put away the ") + d.name : String("Placed the ") + d.name;
  }
  return String(d.name) + " - a " + rarityName(d.rarity) + " treasure";
}

bool itemInUse(int id) {
  for (int i = 0; i < GEAR_SLOTS; i++)
    if (rpg.gear[i] == id) return true;
  for (int i = 0; i < ROOM_SLOTS; i++)
    if (rpg.room[i] == id) return true;
  return false;
}

// ---------------------------------------------------------------- drawing items

// Draws a shape filling a box of size sz centred on (cx, cy). Shapes are laid
// out on a 16 x 16 grid, so P(n) is n sixteenths of the box from its centre.
void drawShape(Gfx& g, int shape, int cx, int cy, int sz, uint32_t a, uint32_t b) {
  float u = sz / 16.0f;
  auto X = [&](float v) { return (int)lroundf(cx + v * u); };
  auto Y = [&](float v) { return (int)lroundf(cy + v * u); };
  auto L = [&](float v) { return max(1, (int)lroundf(v * u)); };
  uint16_t A = to565(a), B = to565(b);
  uint16_t dark = mix(a, 0x000000, 0.45f), white = C_WHITE, black = RGB565(16, 16, 24);
  switch (shape) {
    case S_FISH:
      g.fillEllipse(X(-1), Y(0), L(5), L(3), A);
      g.fillTriangle(X(3), Y(0), X(7), Y(-4), X(7), Y(4), A);
      g.fillCircle(X(-4), Y(-1), L(0.8f), black);
      break;
    case S_COOKIE:
      g.fillCircle(X(0), Y(0), L(6), A);
      g.fillCircle(X(-2), Y(-2), L(1), B);
      g.fillCircle(X(2), Y(1), L(1), B);
      g.fillCircle(X(-1), Y(3), L(1), B);
      break;
    case S_BERRY:
      g.fillCircle(X(-2), Y(1), L(3), A);
      g.fillCircle(X(2), Y(2), L(3), A);
      g.fillCircle(X(0), Y(-2), L(3), A);
      g.fillTriangle(X(-1), Y(-5), X(3), Y(-7), X(1), Y(-4), B);
      break;
    case S_CARROT:
      g.fillTriangle(X(-3), Y(-3), X(3), Y(-3), X(0), Y(7), A);
      g.fillTriangle(X(-2), Y(-3), X(0), Y(-7), X(1), Y(-3), B);
      g.fillTriangle(X(0), Y(-3), X(3), Y(-6), X(2), Y(-3), B);
      break;
    case S_CAKE:
      g.fillRect(X(-5), Y(0), L(10), L(6), B);
      g.fillCircle(X(0), Y(-1), L(5), A);
      g.fillCircle(X(0), Y(-6), L(1.5f), C_RED);
      break;
    case S_STEAK:
      g.fillEllipse(X(0), Y(1), L(7), L(5), A);
      g.fillEllipse(X(-2), Y(0), L(3), L(2), B);
      break;
    case S_PIZZA:
      g.fillTriangle(X(-6), Y(-5), X(6), Y(-5), X(0), Y(7), A);
      g.fillRect(X(-6), Y(-6), L(12), L(2), mix(a, 0x8B5A2B, 0.6f));
      g.fillCircle(X(-2), Y(-2), L(1.3f), B);
      g.fillCircle(X(2), Y(-1), L(1.3f), B);
      g.fillCircle(X(0), Y(2), L(1.2f), B);
      break;
    case S_APPLE:
      g.fillCircle(X(-2), Y(1), L(5), A);
      g.fillCircle(X(2), Y(1), L(5), A);
      g.fillRect(X(0), Y(-7), L(1), L(4), mix(0x6B4423, 0, 0));
      g.fillEllipse(X(3), Y(-6), L(2.5f), L(1.2f), B);
      break;
    case S_CANDY:
      g.drawWideLine(X(1), Y(7), X(1), Y(-3), u * 1.5f, A);
      g.fillArc(X(-2), Y(-3), L(4), L(2), 180, 360, A);
      for (int i = 0; i < 3; i++) g.drawWideLine(X(0), Y(5 - i * 3), X(2), Y(3 - i * 3), u * 0.7f, B);
      break;
    case S_EGG:
      g.fillEllipse(X(0), Y(1), L(5), L(7), A);
      g.fillRect(X(-5), Y(0), L(10), L(1.5f), B);
      g.fillCircle(X(-2), Y(-3), L(1), B);
      g.fillCircle(X(2), Y(4), L(1), B);
      break;
    case S_BALL:
      g.fillCircle(X(0), Y(0), L(6), A);
      g.fillRect(X(-6), Y(-1), L(12), L(2), B);
      break;
    case S_YARN:
      g.fillCircle(X(0), Y(0), L(6), A);
      g.drawWideLine(X(-4), Y(-3), X(4), Y(3), u * 0.6f, B);
      g.drawWideLine(X(-4), Y(2), X(3), Y(-4), u * 0.6f, B);
      g.drawWideLine(X(4), Y(5), X(7), Y(7), u * 0.6f, A);
      break;
    case S_MOUSE:
      g.fillEllipse(X(0), Y(2), L(6), L(4), A);
      g.fillCircle(X(-3), Y(-2), L(2), A);
      g.fillCircle(X(-3), Y(-2), L(1), B);
      g.fillCircle(X(-5), Y(1), L(0.8f), black);
      g.drawWideLine(X(6), Y(2), X(8), Y(-2), u * 0.5f, B);
      break;
    case S_FEATHER:
      g.drawWideLine(X(-6), Y(7), X(4), Y(-5), u * 0.5f, C_GREY);
      g.fillEllipse(X(3), Y(-4), L(3), L(4), A);
      g.fillEllipse(X(4), Y(-5), L(1.5f), L(2.5f), B);
      break;
    case S_BONE:
      g.fillRect(X(-4), Y(-1), L(8), L(3), A);
      g.fillCircle(X(-5), Y(-2), L(2), A);
      g.fillCircle(X(-5), Y(2), L(2), A);
      g.fillCircle(X(5), Y(-2), L(2), A);
      g.fillCircle(X(5), Y(2), L(2), A);
      break;
    case S_WHEEL:
      g.fillCircle(X(0), Y(0), L(7), A);
      g.fillCircle(X(0), Y(0), L(5.5f), mix(a, 0x000000, 0.7f));
      for (int i = 0; i < 4; i++) {
        float an = i * 0.785f;
        g.drawWideLine(X(cosf(an) * 5), Y(sinf(an) * 5), X(-cosf(an) * 5), Y(-sinf(an) * 5), u * 0.5f, B);
      }
      break;
    case S_TEDDY:
      g.fillCircle(X(-4), Y(-5), L(2), A);
      g.fillCircle(X(4), Y(-5), L(2), A);
      g.fillCircle(X(0), Y(-2), L(4), A);
      g.fillEllipse(X(0), Y(4), L(5), L(4), A);
      g.fillCircle(X(0), Y(-1), L(1.5f), B);
      g.fillCircle(X(-1.5f), Y(-3), L(0.6f), black);
      g.fillCircle(X(1.5f), Y(-3), L(0.6f), black);
      break;
    case S_PAPER:
    case S_PAPER_STARS:
    case S_PAPER_DOTS:
    case S_PAPER_WAVES:
      g.fillRect(X(-7), Y(-7), L(14), L(14), A);
      for (int i = 0; i < 4; i++) {
        if (shape == S_PAPER) g.fillRect(X(-7 + i * 4), Y(-7), L(2), L(14), B);
        if (shape == S_PAPER_DOTS || shape == S_PAPER_STARS)
          for (int j = 0; j < 4; j++) g.fillCircle(X(-5 + i * 3.5f + (j % 2) * 1.5f), Y(-5 + j * 3.5f), L(shape == S_PAPER_STARS ? 0.5f : 0.9f), B);
        if (shape == S_PAPER_WAVES) g.fillArc(X(-4 + (i % 2) * 8), Y(-4 + (i / 2) * 7), L(3), L(2), 180, 360, B);
      }
      break;
    case S_WINDOW:
      g.fillRect(X(-7), Y(-7), L(14), L(14), B);
      g.fillRect(X(-6), Y(-6), L(5.5f), L(5.5f), A);
      g.fillRect(X(0.5f), Y(-6), L(5.5f), L(5.5f), A);
      g.fillRect(X(-6), Y(0.5f), L(5.5f), L(5.5f), A);
      g.fillRect(X(0.5f), Y(0.5f), L(5.5f), L(5.5f), A);
      g.fillCircle(X(3), Y(-3), L(1.5f), C_YELLOW);
      break;
    case S_PAINTING:
      g.fillRect(X(-7), Y(-6), L(14), L(12), mix(0xC08A4B, 0, 0));
      g.fillRect(X(-6), Y(-5), L(12), L(10), A);
      g.fillEllipse(X(0), Y(0), L(3), L(2), B);
      g.fillTriangle(X(3), Y(0), X(5), Y(-2), X(5), Y(2), B);
      break;
    case S_CLOCK:
      g.fillCircle(X(0), Y(0), L(7), B);
      g.fillCircle(X(0), Y(0), L(6), A);
      g.drawWideLine(X(0), Y(0), X(0), Y(-4), u * 0.6f, B);
      g.drawWideLine(X(0), Y(0), X(3), Y(1), u * 0.6f, B);
      break;
    case S_RECORD:
      g.fillCircle(X(0), Y(0), L(7), B);
      g.fillCircle(X(0), Y(0), L(6), A);
      g.fillCircle(X(0), Y(0), L(2), C_RED);
      g.fillCircle(X(0), Y(0), L(0.7f), B);
      break;
    case S_PLAQUE:
      g.fillRect(X(-7), Y(-6), L(14), L(12), mix(0x1A1A1A, 0, 0));
      g.fillRect(X(-6), Y(-5), L(12), L(10), B);
      g.fillSmoothRoundRect(X(-3.5f), Y(-2.5f), L(7), L(5), L(1.5f), A);
      g.fillTriangle(X(-1), Y(-1.5f), X(-1), Y(1.5f), X(1.5f), Y(0), C_WHITE);
      break;
    case S_LIGHTS:
      g.drawWideLine(X(-7), Y(-4), X(0), Y(-1), u * 0.4f, C_DIM);
      g.drawWideLine(X(0), Y(-1), X(7), Y(-4), u * 0.4f, C_DIM);
      for (int i = 0; i < 5; i++) g.fillCircle(X(-6 + i * 3), Y(-3 + (i == 2 ? 2 : i % 2 ? 1 : 0)), L(1.1f), i % 2 ? B : A);
      break;
    case S_RUG:
      g.fillEllipse(X(0), Y(0), L(8), L(4), A);
      g.fillEllipse(X(0), Y(0), L(5), L(2.3f), B);
      break;
    case S_RUG_STRIPE:
      g.fillRect(X(-8), Y(-3), L(16), L(6), A);
      for (int i = 0; i < 4; i++) g.fillRect(X(-6 + i * 4), Y(-3), L(1.5f), L(6), B);
      break;
    case S_BASKET:
      g.fillSmoothRoundRect(X(-7), Y(-1), L(14), L(6), L(2), A);
      for (int i = 0; i < 5; i++) g.fillRect(X(-6 + i * 3), Y(0), L(1), L(4), B);
      g.fillEllipse(X(0), Y(-1), L(7), L(1.5f), B);
      break;
    case S_CUSHION:
      g.fillSmoothRoundRect(X(-7), Y(-2), L(14), L(7), L(3), A);
      g.fillEllipse(X(0), Y(1), L(5), L(2), B);
      break;
    case S_DONUT:
      g.fillEllipse(X(0), Y(1), L(8), L(5), A);
      g.fillEllipse(X(0), Y(0), L(4), L(2), B);
      break;
    case S_THRONE:
      g.fillRect(X(-5), Y(-7), L(10), L(9), A);
      g.fillRect(X(-6), Y(1), L(12), L(4), A);
      g.fillRect(X(-4), Y(-5), L(8), L(6), B);
      g.fillRect(X(-6), Y(5), L(2), L(2), A);
      g.fillRect(X(4), Y(5), L(2), L(2), A);
      g.fillCircle(X(0), Y(-7), L(1.3f), C_RED);
      break;
    case S_PLANT:
      g.fillRect(X(-3), Y(2), L(6), L(5), B);
      g.fillEllipse(X(-3), Y(-2), L(2), L(4), A);
      g.fillEllipse(X(3), Y(-2), L(2), L(4), A);
      g.fillEllipse(X(0), Y(-4), L(2), L(4), A);
      break;
    case S_CACTUS:
      g.fillRect(X(-3), Y(3), L(6), L(4), B);
      g.fillSmoothRoundRect(X(-1.5f), Y(-6), L(3), L(10), L(1.5f), A);
      g.fillSmoothRoundRect(X(-5), Y(-3), L(2.5f), L(5), L(1.2f), A);
      g.fillSmoothRoundRect(X(2.5f), Y(-4), L(2.5f), L(4), L(1.2f), A);
      break;
    case S_LAVA:
      g.fillTriangle(X(-3), Y(5), X(3), Y(5), X(0), Y(-7), mix(b, 0x000000, 0.4f));
      g.fillEllipse(X(0), Y(-3), L(2), L(3), A);
      g.fillCircle(X(0), Y(2), L(1.5f), A);
      g.fillRect(X(-4), Y(5), L(8), L(2), C_GREY);
      break;
    case S_TANK:
      g.fillRect(X(-7), Y(-5), L(14), L(10), A);
      g.fillRect(X(-7), Y(5), L(14), L(2), C_GREY);
      g.fillEllipse(X(-1), Y(0), L(2.5f), L(1.5f), B);
      g.fillTriangle(X(1), Y(0), X(3), Y(-1.5f), X(3), Y(1.5f), B);
      break;
    case S_TREE:
      g.fillTriangle(X(-6), Y(4), X(6), Y(4), X(0), Y(-7), A);
      g.fillRect(X(-1), Y(4), L(2), L(3), mix(0x6B4423, 0, 0));
      for (int i = 0; i < 4; i++) g.fillCircle(X(-3 + i * 2), Y(1 - (i % 2) * 3), L(0.8f), i % 2 ? C_RED : B);
      g.fillCircle(X(0), Y(-7), L(1.2f), B);
      break;
    case S_PUMPKIN:
      g.fillEllipse(X(0), Y(1), L(7), L(5), A);
      g.fillRect(X(-0.5f), Y(-6), L(1.5f), L(3), mix(0x2EB872, 0, 0.3f));
      g.fillTriangle(X(-4), Y(-1), X(-2), Y(-1), X(-3), Y(-3), B);
      g.fillTriangle(X(2), Y(-1), X(4), Y(-1), X(3), Y(-3), B);
      g.fillRect(X(-3), Y(2), L(6), L(1.5f), B);
      break;
    case S_TOWER:
      g.fillRect(X(-1), Y(-4), L(2), L(11), B);
      g.fillRect(X(-6), Y(5), L(12), L(2), A);
      g.fillRect(X(-5), Y(-1), L(8), L(2), A);
      g.fillRect(X(-3), Y(-6), L(8), L(2), A);
      break;
    case S_ARCADE:
      g.fillRect(X(-5), Y(-7), L(10), L(14), A);
      g.fillRect(X(-4), Y(-5), L(8), L(5), B);
      g.fillCircle(X(-2), Y(3), L(1), C_RED);
      g.fillCircle(X(2), Y(3), L(1), C_YELLOW);
      break;
    case S_CAP:
      g.fillArc(X(0), Y(2), L(6), 0, 180, 360, A);
      g.fillRect(X(0), Y(1), L(8), L(2), B);
      break;
    case S_BEANIE:
      g.fillArc(X(0), Y(3), L(6), 0, 180, 360, A);
      g.fillRect(X(-6), Y(1), L(12), L(3), B);
      g.fillCircle(X(0), Y(-4), L(1.5f), B);
      break;
    case S_FLOWERS:
      g.fillRect(X(-7), Y(1), L(14), L(1.5f), B);
      for (int i = 0; i < 4; i++) {
        g.fillCircle(X(-5 + i * 3.4f), Y(0), L(1.8f), i % 2 ? A : C_WHITE);
        g.fillCircle(X(-5 + i * 3.4f), Y(0), L(0.7f), C_YELLOW);
      }
      break;
    case S_WIZARD:
      g.fillTriangle(X(-4), Y(3), X(4), Y(3), X(1), Y(-8), A);
      g.fillEllipse(X(0), Y(3), L(7), L(1.5f), A);
      g.fillCircle(X(0), Y(-2), L(1), B);
      g.fillCircle(X(2), Y(0), L(0.6f), B);
      break;
    case S_VIKING:
      g.fillArc(X(0), Y(3), L(5), 0, 180, 360, A);
      g.fillTriangle(X(-5), Y(1), X(-8), Y(-5), X(-4), Y(-1), B);
      g.fillTriangle(X(5), Y(1), X(8), Y(-5), X(4), Y(-1), B);
      g.fillRect(X(-6), Y(1), L(12), L(2), mix(a, 0x000000, 0.25f));
      break;
    case S_CROWN:
      g.fillRect(X(-6), Y(0), L(12), L(4), A);
      g.fillTriangle(X(-6), Y(0), X(-6), Y(-5), X(-2), Y(0), A);
      g.fillTriangle(X(-3), Y(0), X(0), Y(-6), X(3), Y(0), A);
      g.fillTriangle(X(2), Y(0), X(6), Y(-5), X(6), Y(0), A);
      g.fillCircle(X(0), Y(2), L(1), B);
      break;
    case S_SANTA:
      g.fillTriangle(X(-5), Y(2), X(5), Y(2), X(4), Y(-6), A);
      g.fillRect(X(-6), Y(1), L(12), L(2.5f), B);
      g.fillCircle(X(4), Y(-6), L(1.5f), B);
      break;
    case S_BUNNY:
      g.fillEllipse(X(-3), Y(-3), L(2), L(5), A);
      g.fillEllipse(X(3), Y(-3), L(2), L(5), A);
      g.fillEllipse(X(-3), Y(-3), L(1), L(3.5f), B);
      g.fillEllipse(X(3), Y(-3), L(1), L(3.5f), B);
      break;
    case S_PARTY:
      g.fillTriangle(X(-4), Y(3), X(4), Y(3), X(0), Y(-7), A);
      g.fillCircle(X(0), Y(-7), L(1.3f), B);
      g.fillCircle(X(-1), Y(0), L(0.7f), B);
      g.fillCircle(X(1), Y(-3), L(0.7f), B);
      break;
    case S_HALO:
      g.fillArc(X(0), Y(0), L(6), L(4.5f), 0, 360, A);
      break;
    case S_COLLAR:
      g.fillSmoothRoundRect(X(-7), Y(-1.5f), L(14), L(3), L(1.5f), A);
      g.fillCircle(X(0), Y(3), L(2), B);
      break;
    case S_BOWTIE:
      g.fillTriangle(X(0), Y(0), X(-6), Y(-3), X(-6), Y(3), A);
      g.fillTriangle(X(0), Y(0), X(6), Y(-3), X(6), Y(3), A);
      g.fillCircle(X(0), Y(0), L(1.3f), B);
      break;
    case S_SCARF:
      g.fillSmoothRoundRect(X(-7), Y(-2), L(14), L(4), L(2), A);
      g.fillRect(X(2), Y(0), L(3), L(7), A);
      for (int i = 0; i < 3; i++) g.fillRect(X(-5 + i * 4), Y(-2), L(1.5f), L(4), B);
      break;
    case S_MEDAL:
      g.fillTriangle(X(-4), Y(-7), X(0), Y(0), X(-1), Y(-7), B);
      g.fillTriangle(X(4), Y(-7), X(0), Y(0), X(1), Y(-7), B);
      g.fillCircle(X(0), Y(3), L(3.5f), A);
      g.fillCircle(X(0), Y(3), L(2), mix(a, 0xFFFFFF, 0.4f));
      break;
    case S_CAPE:
      g.fillTriangle(X(-4), Y(-6), X(4), Y(-6), X(-7), Y(7), A);
      g.fillTriangle(X(4), Y(-6), X(7), Y(7), X(-7), Y(7), A);
      g.fillRect(X(-4), Y(-7), L(8), L(2), B);
      break;
    case S_BACKPACK:
      g.fillSmoothRoundRect(X(-5), Y(-6), L(10), L(13), L(3), A);
      g.fillSmoothRoundRect(X(-3), Y(0), L(6), L(5), L(1.5f), B);
      break;
    case S_WINGS:
      g.fillEllipse(X(-4), Y(-2), L(4), L(5), A);
      g.fillEllipse(X(4), Y(-2), L(4), L(5), A);
      g.fillEllipse(X(-3), Y(4), L(3), L(3), B);
      g.fillEllipse(X(3), Y(4), L(3), L(3), B);
      break;
    case S_JETPACK:
      g.fillSmoothRoundRect(X(-6), Y(-6), L(5), L(11), L(2), A);
      g.fillSmoothRoundRect(X(1), Y(-6), L(5), L(11), L(2), A);
      g.fillTriangle(X(-5), Y(5), X(-2), Y(5), X(-3.5f), Y(8), B);
      g.fillTriangle(X(2), Y(5), X(5), Y(5), X(3.5f), Y(8), B);
      break;
    case S_BATWINGS:
      g.fillTriangle(X(0), Y(-2), X(-8), Y(-6), X(-6), Y(4), A);
      g.fillTriangle(X(0), Y(-2), X(8), Y(-6), X(6), Y(4), A);
      g.fillTriangle(X(-6), Y(4), X(-3), Y(0), X(-4), Y(5), B);
      g.fillTriangle(X(6), Y(4), X(3), Y(0), X(4), Y(5), B);
      break;
    case S_PEBBLE:
      g.fillEllipse(X(0), Y(1), L(6), L(4), A);
      g.fillEllipse(X(-2), Y(-1), L(2), L(1), B);
      break;
    case S_COIN:
      g.fillCircle(X(0), Y(0), L(6), B);
      g.fillCircle(X(0), Y(0), L(5), A);
      g.fillRect(X(-0.5f), Y(-3), L(1.5f), L(6), B);
      break;
    case S_SHELL:
      g.fillArc(X(0), Y(3), L(7), 0, 180, 360, A);
      for (int i = 0; i < 5; i++) g.drawWideLine(X(0), Y(3), X(-6 + i * 3), Y(-3), u * 0.4f, B);
      break;
    case S_FOSSIL:
      g.fillCircle(X(0), Y(0), L(6), A);
      g.fillArc(X(0), Y(0), L(4), L(3), 0, 300, B);
      g.fillArc(X(0), Y(0), L(2), L(1), 0, 300, B);
      break;
    case S_MAP:
      g.fillRect(X(-7), Y(-5), L(14), L(10), A);
      g.drawWideLine(X(-5), Y(3), X(-1), Y(-1), u * 0.4f, B);
      g.drawWideLine(X(-1), Y(-1), X(2), Y(1), u * 0.4f, B);
      g.drawWideLine(X(3), Y(-3), X(5), Y(-1), u * 0.6f, B);
      g.drawWideLine(X(5), Y(-3), X(3), Y(-1), u * 0.6f, B);
      break;
    case S_CRYSTAL:
      g.fillTriangle(X(0), Y(-7), X(-4), Y(-1), X(4), Y(-1), A);
      g.fillTriangle(X(-4), Y(-1), X(4), Y(-1), X(0), Y(7), B);
      g.fillTriangle(X(0), Y(-7), X(-1), Y(-1), X(2), Y(-1), C_WHITE);
      break;
    case S_STAR:
      g.fillTriangle(X(0), Y(-7), X(-2), Y(-1), X(2), Y(-1), A);
      g.fillTriangle(X(-7), Y(-2), X(7), Y(-2), X(0), Y(2), A);
      g.fillTriangle(X(0), Y(1), X(-5), Y(7), X(-2), Y(-1), A);
      g.fillTriangle(X(0), Y(1), X(5), Y(7), X(2), Y(-1), A);
      g.fillCircle(X(0), Y(0), L(1.2f), B);
      break;
    case S_KEY:
      g.fillCircle(X(-4), Y(0), L(3.5f), A);
      g.fillCircle(X(-4), Y(0), L(1.5f), mix(a, 0x000000, 0.6f));
      g.fillRect(X(-1), Y(-1), L(8), L(2), A);
      g.fillRect(X(4), Y(1), L(1.5f), L(2.5f), A);
      g.fillRect(X(6), Y(1), L(1.5f), L(2), A);
      break;
    case S_GLOBE:
      g.fillCircle(X(0), Y(-1), L(6), A);
      g.fillRect(X(-5), Y(4), L(10), L(3), mix(0x8B5A2B, 0, 0));
      g.fillCircle(X(-2), Y(-3), L(0.8f), B);
      g.fillCircle(X(2), Y(-1), L(0.8f), B);
      g.fillCircle(X(0), Y(1), L(0.8f), B);
      break;
    case S_CLOVER:
      g.fillCircle(X(-2.5f), Y(-2.5f), L(2.8f), A);
      g.fillCircle(X(2.5f), Y(-2.5f), L(2.8f), A);
      g.fillCircle(X(-2.5f), Y(2), L(2.8f), A);
      g.fillCircle(X(2.5f), Y(2), L(2.8f), A);
      g.drawWideLine(X(0), Y(1), X(3), Y(7), u * 0.6f, B);
      break;
    case S_ROSE:
      g.drawWideLine(X(0), Y(0), X(0), Y(7), u * 0.6f, B);
      g.fillEllipse(X(2), Y(4), L(2), L(1), B);
      g.fillCircle(X(0), Y(-3), L(4), A);
      g.fillArc(X(0), Y(-3), L(2.5f), L(1.5f), 30, 330, mix(a, 0x000000, 0.35f));
      break;
    case S_SCALE:
      g.fillCircle(X(0), Y(-1), L(6), A);
      g.fillTriangle(X(-6), Y(-1), X(6), Y(-1), X(0), Y(7), A);
      g.fillCircle(X(-1), Y(-2), L(2), mix(a, 0xFFFFFF, 0.35f));
      g.drawWideLine(X(0), Y(-5), X(0), Y(5), u * 0.4f, B);
      break;
    case S_TROPHY:
      g.fillArc(X(0), Y(-3), L(5), 0, 0, 180, A);
      g.fillRect(X(-5), Y(-6), L(10), L(3), A);
      g.fillRect(X(-1), Y(1), L(2), L(3), A);
      g.fillRect(X(-4), Y(4), L(8), L(3), B);
      break;
    case S_BOX:
      g.fillRect(X(-6), Y(-3), L(12), L(9), A);
      g.fillRect(X(-7), Y(-6), L(14), L(4), mix(a, 0xFFFFFF, 0.2f));
      g.fillRect(X(-1), Y(-6), L(2), L(12), B);
      g.fillCircle(X(-2), Y(-7), L(2), B);
      g.fillCircle(X(2), Y(-7), L(2), B);
      break;
  }
  (void)dark;
  (void)white;
}

void drawItemIcon(Gfx& g, int id, int cx, int cy, int sz) {
  const ItemDef& d = itemDef(id);
  drawShape(g, d.shape, cx, cy, sz, d.c1, d.c2);
}

// A card-like tile with an item, its rarity colour as the border
void drawItemTile(Gfx& g, int id, int x, int y, int w, int h, bool known, int count) {
  uint16_t edge = known ? rarityColour(itemDef(id).rarity) : C_LINE;
  g.fillSmoothRoundRect(x, y, w, h, 6, edge);
  g.fillSmoothRoundRect(x + 1, y + 1, w - 2, h - 2, 5, C_CARD);
  int sz = min(w, h) - 10;
  if (known) {
    drawItemIcon(g, id, x + w / 2, y + h / 2, sz);
  } else {
    g.setFont(F_BOLD);
    g.setTextSize(1);
    g.setTextDatum(D_MC);
    g.setTextColor(C_DIM);
    g.drawString("?", x + w / 2, y + h / 2);
  }
  if (count > 1) {
    g.setFont(F_SMALL);
    g.setTextSize(1);
    g.setTextDatum(D_BR);
    g.setTextColor(C_WHITE);
    g.drawString(String(count), x + w - 3, y + h - 1);
  }
}

// ---------------------------------------------------------------- the pet's room and gear

// The wallpaper behind the pet. Returns true if there is some.
bool drawRoomWallpaper(int floorY, bool dark) {
  bool papered = false;
  int paper = rpg.room[0];
  if (paper != NO_ITEM && !dark) {
    const ItemDef& d = itemDef(paper);
    uint16_t base = to565(d.c1), pat = mix(d.c1, d.c2, d.shape == S_PAPER ? 1.0f : 0.8f);
    canvas.fillRect(0, 0, W, floorY, base);
    for (int x = 0; x < W; x += 20) {
      if (d.shape == S_PAPER) canvas.fillRect(x, 0, 8, floorY, pat);
      for (int y = 6; y < floorY; y += 20) {
        if (d.shape == S_PAPER_DOTS) canvas.fillCircle(x + ((y / 20) % 2) * 10 + 5, y, 2, pat);
        if (d.shape == S_PAPER_STARS && ((x * 7 + y * 13) % 3) == 0) canvas.fillCircle(x + ((y / 20) % 2) * 10 + 4, y, 1, pat);
        if (d.shape == S_PAPER_WAVES) canvas.fillArc(x + 10, y + 6, 8, 6, 180, 360, pat);
      }
    }
    papered = true;
  }
  return papered;
}

// The wall item, rug, bed and corner piece (after the floor is drawn)
void drawRoomItems(int floorY, bool dark) {
  if (dark) return;
  if (rpg.room[1] != NO_ITEM) drawItemIcon(canvas, rpg.room[1], 64, 62, 44);
  if (rpg.room[2] != NO_ITEM) drawItemIcon(canvas, rpg.room[2], W / 2, floorY + 2, 120);
  if (rpg.room[3] != NO_ITEM) drawItemIcon(canvas, rpg.room[3], 46, floorY - 12, 52);
  if (rpg.room[4] != NO_ITEM) drawItemIcon(canvas, rpg.room[4], W - 34, floorY - 24, 52);
}

// The toy, on the floor in front
void drawRoomToy(int floorY, int x) {
  if (rpg.room[5] == NO_ITEM) return;
  drawItemIcon(canvas, rpg.room[5], x, floorY - 8, 20);
}

// Gear worn by a pet drawn at (px, py) with pixel size s. The back piece goes
// behind the pet, so call with behind = true before drawing it.
void drawGear(int px, int py, int s, bool behind) {
  int cx = px + 8 * s;
  if (behind) {
    if (rpg.gear[2] != NO_ITEM) drawItemIcon(canvas, rpg.gear[2], cx, py + 10 * s, 15 * s);
    return;
  }
  if (rpg.gear[1] != NO_ITEM) drawItemIcon(canvas, rpg.gear[1], cx, py + 10 * s, 7 * s);
  if (rpg.gear[0] != NO_ITEM) drawItemIcon(canvas, rpg.gear[0], cx, py + 1 * s - 3 * s, 8 * s);
}
