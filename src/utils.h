#pragma once
#ifndef UTILS_H
#define UTILS_H

#define MAX_LINE_LEN 256

struct GameTime {
    int day;
    int hour;
};

int readInt(void);
void showTime(int day, int hour);
void skipTime(struct GameTime* t);
void analyze_logs(void);

#endif