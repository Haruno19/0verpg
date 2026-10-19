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
#include <fstream>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <cstdio>
#include <termios.h>
#include <unistd.h>
#endif /* _WIN32 */

#include "json.hpp"

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
  std::vector<Potion> inventoryPotions;
  int potions;
  int gold;
  int x, y;
  int xp;
  int level;
  int maxHP;
};

// All of the available characters/heros
struct Character {
  std::string name;
  int health;
  int baseDefense;
  int baseAttack;
};

int keys[7];
std::string configHome = (std::getenv("HOME") ? std::string(std::getenv("HOME")) + std::string("/") : std::string("./")) + ".0verpg.json";
std::string userLang = "en";
std::ifstream cfg(configHome);

using json = nlohmann::json;
json config;
#define CONF config[userLang]

// Equip armour from inventory
void equipArmour(Hero &hero, int index);
// Equip weapon
void equipWeapon(Hero &hero, std::vector<Weapon> &weapons, int index);
// Battle simulation
bool battle(Hero &hero, Enemy enemy);
// Loot room
void lootRoom(Hero &hero, std::vector<Armour> &armours, std::vector<Weapon> &weapons, std::vector<Potion> &potionsList);
// Use potion
void usePotion(Hero &hero);
// Trap room
bool trapRoom(Hero &hero);
// Treasure chest room
void chestRoom(Hero &hero, std::vector<Armour> &armours);
// Explore room
bool exploreRoom(Hero &hero, std::vector<Enemy> &enemies, std::vector<Armour> &armours, std::vector<Weapon> &weapons,char &cell, std::vector<Potion> &potionsList);
// Shop system
void shop(Hero &hero, std::vector<Armour> &armours, std::vector<Potion> &potionsList);
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
std::string trim(std::string str);

int main(int argc, char *argv[]) {
#ifdef _WIN32
  SetConsoleTitle("0verpg");
#endif /*_WIN32 */
  std::srand(static_cast<unsigned>(std::time(nullptr)));
  if (argc >= 2) { userLang = std::string(argv[1]); }
  if (!cfg) { std::cerr << "Error: Could not open " << configHome << "\n"; return EXIT_FAILURE; }
  cfg >> config;
  cfg.close();
  setupKeys();
  int classChoice = 0;
  // All of the available character classes
  std::vector<Character> characters = {
    {trim(CONF["warrior"]), 60, 5, 5},
    {trim(CONF["mage"]), 90, 10, 1},
    {trim(CONF["rogue"]), 70, 7, 3},
    {trim(CONF["paladin"]), 75, 6, 2}
  };
  while (classChoice < 1 || classChoice > characters.size()) {
    std::cout << GREEN << trim(CONF["chooseClass"]) << RESETCOLOUR << ":\n";
    for (unsigned int indexNum = 0U; indexNum < characters.size(); indexNum++) {
      std::cout << YELLOW << indexNum + 1 << ". " << RESETCOLOUR << BLUE << characters[indexNum].name << RESETCOLOUR << "\n";
    }
    std::cin >> classChoice;
  }
  classChoice--;
  std::cout << BLUE << trim(CONF["classChosen"]) << " " << characters[classChoice].name << RESETCOLOUR << std::endl;
  Hero hero;
  hero.name = characters[classChoice].name;
  hero.health = characters[classChoice].health;
  hero.maxHP = hero.health;
  hero.baseDefense = characters[classChoice].baseDefense;
  hero.baseAttack = characters[classChoice].baseAttack;
  hero.equippedArmour = {trim(CONF["none"]), 0};
  hero.equippedWeapon = {trim(CONF["fists"]), 0, 0};
  hero.gold = 50;
  hero.x = 0;
  hero.y = 0;
  hero.level = 1;
  hero.xp = 0;
  // Armours
  std::vector<Armour> armours = {
    {trim(CONF["armour1"]), 5},
    {trim(CONF["armour2"]), 10},
    {trim(CONF["armour3"]), 15},
    {trim(CONF["armour4"]), 20}
  };
  // Weapons
  std::vector<Weapon> weapons = {
    {trim(CONF["weapon1"]), 5, 20},
    {trim(CONF["weapon2"]), 10, 40},
    {trim(CONF["weapon3"]), 15, 60},
    {trim(CONF["weapon4"]), 25, 100}
    };
  // Mage Weapons
  std::vector<Weapon> mageWeapons = {
    {trim(CONF["mageWeapon1"]), 5, 20},
    {trim(CONF["mageWeapon2"]), 10, 40},
    {trim(CONF["mageWeapon3"]), 15, 60},
    {trim(CONF["mageWeapon4"]), 25, 100}
    };
  // Potions
  std::vector<Potion> potionsList = {
    {trim(CONF["potion1"]), 20, 15},
    {trim(CONF["potion2"]), 40, 30},
    {trim(CONF["potion3"]), 60, 45}
  };
  // per level enemies
  std::vector<Enemy> enemies = {
    {trim(CONF["enemy1"]), 8, 30, 20},
    {trim(CONF["enemy2"]), 12, 40, 30},
    {trim(CONF["enemy3"]), 15, 50, 40}
  };
  // Inventory weapons/armours colours for their names
  const char *coloursArr[] = { BLUE, MAGENTA, GREEN, RED, YELLOW, CYAN };
  const int coloursArrLen = sizeof(coloursArr) / sizeof(coloursArr[0]);
  unsigned char wKey = static_cast<unsigned char>(keys[1]);
  unsigned char dKey = static_cast<unsigned char>(keys[0]);
  unsigned char sKey = static_cast<unsigned char>(keys[3]);
  unsigned char aKey = static_cast<unsigned char>(keys[2]);
  unsigned char potionKey = static_cast<unsigned char>(keys[6]);
  unsigned char armourKey = static_cast<unsigned char>(keys[4]);
  unsigned char weaponKey = static_cast<unsigned char>(keys[5]);
  std::cout << "\n" << trim(CONF["chooseStartingArmour"]) << "\n";
  for (size_t i = 0; i < armours.size(); ++i) {
    std::cout << YELLOW << i+1 << ". " << RESETCOLOUR << MAGENTA << armours[i].name << CYAN << " (" << trim(CONF["defense"]) << " +" << RESETCOLOUR << YELLOW << armours[i].defense << RESETCOLOUR << CYAN << ")" << RESETCOLOUR << "\n";
  }
  int choice;
  std::cin >> choice;
  if (choice >= 1 && choice <= (int)armours.size()) {
    hero.equippedArmour = armours[choice-1];
    hero.inventoryArmours.emplace_back(armours[choice-1]);
  }
  std::cout << CYAN << hero.name << RESETCOLOUR << " " << trim(CONF["equipped"]) << " " << BLUE << hero.equippedArmour.name << " (" << trim(CONF["defense"]) << " +" << hero.equippedArmour.defense << ")" << RESETCOLOUR << "\n";
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
    weaponShop(hero, (hero.name == "Mage" || hero.name == "Магьосник") ? mageWeapons : weapons);
    potionShop(hero, potionsList);
    std::cout << YELLOW << "\n🏰 " << trim(CONF["floor"]) << " " << floor << " " << trim(CONF["begins"]) << RESETCOLOUR << "\n";
    while (!floorCleared && hero.health > 0) {
      displayMap(dungeon, visited, hero);
      // Emojis from - www.emojishark.com
      std::cout << YELLOW << "\n💰 " << trim(CONF["gold"]) << ": " << hero.gold << RESETCOLOUR << " | " << MAGENTA << "💗 " << trim(CONF["hp"]) << ": " << hero.health << RESETCOLOUR
        << " | " << GREEN << "🧪 " << trim(CONF["potions"]) << ": " << hero.inventoryPotions.size() << RESETCOLOUR << " | "
        << "💀" << CYAN << " " << trim(CONF["attack"]) << ": " << hero.baseAttack + hero.equippedWeapon.attack << " " << trim(CONF["defense"]) << ": " << hero.baseDefense + hero.equippedArmour.defense << RESETCOLOUR
        << " | " << "👹 " << CYAN << hero.name << RESETCOLOUR << YELLOW << " " << trim(CONF["level"]) << ": " << RESETCOLOUR << MAGENTA << hero.level << RESETCOLOUR << ", " 
        << CYAN << hero.xp << "/" << hero.level * 50 << " " << trim(CONF["xp"]) << RESETCOLOUR << "\n";
      std::cout << YELLOW << trim(CONF["move"]) <<  " (" << wKey << "/" <<  dKey << "/" <<  aKey << "/" <<  sKey << "), " <<  potionKey << " " << trim(CONF["usePotion"]) << ", "
        <<  armourKey << " " << trim(CONF["toChangeArmour"]) << ", " <<  weaponKey << " " << trim(CONF["toChangeWeapon"]) << ", Q " << trim(CONF["toQuit"]) << ": " << RESETCOLOUR << std::endl;
      //std::cin >> move;
      char move = std::tolower(static_cast<unsigned char>(readKey()));
      if (move == 'q') { std::cout << MAGENTA << "🏁 Game ended." << RESETCOLOUR << "\n"; return EXIT_SUCCESS; }
      if (move == potionKey) { usePotion(hero); continue; }
      if (move == armourKey) {
        std::cout << MAGENTA << trim(CONF["curEquippedArmour"]) << ": " << RESETCOLOUR << GREEN << hero.equippedArmour.name << " (" << trim(CONF["defense"]) << " +" << hero.equippedArmour.defense << ")" << RESETCOLOUR << "\n";
        std::cout << trim(CONF["inventoryArmours"]) << ":\n";
        for (size_t i = 0; i < hero.inventoryArmours.size(); ++i) {
          std::cout << YELLOW << i+1 << ". " << RESETCOLOUR << coloursArr[std::rand() % coloursArrLen] << hero.inventoryArmours[i].name << RESETCOLOUR
          << CYAN << " (" << trim(CONF["defense"]) << "+" << RESETCOLOUR << YELLOW << hero.inventoryArmours[i].defense << RESETCOLOUR << CYAN << ")" << RESETCOLOUR << "\n";
        }
        std::cout << trim(CONF["chooseArmourNumber"]) << ": ";
        int idx;
        std::cin >> idx;
        equipArmour(hero, idx - 1);
        continue;
      }
      if (move == weaponKey) {
        std::cout << MAGENTA << trim(CONF["curEquippedWeapon"]) << ": " << RESETCOLOUR << GREEN << hero.equippedWeapon.name << " (" << trim(CONF["attack"]) << " +" << hero.equippedWeapon.attack << ")" << RESETCOLOUR << "\n";
        std::cout << trim(CONF["inventoryWeapons"]) << ":\n";
        for (size_t i = 0; i < hero.inventoryWeapons.size(); ++i) {
          std::cout << YELLOW << i+1 << ". " << RESETCOLOUR << coloursArr[std::rand() % coloursArrLen] << hero.inventoryWeapons[i].name << RESETCOLOUR
          << CYAN << " (" << trim(CONF["attack"]) << " +" << RESETCOLOUR << YELLOW << hero.inventoryWeapons[i].attack << RESETCOLOUR << CYAN << ")" << RESETCOLOUR << "\n";
        }
        std::cout << trim(CONF["chooseWeaponNumber"]) << ": ";
        int idx;
        std::cin >> idx;
        equipWeapon(hero, hero.inventoryWeapons, idx - 1);
        continue;
      }
      int newX = hero.x, newY = hero.y;
      if (move == wKey) newX--;
      else if (move == sKey) newX++;
      else if (move == aKey) newY--;
      else if (move == dKey) newY++;
      // Check boundaries
      if (newX < 0 || newX >= mapSize || newY < 0 || newY >= mapSize) { std::cout << RED << "❌ " << trim(CONF["cantMoveOutsideDungeon"]) << RESETCOLOUR << "\n"; continue; }
      hero.x = newX;
      hero.y = newY;
      visited[hero.x][hero.y] = true;
      // Explore the room
      if (!exploreRoom(hero, enemies, armours, (hero.name == "Mage" || hero.name == "Магьосник") ? mageWeapons : weapons, dungeon[hero.x][hero.y], potionsList)) { std::cout << RED << "\n💀 " << trim(CONF["gameOver"]) << RESETCOLOUR << "\n"; return EXIT_SUCCESS; }
      // Check if boss defeated
      bool bossAlive = false;
      for (int i = 0; i < mapSize; ++i) {
        for (int j = 0; j < mapSize; ++j) {
          if (dungeon[i][j] == 'B' || dungeon[i][j] == 'G' || dungeon[i][j] == 'Z') bossAlive = true;
        }
      }
      if (!bossAlive) {
        std::cout << GREEN << "\n✅ " << trim(CONF["floor"]) << " " << floor << " " << trim(CONF["cleared"]) << RESETCOLOUR << "\n";
        floorCleared = true;
        // Heal hero slightly after round
        hero.health += 15;
        if (hero.health > hero.maxHP) hero.health = hero.maxHP;
        std::cout << CYAN << hero.name << RESETCOLOUR << " " << trim(CONF["restsAndRecoversTo"]) << " " << GREEN << hero.health << RESETCOLOUR << " " << trim(CONF["hp"]) << "." << "\n";
      }
    }
    // Shop between floors
    if (floorCleared && floor < floors) { shop(hero, armours, potionsList); }
  }
  std::cout << GREEN << "\n🎉 " << trim(CONF["win"]) << RESETCOLOUR << "\n";
  return EXIT_SUCCESS;
}

// Equip armour from inventory
void equipArmour(Hero &hero, int index) {
  if (index < 0 || index >= (int)hero.inventoryArmours.size()) { std::cout << trim(CONF["invalidArmourIndex"]) << ".\n"; return; }
  hero.equippedArmour = hero.inventoryArmours[index];
  std::cout << CYAN << hero.name << RESETCOLOUR << " " << trim(CONF["equipped"]) << " " << BLUE << hero.equippedArmour.name << " (" << trim(CONF["defense"]) << " +" << hero.equippedArmour.defense << ")" << RESETCOLOUR << "\n";
}

// Equip weapon from inventory
void equipWeapon(Hero &hero, std::vector<Weapon> &weapons, int index) {
  if (index < 0 || index >= (int)weapons.size()) { std::cout << trim(CONF["invalidWeaponIndex"]) << ".\n"; return; }
  hero.equippedWeapon = weapons[index];
  std::cout << CYAN << hero.name << RESETCOLOUR << " " <<  trim(CONF["equipped"]) << " " << BLUE << hero.equippedWeapon.name << " (" <<  trim(CONF["attack"]) << " +" << hero.equippedWeapon.attack << ")" << RESETCOLOUR << "\n";
}

// Battle simulation
bool battle(Hero &hero, Enemy enemy) {
  std::cout << "\n⚔ " << trim(CONF["battle"]) << " " << CYAN << hero.name << RESETCOLOUR << " vs " << RED << enemy.name << RESETCOLOUR << "\n";
  while (hero.health > 0 && enemy.health > 0) {
    // Hero attacks
    int heroAttack = hero.baseAttack + hero.equippedWeapon.attack;
    if(std::rand() % 100 < 20) { heroAttack *= 2; std::cout << YELLOW << trim(CONF["criticalHit"]) << "!" << RESETCOLOUR << "\n"; }
    enemy.health -= heroAttack;
    std::cout << CYAN << hero.name << RESETCOLOUR << " " << trim(CONF["attacksFor"]) << " " << YELLOW << heroAttack << RESETCOLOUR <<" " << trim(CONF["damage"]) << "! " << enemy.name << " " << trim(CONF["hp"]) << ": " << RED << std::max(0, enemy.health) << RESETCOLOUR << "\n";
    if (enemy.health <= 0) {
      std::cout << MAGENTA << enemy.name << RESETCOLOUR << GREEN << " " << trim(CONF["isDefeated"]) << "!" << RESETCOLOUR << "\n";
      hero.gold += 30;
      std::cout << CYAN << hero.name << RESETCOLOUR << " " << trim(CONF["earns"]) << " " << YELLOW << "30" << RESETCOLOUR << " " << trim(CONF["goldTotalGold"]) << ": " << YELLOW << hero.gold << RESETCOLOUR << "\n";
      gainXP(hero, enemy.xpReward);
      // Random loot drop
      int lootChance = std::rand() % 3;
      if (lootChance == 0) { std::cout << YELLOW << trim(CONF["lootDropPotion"]) << ": " << RESETCOLOUR << hero.inventoryPotions.size() << "\n"; }
      return true;
    }
    // Enemy attacks
    int totalDefense = hero.baseDefense + hero.equippedArmour.defense;
    int damage = enemy.attack - totalDefense;
    if (damage < 0) damage = 0;
    if(std::rand() % 100 < 15) { std::cout << MAGENTA << hero.name << RESETCOLOUR << GREEN << " " << trim(CONF["dodged"]) << "!" << RESETCOLOUR << "\n"; damage = 0; }
    hero.health -= damage;
    std::cout << RED << enemy.name << RESETCOLOUR << " " << trim(CONF["attacksFor"]) << " " << RED << enemy.attack << RESETCOLOUR << " (" << trim(CONF["blocked"]) << " " << MAGENTA << totalDefense << RESETCOLOUR << ") " << trim(CONF["damageTaken"]) << ": " << YELLOW << damage << RESETCOLOUR << " | " << YELLOW << hero.name << RESETCOLOUR << " " << trim(CONF["hp"]) << ": " << GREEN << std::max(0, hero.health) << RESETCOLOUR << "\n";
    // Auto-use potion if low HP
    if (hero.health <= 30 && hero.inventoryPotions.size() > 0) { std::cout << CYAN << "⚠ " << trim(CONF["autoUsingPotion"]) << "..." << RESETCOLOUR << "\n"; usePotion(hero); }
    if (hero.health <= 0) { std::cout << RED << hero.name << " " << trim(CONF["hasFallen"]) << "..." << RESETCOLOUR << "\n"; return false; }
  }
  return false;
}

// Loot room
void lootRoom(Hero &hero, std::vector<Armour> &armours, std::vector<Weapon> &weapons, std::vector<Potion> &potionsList) {
  int lootType = std::rand() % 3;
  if (lootType == 0) {
    Potion lootPotion = potionsList[std::rand() % potionsList.size()];
    hero.inventoryPotions.emplace_back(lootPotion);
    std::cout << GREEN << "🧪 " << trim(CONF["foundHealthPotion"]) << ": " << hero.inventoryPotions.size() << ", " << lootPotion.healAmount << trim(CONF["hp"]) << RESETCOLOUR << "\n";
  }
  else if (lootType == 1) {
    Weapon newWeapon = weapons[std::rand() % weapons.size()];
    hero.inventoryWeapons.emplace_back(newWeapon);
    std::cout << GREEN << "⚔ " << trim(CONF["foundNewWeapon"]) << ": " << newWeapon.name << " (" << trim(CONF["attack"]) << " +" << newWeapon.attack << ")" << RESETCOLOUR << "\n";
  }
  else {
    Armour newArmour = armours[std::rand() % armours.size()];
    hero.inventoryArmours.emplace_back(newArmour);
    std::cout << GREEN << "🛡 " << trim(CONF["foundNewArmour"]) << ": " << newArmour.name << " (" << trim(CONF["defense"]) << " +" << newArmour.defense << ")" << RESETCOLOUR << "\n";
  }
}

// Use potion
void usePotion(Hero &hero) {
  if (hero.inventoryPotions.size() > 0) {
    Potion p = hero.inventoryPotions.back();
    hero.health += p.healAmount;
    if (hero.health > hero.maxHP) { hero.health = hero.maxHP; }
    std::cout << GREEN << trim(CONF["potionUsed"]) << "!" << RESETCOLOUR << "  +" << p.healAmount << " " << trim(CONF["hpCurrentHP"]) << ": " << MAGENTA << hero.health << RESETCOLOUR << "\n";
    hero.inventoryPotions.pop_back();
  } else {
    std::cout << RED << trim(CONF["noPotionsAvailable"]) << "!" << RESETCOLOUR << "\n";
  }
}

// Trap room
bool trapRoom(Hero &hero) {
  int damage = 5 + std::rand() % 16; // 5-20 damage
  hero.health -= damage;
  std::cout << RED << "💥 " << trim(CONF["trapTriggered"]) << " " << damage << " " << trim(CONF["damageHP"]) <<  ": " << hero.health << RESETCOLOUR << "\n";
  return hero.health > 0;
}

// Treasure chest room
void chestRoom(Hero &hero, std::vector<Armour> &armours) {
  int rewardType = std::rand() % 2;
  if (rewardType == 0) {
    int goldFound = 10 + std::rand() % 41; // 10-50 gold
    hero.gold += goldFound;
    std::cout << GREEN << "💰 " << trim(CONF["foundTreasureChestWith"]) << " " << goldFound << " " << trim(CONF["goldTotalGold"]) << ": " << hero.gold << RESETCOLOUR << "\n";
  } else {
    Armour rareArmour = armours[std::rand() % armours.size()];
    hero.inventoryArmours.emplace_back(rareArmour);
    std::cout << GREEN << "✨ " << trim(CONF["foundRareArmour"]) << ": " << rareArmour.name << " (" << trim(CONF["defense"]) << " +" << rareArmour.defense << RESETCOLOUR << ")\n";
  }
}

// Explore room
bool exploreRoom(Hero &hero, std::vector<Enemy> &enemies, std::vector<Armour> &armours, std::vector<Weapon> &weapon, char &cell, std::vector<Potion> &potionsList) {
  if (cell == 'E') {
    Enemy enemy = enemies[std::rand() % enemies.size()];
    cell = '.';
    return battle(hero, enemy);
  } else if (cell == 'L') {
    lootRoom(hero, armours, weapon, potionsList);
    cell = '.';
    return true;
  } else if (cell == 'B') {
    Enemy boss = {trim(CONF["dungeonBoss"]), 25, 80, 50};
    cell = '.';
    return battle(hero, boss);
  } else if (cell == 'G') {
    Enemy boss = {trim(CONF["arenaBoss"]), 26, 85, 60};
    cell = '.';
    return battle(hero, boss);
  } else if (cell == 'Z') {
    Enemy boss = {trim(CONF["tarkanBoss"]), 27, 90, 70};
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
    std::cout << "🚪 " << trim(CONF["emptyRoom"]) << ".\n";
    return true;
  }
}

// Shop system
void shop(Hero &hero, std::vector<Armour> &armours, std::vector<Potion> &potionsList) {
  std::cout << CYAN << "\n🏪 " << trim(CONF["welcomeYouHave"]) << " " << RESETCOLOUR << YELLOW << hero.gold << " " << trim(CONF["gold"]) << "." << RESETCOLOUR << "\n";
  std::cout << YELLOW << "1." << RESETCOLOUR << BLUE << " " << trim(CONF["buyPotion"]) << " " << RESETCOLOUR << YELLOW << "(20 " << trim(CONF["gold"]) << ")" << RESETCOLOUR << "\n";
  std::cout << YELLOW << "2." << RESETCOLOUR << BLUE << " " << trim(CONF["buyArmourUpgrade"]) << " " << RESETCOLOUR << YELLOW << "(50 " << trim(CONF["gold"]) << ")" << RESETCOLOUR << "\n";
  std::cout << "3. Exit Shop\n";
  int choice;
  std::cin >> choice;
  if (choice == 1) {
    if (hero.gold >= 20) {
      hero.gold -= 20;
      Potion selected = potionsList[0];
      hero.inventoryPotions.emplace_back(selected);
      std::cout << GREEN << trim(CONF["boughtPotion"]) << ": " << hero.inventoryPotions.size() << RESETCOLOUR << "\n";
    } else {
      std::cout << RED << trim(CONF["notEnoughGold"]) << "!" << RESETCOLOUR << "\n";
    }
  } else if (choice == 2) {
    if (hero.gold >= 50) {
      hero.gold -= 50;
      Armour newArmour = armours[std::rand() % armours.size()];
      hero.inventoryArmours.emplace_back(newArmour);
      std::cout << BLUE << trim(CONF["boughtArmour"]) << ": " << newArmour.name << " (" << trim(CONF["defense"]) << " +" << newArmour.defense << ")" << RESETCOLOUR << "\n";
    } else { std::cout << RED << trim(CONF["notEnoughGold"]) << "!" << RESETCOLOUR << "\n"; }
  } else { std::cout << YELLOW << trim(CONF["leavingShop"]) << "..." << RESETCOLOUR << "\n"; }
}

// Weapon shop system
void weaponShop(Hero &hero, std::vector<Weapon> &weapons) {
  std::cout << CYAN << "\n⚔ " << trim(CONF["weaponShopYouHave"]) << " " << RESETCOLOUR << YELLOW << hero.gold << " " << trim(CONF["gold"]) << "." << RESETCOLOUR << "\n";
  std::cout << MAGENTA << trim(CONF["curEquippedWeapon"]) << ": " << RESETCOLOUR << CYAN << hero.equippedWeapon.name << " (" << trim(CONF["attack"]) << " +" << hero.equippedWeapon.attack << ")" << RESETCOLOUR << "\n";
  for (size_t i = 0; i < weapons.size(); ++i) { std::cout << YELLOW << i+1 << ". " << RESETCOLOUR << CYAN << weapons[i].name << " (" << trim(CONF["attack"]) << " +" << RESETCOLOUR
    << YELLOW << weapons[i].attack  << RESETCOLOUR << "," << YELLOW << " " << trim(CONF["cost"]) << ": " << weapons[i].cost << " " << trim(CONF["gold"]) << RESETCOLOUR << CYAN << ")" << RESETCOLOUR << "\n"; }
  std::cout << "0. " << trim(CONF["skip"]) << "\n";
  int choice;
  std::cout << BLUE << trim(CONF["chooseWeaponNumber"]) << ": " << RESETCOLOUR;
  if (!(std::cin >> choice) || choice < 0 || choice > (int)weapons.size()) {
    std::cin.clear(); std::cin.ignore(1000, '\n');
    std::cout << RED << trim(CONF["invalidChoiceSkipping"]) << "." << RESETCOLOUR << "\n";
    return;
  }
  if (choice == 0) return;
  Weapon selected = weapons[choice-1];
  if (hero.gold >= selected.cost) {
    hero.gold -= selected.cost;
    hero.inventoryWeapons.emplace_back(selected);
    equipWeapon(hero, weapons, choice - 1);
  } else { std::cout << RED << trim(CONF["notEnoughGold"]) << "!" << RESETCOLOUR << "\n"; }
}

// Potion shop
void potionShop(Hero &hero, std::vector<Potion> &potionsList) {
  std::cout << CYAN << "\n🍵 " << trim(CONF["potionShopYouHave"]) << " " << RESETCOLOUR << YELLOW << hero.gold << " " << trim(CONF["gold"]) << "." << RESETCOLOUR << "\n";
  for (size_t i = 0; i < potionsList.size(); i++) { std::cout << YELLOW << i+1 << ". " << RESETCOLOUR << CYAN << potionsList[i].name << " (" << trim(CONF["heal"]) << " +" << RESETCOLOUR
    << YELLOW << potionsList[i].healAmount << RESETCOLOUR << "," << YELLOW << " " << trim(CONF["cost"]) << ": " << potionsList[i].cost << " " << trim(CONF["gold"]) << RESETCOLOUR << CYAN << ")" << RESETCOLOUR << "\n"; }
  std::cout << "0. " << trim(CONF["skip"]) << "\n";
  int choice;
  std::cout << BLUE << trim(CONF["choosePotion"]) << ": " << RESETCOLOUR;
  if (!(std::cin >> choice) || choice < 0 || choice > (int)potionsList.size()) { std::cin.clear(); std::cin.ignore(1000, '\n'); std::cout << trim(CONF["invalidChoiceSkipping"]) << ".\n"; return; }
  if (choice == 0) return;
  Potion selected = potionsList[choice-1];
  if (hero.gold >= selected.cost) {
    hero.gold -= selected.cost;
    hero.inventoryPotions.emplace_back(selected);
    std::cout << GREEN << trim(CONF["bought"]) << " " << selected.name << ". " << trim(CONF["youNowHave"]) << " " << hero.inventoryPotions.size() << " " << trim(CONF["potions"]) << "." << RESETCOLOUR << "\n";
  } else { std::cout << RED << trim(CONF["notEnoughGold"]) << "!" << RESETCOLOUR << "\n"; }
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
  std::cout << "\n" << GREEN << trim(CONF["dungeonMap"]) << ":" << RESETCOLOUR << "\n";
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
  std::cout << GREEN << hero.name << RESETCOLOUR << YELLOW << " " << trim(CONF["gained"]) << " " << RESETCOLOUR << CYAN << amount << " " << trim(CONF["xp"]) << "!" << RESETCOLOUR << "\n";
  if (hero.xp >= hero.level * 50) { // Level up threshold
    hero.level++;
    hero.health += 20;
    if (hero.health > hero.maxHP) { hero.health = hero.maxHP; }
    hero.baseAttack += 5;
    hero.xp = 0; // reset XP after level up
    std::cout << GREEN << "*** " << trim(CONF["levelUP"]) << "! *** " << RESETCOLOUR << MAGENTA << hero.name << RESETCOLOUR << YELLOW << " " << trim(CONF["isNowLevel"]) << " " << hero.level << RESETCOLOUR << "!\n";
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
  if(tcgetattr(0, &old) < 0) { perror("tcsetattr()"); }
  old.c_lflag &= (~ICANON); // local modes = Non Canonical mode
  old.c_cc[VMIN] = 1;  // control chars (MIN value) = 1
  old.c_cc[VTIME] = 0; // control chars (TIME value) = 0 (No time)
  if(tcsetattr(0, TCSANOW, &old) < 0) { perror("tcsetattr ICANON"); }
  if(read(0, &buf, 1) < 0) { perror("read()"); }
  old.c_lflag |= ICANON; // local modes = Canonical mode
  if(tcsetattr(0, TCSADRAIN, &old) < 0) { perror("tcsetattr ~ICANON"); }
  return buf;
}
#endif /* _WIN32 */

// Configure the movement/inventories/potion consumption
#define GETSTR(str) CONF[str].get<std::string>().c_str()
void setupKeys(void) {
  int x = 0;
  std::cout << trim(CONF["pressKeyFor"]) << " ";
  for (auto text : { "RIGHT", ", UP", ", LEFT", ", DOWN",
      ", Armour Inventory", ", Weapon Inventory", ", and Consume a Potion" }) {
    std::cout << text;
    keys[x++] = readKey();
  }
  std::cout << BLUE << "\n" << trim(CONF["keysEstablished"]) << "\n" << RESETCOLOUR << std::endl;
}

// Trim excess quotes
std::string trim(std::string str) {
  std::string buf;
  for (auto &x : str) {
    if (x == '"') { continue; }
    buf.push_back(x);
  }
  return buf;
}