#define _CRT_SECURE_NO_WARNINGS
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>

#define BASE_GOLD_MULTIPLIER 10

enum MapCells {
    WALL = '#', EMPTY = '.', TREE = 'T', ROCK = 'R', HERO = '@',
    LADDER_DOWN = 'D', LADDER_UP = 'U', TRAP_FLOOR = ',', BROKEN_FLOOR = 'X'
};

void allocate_map(struct Map* map) {
    map->grid = (char**)malloc(map->h * sizeof(char*));
    for (int i = 0; i < map->h; i++) {
        map->grid[i] = (char*)malloc(map->w * sizeof(char));
    }
}

void free_map(struct Map* map) {
    if (map->grid != NULL) {
        for (int i = 0; i < map->h; i++) { free(map->grid[i]); }
        free(map->grid);
        map->grid = NULL;
    }
}

void generate_floor(struct Map* map, struct Player* player) {
    free_map(map);
    map->w = 5 + rand() % 26;
    map->h = 5 + rand() % 26;
    allocate_map(map);

    int obstacle_chance = 15 + rand() % 16;
    for (int y = 0; y < map->h; y++) {
        for (int x = 0; x < map->w; x++) {
            if (y == 0 || y == map->h - 1 || x == 0 || x == map->w - 1) {
                map->grid[y][x] = WALL;
            }
            else if (rand() % 100 < obstacle_chance) {
                map->grid[y][x] = (rand() % 2 == 0) ? TREE : ROCK;
            }
            else if ((rand() % 100) < 25) {
                map->grid[y][x] = TRAP_FLOOR;
            }
            else {
                map->grid[y][x] = EMPTY;
            }
        }
    }

    do {
        player->x = 1 + rand() % (map->w - 2);
        player->y = 1 + rand() % (map->h - 2);
    } while (map->grid[player->y][player->x] == WALL || map->grid[player->y][player->x] == TREE || map->grid[player->y][player->x] == ROCK);

    map->stood_on = map->grid[player->y][player->x];
    map->grid[player->y][player->x] = HERO;

    int dx, dy;
    do {
        dx = 1 + rand() % (map->w - 2);
        dy = 1 + rand() % (map->h - 2);
    } while (map->grid[dy][dx] == WALL || map->grid[dy][dx] == TREE || map->grid[dy][dx] == ROCK || map->grid[dy][dx] == HERO);
    map->grid[dy][dx] = LADDER_DOWN;

    if (map->depth > 1) {
        int start_x = player->x; int start_y = player->y;
        if (map->grid[start_y - 1][start_x] == EMPTY || map->grid[start_y - 1][start_x] == TRAP_FLOOR) map->grid[start_y - 1][start_x] = LADDER_UP;
        else if (map->grid[start_y + 1][start_x] == EMPTY || map->grid[start_y + 1][start_x] == TRAP_FLOOR) map->grid[start_y + 1][start_x] = LADDER_UP;
        else map->grid[start_y][start_x - 1] = LADDER_UP;
    }
}

void print_map(const struct Map* map) {
    printf("\n Подземелье (Этаж %d) \n", map->depth);
    for (int y = 0; y < map->h; y++) {
        for (int x = 0; x < map->w; x++) {
            if (map->grid[y][x] == TRAP_FLOOR) printf("%c", EMPTY);
            else if (map->grid[y][x] == BROKEN_FLOOR) printf(",");
            else printf("%c", map->grid[y][x]);
        }
        printf("\n");
    }
}
void move_hero(struct Map* map, struct Player* player, int dx, int dy, struct GameTime* time_state) {
    int nx = player->x + dx; int ny = player->y + dy;
    if (nx < 0 || nx >= map->w || ny < 0 || ny >= map->h) return;

    if (map->grid[ny][nx] == WALL || map->grid[ny][nx] == TREE || map->grid[ny][nx] == ROCK) {
        printf("Там препятствие!\n"); system("pause"); return;
    }

    // Проверяем эффекты с учетом приоритета Бодрости ---
    bool has_vigor = false;
    bool has_sprain = false;
    bool has_poison = false;

    // Сканируем весь стек, чтобы узнать, какие карточки сейчас наложены
    for (int i = 0; i <= player->stack_top; i++) {
        if (player->status_stack[i].type == EFFECT_VIGOR)  has_vigor = true;
        if (player->status_stack[i].type == EFFECT_SPRAIN) has_sprain = true;
        if (player->status_stack[i].type == EFFECT_POISON) has_poison = true;
    }

    int cost = 1;

    // Если у нас АКТИВНА БОДРОСТЬ — она полностью подавляет все дебаффы!
    if (has_vigor) {
        cost = 0; // Шаг бесплатный, усталость не копится
    }
    // Если Бодрости нет, тогда дебаффы работают в обычном режиме:
    else {
        if (has_sprain) {
            cost = 2; // Растяжение удваивает расход сил
        }
        if (has_poison) {
            player->hp -= 2; // Отравление отнимает по 2 HP за шаг
        }
    }

    player->stamina -= cost;

    // Уменьшаем длительность только ТЕКУЩЕГО верхнего эффекта (как в классическом стеке)
    struct Buff* current = get_current_status(player);
    if (current != NULL) {
        current->duration--;
        if (current->duration <= 0) {
            printf("\n[!] Эффект \"%s\" закончился и удален из стека!\n", get_status_name(current->type));
            pop_status(player);
            system("pause");
        }
    }

    // 3. Случайное событие с шансом 5%
    if (rand() % 100 < 40) {
        int roll = rand() % 10; 

        if (roll >= 0 && roll <= 4) {
            push_status(player, EFFECT_SPRAIN, 10);
            printf("\n[!] Ой! Получен эффект \"Растяжение\" на 10 шагов!\n");
        }
        else if (roll >= 5 && roll <= 8) {
            push_status(player, EFFECT_POISON, 10);
            printf("\n[!] Хрусь! Получен эффект \"Отравление\" на 10 шагов!\n");
        }
        else { 
            push_status(player, EFFECT_VIGOR, 10);
            printf("\n[!] Ого! Получен эффект \"Бодрость\" на 10 шагов!\n");
        }
        system("pause");
    }

    map->grid[player->y][player->x] = map->stood_on;
    player->x = nx; player->y = ny; map->stood_on = map->grid[ny][nx];

    if (map->stood_on == LADDER_DOWN || map->stood_on == BROKEN_FLOOR) {
        if (map->stood_on == BROKEN_FLOOR) printf("\nПОЛ КРАХНУЛСЯ! Вы проваливаетесь ниже!\n");
        else printf("\nСпускаемся ниже...\n");

        map->depth++; player->stamina -= 5; time_state->hour += 1;
        if (time_state->hour >= 24) { time_state->day += 1; time_state->hour %= 24; }

        int reward = BASE_GOLD_MULTIPLIER * map->depth; player->gold += reward;
        printf("Новый уровень! Награда: %d золота.\n", reward);
        generate_floor(map, player); system("pause"); return;
    }
    map->grid[player->y][player->x] = HERO;
}


void play_dungeon_game(struct Player* player, struct GameTime* time_state) {
    struct Map dungeon_map; dungeon_map.grid = NULL; dungeon_map.depth = 1;
    generate_floor(&dungeon_map, player);
    char command; int running = 1;

    while (running) {
        if (player->stamina <= 0 || player->hp <= 0 || (time_state->hour >= 2 && time_state->hour < 8)) {
            printf(player->hp <= 0 ? " ЭКСТРЕННАЯ ЭВАКУАЦИЯ: Потеря сознания от Отравления!\n" : " ЭКСТРЕННАЯ ЭВАКУАЦИЯ: Вы упали без сил!\n");
            int fine = player->gold / 2; player->gold -= fine;

            clear_status_stack(player); // Очищаем стек при эвакуации

            if ((time_state->hour >= 2 && time_state->hour < 8) || player->hp <= 0) {
                time_state->day += 1; time_state->hour = 8;
                player->hp = player->max_hp; player->stamina = player->max_stamina;
                printf("Вы очнулись утром следующего дня (День %d, 08:00).\n", time_state->day);
            }
            else { player->stamina = 0; }
            system("pause"); running = 0; break;
        }

        system("cls");
        printf(" HP: %d/%d | Сил: %d/%d | Золото: %d | Этаж: %d | Время: %02d:00\n",
            player->hp, player->max_hp, player->stamina, player->max_stamina, player->gold, dungeon_map.depth, time_state->hour);

        // Сканируем стек для правильного отображения на экране
        bool show_sprain = false; int dur_sprain = 0;
        bool show_poison = false; int dur_poison = 0;
        bool show_vigor = false;  int dur_vigor = 0;

        for (int i = 0; i <= player->stack_top; i++) {
            if (player->status_stack[i].type == EFFECT_SPRAIN) { show_sprain = true; dur_sprain = player->status_stack[i].duration; }
            if (player->status_stack[i].type == EFFECT_POISON) { show_poison = true; dur_poison = player->status_stack[i].duration; }
            if (player->status_stack[i].type == EFFECT_VIGOR) { show_vigor = true;  dur_vigor = player->status_stack[i].duration; }
        }

        printf(" Эффекты: ");
        printf("Растяжение: %s | ", show_sprain ? "АКТИВНО" : "не активно");
        printf("Отравление: %s | ", show_poison ? "АКТИВНО" : "не активно");
        printf("Бодрость: %s\n", show_vigor ? "АКТИВНО" : "не активно");

        struct Buff* top_effect = get_current_status(player);
        if (top_effect != NULL) {
            printf(" %s (%d ш. осталось)\n", get_status_name(top_effect->type), top_effect->duration);
        }

        print_map(&dungeon_map);
        printf("\nХод (W/A/S/D), Q - Выход: ");
        if (scanf(" %c", &command) != 1) { while (getchar() != '\n'); continue; }

        if (command == 'w' || command == 'W') move_hero(&dungeon_map, player, 0, -1, time_state);
        else if (command == 's' || command == 'S') move_hero(&dungeon_map, player, 0, 1, time_state);
        else if (command == 'a' || command == 'A') move_hero(&dungeon_map, player, -1, 0, time_state);
        else if (command == 'd' || command == 'D') move_hero(&dungeon_map, player, 1, 0, time_state);
        else if (command == 'q' || command == 'Q') {
            if (dungeon_map.depth == 1 || dungeon_map.stood_on == LADDER_UP) { printf("Вы покинули подземелье.\n"); system("pause"); running = 0; }
            else { printf("Сбежать можно только с 1-го этажа или у лестницы U!\n"); system("pause"); }
        }
    }
    free_map(&dungeon_map);
}
