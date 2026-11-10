#ifndef PLAYER_H
#define PLAYER_H

#define INVENTORY_SIZE 10
#define MAX_NAME_LEN 32
#define MAX_ITEMS 10
#define STACK_MAX 10 // Максимальная глубина стека эффектов

enum StatusEffect {
    EFFECT_HEALTHY = 0,
    EFFECT_SPRAIN,      // Растяжение
    EFFECT_POISON,      // Отравление
    EFFECT_VIGOR        // Бодрость
};

// Структура одного элемента стека
struct Buff {
    enum StatusEffect type;
    int duration;
};

// Структура игрока
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

    // СТРУКТУРА СТЕКА ЭФФЕКТОВ
    struct Buff status_stack[STACK_MAX];
    int stack_top; // Индекс вершины стека (-1 означает, что стек пуст)
};

extern char item_names[MAX_ITEMS][MAX_NAME_LEN];

// Функции для работы со стеком
void push_status(struct Player* player, enum StatusEffect type, int duration);
void pop_status(struct Player* player);
struct Buff* get_current_status(struct Player* player);
void clear_status_stack(struct Player* player);

const char* get_item_name(int item_id);
const char* get_status_name(enum StatusEffect effect);
void load_items(void);
void print_inventory(const struct Player* player);
void search_item(const struct Player* player);
void write_diary(const struct Player* player, int day, int hour);
void show_character_card(const struct Player* player);

#endif