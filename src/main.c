/*
 * main.c — командний рядок.                               Відповідальний: C
 *
 *   ./enigma encrypt --key "I-II-III B AAA AAA AB CD" < plain.txt
 *   ./enigma crack [--ngrams data] [--top N] [--stage3-top N] [--restarts N] [--max-plugs N] [--deep] [--both-reflectors] [-v] < cipher.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

#include "crack.h"
#include "enigma.h"
#include "score.h"
#include "text.h"

static void usage(void)
{
    fprintf(stderr,
        "usage:\n"
        "  enigma encrypt --key \"I-II-III B AAA AAA AB CD\" < plain.txt\n"
        "  enigma crack [--ngrams DIR] [--top N] [--stage3-top N] [--restarts N] [--max-plugs N] [--deep] [--both-reflectors] [-v] < cipher.txt\n");
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
        if (c == '\0') {
            free(buf);
            return NULL;
        }
        if (len + 1 >= cap) {
            if (cap > SIZE_MAX / 2) {
                free(buf);
                return NULL;
            }
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
    if (ferror(f)) {
        free(buf);
        return NULL;
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

    if (max > INT_MAX || max > SIZE_MAX / sizeof **out) {
        free(s);
        return -1;
    }
    *out = malloc(sizeof **out * (max ? max : 1));
    if (!*out) {
        free(s);
        return -1;
    }
    int len = text_to_nums(s, *out, (int)max);
    free(s);
    return len;
}

static int print_nums(const int *t, int len)
{
    char *s = malloc((size_t)len + 1);
    if (!s)
        return -1;

    nums_to_text(t, len, s);
    int result = puts(s);
    free(s);

    if (result == EOF || fflush(stdout) == EOF)
        return -1;

    return 0;
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
    int result = print_nums(t, len);
    free(t);

    if (result != 0) {
        fprintf(stderr, "failed to write output\n");
        return 1;
    }

    return 0;
}

/* Перевірка числових параметрів командного рядка. */
static int parse_number(const char *s, int min, int max, int *out)
{
    char *end;
    errno = 0;

    long value = strtol(s, &end, 10);

    if (errno != 0 || end == s || *end != '\0' ||
        value < min || value > max) {
        fprintf(stderr, "invalid number: %s (expected %d..%d)\n",
                s, min, max);
        return -1;
    }

    *out = (int)value;
    return 0;
}

static int cmd_crack(int argc, char **argv)
{
    const char *dir = "data";
    CrackOptions opt;
    crack_default_options(&opt);

    /* Розширений режим задає початкові налаштування. */
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--deep") == 0) {
            opt.top_n = 2000;
            opt.stage3_top = 50;
        }
    }

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--ngrams") == 0 && i + 1 < argc) {
            dir = argv[++i];
        } else if (strcmp(argv[i], "--top") == 0 && i + 1 < argc) {
            if (parse_number(argv[++i], 1, 2109120, &opt.top_n))
                return 2;
        } else if (strcmp(argv[i], "--stage3-top") == 0 && i + 1 < argc) {
            if (parse_number(argv[++i], 1, INT_MAX, &opt.stage3_top))
                return 2;
        } else if (strcmp(argv[i], "--restarts") == 0 && i + 1 < argc) {
            if (parse_number(argv[++i], 1, INT_MAX, &opt.restarts))
                return 2;
        } else if (strcmp(argv[i], "--max-plugs") == 0 && i + 1 < argc) {
            if (parse_number(argv[++i], 0, 10, &opt.max_plugs))
                return 2;
        } else if (strcmp(argv[i], "--deep") == 0) {
            continue;
        } else if (strcmp(argv[i], "--both-reflectors") == 0) {
            opt.both_reflectors = 1;
        } else if (strcmp(argv[i], "-v") == 0) {
            opt.verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            usage();
            return 0;
        } else {
            usage();
            return 2;
        }
    }

    char path[1024];
    if (snprintf(path, sizeof path,
                 "%s/english_bigrams.txt", dir) >= (int)sizeof path) {
        fprintf(stderr, "ngram path is too long\n");
        return 1;
    }
    if (ngram_load(path, 2) != 0) {
        fprintf(stderr, "failed to load %s\n", path);
        return 1;
    }
    if (snprintf(path, sizeof path,
                 "%s/english_trigrams.txt", dir) >= (int)sizeof path) {
        fprintf(stderr, "ngram path is too long\n");
        return 1;
    }
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

    if (len < 3) {
        fprintf(stderr, "ciphertext must contain at least 3 letters\n");
        free(ct);
        return 2;
    }

    EnigmaKey best;
    double score = crack(ct, len, &opt, &best);

    if (!isfinite(score)) {
        fprintf(stderr, "crack failed\n");
        free(ct);
        return 1;
    }

    printf("KEY:   ");
    key_print(&best, stdout);
    printf("\nSCORE: %.2f\n", score);

    enigma_crypt(&best, ct, ct, len);
    int result = print_nums(ct, len);
    free(ct);

    if (result != 0) {
        fprintf(stderr, "failed to write output\n");
        return 1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        usage();
        return 0;
    }

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
