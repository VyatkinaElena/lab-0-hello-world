#define _CRT_SECURE_NO_WARNINGS
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdbool.h>

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
// Один ход героя. Возвращает 1, если герой перешёл на новый этаж.
int move_hero(struct Dungeon* dungeon, struct Player* player, int dx, int dy, struct GameTime* time_state) {
    struct Map* map = &dungeon->map;
    int nx = player->x + dx;
    int ny = player->y + dy;

    if (nx < 0 || nx >= map->w || ny < 0 || ny >= map->h) return 0;

    // Фаза Update: на целевой клетке живая сущность — взаимодействие вместо шага
    struct Entity* ent = find_entity_at(dungeon->entities, dungeon->entity_count, nx, ny);
    if (ent != NULL) {
        player_interact(dungeon, player, ent);
        return 0;
    }

    if (map->grid[ny][nx] == WALL || map->grid[ny][nx] == TREE || map->grid[ny][nx] == ROCK) {
        printf("Там препятствие!\n");
        system("pause");
        return 0;
    }

    // Эффекты статуса 
    bool has_vigor = false;
    bool has_sprain = false;
    bool has_poison = false;
    for (int i = 0; i <= player->stack_top; i++) {
        if (player->status_stack[i].type == EFFECT_VIGOR) has_vigor = true;
        if (player->status_stack[i].type == EFFECT_SPRAIN) has_sprain = true;
        if (player->status_stack[i].type == EFFECT_POISON) has_poison = true;
    }

    int cost = 1;
    if (has_vigor) {
        cost = 0;
    }
    else {
        if (has_sprain) cost = 2;
        if (has_poison) player->hp -= 2;
    }
    player->stamina -= cost;

    struct Buff* current = get_current_status(player);
    if (current != NULL) {
        current->duration--;
        if (current->duration <= 0) {
            printf("\n[!] Эффект \"%s\" закончился и удален из стека!\n", get_status_name(current->type));
            pop_status(player);
            system("pause");
        }
    }

    // Случайное событие
    if (rand() % 100 < 40) {
        int roll = rand() % 10;
        if (roll >= 0 && roll <= 4) {
            push_status(player, EFFECT_SPRAIN, 10);
            printf("\n[!] Получен эффект \"Растяжение\" на 10 шагов!\n");
        }
        else if (roll >= 5 && roll <= 8) {
            push_status(player, EFFECT_POISON, 10);
            printf("\n[!] Получен эффект \"Отравление\" на 10 шагов!\n");
        }
        else {
            push_status(player, EFFECT_VIGOR, 10);
            printf("\n[!] Получен эффект \"Бодрость\" на 10 шагов!\n");
        }
        system("pause");
    }

    map->grid[player->y][player->x] = map->stood_on;
    player->x = nx;
    player->y = ny;
    map->stood_on = map->grid[ny][nx];

    if (map->stood_on == LADDER_DOWN || map->stood_on == BROKEN_FLOOR) {
        if (map->stood_on == BROKEN_FLOOR)
            printf("\nПОЛ КРАХНУЛСЯ! Вы проваливаетесь ниже!\n");
        else
            printf("\nСпускаемся ниже...\n");
        map->depth++;
        player->stamina -= DESCENT_STAMINA_COST;
        time_state->hour += 1;
        if (time_state->hour >= HOURS_IN_DAY) {
            time_state->day += 1;
            time_state->hour %= HOURS_IN_DAY;
        }
        int reward = LADDER_GOLD * map->depth;
        player->gold += reward;
        printf("Новый уровень! Награда: %d золота.\n", reward);
        return 1;
    }

    map->grid[player->y][player->x] = HERO;
    return 0;
}

// Режим приключения: цикл Ввод -> Обновление -> Отрисовка
void play_dungeon_game(struct Player* player, struct GameTime* time_state) {
    struct Dungeon dungeon;
    dungeon.map.grid = NULL;
    dungeon.map.depth = 1;
    dungeon.entities = NULL;
    dungeon.entity_count = 0;
    dungeon.resources = 0;
    dungeon.kills = 0;
    strcpy(dungeon.last_event, "Вы вошли в подземелье!");

    generate_floor(&dungeon.map, player);
    dungeon.entities = create_entities(&dungeon.map, player, &dungeon.entity_count);

    char command;
    int running = 1;
    const char* end_reason = NULL;

    while (running) {
        // Экстренная эвакуация (обморок)
        if (player->stamina <= 0 || player->hp <= 0 ||
            (time_state->hour >= 2 && time_state->hour < 8)) {
            if (player->hp <= 0)
                printf("ЭКСТРЕННАЯ ЭВАКУАЦИЯ: Вы потеряли сознание в бою!\n");
            else if (player->stamina <= 0)
                printf("ЭКСТРЕННАЯ ЭВАКУАЦИЯ: Вы упали без сил!\n");
            else
                printf("ЭКСТРЕННАЯ ЭВАКУАЦИЯ: Глубокая ночь — спасатели унесли вас!\n");
            end_reason = (player->hp <= 0) ? "обморок" : "эвакуация";

            int fine = player->gold / 2;
            player->gold -= fine;
            printf("Спасатели забрали %d золота.\n", fine);
            clear_status_stack(player);

            if (player->hp <= 0 || (time_state->hour >= 2 && time_state->hour < 8)) {
                time_state->day += 1;
                time_state->hour = 8;
                player->hp = player->max_hp;
                player->stamina = player->max_stamina;
                printf("Вы очнулись утром следующего дня (День %d, 08:00).\n", time_state->day);
            }
            else {
                player->stamina = 0;
            }
            system("pause");
            running = 0;
            break;
        }
        //Render(отрисовка)
        system("cls");
        printf(" HP: %d/%d | Силы: %d/%d | Атака: %d | Защита: %d | Золото: %d | Этаж: %d | Время: %02d:00\n",
            player->hp, player->max_hp, player->stamina, player->max_stamina,
            player->attack, player->defense, player->gold, dungeon.map.depth, time_state->hour);

        bool show_sprain = false; int dur_sprain = 0;
        bool show_poison = false; int dur_poison = 0;
        bool show_vigor = false;  int dur_vigor = 0;
        for (int i = 0; i <= player->stack_top; i++) {
            if (player->status_stack[i].type == EFFECT_SPRAIN) { show_sprain = true; dur_sprain = player->status_stack[i].duration; }
            if (player->status_stack[i].type == EFFECT_POISON) { show_poison = true; dur_poison = player->status_stack[i].duration; }
            if (player->status_stack[i].type == EFFECT_VIGOR) { show_vigor = true;  dur_vigor = player->status_stack[i].duration; }
        }
        printf("Эффекты: ");
        printf("Растяжение: %s (%d ш.) | ", show_sprain ? "АКТИВНО" : "нет", dur_sprain);
        printf("Отравление: %s (%d ш.) | ", show_poison ? "АКТИВНО" : "нет", dur_poison);
        printf("Бодрость: %s (%d ш.)\n", show_vigor ? "АКТИВНО" : "нет", dur_vigor);

        print_map(&dungeon.map);
        printf("Последнее событие: %s\n", dungeon.last_event);
        printf("\nХод (W/A/S/D), Q - Выход: ");

        // Фаза Input
        if (scanf(" %c", &command) != 1) {
            while (getchar() != '\n');
            continue;
        }

        int need_new_floor = 0;
        if (command == 'w' || command == 'W') need_new_floor = move_hero(&dungeon, player, 0, -1, time_state);
        else if (command == 's' || command == 'S') need_new_floor = move_hero(&dungeon, player, 0, 1, time_state);
        else if (command == 'a' || command == 'A') need_new_floor = move_hero(&dungeon, player, -1, 0, time_state);
        else if (command == 'd' || command == 'D') need_new_floor = move_hero(&dungeon, player, 1, 0, time_state);
        else if (command == 'q' || command == 'Q') {
            if (dungeon.map.depth == 1 || dungeon.map.stood_on == LADDER_UP) {
                printf("Вы покинули подземелье.\n");
                end_reason = "штатный выход";
                system("pause");
                running = 0;
            }
            else {
                printf("Сбежать можно только с 1-го этажа или у лестницы U!\n");
                system("pause");
            }
            continue;
        }
        else {
            continue;
        }

        // Фаза Update: смена этажа или блуждание монстров
        if (need_new_floor) {
            free_entities(dungeon.entities, dungeon.entity_count);
            dungeon.entities = NULL;
            dungeon.entity_count = 0;
            dungeon.resources = 0;
            dungeon.kills = 0;
            generate_floor(&dungeon.map, player);
            dungeon.entities = create_entities(&dungeon.map, player, &dungeon.entity_count);
            strcpy(dungeon.last_event, "Спуск на новый этаж!");
            system("pause");
        }
        else {
            move_monsters(&dungeon, player);
        }
    }

    // Журнал походов: дозапись в diary.txt при любом завершении
    write_dungeon_diary(&dungeon, time_state->day, time_state->hour,
        end_reason ? end_reason : "выход");
    free_entities(dungeon.entities, dungeon.entity_count);
    free_map(&dungeon.map);
}