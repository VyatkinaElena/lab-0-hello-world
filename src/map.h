#pragma once
#ifndef MAP_H
#define MAP_H

#include "player.h"
#include "utils.h"
#include "entity.h"

#define MAP_SIZE_MIN 5
#define MAP_SIZE_SPAN 26
#define OBSTACLE_CHANCE_MIN 15
#define OBSTACLE_CHANCE_SPAN 16
#define TRAP_CHANCE 25
#define LADDER_GOLD 10
#define DESCENT_STAMINA_COST 5

enum MapCells {
    WALL = '#', EMPTY = '.', TREE = 'T', ROCK = 'R', HERO = '@',
    LADDER_DOWN = 'D', LADDER_UP = 'U', TRAP_FLOOR = ',', BROKEN_FLOOR = 'X',
    MONSTER = 'M', HEAL = '+'
};

struct Map {
    char** grid;
    int w;
    int h;
    int depth;
    char stood_on;
};

struct Dungeon {
    struct Map map;
    struct Entity* entities;
    int entity_count;
    int resources;
    int kills;
    char last_event[MAX_LINE_LEN];
};

void generate_floor(struct Map* map, struct Player* player);
void print_map(const struct Map* map);
void free_map(struct Map* map);
void play_dungeon_game(struct Player* player, struct GameTime* time_state);

#endif
