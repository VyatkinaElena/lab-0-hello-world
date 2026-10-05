#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <windows.h>

// Константы
#define INVENTORY_SIZE 10
#define HOURS_IN_DAY 24
#define MAX_NAME_LEN 32
#define MAX_LINE_LEN 256
#define MAX_ITEMS 10


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

// Глобальные переменные
char farmer_name[MAX_NAME_LEN] = "Фермер";
char item_names[MAX_ITEMS] [MAX_NAME_LEN];
int inventory[INVENTORY_SIZE] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
int current_day = 1;
int current_hour = 8;

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
        default:
            printf("ошибка: неверный пункт меню\n");
            break;
        }
    }
    return 0;
}