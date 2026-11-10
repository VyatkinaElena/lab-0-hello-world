#pragma once
#ifndef MAP_H
#define MAP_H

#include "player.h"
#include "utils.h"

struct Map {
    char** grid;
    int w;
    int h;
    int depth;
    char stood_on;
};

void play_dungeon_game(struct Player* player, struct GameTime* time_state);

#endif