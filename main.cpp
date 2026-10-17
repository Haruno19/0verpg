/*
 * Copyright 10/13/2026 https://github.com/su8/0verpg
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <random>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif /* _WIN32 */

#define RESETCOLOUR "\033[0;0m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"
#define RED "\033[1;31m"
#define MAGENTA "\033[1;35m"
#define BLUE "\033[1;34m"

// Armour structure
struct Armour {
  std::string name;
  int defense;
};

// Weapon structure
struct Weapon {
  std::string name;
  int attack;
  int cost;
};

// Potion structure
struct Potion {
  std::string name;
  int healAmount;
  int cost;
};

// Enemy structure
struct Enemy {
  std::string name;
  int attack;
  int health;
  int xpReward;
};

// Hero structure
struct Hero {
  std::string name;
  int health;
  int baseDefense;
  int baseAttack;
  Armour equippedArmour;
  Weapon equippedWeapon;
  std::vector<Armour> inventoryArmours;
  std::vector<Weapon> inventoryWeapons;
  std::vector<int> allPotions;
  int potions;
  int gold;
  int x, y;
  int xp;
  int level;
};

// Used to index the last potion and pop it back from the "hero.allPotions" vector, which stores all the different potion types
size_t potionIdx = 0U;
size_t lastPotionIdxFromList = 0U;
int keys[7];

// Equip armour from inventory
void equipArmour(Hero &hero, int index);
// Equip weapon
void equipWeapon(Hero &hero, std::vector<Weapon> &weapons, int index);
// Battle simulation
bool battle(Hero &hero, Enemy enemy);
// Loot room
void lootRoom(Hero &hero, std::vector<Armour> &armours, std::vector<Weapon> &weapons);
// Use potion
void usePotion(Hero &hero);
// Trap room
bool trapRoom(Hero &hero);
// Treasure chest room
void chestRoom(Hero &hero, std::vector<Armour> &armours);
// Explore room
bool exploreRoom(Hero &hero, std::vector<Enemy> &enemies, std::vector<Armour> &armours, std::vector<Weapon> &weapons,char &cell);
// Shop system
void shop(Hero &hero, std::vector<Armour> &armours);
// Weapon shop system
void weaponShop(Hero &hero, std::vector<Weapon> &weapons);
// Potion shop system
void potionShop(Hero &hero, std::vector<Potion> &potionsList);
// Generate map with enemies, loot, traps, chests, and boss
std::vector<std::vector<char>> generateMap(int size);
// Display map with fog of war
void displayMap(const std::vector<std::vector<char>> &map, const std::vector<std::vector<bool>> &visited, const Hero &hero);
// Gain XP and check for level up
void gainXP(Hero &her, int amount);
// On startup setup the movement and inventories keys
int readKey(void);
void setupKeys(void);

int main(void) {
#ifdef _WIN32
  SetConsoleTitle("0verpg");
#endif /*_WIN32 */
  std::srand(static_cast<unsigned>(std::time(nullptr)));
  setupKeys();
  Hero hero;
  std::cout << "Enter hero name: ";
  std::getline(std::cin, hero.name);
  hero.health = 100;
  hero.baseDefense = 5;
  hero.baseAttack = 5;
  hero.equippedArmour = {"None", 0};
  hero.equippedWeapon = {"Fists", 0, 0};
  hero.potions = 0;
  hero.allPotions.emplace_back(20);
  hero.gold = 50;
  hero.x = 0;
  hero.y = 0;
  hero.level = 1;
  hero.xp = 0;
  // Armours
  std::vector<Armour> armours = {
    {"Leather Armour", 5},
    {"Iron Armour", 10},
    {"Magic Armour", 15},
    {"Dragon Armour", 20}
  };
  // Weapons
  std::vector<Weapon> weapons = {
    {"Wooden Sword", 5, 20},
    {"Steel Sword", 10, 40},
    {"Magic Sword", 15, 60},
    {"Dragon Slayer", 25, 100}
    };
  // Potions
  std::vector<Potion> potionsList = {
    {"Small Potion", 20, 15},
    {"Large Potion", 40, 30},
    {"Huge Potion", 60, 45}
  };
  // per level enemies
  std::vector<Enemy> enemies = {
    {"Goblin", 8, 30, 20},
    {"Orc", 12, 40, 30},
    {"Skeleton", 15, 50, 40}
  };
  // Inventory weapons/armours colours for their names
  const char *coloursArr[] = { BLUE, MAGENTA, GREEN, RED, YELLOW, CYAN };
  const int coloursArrLen = sizeof(coloursArr) / sizeof(coloursArr[0]);
  lastPotionIdxFromList = potionsList[potionsList.size() - 1].healAmount;
  std::cout << "\nChoose starting armour:\n";
  for (size_t i = 0; i < armours.size(); ++i) {
    std::cout << YELLOW << i+1 << ". " << RESETCOLOUR << MAGENTA << armours[i].name << CYAN << " (Defense +" << RESETCOLOUR << YELLOW << armours[i].defense << RESETCOLOUR << CYAN << ")" << RESETCOLOUR << "\n";
  }
  int choice;
  std::cin >> choice;
  if (choice >= 1 && choice <= (int)armours.size()) {
    hero.equippedArmour = armours[choice-1];
    hero.inventoryArmours.emplace_back(armours[choice-1]);
  }
  std::cout << CYAN << hero.name << RESETCOLOUR <<" equipped " << BLUE << hero.equippedArmour.name << " (Defense +" << hero.equippedArmour.defense << ")" << RESETCOLOUR << "\n";
  int floors = 3;      // Number of dungeon floors
  int mapSize = 5;     // Map size (square grid)
  for (int floor = 1; floor <= floors; ++floor) {
    // Generate map and visited tracker
    std::vector<std::vector<char>> dungeon = generateMap(mapSize);
    std::vector<std::vector<bool>> visited(mapSize, std::vector<bool>(mapSize, false));
    hero.x = 0;
    hero.y = 0;
    visited[0][0] = true;
    bool floorCleared = false;
    weaponShop(hero, weapons);
    potionShop(hero, potionsList);
    std::cout << YELLOW << "\n🏰 Floor " << floor << " begins!" << RESETCOLOUR << "\n";
    while (!floorCleared && hero.health > 0) {
      displayMap(dungeon, visited, hero);
      // Emojis from - www.emojishark.com
      std::cout << YELLOW << "\n💰 Gold: " << hero.gold << RESETCOLOUR << " | " << MAGENTA << "💗 HP: " << hero.health << RESETCOLOUR
        << " | " << GREEN << "🧪 Potions: " << hero.potions << RESETCOLOUR << " | "
        << "💀" << CYAN << " Attack: " << hero.baseAttack + hero.equippedWeapon.attack << " Defense: " << hero.baseDefense + hero.equippedArmour.defense << RESETCOLOUR
        << " | " << "👹 " << CYAN << hero.name << RESETCOLOUR << YELLOW << " Level: " << RESETCOLOUR << MAGENTA << hero.level << RESETCOLOUR << ", " 
        << CYAN << hero.xp << "/" << hero.level * 50 << " XP" << RESETCOLOUR << "\n";
      std::cout << YELLOW << "Move (" << static_cast<unsigned char>(keys[0]) << "/" <<  static_cast<unsigned char>(keys[1]) << "/" <<  static_cast<unsigned char>(keys[2]) << "/" <<  static_cast<unsigned char>(keys[3]) << "), "
        <<  static_cast<unsigned char>(keys[4]) << " to use potion, " <<  static_cast<unsigned char>(keys[5]) << " to change armour, " <<  static_cast<unsigned char>(keys[6]) << " to change weapon, Q to quit: " << RESETCOLOUR << std::endl;
      //std::cin >> move;
      char move = std::tolower(static_cast<unsigned char>(readKey()));
      if (move == 'q') { std::cout << MAGENTA << "🏁 Game ended." << RESETCOLOUR << "\n"; return EXIT_SUCCESS; }
      if (move == static_cast<unsigned char>(keys[4])) { usePotion(hero); continue; }
      if (move == static_cast<unsigned char>(keys[5])) {
        std::cout << MAGENTA << "Currently equipped armour: " << RESETCOLOUR << GREEN << hero.equippedArmour.name << " (Defense +" << hero.equippedArmour.defense << ")" << RESETCOLOUR << "\n";
        std::cout << "Inventory Armours:\n";
        for (size_t i = 0; i < hero.inventoryArmours.size(); ++i) {
          std::cout << YELLOW << i+1 << ". " << RESETCOLOUR << coloursArr[std::rand() % coloursArrLen] << hero.inventoryArmours[i].name << RESETCOLOUR
          << CYAN << " (Defense +" << RESETCOLOUR << YELLOW << hero.inventoryArmours[i].defense << RESETCOLOUR << CYAN << ")" << RESETCOLOUR << "\n";
        }
        std::cout << "Choose armour number: ";
        int idx;
        std::cin >> idx;
        equipArmour(hero, idx - 1);
        continue;
      }
      if (move == static_cast<unsigned char>(keys[6])) {
        std::cout << MAGENTA << "Currently equipped weapon: " << RESETCOLOUR << GREEN << hero.equippedWeapon.name << " (Attack +" << hero.equippedWeapon.attack << ")" << RESETCOLOUR << "\n";
        std::cout << "Inventory Weapons:\n";
        for (size_t i = 0; i < hero.inventoryWeapons.size(); ++i) {
          std::cout << YELLOW << i+1 << ". " << RESETCOLOUR << coloursArr[std::rand() % coloursArrLen] << hero.inventoryWeapons[i].name << RESETCOLOUR
          << CYAN << " (Attack +" << RESETCOLOUR << YELLOW << hero.inventoryWeapons[i].attack << RESETCOLOUR << CYAN << ")" << RESETCOLOUR << "\n";
        }
        std::cout << "Choose weapon number: ";
        int idx;
        std::cin >> idx;
        equipWeapon(hero, hero.inventoryWeapons, idx - 1);
        continue;
      }
      int newX = hero.x, newY = hero.y;
      if (move == static_cast<unsigned char>(keys[1])) newX--;
      else if (move == static_cast<unsigned char>(keys[3])) newX++;
      else if (move == static_cast<unsigned char>(keys[2])) newY--;
      else if (move == static_cast<unsigned char>(keys[0])) newY++;
      // Check boundaries
      if (newX < 0 || newX >= mapSize || newY < 0 || newY >= mapSize) { std::cout << RED << "❌ Can't move outside the dungeon!" << RESETCOLOUR << "\n"; continue; }
      hero.x = newX;
      hero.y = newY;
      visited[hero.x][hero.y] = true;
      // Explore the room
      if (!exploreRoom(hero, enemies, armours, weapons, dungeon[hero.x][hero.y])) { std::cout << RED << "\n💀 Game Over!" << RESETCOLOUR << "\n"; return EXIT_SUCCESS; }
      // Check if boss defeated
      bool bossAlive = false;
      for (int i = 0; i < mapSize; ++i) {
        for (int j = 0; j < mapSize; ++j) {
          if (dungeon[i][j] == 'B' || dungeon[i][j] == 'G' || dungeon[i][j] == 'Z') bossAlive = true;
        }
      }
      if (!bossAlive) {
        std::cout << GREEN << "\n✅ Floor " << floor << " cleared!" << RESETCOLOUR << "\n";
        floorCleared = true;
        // Heal hero slightly after round
        hero.health += 15;
        if (hero.health > 100) hero.health = 100;
        std::cout << CYAN << hero.name << RESETCOLOUR << " rests and recovers to " << GREEN << hero.health << RESETCOLOUR << " HP." << "\n";
      }
    }
    // Shop between floors
    if (floorCleared && floor < floors) { shop(hero, armours); }
  }
  std::cout << GREEN << "\n🎉 Congratulations! You defeated all bosses and cleared the dungeon!" << RESETCOLOUR << "\n";
  return EXIT_SUCCESS;
}

// Equip armour from inventory
void equipArmour(Hero &hero, int index) {
  if (index < 0 || index >= (int)hero.inventoryArmours.size()) { std::cout << "Invalid armour index.\n"; return; }
  hero.equippedArmour = hero.inventoryArmours[index];
  std::cout << CYAN << hero.name << RESETCOLOUR <<" equipped " << BLUE << hero.equippedArmour.name << " (Defense +" << hero.equippedArmour.defense << ")" << RESETCOLOUR << "\n";
}

// Equip weapon from inventory
void equipWeapon(Hero &hero, std::vector<Weapon> &weapons, int index) {
  if (index < 0 || index >= (int)weapons.size()) { std::cout << "Invalid weapon index.\n"; return; }
  hero.equippedWeapon = weapons[index];
  std::cout << CYAN << hero.name << RESETCOLOUR << " equipped " << BLUE << hero.equippedWeapon.name << " (Attack +" << hero.equippedWeapon.attack << ")" << RESETCOLOUR << "\n";
}

// Battle simulation
bool battle(Hero &hero, Enemy enemy) {
  std::cout << "\n⚔ Battle: " << CYAN << hero.name << RESETCOLOUR << " vs " << RED << enemy.name << RESETCOLOUR << "\n";
  while (hero.health > 0 && enemy.health > 0) {
    // Hero attacks
    int heroAttack = hero.baseAttack + hero.equippedWeapon.attack;
    if(std::rand() % 100 < 20) { heroAttack *= 2; std::cout << YELLOW << "CRITICAL HIT!" << RESETCOLOUR << "\n"; }
    enemy.health -= heroAttack;
    std::cout << CYAN << hero.name << RESETCOLOUR << " attacks for " << YELLOW << heroAttack << RESETCOLOUR <<" damage! " << enemy.name << " HP: " << RED << std::max(0, enemy.health) << RESETCOLOUR << "\n";
    if (enemy.health <= 0) {
      std::cout << MAGENTA << enemy.name << RESETCOLOUR << GREEN << " is defeated!" << RESETCOLOUR << "\n";
      hero.gold += 30;
      std::cout << CYAN << hero.name << RESETCOLOUR << " earns " << YELLOW << "30" << RESETCOLOUR << " gold! Total gold: " << YELLOW << hero.gold << RESETCOLOUR << "\n";
      gainXP(hero, enemy.xpReward);
      // Random loot drop
      int lootChance = std::rand() % 3;
      if (lootChance == 0) { hero.potions++; std::cout << YELLOW << "Loot drop: Potion found! Total potions: " << RESETCOLOUR << hero.potions << "\n"; }
      return true;
    }
    // Enemy attacks
    int totalDefense = hero.baseDefense + hero.equippedArmour.defense;
    int damage = enemy.attack - totalDefense;
    if (damage < 0) damage = 0;
    if(std::rand() % 100 < 15) { std::cout << MAGENTA << hero.name << RESETCOLOUR << GREEN << " dodged!" << RESETCOLOUR << "\n"; damage = 0; }
    hero.health -= damage;
    std::cout << RED << enemy.name << RESETCOLOUR << " attacks for " << RED << enemy.attack << RESETCOLOUR << " (Blocked: " << MAGENTA << totalDefense << RESETCOLOUR << ") Damage taken: " << YELLOW << damage << RESETCOLOUR << " | " << YELLOW << hero.name << RESETCOLOUR << " HP: " << GREEN << std::max(0, hero.health) << RESETCOLOUR << "\n";
    // Auto-use potion if low HP
    if (hero.health <= 30 && hero.potions > 0) { std::cout << CYAN << "⚠ Low HP! Auto-using potion..." << RESETCOLOUR << "\n"; usePotion(hero); }
    if (hero.health <= 0) { std::cout << RED << hero.name << " has fallen..." << RESETCOLOUR << "\n"; return false; }
  }
  return false;
}

// Loot room
void lootRoom(Hero &hero, std::vector<Armour> &armours, std::vector<Weapon> &weapons) {
  int lootType = std::rand() % 3;
  if (lootType == 0) {
    hero.potions++;
    std::cout << GREEN << "🧪 Found a health potion! Potions: " << hero.potions << RESETCOLOUR << "\n";
  }
  else if (lootType == 1) {
    Weapon newWeapon = weapons[std::rand() % weapons.size()];
    hero.inventoryWeapons.emplace_back(newWeapon);
    std::cout << GREEN << "⚔ Found new weapon: " << newWeapon.name << " (Attack +" << newWeapon.attack << ")" << RESETCOLOUR << "\n";
  }
  else {
    Armour newArmour = armours[std::rand() % armours.size()];
    hero.inventoryArmours.emplace_back(newArmour);
    std::cout << GREEN << "🛡 Found new armour: " << newArmour.name << " (Defense +" << newArmour.defense << ")" << RESETCOLOUR << "\n";
  }
}

// Use potion
void usePotion(Hero &hero) {
  if (hero.potions > 0) {
    //int heal = 20 + std::rand() % 21;
    int heal = 0;
    if (potionIdx >= 0U) { heal = (hero.allPotions[potionIdx] > lastPotionIdxFromList ? lastPotionIdxFromList : hero.allPotions[potionIdx]); if (potionIdx > 0) { potionIdx--; } }
    hero.health += heal;
    if (hero.health > 100) { hero.health = 100; }
    hero.potions--;
    hero.allPotions.pop_back();
    std::cout << GREEN << "Potion used!" << RESETCOLOUR << "  +" << heal << " HP | Current HP: " << MAGENTA << hero.health << RESETCOLOUR << "\n";
  } else {
    std::cout << RED << "No potions available!" << RESETCOLOUR << "\n";
  }
}

// Trap room
bool trapRoom(Hero &hero) {
  int damage = 5 + std::rand() % 16; // 5-20 damage
  hero.health -= damage;
  std::cout << RED << "💥 Trap triggered! You take " << damage << " damage. HP: " << hero.health << RESETCOLOUR << "\n";
  return hero.health > 0;
}

// Treasure chest room
void chestRoom(Hero &hero, std::vector<Armour> &armours) {
  int rewardType = std::rand() % 2;
  if (rewardType == 0) {
    int goldFound = 10 + std::rand() % 41; // 10-50 gold
    hero.gold += goldFound;
    std::cout << GREEN << "💰 Found a treasure chest with " << goldFound << " gold! Total gold: " << hero.gold << RESETCOLOUR << "\n";
  } else {
    Armour rareArmour = armours[std::rand() % armours.size()];
    hero.inventoryArmours.emplace_back(rareArmour);
    std::cout << GREEN << "✨ Found rare armour: " << rareArmour.name << " (Defense +" << rareArmour.defense << RESETCOLOUR << ")\n";
  }
}

// Explore room
bool exploreRoom(Hero &hero, std::vector<Enemy> &enemies, std::vector<Armour> &armours, std::vector<Weapon> &weapon, char &cell) {
  if (cell == 'E') {
    Enemy enemy = enemies[std::rand() % enemies.size()];
    cell = '.';
    return battle(hero, enemy);
  } else if (cell == 'L') {
    lootRoom(hero, armours, weapon);
    cell = '.';
    return true;
  } else if (cell == 'B') {
    Enemy boss = {"Dungeon Boss", 25, 80, 50};
    cell = '.';
    return battle(hero, boss);
  } else if (cell == 'G') {
    Enemy boss = {"Arena Boss", 26, 85, 60};
    cell = '.';
    return battle(hero, boss);
  } else if (cell == 'Z') {
    Enemy boss = {"Tarkan Boss", 27, 90, 70};
    cell = '.';
    return battle(hero, boss);
  } else if (cell == 'T') {
    cell = '.';
    return trapRoom(hero);
  } else if (cell == 'C') {
    chestRoom(hero, armours);
    cell = '.';
    return true;
  } else {
    std::cout << "🚪 Empty room.\n";
    return true;
  }
}

// Shop system
void shop(Hero &hero, std::vector<Armour> &armours) {
  std::cout << CYAN << "\n🏪 Welcome to the shop! You have " << RESETCOLOUR << YELLOW << hero.gold << " gold." << RESETCOLOUR << "\n";
  std::cout << YELLOW << "1." << RESETCOLOUR << BLUE << " Buy Potion " << RESETCOLOUR << YELLOW << "(20 gold)" << RESETCOLOUR << "\n";
  std::cout << YELLOW << "2." << RESETCOLOUR << BLUE << " Buy Armour Upgrade "<< RESETCOLOUR << YELLOW << "(50 gold)" << RESETCOLOUR << "\n";
  std::cout << "3. Exit Shop\n";
  int choice;
  std::cin >> choice;
  if (choice == 1) {
    if (hero.gold >= 20) {
      hero.gold -= 20;
      hero.potions++;
      std::cout << GREEN << "Bought a potion! Potions: " << hero.potions << RESETCOLOUR << "\n";
    } else {
      std::cout << RED << "Not enough gold!" << RESETCOLOUR << "\n";
    }
  } else if (choice == 2) {
    if (hero.gold >= 50) {
      hero.gold -= 50;
      Armour newArmour = armours[std::rand() % armours.size()];
      hero.inventoryArmours.emplace_back(newArmour);
      std::cout << BLUE << "Bought armour: " << newArmour.name << " (Defense +" << newArmour.defense << ")" << RESETCOLOUR << "\n";
    } else { std::cout << RED << "Not enough gold!" << RESETCOLOUR << "\n"; }
  } else { std::cout << YELLOW << "Leaving shop..." << RESETCOLOUR << "\n"; }
}

// Weapon shop system
void weaponShop(Hero &hero, std::vector<Weapon> &weapons) {
  std::cout << CYAN << "\n⚔ Weapon Shop — You have " << RESETCOLOUR << YELLOW << hero.gold << " gold." << RESETCOLOUR << "\n";
  std::cout << MAGENTA << "Currently equipped weapon: " << RESETCOLOUR << CYAN << hero.equippedWeapon.name << " (Attack +" << hero.equippedWeapon.attack << ")" << RESETCOLOUR << "\n";
  for (size_t i = 0; i < weapons.size(); ++i) { std::cout << YELLOW << i+1 << ". " << RESETCOLOUR << CYAN << weapons[i].name << " (Attack +" << RESETCOLOUR
    << YELLOW << weapons[i].attack  << RESETCOLOUR << "," << YELLOW << " Cost: " << weapons[i].cost << " gold" << RESETCOLOUR << CYAN << ")" << RESETCOLOUR << "\n"; }
  std::cout << "0. Skip\n";
  int choice;
  std::cout << BLUE << "Choose weapon: " << RESETCOLOUR;
  if (!(std::cin >> choice) || choice < 0 || choice > (int)weapons.size()) {
    std::cin.clear(); std::cin.ignore(1000, '\n');
    std::cout << RED << "Invalid choice. Skipping." << RESETCOLOUR << "\n";
    return;
  }
  if (choice == 0) return;
  Weapon selected = weapons[choice-1];
  if (hero.gold >= selected.cost) {
    hero.gold -= selected.cost;
    hero.inventoryWeapons.emplace_back(selected);
    equipWeapon(hero, weapons, choice - 1);
  } else { std::cout << RED << "Not enough gold!" << RESETCOLOUR << "\n"; }
}

// Potion shop
void potionShop(Hero &hero, std::vector<Potion> &potionsList) {
  std::cout << CYAN << "\n🍵 Potion Shop — You have " << RESETCOLOUR << YELLOW << hero.gold << " gold." << RESETCOLOUR << "\n";
  for (size_t i = 0; i < potionsList.size(); i++) { std::cout << YELLOW << i+1 << ". " << RESETCOLOUR << CYAN << potionsList[i].name << " (Heal +" << RESETCOLOUR
    << YELLOW << potionsList[i].healAmount << RESETCOLOUR << "," << YELLOW << " Cost: " << potionsList[i].cost << " gold" << RESETCOLOUR << CYAN << ")" << RESETCOLOUR << "\n"; }
  std::cout << "0. Skip\n";
  int choice;
  std::cout << BLUE << "Choose potion: " << RESETCOLOUR;
  if (!(std::cin >> choice) || choice < 0 || choice > (int)potionsList.size()) { std::cin.clear(); std::cin.ignore(1000, '\n'); std::cout << "Invalid choice. Skipping.\n"; return; }
  if (choice == 0) return;
  Potion selected = potionsList[choice-1];
  if (hero.gold >= selected.cost) {
    hero.gold -= selected.cost;
    hero.potions++;
    hero.allPotions.emplace_back(selected.healAmount);
    potionIdx++;
    std::cout << GREEN << "Bought " << selected.name << ". You now have " << hero.potions << " potions." << RESETCOLOUR << "\n";
  } else { std::cout << RED << "Not enough gold!" << RESETCOLOUR << "\n"; }
}

// Generate map with enemies, loot, traps, chests, and boss
std::vector<std::vector<char>> generateMap(int size) {
  std::vector<std::vector<char>> map(size, std::vector<char>(size, '.'));
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      int r = std::rand() % 6; // More variety
      if (r == 0) map[i][j] = 'E'; // Enemy
      else if (r == 1) map[i][j] = 'L'; // Loot
      else if (r == 2) map[i][j] = 'T'; // Trap
      else if (r == 3) map[i][j] = 'C'; // Chest
    }
  }
  // Place bosses in random locations
  bool bossSpawned = false;
  int bx = std::rand() % size;
  int by = std::rand() % size;
  map[bx][by] = 'B';
  int bx2 = std::rand() % size;
  int by2 = std::rand() % size;
  int bx3 = std::rand() % size;
  int by3 = std::rand() % size;
  if (by != by2 && bx != bx2) { map[bx2][by2] = 'G'; bossSpawned = true; }
  if (!bossSpawned && (by3 != by && bx3 != bx)) { map[bx3][by3] = 'Z'; }
  return map;
}

// Display map with fog of war
void displayMap(const std::vector<std::vector<char>> &map, const std::vector<std::vector<bool>> &visited, const Hero &hero) {
  std::cout << "\n" << GREEN << "Dungeon Map:" << RESETCOLOUR << "\n";
  for (int i = 0; i < map.size(); ++i) {
    for (int j = 0; j < map[i].size(); ++j) {
      if (i == hero.x && j == hero.y) std::cout << GREEN << 'H' << RESETCOLOUR << ' ';
      else if (visited[i][j]) std::cout << map[i][j] << ' ';
      else std::cout << YELLOW << '?' << RESETCOLOUR << ' ';
    }
    std::cout << "\n";
  }
}

// Gain XP and check for level up
void gainXP(Hero &hero, int amount) {
  hero.xp += amount;
  std::cout << GREEN << hero.name << RESETCOLOUR << YELLOW << " gained " << RESETCOLOUR << CYAN << amount << " XP!" << RESETCOLOUR << "\n";
  if (hero.xp >= hero.level * 50) { // Level up threshold
    hero.level++;
    hero.health += 20;
    if (hero.health > 100) { hero.health = 100; }
    hero.baseAttack += 5;
    hero.xp = 0; // reset XP after level up
    std::cout << GREEN << "*** LEVEL UP! *** " << RESETCOLOUR << MAGENTA << hero.name << RESETCOLOUR << YELLOW << " is now level " << hero.level << RESETCOLOUR << "!\n";
  }
}

#ifdef _WIN32
int readKey(void) {
  int c = _getch();
  if (c == 0 || c == 224) // in some cases on windows(?), pressing arrow keys returns two of these values, where the first is 0 or 224.
    c = _getch() + (c<<8);
  return c;
}
#else
int readKey(void) {
  char buf = 0;
  struct termios old = {0};
  fflush(stdout);
  if(tcgetattr(0, &old) < 0)
    perror("tcsetattr()");
  old.c_lflag &= (~ICANON); // local modes = Non Canonical mode and Disable echo
  old.c_cc[VMIN] = 1;  // control chars (MIN value) = 1
  old.c_cc[VTIME] = 0; // control chars (TIME value) = 0 (No time)
  if(tcsetattr(0, TCSANOW, &old) < 0)
    perror("tcsetattr ICANON");
  if(read(0, &buf, 1) < 0)
    perror("read()");
  old.c_lflag |= ICANON; // local modes = Canonical mode and Enable echo
  if(tcsetattr(0, TCSADRAIN, &old) < 0)
    perror("tcsetattr ~ICANON");
  return buf;
}
#endif /* _WIN32 */

void setupKeys(void) {
  int x = 0;
  std::cout<<"Press key for ";
  for (auto text : { "RIGHT",", UP",", LEFT",", DOWN",", Armour Inventory", ", Weapon Inventory", " comsume Potion" }) {
    std::cout << text;
    keys[x++] = readKey();
  }
  std::cout << BLUE << "\nKeys established\n" << RESETCOLOUR << std::endl;
}