#pragma once
#ifndef PLAYER_H
#define PLAYER_H

#define INVENTORY_SIZE 10
#define MAX_NAME_LEN 32
#define MAX_ITEMS 10

enum StatusEffect {
    EFFECT_HEALTHY = 0,
    EFFECT_SPRAIN,
    EFFECT_POISON,
    EFFECT_VIGOR
};

struct Buff {
    enum StatusEffect type;
    int duration;
};

struct Player {
    char name[MAX_NAME_LEN];
    int hp;
    int max_hp;
    int stamina;
    int max_stamina;
    int gold;
    int x;
    int y;
    int inventory[INVENTORY_SIZE];
    struct Buff status;
};

// Глобальный каталог (объявляем через extern, чтобы его видели другие модули)
extern char item_names[MAX_ITEMS][MAX_NAME_LEN];

const char* get_item_name(int item_id);
const char* get_status_name(enum StatusEffect effect);
void load_items(void);
void print_inventory(const struct Player* player);
void search_item(const struct Player* player);
void write_diary(const struct Player* player, int day, int hour);
void show_character_card(const struct Player* player);

#endif