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

#define RESETCOLOUR "\033[0;0m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"
#define RED "\033[1;31m"
#define MAGENTA "\033[1;35m"

// Armor structure
struct Armor {
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
};

// Hero structure
struct Hero {
  std::string name;
  int health;
  int baseDefense;
  int baseAttack;
  Armor equippedArmor;
  Weapon equippedWeapon;
  std::vector<Armor> inventoryArmors;
  int potions;
  int gold;
  int x, y;
};

// Equip armor from inventory
void equipArmor(Hero &hero, int index);
// Equip weapon
void equipWeapon(Hero &hero, const Weapon &weapon);
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
// Weapon shop system
void weaponShop(Hero &hero, std::vector<Weapon> &weapons);
// Potion shop system
void potionShop(Hero &hero, std::vector<Potion> &potionsList);
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
  hero.baseAttack = 5;
  hero.equippedArmor = {"None", 0};
  hero.equippedWeapon = {"Fists", 0, 0};
  hero.potions = 0;
  hero.gold = 50;
  hero.x = 0;
  hero.y = 0;
  std::vector<Armor> armors = {
    {"Leather Armor", 5},
    {"Iron Armor", 10},
    {"Magic Armor", 15},
    {"Dragon Armor", 20}
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
    {"Large Potion", 40, 30}
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

  int floors = 3;      // Number of dungeon floors
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
    weaponShop(hero, weapons);
    potionShop(hero, potionsList);
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
      if (!bossAlive) {
        std::cout << "\n✅ Floor " << floor << " cleared!\n"; floorCleared = true;
        // Heal hero slightly after round
        hero.health += 15;
        if (hero.health > 100) hero.health = 100;
        std::cout << hero.name << " rests and recovers to " << hero.health << " HP.\n";
      }
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
  std::cout << "\n⚔ Battle: " << hero.name << " vs " << enemy.name << "\n";
  while (hero.health > 0 && enemy.health > 0) {
    // Hero attacks
    int heroAttack = hero.baseAttack + hero.equippedWeapon.attack;
    if(std::rand() % 100 < 20) { heroAttack *= 2; std::cout << YELLOW <<"CRITICAL HIT!" << RESETCOLOUR << "\n"; }
    enemy.health -= heroAttack;
    std::cout << CYAN << hero.name << RESETCOLOUR << " attacks for " << YELLOW << heroAttack << RESETCOLOUR <<" damage! " << enemy.name << " HP: " << RED << std::max(0, enemy.health) << RESETCOLOUR << "\n";
    if (enemy.health <= 0) {
      std::cout << MAGENTA << enemy.name << RESETCOLOUR << GREEN << " is defeated!" << RESETCOLOUR << "\n";
      hero.gold += 30;
      std::cout << CYAN << hero.name << RESETCOLOUR << " earns 30 gold! Total gold: " << YELLOW << hero.gold << RESETCOLOUR << "\n";
      // Random loot drop
      int lootChance = std::rand() % 3;
      if (lootChance == 0) { hero.potions++; std::cout << YELLOW << "Loot drop: Potion found! Total potions: " << RESETCOLOUR << hero.potions << "\n"; }
      return true;
    }
    // Enemy attacks
    int totalDefense = hero.baseDefense + hero.equippedArmor.defense;
    int damage = enemy.attack - totalDefense;
    if (damage < 0) damage = 0;
    if(std::rand() % 100 < 15) { std::cout << hero.name << " dodged!\n"; damage = 0; }
    hero.health -= damage;
    std::cout << RED << enemy.name << RESETCOLOUR << " attacks for " << RED << enemy.attack << RESETCOLOUR << " (Blocked: " << MAGENTA << totalDefense << RESETCOLOUR << ") Damage taken: " << YELLOW << damage << RESETCOLOUR << " | " << YELLOW << hero.name << RESETCOLOUR << " HP: " << GREEN << std::max(0, hero.health) << RESETCOLOUR << "\n";
    // Auto-use potion if low HP
    if (hero.health <= 30 && hero.potions > 0) { std::cout << CYAN << "⚠ Low HP! Auto-using potion..." << RESETCOLOUR << "\n"; usePotion(hero); }
    if (hero.health <= 0) { std::cout << hero.name << " has fallen...\n"; return false; }
  }
  return false;
}

// Loot room
void lootRoom(Hero &hero, std::vector<Armor> &armors) {
  int lootType = std::rand() % 2;
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
    int heal = 20 + std::rand() % 21;
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
  int damage = 5 + std::rand() % 16; // 5-20 damage
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
      Armor newArmor = armors[std::rand() % armors.size()];
      hero.inventoryArmors.push_back(newArmor);
      std::cout << "Bought armor: " << newArmor.name << " (Defense +" << newArmor.defense << ")\n";
    } else { std::cout << "Not enough gold!\n"; }
  } else { std::cout << "Leaving shop...\n"; }
}

// Weapon shop system
void weaponShop(Hero &hero, std::vector<Weapon> &weapons) {
  std::cout << "\n⚔ Weapon Shop — You have " << hero.gold << " gold.\n";
  for (size_t i = 0; i < weapons.size(); ++i) { std::cout << i+1 << ". " << weapons[i].name << " (Attack +" << weapons[i].attack  << ", Cost: " << weapons[i].cost << " gold)\n"; }
  std::cout << "0. Skip\n";
  int choice;
  std::cout << "Choose weapon: ";
  if (!(std::cin >> choice) || choice < 0 || choice > (int)weapons.size()) {
    std::cin.clear(); std::cin.ignore(1000, '\n');
    std::cout << "Invalid choice. Skipping.\n";
    return;
  }
  if (choice == 0) return;
  Weapon selected = weapons[choice-1];
  if (hero.gold >= selected.cost) {
    hero.gold -= selected.cost;
    equipWeapon(hero, selected);
  } else { std::cout << "Not enough gold!\n"; }
}

// Potion shop
void potionShop(Hero &hero, std::vector<Potion> &potionsList) {
  std::cout << "\n🍵 Potion Shop — You have " << hero.gold << " gold.\n";
  for (size_t i = 0; i < potionsList.size(); ++i) { std::cout << i+1 << ". " << potionsList[i].name << " (Heal +" << potionsList[i].healAmount << ", Cost: " << potionsList[i].cost << " gold)\n"; }
  std::cout << "0. Skip\n";
  int choice;
  std::cout << "Choose potion: ";
  if (!(std::cin >> choice) || choice < 0 || choice > (int)potionsList.size()) { std::cin.clear(); std::cin.ignore(1000, '\n'); std::cout << "Invalid choice. Skipping.\n"; return; }
  if (choice == 0) return;
  Potion selected = potionsList[choice-1];
  if (hero.gold >= selected.cost) {
    hero.gold -= selected.cost;
    hero.potions++;
    std::cout << "Bought " << selected.name << ". You now have " << hero.potions << " potions.\n";
  } else { std::cout << "Not enough gold!\n"; }
}

// Equip weapon
void equipWeapon(Hero &hero, const Weapon &weapon) {
  hero.equippedWeapon = weapon;
  std::cout << hero.name << " equipped " << weapon.name << " (Attack +" << weapon.attack << ")\n";
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