// Лабораторная №2: решатель числовых ребусов.
// Базовая версия: полный перебор без оптимизаций.
// Все проверки (ведущие нули, сумма) выполняются только когда
// всем буквам уже назначены цифры.

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_WORDS 8   // до 7 слагаемых + результат
#define MAX_LEN   32

typedef struct {
    char words[MAX_WORDS][MAX_LEN]; // words[0..n-1] - слагаемые
    int  n;                         // число слагаемых
    char result[MAX_LEN];           // правая часть
    char letters[26];               // уникальные буквы
    int  nletters;
    int  digit[26];                 // буква -> цифра
    int  used[10];                  // цифра занята?
} Puzzle;

// Разбор строки вида "SEND + MORE = MONEY"
static int parse(const char *s, Puzzle *p) {
    memset(p, 0, sizeof *p);
    int seen[26] = {0};
    int after_eq = 0;
    while (*s) {
        if (*s == '=') {
            after_eq = 1;
            s++;
        } else if (isupper((unsigned char)*s)) {
            char buf[MAX_LEN];
            int len = 0;
            while (isupper((unsigned char)*s) && len < MAX_LEN - 1) {
                buf[len++] = *s;
                if (!seen[*s - 'A']) {
                    seen[*s - 'A'] = 1;
                    p->letters[p->nletters++] = *s;
                }
                s++;
            }
            buf[len] = '\0';
            if (after_eq) {
                strcpy(p->result, buf);
            } else {
                if (p->n >= MAX_WORDS - 1) return 0;
                strcpy(p->words[p->n++], buf);
            }
        } else {
            s++; // пробелы и '+'
        }
    }
    return p->n >= 2 && p->result[0] != '\0' && p->nletters <= 10;
}

static unsigned long long word_value(const Puzzle *p, const char *w) {
    unsigned long long v = 0;
    for (; *w; w++) v = v * 10 + p->digit[*w - 'A'];
    return v;
}

// Проверка полностью назначенных цифр
static int check(const Puzzle *p) {
    // ведущие нули не допускаются
    for (int i = 0; i < p->n; i++)
        if (p->digit[p->words[i][0] - 'A'] == 0) return 0;
    if (p->digit[p->result[0] - 'A'] == 0) return 0;

    unsigned long long sum = 0;
    for (int i = 0; i < p->n; i++) sum += word_value(p, p->words[i]);
    return sum == word_value(p, p->result);
}

// Перебор всех назначений цифр буквам
static int search(Puzzle *p, int pos) {
    if (pos == p->nletters) return check(p);
    int idx = p->letters[pos] - 'A';
    for (int d = 0; d <= 9; d++) {
        if (p->used[d]) continue;
        p->used[d] = 1;
        p->digit[idx] = d;
        if (search(p, pos + 1)) return 1;
        p->used[d] = 0;
    }
    return 0;
}

// Основная функция: ребус -> решённое выражение.
// Возвращает 1 при успехе, 0 если решения нет / ошибка разбора.
int solve_rebus(const char *input, char *out, size_t out_size) {
    Puzzle p;
    if (!parse(input, &p)) return 0;
    if (!search(&p, 0)) return 0;

    size_t k = 0;
    for (const char *s = input; *s && k + 1 < out_size; s++)
        out[k++] = isupper((unsigned char)*s) ? (char)('0' + p.digit[*s - 'A']) : *s;
    out[k] = '\0';
    return 1;
}

#ifndef NO_MAIN
int main(void) {
    const char *tests[] = {
        "SEND + MORE = MONEY",
        "BE + BE = MOO",
        "BIG + CAT = LION",
        "ELEVEN + NINE + FIVE + FIVE = THIRTY",
    };
    for (size_t i = 0; i < sizeof tests / sizeof *tests; i++) {
        char out[128];
        clock_t t0 = clock();

    int ok = solve_rebus(tests[i], out, sizeof out);

    clock_t t1 = clock();

    double ms = (double)(t1 - t0) * 1000.0
          / CLOCKS_PER_SEC;
        printf("%-40s -> %-36s %9.3f ms\n", tests[i], ok ? out : "нет решения", ms);
    }
    return 0;
}
#endif
