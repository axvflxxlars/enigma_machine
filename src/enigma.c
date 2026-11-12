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
static int mod(int a)
{
    if(a > -1){
        a %= 26;
        return a;
    }
    while (a < 0){
        a += 26; 
    }
    return a;
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
        pos[1] = mod(pos[1]);
        pos[0] = mod(pos[0]);
    }
    if(pos[2] == (int)(ROTOR_NOTCH[rotor[2]] - 'A') && !made_step){
        pos[1]++;
        pos[1] = mod(pos[1]);
    }
    pos[2]++;
    pos[2] = mod(pos[2]);
}

static int rotor_forward(int c, int n, int step){
    c = mod(c + step);
    c = ROTOR_WIRING[n][c] - 'A';
    c = mod(c - step);
    return c;

}

static int rotor_backward(int c, int n, int step, int inv[5][26]){
    c = mod(c + step);
    c = inv[n][c];
    c = mod(c - step);
    return c;

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
    int inv[5][26];
    for (int r = 0; r != NUM_ROTORS; r++)
    {   for (int i = 0; i != ALPHA; i++){
            int j = ROTOR_WIRING[r][i] - 'A';
            inv[r][j] = i;
        }
    }
    
    
    int p[3] = {key -> pos[0], key -> pos[1], key -> pos[2]};

    for (int i = 0; i < len; i++)
    {   enigma_step(key -> rotor, p);

        int c = in[i];
        int f = 2;
        int step = p[f] - key -> ring[f];
        int n = key -> rotor[f];
        c = rotor_forward(c, n, step);
        f--;
        step = p[f] - key -> ring[f];
        n = key -> rotor[f];

        c = rotor_forward(c, n, step);
        f--;
        step = p[f] - key -> ring[f];
        n = key -> rotor[f];

        c = rotor_forward(c, n, step);

        c = REFLECTOR_WIRING[key -> reflector][c] - 'A';

        c = rotor_backward(c, n, step, inv);
        f++;
        step = p[f] - key -> ring[f];
        n = key -> rotor[f];

        c = rotor_backward(c, n, step, inv);
        f++;
        step = p[f] - key -> ring[f];
        n = key -> rotor[f];

        c = rotor_backward(c, n, step, inv);

        out[i] = c;
    } 
    

    
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
