#define _CRT_SECURE_NO_WARNINGS
#pragma execution_character_set("utf-8")
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <locale.h>

// Подключаем заголовочные файлы других модулей
#include "player.h"
#include "map.h"
#include "utils.h"

void printMenu(const char* farmer) {
    printf("\n МЕНЮ | Фермер: %s \n", farmer);
    printf("\n1. - Создать / пересоздать персонажа\n");
    printf("2. - Карточка персонажа\n");
    printf("3. - Управление временем\n");
    printf("4. - Посмотреть инвентарь\n");
    printf("5. - Положить предмет в слот\n");
    printf("6. - Выбросить предмет\n");
    printf("7. - Найти предмет в инвентаре\n");
    printf("8. - Записать состояние в дневник фермера\n");
    printf("9. - Анализ лога\n");
    printf("10. - Отправиться в подземелье\n");
    printf("0. - Выход\n");
    printf("\nВыбор: ");
}

void timeMenu(struct Player* p, struct GameTime* t) {
    int choice;
    do {
        printf("\n УПРАВЛЕНИЕ ВРЕМЕНЕМ \n");
        printf("1. - Посмотреть на часы\n");
        printf("2. - Пропустить время (Ждать)\n");
        printf("3. - Отдых (Лечь спать)\n");
        printf("0. - Назад в главное меню\n");
        printf("Выбор: ");

        choice = readInt();
        switch (choice) {
        case 0: break;
        case 1: showTime(t->day, t->hour); break;
        case 2: skipTime(t); break;
        case 3: {
            printf("Вы ложитесь спать...\n");
            t->day += 1; t->hour = 8;
            p->hp = p->max_hp; p->stamina = p->max_stamina;
            p->status.type = EFFECT_HEALTHY; p->status.duration = 0;
            printf("Наступило утро! День %d, время: %d:00\n", t->day, t->hour);
        } break;
        default: printf("Нет такого пункта.\n");
        }
    } while (choice != 0);
}

int main(void) {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    srand((unsigned int)time(NULL));
    load_items();

    struct Player* player = NULL;
    struct GameTime time_state = { 1, 8 };
    bool is_running = true;

    while (is_running) {
        printMenu(player ? player->name : "Персонаж не создан");
        int choice = readInt();
        printf("\n");

        if (choice == 0) {
            printf("Завершение работы программы. До свидания!\n");
            if (player != NULL) { printf("Освобождение памяти по адресу: %p\n", (void*)player); free(player); }
            is_running = false;
            continue;
        }

        if (choice == 1) {
            if (player != NULL) {
                printf("Вы точно хотите удалить текущего героя? (1 - Да, 0 - Нет): ");
                if (readInt() != 1) continue;
                printf("Освобождение памяти по адресу: %p\n", (void*)player);
                free(player); player = NULL;
            }
            player = (struct Player*)malloc(sizeof(struct Player));
            if (player == NULL) exit(1);
            printf("Выделена память по адресу: %p\n", (void*)player);
            player->hp = 100; player->max_hp = 100; player->stamina = 50; player->max_stamina = 50; player->gold = 10;
            player->status.type = EFFECT_HEALTHY; player->status.duration = 0;
            for (int i = 0; i < INVENTORY_SIZE; i++) player->inventory[i] = i;
            printf("Введите имя нового фермера: ");
            if (scanf("%31s", player->name) != 1) strcpy(player->name, "Фермер");
            while (getchar() != '\n');
            continue;
        }

        if (player == NULL) {
            printf("Внимание: Персонаж еще не создан! Выберите пункт 1.\n");
            continue;
        }

        switch (choice) {
        case 2: show_character_card(player); break;
        case 3: timeMenu(player, &time_state); break;
        case 4: print_inventory(player); break;
        case 5: {
            printf("Введите индекс слота (0-9): "); int idx = readInt();
            if (idx >= 0 && idx < INVENTORY_SIZE) {
                printf("Введите ID предмета (0-9): "); int id = readInt();
                if (id >= 0 && id < MAX_ITEMS) { player->inventory[idx] = id; printf("Успешно.\n"); }
            }
            break;
        }
        case 6: {
            printf("Введите индекс слота (0-9): "); int idx = readInt();
            if (idx >= 0 && idx < INVENTORY_SIZE) { player->inventory[idx] = 0; printf("Очищено.\n"); }
            break;
        }
        case 7: search_item(player); break;
        case 8: write_diary(player, time_state.day, time_state.hour); break;
        case 9: analyze_logs(); break;
        case 10:
            if (player->stamina <= 0) printf("Вы слишком устали!\n");
            else if (time_state.hour >= 2 && time_state.hour < 8) printf("Глубокая ночь! Идти нельзя.\n");
            else play_dungeon_game(player, &time_state);
            break;
        default: printf("Ошибка: неверный пункт меню\n"); break;
        }
    }
    return 0;
}