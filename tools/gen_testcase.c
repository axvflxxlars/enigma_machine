/*
 * gen_testcase.c — генератор тестових шифротекстів.       Відповідальний: B
 *
 *   ./build/gen_testcase <plain.txt> <out_prefix> [num_plugs=10] [length=0] [seed]
 *
 * Читає англійський текст, бере перші length літер (0 = усі), шифрує випадковим
 * ключем і пише:
 *   <out_prefix>.cipher.txt — шифротекст
 *   <out_prefix>.key.txt    — правильний ключ (формат key_print)
 *
 * Приклад набору: для кожного з 5 текстів довжини 100, 200, 300, 500 → 20 тестів у tests/testset/.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../src/enigma.h"
#include "../src/text.h"

/* Випадковий ключ: 3 різні ротори з 5, випадкові ринги/позиції, num_plugs пар. */
static void key_random(EnigmaKey *key, int num_plugs)
{
    key_init(key);
    /* TODO(B): rand() для роторів (без повторів), ring, pos, reflector (B),
     * і num_plugs випадкових пар у plug[] (обидва напрямки, без повторів літер). */
    (void)num_plugs;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s <plain.txt> <out_prefix> [num_plugs] [length] [seed]\n", argv[0]);
        return 2;
    }
    const char *plain_path = argv[1];
    const char *prefix     = argv[2];
    int num_plugs = argc > 3 ? atoi(argv[3]) : 10;
    int length    = argc > 4 ? atoi(argv[4]) : 0;
    unsigned seed = argc > 5 ? (unsigned)strtoul(argv[5], NULL, 10) : (unsigned)time(NULL);
    srand(seed);

    EnigmaKey key;
    key_random(&key, num_plugs);

    /* TODO(B):
     *  1. прочитати plain_path, text_to_nums;
     *  2. обрізати до length, якщо length > 0;
     *  3. enigma_crypt;
     *  4. записати <prefix>.cipher.txt (nums_to_text) і <prefix>.key.txt (key_print). */
    (void)plain_path;
    (void)prefix;
    (void)length;

    fprintf(stderr, "gen_testcase: not implemented yet\n");
    return 1;
}
