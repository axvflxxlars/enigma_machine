/*
 * main.c — командний рядок.                               Відповідальний: C
 *
 *   ./enigma encrypt --key "I-II-III B AAA AAA AB CD" < plain.txt
 *   ./enigma crack [--ngrams data] [--top N] [--restarts N] [--both-reflectors] [-v] < cipher.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crack.h"
#include "enigma.h"
#include "score.h"
#include "text.h"

static void usage(void)
{
    fprintf(stderr,
        "usage:\n"
        "  enigma encrypt --key \"I-II-III B AAA AAA AB CD\" < plain.txt\n"
        "  enigma crack [--ngrams DIR] [--top N] [--restarts N] [--both-reflectors] [-v] < cipher.txt\n");
}

/* Прочитати весь stdin у рядок. Повертає malloc-буфер або NULL. */
static char *read_all(FILE *f)
{
    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    if (!buf)
        return NULL;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char *nb = realloc(buf, cap);
            if (!nb) {
                free(buf);
                return NULL;
            }
            buf = nb;
        }
        buf[len++] = (char)c;
    }
    buf[len] = '\0';
    return buf;
}

/* Прочитати stdin і перетворити на числа. *out — malloc-масив. Повертає довжину або -1. */
static int read_input(int **out)
{
    char *s = read_all(stdin);
    if (!s)
        return -1;
    size_t max = strlen(s);
    *out = malloc(sizeof **out * (max ? max : 1));
    if (!*out) {
        free(s);
        return -1;
    }
    int len = text_to_nums(s, *out, (int)max);
    free(s);
    return len;
}

static void print_nums(const int *t, int len)
{
    char *s = malloc((size_t)len + 1);
    if (!s)
        return;
    nums_to_text(t, len, s);
    puts(s);
    free(s);
}

static int cmd_encrypt(int argc, char **argv)
{
    const char *key_str = NULL;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--key") == 0 && i + 1 < argc)
            key_str = argv[++i];
        else {
            usage();
            return 2;
        }
    }
    if (!key_str) {
        usage();
        return 2;
    }

    EnigmaKey key;
    if (key_parse(key_str, &key) != 0) {
        fprintf(stderr, "invalid key: %s\n", key_str);
        return 2;
    }

    int *t;
    int len = read_input(&t);
    if (len < 0) {
        fprintf(stderr, "failed to read input\n");
        return 1;
    }
    enigma_crypt(&key, t, t, len);
    print_nums(t, len);
    free(t);
    return 0;
}

static int cmd_crack(int argc, char **argv)
{
    const char *dir = "data";
    CrackOptions opt;
    crack_default_options(&opt);

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--ngrams") == 0 && i + 1 < argc)
            dir = argv[++i];
        else if (strcmp(argv[i], "--top") == 0 && i + 1 < argc)
            opt.top_n = atoi(argv[++i]);
        else if (strcmp(argv[i], "--restarts") == 0 && i + 1 < argc)
            opt.restarts = atoi(argv[++i]);
        else if (strcmp(argv[i], "--both-reflectors") == 0)
            opt.both_reflectors = 1;
        else if (strcmp(argv[i], "-v") == 0)
            opt.verbose = 1;
        else {
            usage();
            return 2;
        }
    }
    if (opt.top_n < 1)
        opt.top_n = 1;

    char path[1024];
    snprintf(path, sizeof path, "%s/english_bigrams.txt", dir);
    if (ngram_load(path, 2) != 0) {
        fprintf(stderr, "failed to load %s\n", path);
        return 1;
    }
    snprintf(path, sizeof path, "%s/english_trigrams.txt", dir);
    if (ngram_load(path, 3) != 0) {
        fprintf(stderr, "failed to load %s\n", path);
        return 1;
    }

    int *ct;
    int len = read_input(&ct);
    if (len < 0) {
        fprintf(stderr, "failed to read input\n");
        return 1;
    }

    EnigmaKey best;
    double score = crack(ct, len, &opt, &best);

    printf("KEY:   ");
    key_print(&best, stdout);
    printf("\nSCORE: %.2f\n", score);

    enigma_crypt(&best, ct, ct, len);
    print_nums(ct, len);
    free(ct);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        return 2;
    }
    if (strcmp(argv[1], "encrypt") == 0)
        return cmd_encrypt(argc - 2, argv + 2);
    if (strcmp(argv[1], "crack") == 0)
        return cmd_crack(argc - 2, argv + 2);
    usage();
    return 2;
}
