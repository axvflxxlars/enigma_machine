/*
 * enigma.h — симулятор Enigma M3.                         Відповідальний: A
 *
 * Усередині програми літери завжди числа 0..25 (A = 0, ..., Z = 25).
 * Інтерфейс заморожено 12.11. Змінювати тільки після обговорення з усіма.
 */
#ifndef ENIGMA_H
#define ENIGMA_H

#include <stdio.h>

#define ALPHA      26
#define NUM_ROTORS 5          /* I, II, III, IV, V */

enum { REFL_B = 0, REFL_C = 1, NUM_REFLECTORS = 2 };

/* Дані машини (визначені в enigma.c). */
extern const char *const ROTOR_WIRING[NUM_ROTORS];
extern const char        ROTOR_NOTCH[NUM_ROTORS];    /* літера в вікні, на якій ротор штовхає сусіда */
extern const char *const REFLECTOR_WIRING[NUM_REFLECTORS];

typedef struct {
    int rotor[3];     /* 0..4 = I..V, зліва направо (лівий, середній, правий) */
    int ring[3];      /* Ringstellung, 0..25 */
    int pos[3];       /* стартові позиції (Grundstellung), 0..25 */
    int reflector;    /* REFL_B або REFL_C */
    int plug[ALPHA];  /* plug[i] = j і plug[j] = i; без пари plug[i] = i */
} EnigmaKey;

/* Ключ за замовчуванням: I-II-III, UKW B, ринги AAA, позиції AAA, без плагборду. */
void key_init(EnigmaKey *key);

/*
 * Зашифрувати/розшифрувати len літер. in і out можуть бути одним масивом.
 * key не змінюється: позиції роторів крокують у локальній копії.
 */
void enigma_crypt(const EnigmaKey *key, const int *in, int *out, int len);

/*
 * Таблиця шляху для брейкера: path[i][c] — куди ротори з рефлектором переводять
 * літеру c у позиції i. Плагборд у key ігнорується.
 * Пам'ять (len рядків по ALPHA) виділяє той, хто викликає.
 */
void enigma_build_path(const EnigmaKey *key, int len, int (*path)[ALPHA]);

/*
 * Розшифрувати за готовою таблицею з плагбордом plug: out[i] = plug[path[i][plug[in[i]]]].
 * Результат збігається з enigma_crypt для ключа, з якого побудовано path, і його plug.
 */
void enigma_crypt_path(const int (*path)[ALPHA], const int *plug,
                       const int *in, int *out, int len);

/*
 * Один крок роторів (як при натисканні клавіші), включно з double-step
 * середнього ротора. Винесено окремо, щоб тестувати крок.
 */
void enigma_step(const int rotor[3], int pos[3]);

/*
 * Розбір ключа з рядка формату:
 *     "I-II-III B AAA AAA AB CD EF"
 *      ротори   рефл. ринги позиції пари плагборду (0..13 пар)
 * Повертає 0 при успіху, -1 при помилці.
 */
int key_parse(const char *s, EnigmaKey *key);

/* Друк ключа в тому самому форматі, що читає key_parse (без '\n' в кінці). */
void key_print(const EnigmaKey *key, FILE *f);

#endif
