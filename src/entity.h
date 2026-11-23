#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include "player.h"
#include "utils.h"

#define ACTION_STAMINA_COST 2      // выносливость за активное действие
#define MIN_DAMAGE 1               // минимальный урон
#define DIFFICULTY_PERCENT 15      // рост силы врагов с каждым этажом, %
#define MONSTER_PERCENT 5          // монстры, % от площади карты
#define HEAL_PERCENT 3             // аптечки, % от площади карты
#define MIN_MONSTERS 2
#define MIN_HEALS 1
#define TREE_BASE_HP 10
#define ROCK_BASE_HP 20
#define MONSTER_BASE_HP 10
#define MONSTER_BASE_ATTACK 4
#define MONSTER_BASE_DEFENSE 1
#define MONSTER_GOLD_REWARD 15
#define TREASURE_CHANCE 30         // шанс трофея, %
#define HEAL_HP 30
#define HEAL_STAMINA 20
#define ITEM_TREE 1
#define ITEM_ROCK 2

enum EntityType {
    ENT_TREE,
    ENT_ROCK,
    ENT_HEAL,
    ENT_MONSTER
};

struct Entity {
    enum EntityType type;
    char symbol;
    int x, y;
    int hp, max_hp;
    int attack;
    int defense;
    int drop_item_id;
    int is_alive;
};

//объявления структур из map.h 
struct Map;
struct Dungeon;

struct Entity* create_entities(struct Map* map, struct Player* player, int* out_count);
void free_entities(struct Entity* entities, int count);
struct Entity* find_entity_at(struct Entity* entities, int count, int x, int y);
int calc_damage(int attack, int defense);
void player_interact(struct Dungeon* dungeon, struct Player* player, struct Entity* ent);
void fight_monster(struct Dungeon* dungeon, struct Player* player, struct Entity* monster);
int move_monsters(struct Dungeon* dungeon, struct Player* player);
void write_dungeon_diary(const struct Dungeon* dungeon, int day, int hour, const char* reason);

#endif