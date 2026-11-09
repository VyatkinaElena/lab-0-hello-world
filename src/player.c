#define _CRT_SECURE_NO_WARNINGS
#include "player.h"
#include <stdio.h>
#include <string.h>

enum ItemID {
    ITEM_EMPTY = 0, ITEM_WOOD, ITEM_STONE, ITEM_SEEDS, ITEM_IRON,
    ITEM_GOLD, ITEM_COAL, ITEM_DIAMOND, ITEM_RUBY, ITEM_EMERALD
};

char item_names[MAX_ITEMS][MAX_NAME_LEN];

const char* get_item_name(int item_id) {
    switch (item_id) {
    case ITEM_EMPTY: return "Пусто";
    case ITEM_WOOD: return "Дерево";
    case ITEM_STONE: return "Камень";
    case ITEM_SEEDS: return "Семена";
    case ITEM_IRON: return "Железо";
    case ITEM_GOLD: return "Золото";
    case ITEM_COAL: return "Уголь";
    case ITEM_DIAMOND: return "Алмаз";
    case ITEM_RUBY: return "Рубин";
    case ITEM_EMERALD: return "Изумруд";
    default: return "Неизвестный предмет";
    }
}

const char* get_status_name(enum StatusEffect effect) {
    switch (effect) {
    case EFFECT_HEALTHY: return "Здоров";
    case EFFECT_SPRAIN:  return "Растяжение";
    case EFFECT_POISON:  return "Отравление";
    case EFFECT_VIGOR:   return "Бодрость";
    default:             return "Неизвестно";
    }
}

void load_items(void) {
    FILE* f = fopen("items.txt", "r");
    if (f == NULL) {
        for (int i = 0; i < MAX_ITEMS; i++) {
            strcpy(item_names[i], get_item_name(i));
        }
        return;
    }
    int id;
    char name[MAX_NAME_LEN];
    while (fscanf(f, "%d %31s", &id, name) == 2) {
        if (id >= 0 && id < MAX_ITEMS) {
            strcpy(item_names[id], name);
        }
    }
    fclose(f);
}

void print_inventory(const struct Player* player) {
    printf(" Содержимое рюкзака \n");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        int id = player->inventory[i];
        if (id >= 0 && id < MAX_ITEMS) {
            printf("  Слот %d: [%d] - %s\n", i, id, item_names[id]);
        }
        else {
            printf("  Слот %d: [%d] - Неизвестный предмет\n", i, id);
        }
    }
}

void search_item(const struct Player* player) {
    char search_name[MAX_NAME_LEN];
    printf("Введите название предмета для поиска: ");
    if (scanf("%31s", search_name) != 1) { return; }
    while (getchar() != '\n');

    int target_id = -1;
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(item_names[i], search_name) == 0) {
            target_id = i;
            break;
        }
    }
    if (target_id == -1) {
        printf("Такого предмета нет в каталоге!\n");
        return;
    }

    int count = 0;
    printf("Предмет найден в слотах: ");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i] == target_id) {
            printf("%d ", i);
            count++;
        }
    }
    if (count == 0) printf("Нигде не найден.");
    printf("\nВсего включений предмета в рюкзаке: %d\n", count);
}

void write_diary(const struct Player* player, int day, int hour) {
    FILE* f = fopen("diary.txt", "a");
    if (f == NULL) {
        printf("Ошибка: не удалось открыть файл!\n");
        return;
    }
    fprintf(f, "Имя фермера: %s\n", player->name);
    fprintf(f, "Текущее время: День %d, %02d:00\n", day, hour);
    fprintf(f, "Рюкзак:\n");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        int id = player->inventory[i];
        if (id >= 0 && id < MAX_ITEMS) {
            fprintf(f, " Слот %d: %s\n", i, item_names[id]);
        }
        else {
            fprintf(f, " Слот %d: Неизвестный предмет\n", i);
        }
    }
    fclose(f);
    printf("Состояние успешно записано в diary.txt\n");
}

void show_character_card(const struct Player* player) {
    printf("       КАРТОЧКА ПЕРСОНАЖА: %s\n", player->name);
    printf("  Здоровье (HP):  %d / %d\n", player->hp, player->max_hp);
    printf("  Выносливость:   %d / %d\n", player->stamina, player->max_stamina);
    printf("  Золото:         %d монет\n", player->gold);
    printf("  Текущий статус: %s\n", get_status_name(player->status.type));
}
