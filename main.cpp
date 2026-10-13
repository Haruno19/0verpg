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

// Armor structure
struct Armor {
  std::string name;
  int defense;
};

// Enemy structure
struct Enemy {
  std::string name;
  int attack;
  int health;
};

// Hero structure
struct Hero {
  std::string name;
  int health;
  int baseDefense;
  Armor equippedArmor;
  std::vector<Armor> inventoryArmors;
  int potions;
  int gold;
  int x, y;
};

// Equip armor from inventory
void equipArmor(Hero &hero, int index);
// Battle simulation
bool battle(Hero &hero, Enemy enemy);
// Loot room
void lootRoom(Hero &hero, std::vector<Armor> &armors);
// Use potion
void usePotion(Hero &hero);
// Trap room
bool trapRoom(Hero &hero);
// Treasure chest room
void chestRoom(Hero &hero, std::vector<Armor> &armors);
// Explore room
bool exploreRoom(Hero &hero, std::vector<Enemy> &enemies, std::vector<Armor> &armors, char &cell);
// Shop system
void shop(Hero &hero, std::vector<Armor> &armors);
// Generate map with enemies, loot, traps, chests, and boss
std::vector<std::vector<char>> generateMap(int size);
// Display map with fog of war
void displayMap(const std::vector<std::vector<char>> &map, const std::vector<std::vector<bool>> &visited, const Hero &hero);

int main(void) {
  std::srand(static_cast<unsigned>(std::time(nullptr)));
  Hero hero;
  std::cout << "Enter hero name: ";
  std::getline(std::cin, hero.name);
  hero.health = 100;
  hero.baseDefense = 5;
  hero.equippedArmor = {"None", 0};
  hero.potions = 0;
  hero.gold = 0;
  hero.x = 0;
  hero.y = 0;
  std::vector<Armor> armors = {
    {"Leather Armor", 5},
    {"Iron Armor", 10},
    {"Magic Armor", 15},
    {"Dragon Armor", 20}
  };

  std::cout << "\nChoose starting armor:\n";
  for (size_t i = 0; i < armors.size(); ++i) {
    std::cout << i+1 << ". " << armors[i].name << " (Defense +" << armors[i].defense << ")\n";
  }
  int choice;
  std::cin >> choice;
  if (choice >= 1 && choice <= (int)armors.size()) {
    hero.equippedArmor = armors[choice-1];
    hero.inventoryArmors.push_back(armors[choice-1]);
  }

  std::vector<Enemy> enemies = {
    {"Goblin", 8, 30},
    {"Orc", 12, 40},
    {"Skeleton", 15, 50}
  };

  int floors = 2;      // Number of dungeon floors
  int mapSize = 5;     // Map size (square grid)

  for (int floor = 1; floor <= floors; ++floor) {
    std::cout << "\n🏰 Floor " << floor << " begins!\n";
    // Generate map and visited tracker
    std::vector<std::vector<char>> dungeon = generateMap(mapSize);
    std::vector<std::vector<bool>> visited(mapSize, std::vector<bool>(mapSize, false));
    hero.x = 0;
    hero.y = 0;
    visited[0][0] = true;
    bool floorCleared = false;
    while (!floorCleared && hero.health > 0) {
      displayMap(dungeon, visited, hero);
      std::cout << "\n💎 Gold: " << hero.gold << " | ❤️ HP: " << hero.health << " | 🧪 Potions: " << hero.potions << "\n";
      std::cout << "Move (W/A/S/D), P to use potion, I to change armor, Q to quit: ";
      char move;
      std::cin >> move;
      move = std::toupper(static_cast<unsigned char>(move));
      if (move == 'Q') { std::cout << "🏁 Game ended.\n"; return EXIT_SUCCESS; }
      if (move == 'P') { usePotion(hero); continue; }
      if (move == 'I') {
        std::cout << "\nInventory:\n";
        for (size_t i = 0; i < hero.inventoryArmors.size(); ++i) {
          std::cout << i+1 << ". " << hero.inventoryArmors[i].name << " (Defense +" << hero.inventoryArmors[i].defense << ")\n";
        }
        std::cout << "Choose armor number: ";
        int idx;
        std::cin >> idx;
        equipArmor(hero, idx - 1);
        continue;
      }
      int newX = hero.x, newY = hero.y;
      if (move == 'W') newX--;
      else if (move == 'S') newX++;
      else if (move == 'A') newY--;
      else if (move == 'D') newY++;
      // Check boundaries
      if (newX < 0 || newX >= mapSize || newY < 0 || newY >= mapSize) { std::cout << "❌ Can't move outside the dungeon!\n"; continue; }
      hero.x = newX;
      hero.y = newY;
      visited[hero.x][hero.y] = true;
      // Explore the room
      if (!exploreRoom(hero, enemies, armors, dungeon[hero.x][hero.y])) { std::cout << "\n💀 Game Over!\n"; return EXIT_SUCCESS; }
      // Check if boss defeated
      bool bossAlive = false;
      for (int i = 0; i < mapSize; ++i) {
        for (int j = 0; j < mapSize; ++j) {
          if (dungeon[i][j] == 'B') bossAlive = true;
        }
      }
      if (!bossAlive) { std::cout << "\n✅ Floor " << floor << " cleared!\n"; floorCleared = true; }
    }
    // Shop between floors
    if (floorCleared && floor < floors) { shop(hero, armors); }
  }
  std::cout << "\n🎉 Congratulations! You defeated all bosses and cleared the dungeon!\n";
  return EXIT_SUCCESS;
}

// Equip armor from inventory
void equipArmor(Hero &hero, int index) {
  if (index < 0 || index >= (int)hero.inventoryArmors.size()) { std::cout << "Invalid armor index.\n"; return; }
  hero.equippedArmor = hero.inventoryArmors[index];
  std::cout << hero.name << " equipped " << hero.equippedArmor.name << " (Defense +" << hero.equippedArmor.defense << ")\n";
}

// Battle simulation
bool battle(Hero &hero, Enemy enemy) {
  std::cout << "\n⚔ Battle starts! " << hero.name << " vs " << enemy.name << "\n";
  while (enemy.health > 0 && hero.health > 0) {
    int totalDefense = hero.baseDefense + hero.equippedArmor.defense;
    int damageToHero = enemy.attack - totalDefense;
    if (damageToHero < 0) damageToHero = 0;
    hero.health -= damageToHero;
    std::cout << enemy.name << " attacks! Damage: " << damageToHero << " | Hero HP: " << hero.health << "\n";
    int heroAttack = 10 + rand() % 11; // Hero attack 10-20
    enemy.health -= heroAttack;
    std::cout << "\033[1;36m" << hero.name << "\033[0;0m" << " strikes! Damage: " << "\033[1;33m" << heroAttack << "\033[0;0m" << " | Enemy HP: " << "\033[1;31m" << enemy.health << "\033[0;0m" << "\n";
  }
  if (hero.health <= 0) { std::cout << "\033[1;31m" << hero.name << " has fallen..." << "\033[0;0m" << "\n"; return false; }
  std::cout << hero.name << " defeated " << enemy.name << "!\n";
  return true;
}

// Loot room
void lootRoom(Hero &hero, std::vector<Armor> &armors) {
  int lootType = rand() % 2;
  if (lootType == 0) {
    hero.potions++;
    std::cout << "🧪 Found a health potion! Potions: " << hero.potions << "\n";
  } else {
    Armor newArmor = armors[std::rand() % armors.size()];
    hero.inventoryArmors.push_back(newArmor);
    std::cout << "🛡 Found new armor: " << newArmor.name << " (Defense +" << newArmor.defense << ")\n";
  }
}

// Use potion
void usePotion(Hero &hero) {
  if (hero.potions > 0) {
    int heal = 20 + rand() % 21;
    hero.health += heal;
    if (hero.health > 100) hero.health = 100;
    hero.potions--;
    std::cout << "Potion used! +" << heal << " HP | Current HP: " << hero.health << "\n";
  } else {
    std::cout << "No potions available!\n";
  }
}

// Trap room
bool trapRoom(Hero &hero) {
  int damage = 5 + rand() % 16; // 5-20 damage
  hero.health -= damage;
  std::cout << "💥 Trap triggered! You take " << damage << " damage. HP: " << hero.health << "\n";
  return hero.health > 0;
}

// Treasure chest room
void chestRoom(Hero &hero, std::vector<Armor> &armors) {
  int rewardType = std::rand() % 2;
  if (rewardType == 0) {
    int goldFound = 10 + std::rand() % 41; // 10-50 gold
    hero.gold += goldFound;
    std::cout << "💰 Found a treasure chest with " << goldFound << " gold! Total gold: " << hero.gold << "\n";
  } else {
    Armor rareArmor = armors[std::rand() % armors.size()];
    hero.inventoryArmors.push_back(rareArmor);
    std::cout << "✨ Found rare armor: " << rareArmor.name << " (Defense +" << rareArmor.defense << ")\n";
  }
}

// Explore room
bool exploreRoom(Hero &hero, std::vector<Enemy> &enemies, std::vector<Armor> &armors, char &cell) {
  if (cell == 'E') {
    Enemy enemy = enemies[std::rand() % enemies.size()];
    cell = '.';
    return battle(hero, enemy);
  } else if (cell == 'L') {
    lootRoom(hero, armors);
    cell = '.';
    return true;
  } else if (cell == 'B') {
    Enemy boss = {"Dungeon Boss", 25, 80};
    cell = '.';
    return battle(hero, boss);
  } else if (cell == 'T') {
    cell = '.';
    return trapRoom(hero);
  } else if (cell == 'C') {
    chestRoom(hero, armors);
    cell = '.';
    return true;
  } else {
    std::cout << "🚪 Empty room.\n";
    return true;
  }
}

// Shop system
void shop(Hero &hero, std::vector<Armor> &armors) {
  std::cout << "\n🏪 Welcome to the shop! You have " << hero.gold << " gold.\n";
  std::cout << "1. Buy Potion (20 gold)\n";
  std::cout << "2. Buy Armor Upgrade (50 gold)\n";
  std::cout << "3. Exit Shop\n";
  int choice;
  std::cin >> choice;
  if (choice == 1) {
    if (hero.gold >= 20) {
      hero.gold -= 20;
      hero.potions++;
      std::cout << "Bought a potion! Potions: " << hero.potions << "\n";
    } else {
      std::cout << "Not enough gold!\n";
    }
  } else if (choice == 2) {
    if (hero.gold >= 50) {
      hero.gold -= 50;
      Armor newArmor = armors[rand() % armors.size()];
      hero.inventoryArmors.push_back(newArmor);
      std::cout << "Bought armor: " << newArmor.name << " (Defense +" << newArmor.defense << ")\n";
    } else { std::cout << "Not enough gold!\n"; }
  } else { std::cout << "Leaving shop...\n"; }
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
   // Place boss in random location
  int bx = std::rand() % size;
  int by = std::rand() % size;
  map[bx][by] = 'B';
  return map;
}

// Display map with fog of war
void displayMap(const std::vector<std::vector<char>> &map, const std::vector<std::vector<bool>> &visited, const Hero &hero) {
  std::cout << "\nDungeon Map:\n";
  for (int i = 0; i < map.size(); ++i) {
    for (int j = 0; j < map[i].size(); ++j) {
      if (i == hero.x && j == hero.y) std::cout << 'H' << ' ';
      else if (visited[i][j]) std::cout << map[i][j] << ' ';
      else std::cout << '?' << ' ';
    }
    std::cout << "\n";
  }
}