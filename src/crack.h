/*
 * crack.h — злам Енігми тільки по шифротексту.            Відповідальний: C
 *
 * Метод: IoC + n-грами (Gillogly, Weierud–Sullivan).
 *   1. Ротори і стартові позиції: перебір за IoC, ринги AAA, без плагборду.
 *   2. Ринги правого і середнього ротора для найкращих кандидатів.
 *   3. Плагборд: hill-climbing (IoC -> біграми -> триграми).
 *
 * Перед викликом crack() треба завантажити біграми і триграми (ngram_load).
 */
#ifndef CRACK_H
#define CRACK_H

#include "enigma.h"

typedef struct {
    int top_n;             /* скільки кандидатів лишати після кроку 1 */
    int restarts;          /* перезапуски hill-climbing плагборду */
    int max_plugs;         /* максимум пар плагборду */
    int both_reflectors;   /* 1 = пробувати UKW B і C, 0 = тільки B */
    int verbose;           /* 1 = друкувати прогрес у stderr */
} CrackOptions;

void crack_default_options(CrackOptions *opt);

/* Знайти найкращий ключ для шифротексту ct (числа 0..25). Повертає фінальну оцінку. */
double crack(const int *ct, int len, const CrackOptions *opt, EnigmaKey *best);

#endif
