#define _CRT_SECURE_NO_WARNINGS
#include "utils.h"
#include <stdio.h>
#include <string.h>

#define HOURS_IN_DAY 24

int readInt(void) {
    int value;
    while (1) {
        if (scanf("%d", &value) == 1) {
            while (getchar() != '\n');
            return value;
        }
        else {
            printf("Ошибка: введено не число! Попробуй снова: ");
            while (getchar() != '\n');
        }
    }
}

void showTime(int day, int hour) {
    printf("На часах: День %d, время: %d:00\n", day, hour);
}

void skipTime(struct GameTime* t) {
    printf("Сколько часов подождать? ");
    int skip = readInt();
    if (skip < 1) {
        printf("Ошибка: Время должно двигаться вперед!\n");
        return;
    }

    int hours_until_deadline = (t->hour >= 8) ? ((24 - t->hour) + 2) : (2 - t->hour);

    if (skip >= hours_until_deadline) {
        printf("Предупреждение: Нельзя мотать время дальше 02:00 ночи без сна!\n");
    }
    else {
        t->hour += skip;
        if (t->hour >= HOURS_IN_DAY) {
            t->day += t->hour / HOURS_IN_DAY;
            t->hour = t->hour % HOURS_IN_DAY;
        }
        printf("Вы подождали. Теперь время: %d:00\n", t->hour);
    }
}

void analyze_logs(void) {
    FILE* in = fopen("./input.txt", "r");
    FILE* out = fopen("./output.txt", "w");
    if (in == NULL || out == NULL) {
        printf("Ошибка работы с файлами логов\n");
        if (in) fclose(in);
        if (out) fclose(out);
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
    printf("Результат сохранен в output.txt\n");
}