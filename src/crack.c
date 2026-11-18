/*
 * crack.c — злам Енігми тільки по шифротексту.            Відповідальний: C
 */
#include "crack.h"

#include <stdio.h>
#include <stdlib.h>

#include "score.h"

typedef struct {
    EnigmaKey key;
    double    score;
} Candidate;

void crack_default_options(CrackOptions *opt)
{
    opt->top_n           = 200;
    opt->restarts        = 5;
    opt->max_plugs       = 10;
    opt->both_reflectors = 0;
    opt->verbose         = 0;
}

/*
 * Крок 1. Перебрати 60 порядків роторів (3 різні з 5) × 26^3 стартових позицій
 * (× 2 рефлектори, якщо both_reflectors). Ринги AAA, плагборд порожній.
 * Розшифрувати, порахувати score_ioc, тримати top кращих (масив або min-heap).
 * Повертає кількість заповнених кандидатів (<= max).
 */




static int stage1_rotors(const int *ct, int len, const CrackOptions *opt,
                         Candidate *top, int max)
{
    if (len <= 0 || max <= 0) {
        return 0;
    }

    int *decoded = malloc(sizeof *decoded * (size_t)len);
    if (decoded == NULL) {
        fprintf(stderr, "crack: out of memory\n");
        return 0;
    }

    int count = 0;
    int saved = 0;

    int reflectors = opt->both_reflectors ? NUM_REFLECTORS : 1;

    for (int r = 0; r < reflectors; r++) {
        for (int a = 0; a < NUM_ROTORS; a++) {
            for (int b = 0; b < NUM_ROTORS; b++) {
                for (int c = 0; c < NUM_ROTORS; c++) {
                    if (a == b || a == c || b == c) {
                        continue;
                    }

                    for (int p1 = 0; p1 < ALPHA; p1++) {
                        for (int p2 = 0; p2 < ALPHA; p2++) {
                            for (int p3 = 0; p3 < ALPHA; p3++) {

                                EnigmaKey key;
                                key_init(&key);

                                key.rotor[0] = a;
                                key.rotor[1] = b;
                                key.rotor[2] = c;

                                key.pos[0] = p1;
                                key.pos[1] = p2;
                                key.pos[2] = p3;
                                key.reflector = r;

                                enigma_crypt(&key, ct, decoded, len);
                                double score = score_ioc(decoded, len);

                                if (saved < max) {
                                    top[saved].key = key;
                                    top[saved].score = score;
                                    saved++;
                                } else {
                                    int worst = 0;

                                    for (int i = 1; i < saved; i++) {
                                        if (top[i].score < top[worst].score) {
                                            worst = i;
                                        }
                                    }

                                    if (score > top[worst].score) {
                                        top[worst].key = key;
                                        top[worst].score = score;
                                    }
                                }

                                count++;
                            }
                        }
                    }
                }
            }
        }
    }

    if (opt->verbose) {
        fprintf(stderr, "Checked combinations: %d\n", count);
    }

    free(decoded);
    return saved;
}


/*
 * Крок 2. Для кандидата перебрати ринги правого і середнього ротора (26 × 26).
 * Змінюючи ring[i] на d, змінювати й pos[i] на d, щоб проводка стояла так само,
 * а мінявся тільки момент кроку. Ринг лівого ротора не впливає на результат.
 * Оцінка — score_ioc. Оновити c->key і c->score, якщо стало краще.
 */

static void stage2_rings(const int *ct, int len, Candidate *c)
{
    if (len <= 0) {
        return;
    }

    int *decoded = malloc(sizeof *decoded * (size_t)len);
    if (decoded == NULL) {
        fprintf(stderr, "crack: out of memory\n");
        return;
    }

    EnigmaKey original = c->key;
    EnigmaKey best_key = original;
    double best_score = c->score;

    for (int middle = 0; middle < ALPHA; middle++) {
        for (int right = 0; right < ALPHA; right++) {
            EnigmaKey key = original;

            key.ring[1] = middle;
            key.ring[2] = right;

            key.pos[1] = (original.pos[1] + middle) % ALPHA;
            key.pos[2] = (original.pos[2] + right) % ALPHA;

            enigma_crypt(&key, ct, decoded, len);

            double score = score_ioc(decoded, len);

            if (score > best_score) {
                best_score = score;
                best_key = key;
            }
        }
    }

    c->key = best_key;
    c->score = best_score;

    free(decoded);
}


/*
 * Крок 3. Hill-climbing плагборду для key: пробувати додати / прибрати / поміняти
 * пару (i, j), лишати зміну, якщо оцінка зросла. Оцінка: спочатку IoC, потім
 * score_bigram, в кінці score_trigram. Не більше opt->max_plugs пар,
 * opt->restarts перезапусків. Повертає фінальну оцінку (триграми).
 */
static double stage3_plugboard(const int *ct, int len, const CrackOptions *opt,
                               EnigmaKey *key)
{
    /* TODO(C) */
    (void)ct;
    (void)len;
    (void)opt;
    (void)key;
    return 0.0;
}

double crack(const int *ct, int len, const CrackOptions *opt, EnigmaKey *best)
{
    key_init(best);

    Candidate *top = malloc(sizeof *top * (size_t)opt->top_n);
    if (!top) {
        fprintf(stderr, "crack: out of memory\n");
        return 0.0;
    }

    int n = stage1_rotors(ct, len, opt, top, opt->top_n);
    if (opt->verbose)
        fprintf(stderr, "stage 1: %d candidates\n", n);

    double best_score = 0.0;
    int found = 0;
    for (int i = 0; i < n; i++) {
        stage2_rings(ct, len, &top[i]);
        double s = stage3_plugboard(ct, len, opt, &top[i].key);
        if (!found || s > best_score) {
            best_score = s;
            *best = top[i].key;
            found = 1;
        }
    }

    free(top);
    return best_score;
}
