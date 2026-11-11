/*
 * enigma.c — симулятор Enigma M3.                         Відповідальний: A
 */
#include "enigma.h"
#include <string.h>
#include <stdbool.h>


/* Проводка роторів: ROTOR_WIRING[r][i] — куди йде літера i (вхід справа). */
const char *const ROTOR_WIRING[NUM_ROTORS] = {
    "EKMFLGDQVZNTOWYHXUSPAIBRCJ",   /* I   */
    "AJDKSIRUXBLHWTMCQGZNPYFVOE",   /* II  */
    "BDFHJLCPRTXVZNYEIWGAKMUSQO",   /* III */
    "ESOVPZJAYQUIRHXLNFTGKDCMWB",   /* IV  */
    "VZBRGITYUPSDNHLXAWMKCQFJEO",   /* V   */
};

/* Коли ротор стоїть на цій літері, наступне натискання штовхає ротор зліва. */
const char ROTOR_NOTCH[NUM_ROTORS] = { 'Q', 'E', 'V', 'J', 'Z' };

const char *const REFLECTOR_WIRING[NUM_REFLECTORS] = {
    "YRUHQSLDPXNGOKMIEBFZCWVJAT",   /* UKW-B */
    "FVPJIAOYEDRZXWGCTKUQSBNMHL",   /* UKW-C */
};

void key_init(EnigmaKey *key)
{
    memset(key, 0, sizeof *key);
    key->rotor[0] = 0;   /* I   */
    key->rotor[1] = 1;   /* II  */
    key->rotor[2] = 2;   /* III */
    key->reflector = REFL_B;
    for (int i = 0; i < ALPHA; i++)
        key->plug[i] = i;
}

void enigma_step(const int rotor[3], int pos[3])
{
    /*
     * TODO(A): крок роторів.
     *  - правий ротор крокує завжди;
     *  - якщо правий стоїть на своєму нотчі, крокує середній;
     *  - double-step: якщо середній стоїть на своєму нотчі, крокують і середній, і лівий.
     * Перевірка: I-II-III, позиції ADU -> ADV -> AEW -> BFX.
     */
    bool made_step = false;
    if (pos[1] == (int)(ROTOR_NOTCH[rotor[1]] - 'A')){
        pos[0]++;
        pos[1]++;
        made_step = true;
        if (pos[0] == 26){
            pos[0] = 0;
        }
         if (pos[1] == 26){
            pos[1] = 0;
        }
    }
    if(pos[2] == (int)(ROTOR_NOTCH[rotor[2]] - 'A') && !made_step){
        pos[1]++;
        if (pos[1] == 26){
            pos[1] = 0;
        }
    }
    pos[2]++;
     if (pos[2] == 26){
        pos[2] = 0;
    }
}

void enigma_crypt(const EnigmaKey *key, const int *in, int *out, int len)
{
    /*
     * TODO(A): для кожної літери:
     *  1. enigma_step() на локальній копії позицій (крок ДО шифрування);
     *  2. плагборд;
     *  3. ротори справа наліво (з урахуванням позиції і рингу: зсув = pos - ring);
     *  4. рефлектор;
     *  5. ротори зліва направо через ОБЕРНЕНУ проводку (передобчисліть її);
     *  6. плагборд.
     * Порада для швидкості: передобчисліть таблиці один раз, без malloc у циклі,
     * бо брейкер викликає цю функцію мільйони разів.
     */
    (void)key;
    for (int i = 0; i < len; i++)
        out[i] = in[i];
}

int key_parse(const char *s, EnigmaKey *key)
{
    /* TODO(A): розібрати "I-II-III B AAA AAA AB CD EF". Перевірити, що ротори різні,
     * а літери в парах плагборду не повторюються. */
    (void)s;
    key_init(key);
    return -1;
}

void key_print(const EnigmaKey *key, FILE *f)
{
    /* TODO(A): друкувати у форматі key_parse. */
    (void)key;
    fputs("I-II-III B AAA AAA", f);
}
