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
    char words[MAX_WORDS][MAX_LEN];
    int  n;
    int  word_lengths[MAX_WORDS]; // длины слагаемых
    char result[MAX_LEN];
    int  result_len;              // длина результата
    int  max_len;                 // максимальная длина слова
    char letters[26];               // уникальные буквы
    int  nletters;
    int  digit[26];                 // буква -> цифра
    int  used[10];                  // цифра занята?
} Puzzle;

// Разбор строки вида "SEND + MORE = MONEY"
static int parse(const char *s, Puzzle *p) {
    memset(p, 0, sizeof *p);
    for (int i = 0; i < 26; i++) {
    p->digit[i] = -1;
    }
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
    if (p->n < 2 || p->result[0] == '\0' || p->nletters > 10) {
    return 0;
    }

    p->result_len = (int)strlen(p->result);
    p->max_len = p->result_len;

    for (int i = 0; i < p->n; i++) {
        p->word_lengths[i] = (int)strlen(p->words[i]);

        if (p->word_lengths[i] > p->max_len) {
            p->max_len = p->word_lengths[i];
        }
    }

    return 1;
}

// Проверка, является ли буква первой в каком-либо слове
static int is_leading_letter(const Puzzle *p, int idx) {
    for (int i = 0; i < p->n; i++) {
        if (p->words[i][0] - 'A' == idx) return 1;
    }

    return p->result[0] - 'A' == idx;
}

// Переход к следующему столбцу
static int column_search(Puzzle *p, int col, int carry);

// Перебор слагаемых в текущем столбце
static int column_addends(Puzzle *p, int col, int word_idx,
                          unsigned long long sum) {
    if (word_idx == p->n) {
        int result_len = p->result_len;

        // Все разряды результата уже обработаны
        if (col >= result_len) {
            if (sum % 10 != 0) return 0;
            return column_search(p, col + 1, (int)(sum / 10));
        }

        int idx = p->result[result_len - 1 - col] - 'A';
        int expected = (int)(sum % 10);
        int carry = (int)(sum / 10);

        // Цифра результата уже назначена
        if (p->digit[idx] != -1) {
            if (p->digit[idx] != expected) return 0;
            return column_search(p, col + 1, carry);
        }

        // Цифра результата ещё не назначена
        if (p->used[expected]) return 0;
        if (expected == 0 && is_leading_letter(p, idx)) return 0;

        p->digit[idx] = expected;
        p->used[expected] = 1;

        int ok = column_search(p, col + 1, carry);

        if (ok) return 1;

        p->used[expected] = 0;
        p->digit[idx] = -1;

        return 0;
    }

    int len = p->word_lengths[word_idx];

    // В этом слагаемом больше нет цифр
    if (col >= len) {
        return column_addends(p, col, word_idx + 1, sum);
    }

    int idx = p->words[word_idx][len - 1 - col] - 'A';

    // Букве уже назначена цифра
    if (p->digit[idx] != -1) {
        return column_addends(p, col, word_idx + 1,
                              sum + p->digit[idx]);
    }

    // Перебираем цифры для ещё не назначенной буквы
    for (int d = 0; d <= 9; d++) {
        if (p->used[d]) continue;
        if (d == 0 && is_leading_letter(p, idx)) continue;

        p->digit[idx] = d;
        p->used[d] = 1;

        if (column_addends(p, col, word_idx + 1, sum + d)) {
            return 1;
        }

        p->used[d] = 0;
        p->digit[idx] = -1;
    }

    return 0;
}

// Проверяем ребус по столбцам справа налево
static int column_search(Puzzle *p, int col, int carry) {
    if (col >= p->max_len) {
        return carry == 0;
    }

    return column_addends(p, col, 0, (unsigned long long)carry);
}

// Основная функция: ребус -> решённое выражение.
// Возвращает 1 при успехе, 0 если решения нет / ошибка разбора.
int solve_rebus(const char *input, char *out, size_t out_size) {
    Puzzle p;
    if (!parse(input, &p)) return 0;
    if (!column_search(&p, 0, 0)) return 0;

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
        const int repetitions = 10000;
        int ok = 0;

        clock_t t0 = clock();

        for (int j = 0; j < repetitions; j++) {
            ok = solve_rebus(tests[i], out, sizeof out);
        }

        clock_t t1 = clock();

        double ms = 1000.0 * (double)(t1 - t0) /
                    CLOCKS_PER_SEC / repetitions;

        printf("%-40s -> %-36s %9.6f ms\n",
               tests[i], ok ? out : "нет решения", ms);
    }

    return 0;
}
#endif
