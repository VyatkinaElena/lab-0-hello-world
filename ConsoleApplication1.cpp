#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <windows.h>
#include <time.h>

// Константы
#define INVENTORY_SIZE 10
#define HOURS_IN_DAY 24
#define MAX_NAME_LEN 32
#define MAX_LINE_LEN 256
#define MAX_ITEMS 10
#define WALL '#'
#define EMPTY '.'
#define TREE 'T'
#define ROCK 'R'
#define HERO '@'
#define LADDER_DOWN 'D'
#define LADDER_UP 'U'
#define TRAP_FLOOR ','

// Список ID предметов 
#define ITEM_EMPTY 0
#define ITEM_WOOD 1
#define ITEM_STONE 2
#define ITEM_SEEDS 3
#define ITEM_IRON 4
#define ITEM_GOLD 5
#define ITEM_COAL 6
#define ITEM_DIAMOND 7
#define ITEM_RUBY 8
#define ITEM_EMERALD 9

void play_dungeon_game(void);

// Глобальные переменные
char **map = NULL;
char farmer_name[MAX_NAME_LEN] = "Фермер";
char item_names[MAX_ITEMS] [MAX_NAME_LEN];
int inventory[INVENTORY_SIZE] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
int current_day = 1;
int current_hour = 8;
int hero_x, hero_y;
int depth = 1;
int map_w, map_h;
int start_x, start_y;

// "защита от дурака"
int get_integer_input(const char* promt) {
    int value;
    while (1) {
        printf("%s", promt);
        if (scanf("%d", &value) == 1) {
            while (getchar() != '\n'); // Очистка буфера
            return value;
        }
        else {
            printf("Ошибка: введено не число! Попробуй снова. \n");
            while (getchar() != '\n');
        }
    }
}
//1 Выделение памяти
void allocate_map() {
    map = (char**)malloc(map_h * sizeof(char*));
    for (int i = 0; i < map_h; i++) {
        map[i] = (char*)malloc(map_w * sizeof(char));
    }
}
//2 Освобождение памяти в обратном порядке
void free_map() {
    if (map != NULL) {
        for (int i = 0; i < map_h; i++) {
            free(map[i]);
        }
        free(map);
        map = NULL;
    }
}
// 3. Генерация нового этажа
void generate_floor() {
    free_map(); // Освобождаем старую память при переходе

    // Случайный размер от 5 до 30
    map_w = 5 + rand() % 26;
    map_h = 5 + rand() % 26;
    allocate_map();

    // Заполняем стены и препятствия (15-30%)
    int obstacle_chance = 15 + rand() % 16;
    for (int y = 0; y < map_h; y++) {
        for (int x = 0; x < map_w; x++) {
            if (y == 0 || y == map_h - 1 || x == 0 || x == map_w - 1) {
                map[y][x] = WALL;
            }
            else if (rand() % 100 < obstacle_chance) {
                map[y][x] = (rand() % 2 == 0) ? TREE : ROCK;
            }
            else {
                map[y][x] = EMPTY;
            }
        }
    }

    // Спавн героя
    do {
        hero_x = 1 + rand() % (map_w - 2);
        hero_y = 1 + rand() % (map_h - 2);
    } while (map[hero_y][hero_x] != EMPTY);

    start_x = hero_x;
    start_y = hero_y;
    map[hero_y][hero_x] = HERO;

    // Спавн лестницы вниз D
    int dx, dy;
    do {
        dx = 1 + rand() % (map_w - 2);
        dy = 1 + rand() % (map_h - 2);
    } while (map[dy][dx] != EMPTY);
    map[dy][dx] = LADDER_DOWN;

    // Лестница вверх U (если этаж > 1) рядом со стартом
    if (depth > 1) {
        if (map[start_y - 1][start_x] == EMPTY) map[start_y - 1][start_x] = LADDER_UP;
        else if (map[start_y + 1][start_x] == EMPTY) map[start_y + 1][start_x] = LADDER_UP;
        else map[start_y][start_x - 1] = LADDER_UP;
    }
}
// 4. Отрисовка карты
void print_map() {
    printf("\n--- Этаж %d (%dx%d) ---\n", depth, map_w, map_h);
    for (int y = 0; y < map_h; y++) {
        for (int x = 0; x < map_w; x++) {
            printf("%c", map[y][x]);
        }
        printf("\n");
    }
}

// 5. Логика движения и проверок
void move_hero(int dx, int dy) {
    int next_x = hero_x + dx;
    int next_y = hero_y + dy;

    // Защита от Segfault (выход за границы массива)
    if (next_x < 0 || next_x >= map_w || next_y < 0 || next_y >= map_h) return;

    // Проверка проходимости
    char next_cell = map[next_y][next_x];
    if (next_cell == WALL || next_cell == TREE || next_cell == ROCK) {
        printf("Там препятствие!\n");
        return;
    }

    // Возвращаем старый символ на место, где стоял герой
    if (hero_x == start_x && hero_y == start_y && depth > 1) {
        map[hero_y][hero_x] = LADDER_UP;
    }
    else {
        map[hero_y][hero_x] = EMPTY;
    }

    hero_x = next_x;
    hero_y = next_y;

    // Если наступили на лестницу вниз
    if (next_cell == LADDER_DOWN) {
        printf("\nСпускаемся ниже...\n");
        depth++;
        generate_floor();
    }
    else  if (next_cell == TRAP_FLOOR) {
        printf("\nОсторожно, скрепучий пол (,)! ");
        if (rand() % 2 == 0) { // Проверка вероятности ровно 50%
            printf("Пол ПРОВАЛИВАЕТСЯ! Вы падаете на этаж ниже!\n");
            depth++;
            generate_floor();
        }
        else {
            printf("Пол скрипит.\n");
            map[hero_y][hero_x] = HERO;
        }
    }
    else {
        map[hero_y][hero_x] = HERO;
    }
} 

// функция для получения названия предмета по его ID
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
// Загрузка каталога предметов из файла
void load_items(void) {
        strcpy(item_names[0], "Пусто");
        strcpy(item_names[1], "Дерево");
        strcpy(item_names[2], "Камень");
        strcpy(item_names[3], "Семена");
        strcpy(item_names[4], "Железо");
        strcpy(item_names[5], "Золото");
        strcpy(item_names[6], "Уголь");
        strcpy(item_names[7], "Алмаз");
        strcpy(item_names[8], "Рубин");
        strcpy(item_names[9], "Изумруд");
        FILE *f = fopen("items.txt", "r");
        if (f == NULL) {
            printf("Ошибка: не удалось найти файл!\n");
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
// Вывод инвентаря
void print_inventory(void) {
    printf("\nСодержимое рюкзака\n");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        int id = inventory[i];
        if (id >= 0 && id < MAX_ITEMS) {
            printf("Слот %d: [%d] - %s\n", i, id, item_names[id]);
        }
        else {
            printf("Слот %d: [%d] - Неизвестный предмет\n", i, id);
        }
    }
}
// Поиск предмета в рюкзаке по названию
void search_item(void) {
    char search_name[MAX_NAME_LEN];
    printf("Введите названия предмета для поиска: ");
    if (scanf_s("%31s", search_name) != 1) {
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    // Ищем какому ID соответствует это имя в каталоге
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
    //Проверяем, есть ли этот ID в рюкзаке, выводим номер слота
    int count = 0;
    printf("Предмет найден в слотах: ");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == target_id) {
            printf("%d ", i);
            count++;
        }
    }
    if (count == 0) {
        printf("Нигде не найден.");
    }
    printf("\nВсего включений предмета в рюкзаке: %d\n", count);
}
// Запись состояния в дневник фермера
void write_diary(void) {
    FILE* f = fopen("diary.txt", "a");
    if (f == NULL) {
        printf("Ошибка: не удалось открыть файл!\n");
        return;
    }
    fprintf(f, "Имя фермера: %s\n", farmer_name);
    fprintf(f, "Текущее время: День %d, %02d:00\n", current_day, current_hour);
    fprintf(f, "Рюкзак:\n");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        int id = inventory[i];
        if (id >= 0 && id < MAX_ITEMS) {
            fprintf(f, " Слот %d: %s\n", i, item_names[id]);
        } else {
            fprintf(f, " Слот %d: Неизвестный предмет\n", i);
        }
    }
    fclose(f);
    printf("Состояние успешно записано в diary.txt\n");
}
// Анализатор логов
void analyze_logs(void) {
    FILE* in = fopen("./input.txt", "r");
    FILE *out = fopen("./output.txt", "w");
    if (in == NULL || out == NULL) {
        printf("Ошибка работы с файлами логов\n");
        if (in) fclose(in);
        if (out) fclose(out);
        system("explorer .");
        return;
    }
    char line[MAX_LINE_LEN];
    int line_num = 0;
    int info = 0, warn = 0, error = 0;
    while (fgets(line, sizeof(line), in) != NULL) {
        line_num++;
        if (strncmp(line, "[INFO]", 6) == 0) {
            info++;
        }
        else if (strncmp(line, "[WARN]", 6) == 0) {
            warn++;
            fprintf(out, "№%d: %s", line_num, line + 7);
        }
        else if (strncmp(line, "[ERROR]", 7) == 0) {
            error++;
            fprintf(out, "№%d: %s", line_num, line + 8);
        }
    }
    fclose(in);
    fclose(out);
    printf(" Статистика логов\n");
    printf("Обнаружено сообщений INFO: %d, WARN: %d, ERROR: %d\n", info, warn, error);
    printf("Резултат сохранен\n");
}
// Основная программа
void play_dungeon_game() {
    srand(time(NULL));
    generate_floor();

    char command;
    int running = 1;

    while (running) {
        print_map();
        printf("Ход (W/A/S/D), Q - Выход: ");

        // "Защита от дурака": считываем символ и чистим буфер
        if (scanf(" %c", &command) != 1) {
            while (getchar() != '\n');
            continue;
        }

        if (command == 'w' || command == 'W') move_hero(0, -1);
        else if (command == 's' || command == 'S') move_hero(0, 1);
        else if (command == 'a' || command == 'A') move_hero(-1, 0);
        else if (command == 'd' || command == 'D') move_hero(1, 0);
        else if (command == 'q' || command == 'Q') {
            if (depth == 1 || (hero_x == start_x && hero_y == start_y)) {
                printf("Вы успешно вернулись домой!\n");
                running = 0;
            }
            else {
                printf("Сбежать можно только с 1-го этажа или стоя на лестнице U!\n");
            }
        }
    }
    free_map(); // Чистим память перед выходом
}

 int main(void) {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    load_items();
    int current_day = 1;
    int currenr_hour = 8;

    // Инициализация инвенторя
    int inventory[INVENTORY_SIZE] = { 0 };

    // Разложим предметы в инвентарь
    inventory[0] = ITEM_EMPTY;
    inventory[1] = ITEM_WOOD;
    inventory[2] = ITEM_STONE;
    inventory[3] = ITEM_SEEDS;
    inventory[4] = ITEM_IRON;
    inventory[5] = ITEM_GOLD;
    inventory[6] = ITEM_COAL;
    inventory[7] = ITEM_DIAMOND;
    inventory[8] = ITEM_RUBY;
    inventory[9] = ITEM_EMERALD;

    printf("Введите имя фермера: ");
    char temp_name[MAX_NAME_LEN];
    if (scanf("%31s", farmer_name) != 1) {
        strcpy(farmer_name, "Фермер");
    }
    else {
        OemToCharA(farmer_name, temp_name);
    }
    while (getchar() != '\n');

    bool is_running = true;

    while (is_running) {
        printf("\nДобро пожаловать на ферму, %s!\n", farmer_name);
        printf("[0] Выход\n");
        printf("[1] Посмотреть на часы\n");
        printf("[2] Промотать время (Поработать)\n");
        printf("[3] Посмотреть инвентарь\n");
        printf("[4] Положить предмет в слот\n");
        printf("[5] Выбросить предмет\n");
        printf("[6] ревизия ресурсов\n");
        printf("[7] Поиск предмета в рюкзаке по названию\n");
        printf("[8] Записать состояние в дневник\n");
        printf("[9] Анализатор логов\n");
        printf("[10] Отправимся в подземелье\n");

        int choice = get_integer_input("Выберите пункт меню: ");
        printf("\n");

        switch (choice) {
        case 0:
            printf("Завершение работы программы. До свидания!\n");
            is_running = true;
            break;

        case 1:
            printf("Текущее время: День %d, %02d:00\n", current_day, currenr_hour);
            break;

        case 2: {
            int hours_to_work = get_integer_input("Сколько часов вы поработаете? ");
            if (hours_to_work < 0) {
                printf("Ошибка: количество часов не может быть отрицательным!\n");
            }
            else {
                currenr_hour += hours_to_work;
                if (currenr_hour >= HOURS_IN_DAY) {
                    current_day += currenr_hour / HOURS_IN_DAY;
                    currenr_hour = currenr_hour % HOURS_IN_DAY;
                }
                printf("Вы хорошо поработали. Наступил День %d, время %02d:00.\n", current_day, currenr_hour);
            }
            break;
        }
        case 3:
            print_inventory();
            break;

        case 4: {
            int slot_index = get_integer_input("Введите индекс слота (0-9): ");
            if (slot_index < 0 || slot_index >= INVENTORY_SIZE) {
                printf("Ошибка\n");
            }
            else {
                printf("Доступные: 0-Пусто, 1-Дерево, 2-Камень, 3-Семена, 4-Железо, 5-Золото, 6-Уголь, 7-Алмаз, 8-Рубин, 9-Изумруд\n");
                int item_id = get_integer_input("Введите ID предмета: ");
                if (item_id < 0 || item_id > 9) {
                    printf("Ошибка: ID предмета должен быть от 0 до 9\n");
                }
                else {
                    inventory[slot_index] = item_id;
                    printf("Предмет [%d] (%s) успешно помещен в слот %d.\n", item_id, get_item_name(item_id), slot_index);
                }
            }
            break;
        }
        case 5: {
            int slot_index = get_integer_input("Введите индекс слота для очистки (0-9): ");
            if (slot_index < 0 || slot_index >= INVENTORY_SIZE) {
                printf("Ошибка\n");
            }
            else {
                inventory[slot_index] = ITEM_EMPTY;
                printf("Слот %d очищен.\n", slot_index);
            }
            break;
        }

        case 6: {
            printf("Ревизия ресурсов\n");
            int search_id = get_integer_input("Введите ID предмета для поиска (0-9): ");
            int count = 0;
            printf("Предмет найден в слотах: ");

            for (int i = 0; i < INVENTORY_SIZE; i++) {
                if (inventory[i] == search_id) {
                    printf("%d ", i);
                    count++;
                }
            }
            if (count == 0) {
                printf("не найден");
            }
            printf("\nВсего включений этого предмета в инвентаре: %d\n", count);
            break;
        }
        case 7:
            search_item();
            break;

        case 8:
            write_diary();
            break;

        case 9:
            analyze_logs();
            break;
        case 10:
            play_dungeon_game();
            break;
        default:
            printf("ошибка: неверный пункт меню\n");
            break;
        }
    }
    return 0;
}