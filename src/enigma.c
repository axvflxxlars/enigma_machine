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

 static const char *const rotor_numbers[5] = {
        "I",
        "II",
        "III",
        "IV",
        "V"
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
    bool made_step = false;
    if (pos[1] == (int)(ROTOR_NOTCH[rotor[1]] - 'A')){
        pos[0]++;
        pos[1]++;
        made_step = true;
        pos[1] = (pos[1] == 26) ? 0 : pos[1];
        pos[0] = (pos[0] == 26) ? 0 : pos[0];
    }
    if(pos[2] == (int)(ROTOR_NOTCH[rotor[2]] - 'A') && !made_step){
        pos[1]++;
        pos[1] = (pos[1] == 26) ? 0 : pos[1];
    }
    pos[2]++;
    pos[2] = (pos[2] == 26) ? 0 : pos[2];
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

static int rotor_forward(int c, int n, int step){
    c = c + step;
    if (c >= 26) c -= 26;
    c = fwd[n][c];
    c = c - step;
    if (c < 0) c += 26;
    return c;
}

static int rotor_backward(int c, int n, int step){
    c = c + step;
    if (c >= 26) c -= 26;
    c = bwd[n][c];
    c = c - step;
    if (c < 0) c += 26;
    return c;
}

static int through_rotors(int letter, const EnigmaKey *key, const int p[3]){
    int f = 2;
    int step = p[f] - key -> ring[f];
    if (step < 0) step += 26;
    int n = key -> rotor[f];
    letter = rotor_forward(letter, n, step);
    f--;
    step = p[f] - key -> ring[f];
    if (step < 0) step += 26;
    n = key -> rotor[f];
    letter = rotor_forward(letter, n, step);
    f--;
    step = p[f] - key -> ring[f];
    if (step < 0) step += 26;
    n = key -> rotor[f];
    letter = rotor_forward(letter, n, step);
    letter = REFLECTOR_WIRING[key -> reflector][letter] - 'A';
    letter = rotor_backward(letter, n, step);
    f++;
    step = p[f] - key -> ring[f];
    if (step < 0) step += 26;
    n = key -> rotor[f];
    letter = rotor_backward(letter, n, step);
    f++;
    step = p[f] - key -> ring[f];
    if (step < 0) step += 26;
    n = key -> rotor[f];
    letter = rotor_backward(letter, n, step);
    return letter;
}

void enigma_crypt(const EnigmaKey *key, const int *in, int *out, int len)
{
    if (!tables_ready)
        init_tables();

    int p[3] = {key -> pos[0], key -> pos[1], key -> pos[2]};

    for (int i = 0; i < len; i++)
    {   enigma_step(key -> rotor, p);
        int c = in[i];
        c = key -> plug[c];
        c = through_rotors(c, key, p);
        c = key -> plug[c];
        out[i] = c;
    } 
}

static int rotor_index(const  char *s){
    for (int i = 0; i <= 4; i++)
    {
        if (strcmp(s, rotor_numbers[i]) == 0) return i;
    }
    return -1;
}

static int char_to_number(const char *s, int *out){
    if (strlen(s) != 3) return -1;
    for (int i = 0; i <= 2; i++)
    {   if (s[i] < 'A' || s[i] > 'Z') return -1;
        out[i] = s[i] - 'A';
    }
    return 0;

}

int key_parse(const char *s, EnigmaKey *key)
{
    key_init(key);
    char str[128];
    snprintf(str, sizeof(str), "%s", s);
    char *rotor_word, *refl_word, *ring_word, *pos_word;
    char frotor_word [4], srotor_word [4], trotor_word [4];
    rotor_word = strtok(str, " ");
    if (rotor_word == NULL) return -1;
    refl_word = strtok(NULL, " "); 
    if (refl_word == NULL) return -1;
    ring_word = strtok(NULL, " ");
    if (ring_word == NULL) return -1;
    pos_word = strtok(NULL, " ");
    if (pos_word == NULL) return -1;

    if (sscanf(rotor_word, "%3[^-]-%3[^-]-%3s", frotor_word, srotor_word, trotor_word) != 3){
        return -1;
    }
    key->rotor[0] = rotor_index(frotor_word);
    if (key->rotor[0] == -1) return -1;
    key->rotor[1] = rotor_index(srotor_word);
    if (key->rotor[1] == -1) return -1;
    key->rotor[2] = rotor_index(trotor_word);
    if (key->rotor[2] == -1) return -1;
    
    if (key->rotor[0] == key->rotor[1] || key->rotor[1] == key->rotor[2] || key->rotor[0] == key->rotor[2]) return -1;

    if (strlen(refl_word) != 1) return -1;

    if (refl_word[0] != 'B' && refl_word[0] != 'C') return -1;

    if (refl_word[0] == 'B'){
        key->reflector = REFL_B;
    }
    else{
        key->reflector = REFL_C;
    }

    if (char_to_number(ring_word, key->ring) || char_to_number(pos_word, key->pos)) return -1;

    char *pair = strtok(NULL, " ");

    while (pair != NULL){
        if (strlen(pair) != 2) return -1;
        if (pair[0] < 'A' || pair[0] > 'Z') return -1;
        if (pair[1] < 'A' || pair[1] > 'Z') return -1;
        int a = pair[0] - 'A';
        int b = pair[1] - 'A';
        if (a == b) return -1;
        if (key->plug[a] != a || key->plug[b] != b) return -1;
        key->plug[a] = b;
        key->plug[b] = a;
        pair = strtok(NULL, " ");
    }

    return 0;
}

void key_print(const EnigmaKey *key, FILE *f)
{   char reflector = key->reflector == REFL_B ? 'B' : 'C';
    
    for (int i = 0; i < 3; i++)
    {
        fputs(rotor_numbers[key->rotor[i]], f);
        if (i != 2){
            fputc( '-' , f);
        }
    }

    fputc(' ', f);

    fputc(reflector, f);

    fputc(' ', f);

    for (int i = 0; i < 3; i++)
    {
        fputc(key->ring[i] + 'A', f);
        
    }

    fputc(' ', f);

    for (int i = 0; i < 3; i++)
    {
        fputc(key->pos[i] + 'A', f);
        
    }

    for (int i = 0; i < 26; i++)
    {   if (key->plug[i] > i)
        {   
            fputc(' ', f);
            fputc(i + 'A', f);
            fputc(key->plug[i] + 'A', f);
        }
    
        
    }
}
