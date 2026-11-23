#define _CRT_SECURE_NO_WARNINGS
#include "entity.h"
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Множитель сложности: base * (1 + (глубина-1) * 15%)
static int scale_by_depth(int base, int depth) {
    return base * (100 + (depth - 1) * DIFFICULTY_PERCENT) / 100;
}

// Урон = Атака - (Защита / 2)
int calc_damage(int attack, int defense) {
    int dmg = attack - defense / 2;
    if (dmg < MIN_DAMAGE) dmg = MIN_DAMAGE;
    return dmg;
}

// Поиск случайной пустой клетки 
static int random_empty_cell(const struct Map* map, int* px, int* py) {
    for (int attempt = 0; attempt < 200; attempt++) {
        int x = 1 + rand() % (map->w - 2);
        int y = 1 + rand() % (map->h - 2);
        if (map->grid[y][x] == EMPTY) {
            *px = x;
            *py = y;
            return 1;
        }
    }
    return 0;
}

// Генерация массива сущностей для нового этажа
struct Entity* create_entities(struct Map* map, struct Player* player, int* out_count) {
    (void)player;

    // 1. Считаем уже размещённые генерацией ресурсы (T и R)
    int res_count = 0;
    for (int y = 0; y < map->h; y++) {
        for (int x = 0; x < map->w; x++) {
            if (map->grid[y][x] == TREE || map->grid[y][x] == ROCK) res_count++;
        }
    }

    // 2. Монстры и аптечки — в процентах от площади этажа
    int area = (map->w - 2) * (map->h - 2);
    if (area < 1) area = 1;
    int monsters = area * MONSTER_PERCENT / 100;
    int heals = area * HEAL_PERCENT / 100;
    if (monsters < MIN_MONSTERS) monsters = MIN_MONSTERS;
    if (heals < MIN_HEALS) heals = MIN_HEALS;

    int total = res_count + monsters + heals;

    // 3. Динамическое выделение памяти
    struct Entity* arr = (struct Entity*)malloc(total * sizeof(struct Entity));
    if (arr == NULL) {
        printf("Ошибка: не удалось выделить память под сущности!\n");
        *out_count = 0;
        return NULL;
    }
    printf("Выделена память под массив сущностей по адресу: %p\n", (void*)arr);

    int idx = 0;

    // 4. Ресурсы: прочность растёт с глубиной
    for (int y = 0; y < map->h; y++) {
        for (int x = 0; x < map->w; x++) {
            char c = map->grid[y][x];
            if (c != TREE && c != ROCK) continue;
            arr[idx].type = (c == TREE) ? ENT_TREE : ENT_ROCK;
            arr[idx].symbol = c;
            arr[idx].x = x;
            arr[idx].y = y;
            arr[idx].max_hp = scale_by_depth((c == TREE) ? TREE_BASE_HP : ROCK_BASE_HP, map->depth);
            arr[idx].hp = arr[idx].max_hp;
            arr[idx].attack = 0;
            arr[idx].defense = 0;
            arr[idx].drop_item_id = (c == TREE) ? ITEM_TREE : ITEM_ROCK;
            arr[idx].is_alive = 1;
            idx++;
        }
    }

    // 5. Монстры: hp, атака и защита тоже растут с глубиной
    for (int i = 0; i < monsters; i++) {
        int px, py;
        if (!random_empty_cell(map, &px, &py)) break;
        arr[idx].type = ENT_MONSTER;
        arr[idx].symbol = MONSTER;
        arr[idx].x = px;
        arr[idx].y = py;
        arr[idx].max_hp = scale_by_depth(MONSTER_BASE_HP, map->depth);
        arr[idx].hp = arr[idx].max_hp;
        arr[idx].attack = scale_by_depth(MONSTER_BASE_ATTACK, map->depth);
        arr[idx].defense = scale_by_depth(MONSTER_BASE_DEFENSE, map->depth);
        arr[idx].drop_item_id = 0;
        arr[idx].is_alive = 1;
        map->grid[py][px] = MONSTER;
        idx++;
    }

    // 6. Целебные кусты
    for (int i = 0; i < heals; i++) {
        int px, py;
        if (!random_empty_cell(map, &px, &py)) break;
        arr[idx].type = ENT_HEAL;
        arr[idx].symbol = HEAL;
        arr[idx].x = px;
        arr[idx].y = py;
        arr[idx].max_hp = 0;
        arr[idx].hp = 0;
        arr[idx].attack = 0;
        arr[idx].defense = 0;
        arr[idx].drop_item_id = 0;
        arr[idx].is_alive = 1;
        map->grid[py][px] = HEAL;
        idx++;
    }

    *out_count = idx;
    return arr;
}

void free_entities(struct Entity* entities, int count) {
    (void)count;
    if (entities != NULL) {
        printf("Память массива сущностей по адресу %p освобождена\n", (void*)entities);
        free(entities);
    }
}

struct Entity* find_entity_at(struct Entity* entities, int count, int x, int y) {
    if (entities == NULL) return NULL;
    for (int i = 0; i < count; i++) {
        if (entities[i].is_alive && entities[i].x == x && entities[i].y == y) {
            return &entities[i];
        }
    }
    return NULL;
}

void fight_monster(struct Dungeon* dungeon, struct Player* player, struct Entity* ent) {
    struct Map* map = &dungeon->map;

    int pdmg = player->attack - ent->defense;
    if (pdmg < 1) pdmg = 1;
    ent->hp -= pdmg;
    printf("Вы ударили монстра: -%d HP (у монстра %d/%d)\n",
        pdmg, ent->hp > 0 ? ent->hp : 0, ent->max_hp);

    if (ent->hp <= 0) {
        ent->is_alive = 0;
        map->grid[ent->y][ent->x] = EMPTY;
        int reward = MONSTER_GOLD_REWARD * map->depth;
        player->gold += reward;
        dungeon->kills++;
        printf("Монстр повержен! Награда: %d золота.\n", reward);
        return;
    }

    int mdmg = ent->attack - player->defense;
    if (mdmg < 1) mdmg = 1;
    player->hp -= mdmg;
    printf("Монстр ударил вас: -%d HP (у вас %d/%d)\n",
        mdmg, player->hp, player->max_hp);
}

// Взаимодействие с сущностью на целевой клетке
void player_interact(struct Dungeon* dungeon, struct Player* player, struct Entity* ent) {
    struct Map* map = &dungeon->map;
    char* last_event = dungeon->last_event;

    // Активное действие стоит выносливости
    player->stamina -= ACTION_STAMINA_COST;

    switch (ent->type) {

    case ENT_TREE:
    case ENT_ROCK: {
        int dmg = calc_damage(player->attack, 0);
        ent->hp -= dmg;
        if (ent->hp <= 0) {
            // Ресурс уничтожен
            map->grid[ent->y][ent->x] = EMPTY;
            ent->is_alive = 0;
            dungeon->resources++;
            int slot = find_free_slot(player);
            if (slot == -1) {
                printf("Рюкзак полон! Ресурс пропал.\n");
                snprintf(last_event, MAX_LINE_LEN, "Рюкзак полон! Ресурс пропал.");
            }
            else {
                player->inventory[slot] = ent->drop_item_id;
                printf("Вы разрубили объект и получили: %s!\n", get_item_name(ent->drop_item_id));
                snprintf(last_event, MAX_LINE_LEN, "Добыт ресурс: %s!", get_item_name(ent->drop_item_id));
            }
        }
        else {
            printf("Вы добываете ресурс: прочность %d/%d.\n", ent->hp, ent->max_hp);
            snprintf(last_event, MAX_LINE_LEN, "Удар по ресурсу: -%d прочности!", dmg);
        }
        system("pause");
        break;
    }

    case ENT_HEAL: {
        player->hp += HEAL_HP;
        if (player->hp > player->max_hp) player->hp = player->max_hp;
        player->stamina += HEAL_STAMINA;
        if (player->stamina > player->max_stamina) player->stamina = player->max_stamina;

        // Куст исчезает, герой перемещается на его клетку
        map->grid[ent->y][ent->x] = EMPTY;
        ent->is_alive = 0;
        map->grid[player->y][player->x] = map->stood_on;
        player->x = ent->x;
        player->y = ent->y;
        map->stood_on = EMPTY;
        map->grid[player->y][player->x] = HERO;

        printf("Вы восстановили силы: +%d HP, +%d выносливости!\n", HEAL_HP, HEAL_STAMINA);
        snprintf(last_event, MAX_LINE_LEN, "Съеден целебный куст: +HP и +выносливость!");
        system("pause");
        break;
    }

    case ENT_MONSTER:
        fight_monster(dungeon, player, ent);
        system("pause");
        break;
    }
}

// ВАРИАНТ 3: блуждающие хищники.
int move_monsters(struct Dungeon* dungeon, struct Player* player) {
    struct Map* map = &dungeon->map;
    if (player->hp <= 0) return 1;

    for (int i = 0; i < dungeon->entity_count; i++) {
        struct Entity* m = &dungeon->entities[i];
        if (!m->is_alive || m->type != ENT_MONSTER) continue;

        int dx = 0, dy = 0;
        int dir = rand() % 4;
        if (dir == 0) dy = -1;
        else if (dir == 1) dy = 1;
        else if (dir == 2) dx = -1;
        else dx = 1;

        int nx = m->x + dx;
        int ny = m->y + dy;
        if (nx <= 0 || nx >= map->w - 1 || ny <= 0 || ny >= map->h - 1) continue;

        // Монстр настигает героя — бьёт первым
        if (nx == player->x && ny == player->y) {
            int dmg = calc_damage(m->attack, player->defense);
            player->hp -= dmg;
            printf("Монстр напал первым: -%d HP (осталось %d/%d)!\n",
                dmg, player->hp > 0 ? player->hp : 0, player->max_hp);
            snprintf(dungeon->last_event, MAX_LINE_LEN, "Монстр напал первым: -%d HP!", dmg);
            continue;
        }

        // Шаг только на пустую клетку и туда, где нет другой сущности
        if (map->grid[ny][nx] != EMPTY) continue;
        if (find_entity_at(dungeon->entities, dungeon->entity_count, nx, ny) != NULL) continue;

        map->grid[m->y][m->x] = EMPTY;
        m->x = nx;
        m->y = ny;
        map->grid[ny][nx] = MONSTER;
    }
    return player->hp <= 0;
}

// Дозапись отчёта о вылазке в diary.txt (режим "a")
void write_dungeon_diary(const struct Dungeon* dungeon, int day, int hour, const char* reason) {
    FILE* f = fopen("diary.txt", "a");
    if (f == NULL) {
        printf("Ошибка: не удалось открыть diary.txt!\n");
        return;
    }
    fprintf(f, "=== Поход завершён: %s ===\n", reason);
    fprintf(f, "День %d, время %02d:00, этаж: %d\n", day, hour, dungeon->map.depth);
    fprintf(f, "Добыто ресурсов: %d, побеждено монстров: %d\n", dungeon->resources, dungeon->kills);
    fprintf(f, "\n");
    fclose(f);
    printf("Отчёт о вылазке дозаписан в diary.txt\n");
}