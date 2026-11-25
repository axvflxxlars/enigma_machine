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
#include <string.h>
#include <limits.h>

#include "../src/enigma.h"
#include "../src/text.h"

/* Випадковий ключ: 3 різні ротори з 5, випадкові ринги/позиції, num_plugs пар. */
static void key_random(EnigmaKey *key, int num_plugs)
{
    key_init(key);

    int rotors[5] = {0, 1, 2, 3, 4};

    for (int i = 4; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = rotors[i];
        rotors[i] = rotors[j];
        rotors[j] = temp;
    }

    for (int i = 0; i < 3; i++)
    {
        key->rotor[i] = rotors[i];
        key->ring[i] = rand() % 26;
        key->pos[i] = rand() % 26;
    }

    key->reflector = REFL_B;

    int letters[26];

    for (int i = 0; i < 26; i++)
        letters[i] = i;

    for (int i = 25; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = letters[i];
        letters[i] = letters[j];
        letters[j] = temp;
    }

    for (int i = 0; i < num_plugs; i++)
    {
        int a = letters[i * 2];
        int b = letters[i * 2 + 1];

        key->plug[a] = b;
        key->plug[b] = a;
    }
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
    if (num_plugs < 0 || num_plugs > 13)
{
    fprintf(stderr, "Number of plugboard pairs must be between 0 and 13\n");
    return 1;
}
    int length    = argc > 4 ? atoi(argv[4]) : 0;
    unsigned seed = argc > 5 ? (unsigned)strtoul(argv[5], NULL, 10) : (unsigned)time(NULL);
    srand(seed);

    EnigmaKey key;
    key_random(&key, num_plugs);

   FILE *file = fopen(plain_path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Cannot open input file\n");
        return 1;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return 1;
    }

    long size = ftell(file);
    if (size < 0 || fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        return 1;
    }

    char *text = malloc((size_t)size + 1);
    if (text == NULL)
    {
        fclose(file);
        return 1;
    }

    size_t bytes = fread(text, 1, (size_t)size, file);
    if (ferror(file))
    {
        fprintf(stderr, "Cannot read input file\n");
        free(text);
        fclose(file);
        return 1;
    }

    text[bytes] = '\0';
    fclose(file);

    int *plain = malloc((bytes + 1) * sizeof(int));
    if (plain == NULL)
    {
        free(text);
        return 1;
    }

    int len = text_to_nums(text, plain, (int)(bytes > INT_MAX ? INT_MAX : bytes));
    free(text);

    if (len <= 0)
    {
        fprintf(stderr, "No English letters found\n");
        free(plain);
        return 1;
    }

    if (length > 0 && length < len)
        len = length;

    int *cipher = malloc((size_t)len * sizeof(int));
    char *result = malloc((size_t)len + 1);

    if (cipher == NULL || result == NULL)
    {
        free(plain);
        free(cipher);
        free(result);
        return 1;
    }

    enigma_crypt(&key, plain, cipher, len);
    nums_to_text(cipher, len, result);

    size_t path_len = strlen(prefix) + 16;
    char *path = malloc(path_len);

    if (path == NULL)
    {
        free(plain);
        free(cipher);
        free(result);
        return 1;
    }

    snprintf(path, path_len, "%s.cipher.txt", prefix);

    FILE *out = fopen(path, "w");
    if (out == NULL)
    {
        fprintf(stderr, "Cannot create cipher file\n");
        free(path);
        free(plain);
        free(cipher);
        free(result);
        return 1;
    }

    fprintf(out, "%s\n", result);
    fclose(out);

    snprintf(path, path_len, "%s.key.txt", prefix);

    out = fopen(path, "w");
    if (out == NULL)
    {
        fprintf(stderr, "Cannot create key file\n");
        free(path);
        free(plain);
        free(cipher);
        free(result);
        return 1;
    }

    key_print(&key, out);
    fprintf(out, "\n");
    fclose(out);

    free(path);
    free(plain);
    free(cipher);
    free(result);

    return 0;
}
