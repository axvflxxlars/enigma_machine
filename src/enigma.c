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

/* Назви роторів для key_parse і key_print. */
static const char *const ROTOR_NAMES[NUM_ROTORS] = { "I", "II", "III", "IV", "V" };

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

/* Збільшити позицію ротора на 1 з переходом Z -> A. */
static int next_pos(int p)
{
    return p == ALPHA - 1 ? 0 : p + 1;
}

void enigma_step(const int rotor[3], int pos[3])
{
    bool middle_at_notch = pos[1] == ROTOR_NOTCH[rotor[1]] - 'A';
    bool right_at_notch  = pos[2] == ROTOR_NOTCH[rotor[2]] - 'A';

    if (middle_at_notch) {          /* double-step: крокують лівий і середній */
        pos[0] = next_pos(pos[0]);
        pos[1] = next_pos(pos[1]);
    } else if (right_at_notch) {    /* правий штовхає середній */
        pos[1] = next_pos(pos[1]);
    }
    pos[2] = next_pos(pos[2]);
}

static int fwd[NUM_ROTORS][ALPHA];
static int bwd[NUM_ROTORS][ALPHA];
static bool tables_ready = false;

static void init_tables(void)
{
    for (int r = 0; r < NUM_ROTORS; r++) {
        for (int i = 0; i < ALPHA; i++) {
            int j = ROTOR_WIRING[r][i] - 'A';
            fwd[r][i] = j;
            bwd[r][j] = i;
        }
    }
    tables_ready = true;
}

/* Пропустити літеру c через проводку table зі зсувом step (0..25). */
static int rotor_pass(const int table[ALPHA], int c, int step)
{
    c += step;
    if (c >= ALPHA) c -= ALPHA;
    c = table[c];
    c -= step;
    if (c < 0) c += ALPHA;
    return c;
}

/* Зсув ротора f з урахуванням рингу, 0..25. */
static int rotor_step(const EnigmaKey *key, const int p[3], int f)
{
    int step = p[f] - key->ring[f];
    return step < 0 ? step + ALPHA : step;
}

/* Три ротори вперед, рефлектор, три ротори назад. Плагборд не входить. */
static int through_rotors(int c, const EnigmaKey *key, const int p[3])
{
    const int sl = rotor_step(key, p, 0);
    const int sm = rotor_step(key, p, 1);
    const int sr = rotor_step(key, p, 2);
    const int *rotor = key->rotor;

    c = rotor_pass(fwd[rotor[2]], c, sr);
    c = rotor_pass(fwd[rotor[1]], c, sm);
    c = rotor_pass(fwd[rotor[0]], c, sl);
    c = REFLECTOR_WIRING[key->reflector][c] - 'A';
    c = rotor_pass(bwd[rotor[0]], c, sl);
    c = rotor_pass(bwd[rotor[1]], c, sm);
    c = rotor_pass(bwd[rotor[2]], c, sr);
    return c;
}

void enigma_crypt(const EnigmaKey *key, const int *in, int *out, int len)
{
    if (!tables_ready)
        init_tables();

    int p[3] = { key->pos[0], key->pos[1], key->pos[2] };

    for (int i = 0; i < len; i++) {
        enigma_step(key->rotor, p);
        int c = key->plug[in[i]];
        c = through_rotors(c, key, p);
        out[i] = key->plug[c];
    }
}

void enigma_build_path(const EnigmaKey *key, int len, int (*path)[ALPHA])
{
    if (!tables_ready)
        init_tables();

    int p[3] = { key->pos[0], key->pos[1], key->pos[2] };

    for (int i = 0; i < len; i++) {
        enigma_step(key->rotor, p);
        for (int c = 0; c < ALPHA; c++)
            path[i][c] = through_rotors(c, key, p);
    }
}

void enigma_crypt_path(const int (*path)[ALPHA], const int *plug,
                       const int *in, int *out, int len)
{
    for (int i = 0; i < len; i++)
        out[i] = plug[path[i][plug[in[i]]]];
}

/* "I".."V" -> 0..4, інакше -1. */
static int rotor_index(const char *s)
{
    for (int i = 0; i < NUM_ROTORS; i++)
        if (strcmp(s, ROTOR_NAMES[i]) == 0)
            return i;
    return -1;
}

static bool is_upper(char ch)
{
    return ch >= 'A' && ch <= 'Z';
}

/* Рівно три великі літери -> три числа 0..25. Повертає 0 або -1. */
static int parse_letters3(const char *s, int out[3])
{
    if (strlen(s) != 3)
        return -1;
    for (int i = 0; i < 3; i++) {
        if (!is_upper(s[i]))
            return -1;
        out[i] = s[i] - 'A';
    }
    return 0;
}

/* "I-II-III" -> три різні ротори. Повертає 0 або -1. */
static int parse_rotors(const char *s, int rotor[3])
{
    char name[3][4];
    int used = 0;

    if (sscanf(s, "%3[^-]-%3[^-]-%3s%n", name[0], name[1], name[2], &used) != 3
        || s[used] != '\0')
        return -1;

    for (int i = 0; i < 3; i++) {
        rotor[i] = rotor_index(name[i]);
        if (rotor[i] < 0)
            return -1;
    }
    if (rotor[0] == rotor[1] || rotor[1] == rotor[2] || rotor[0] == rotor[2])
        return -1;
    return 0;
}

int key_parse(const char *s, EnigmaKey *key)
{
    static const char SEP[] = " \t\r\n";
    char str[128];

    key_init(key);
    if (strlen(s) >= sizeof str)        /* не обрізати мовчки */
        return -1;
    strcpy(str, s);

    char *rotors = strtok(str, SEP);
    char *refl   = strtok(NULL, SEP);
    char *rings  = strtok(NULL, SEP);
    char *pos    = strtok(NULL, SEP);
    if (pos == NULL)
        return -1;

    if (parse_rotors(rotors, key->rotor) != 0)
        return -1;

    if (strcmp(refl, "B") == 0)
        key->reflector = REFL_B;
    else if (strcmp(refl, "C") == 0)
        key->reflector = REFL_C;
    else
        return -1;

    if (parse_letters3(rings, key->ring) != 0 || parse_letters3(pos, key->pos) != 0)
        return -1;

    for (char *pair = strtok(NULL, SEP); pair != NULL; pair = strtok(NULL, SEP)) {
        if (strlen(pair) != 2 || !is_upper(pair[0]) || !is_upper(pair[1]))
            return -1;
        int a = pair[0] - 'A';
        int b = pair[1] - 'A';
        if (a == b || key->plug[a] != a || key->plug[b] != b)
            return -1;
        key->plug[a] = b;
        key->plug[b] = a;
    }
    return 0;
}

void key_print(const EnigmaKey *key, FILE *f)
{
    fprintf(f, "%s-%s-%s %c ",
            ROTOR_NAMES[key->rotor[0]], ROTOR_NAMES[key->rotor[1]], ROTOR_NAMES[key->rotor[2]],
            key->reflector == REFL_B ? 'B' : 'C');

    for (int i = 0; i < 3; i++)
        fputc('A' + key->ring[i], f);
    fputc(' ', f);
    for (int i = 0; i < 3; i++)
        fputc('A' + key->pos[i], f);

    for (int i = 0; i < ALPHA; i++)
        if (key->plug[i] > i)
            fprintf(f, " %c%c", 'A' + i, 'A' + key->plug[i]);
}
