
/*
 * crack.c — злам Enigma M3 тільки по шифротексту.
 * Відповідальний: C
 */

#include "crack.h"

#include <stdio.h>
#include <stdlib.h>

#include "score.h"

typedef struct {
    EnigmaKey key;
    double score;
} Candidate;


/* ---------- Допоміжні функції плагборду ---------- */

static int plug_count(const EnigmaKey *key)
{
    int count = 0;

    for (int i = 0; i < ALPHA; i++) {
        if (key->plug[i] > i) {
            count++;
        }
    }

    return count;
}

static void plug_remove(EnigmaKey *key, int a)
{
    int b = key->plug[a];

    key->plug[a] = a;
    key->plug[b] = b;
}

static void plug_add(EnigmaKey *key, int a, int b)
{
    plug_remove(key, a);
    plug_remove(key, b);

    key->plug[a] = b;
    key->plug[b] = a;
}

static double plug_score(const int *ct, int len,
                         const EnigmaKey *key,
                         int *decoded, int score_type)
{
    enigma_crypt(key, ct, decoded, len);

    if (score_type == 0) {
        return score_ioc(decoded, len);
    }

    if (score_type == 1) {
        return score_bigram(decoded, len);
    }

    return score_trigram(decoded, len);
}


/* ---------- Налаштування ---------- */

void crack_default_options(CrackOptions *opt)
{
    if (opt == NULL) {
        return;
    }

    opt->top_n = 200;
    opt->restarts = 5;
    opt->max_plugs = 10;
    opt->both_reflectors = 0;
    opt->verbose = 0;
}


/* ---------- Етап 1: ротори та позиції ---------- */

static int stage1_rotors(const int *ct, int len,
                         const CrackOptions *opt,
                         Candidate *top, int max)
{
    if (len <= 0 || max <= 0) {
        return 0;
    }

    int *decoded = malloc((size_t)len * sizeof *decoded);

    if (decoded == NULL) {
        fprintf(stderr, "crack: out of memory\n");
        return 0;
    }

    int checked = 0;
    int saved = 0;

    int reflectors = opt->both_reflectors
        ? NUM_REFLECTORS
        : 1;

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

                                enigma_crypt(
                                    &key, ct, decoded, len
                                );

                                double score = score_ioc(
                                    decoded, len
                                );

                                if (saved < max) {
                                    top[saved].key = key;
                                    top[saved].score = score;
                                    saved++;
                                } else {
                                    int worst = 0;

                                    for (int i = 1; i < saved; i++) {
                                        if (top[i].score <
                                            top[worst].score) {
                                            worst = i;
                                        }
                                    }

                                    if (score > top[worst].score) {
                                        top[worst].key = key;
                                        top[worst].score = score;
                                    }
                                }

                                checked++;
                            }
                        }
                    }
                }
            }
        }
    }

    if (opt->verbose) {
        fprintf(
            stderr,
            "stage 1: checked %d combinations\n",
            checked
        );
    }

    free(decoded);

    return saved;
}


/* ---------- Етап 2: підбір кілець ---------- */

static void stage2_rings(const int *ct, int len, Candidate *c)
{
    if (len <= 0) {
        return;
    }

    int *decoded = malloc((size_t)len * sizeof *decoded);

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

            key.pos[1] =
                (original.pos[1] + middle) % ALPHA;

            key.pos[2] =
                (original.pos[2] + right) % ALPHA;

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


/* ---------- Етап 3: оптимізація плагборду ---------- */

static double stage3_plugboard(const int *ct, int len,
                               const CrackOptions *opt,
                               EnigmaKey *key)
{
    if (len <= 0) {
        return 0.0;
    }

    int *decoded = malloc((size_t)len * sizeof *decoded);

    if (decoded == NULL) {
        fprintf(stderr, "crack: out of memory\n");
        return 0.0;
    }

    EnigmaKey initial = *key;
    EnigmaKey best_key = initial;

    double best_score = -1e300;

    int max_plugs = opt->max_plugs;

    if (max_plugs < 0) {
        max_plugs = 0;
    }

    if (max_plugs > ALPHA / 2) {
        max_plugs = ALPHA / 2;
    }

    int restarts = opt->restarts > 0
        ? opt->restarts
        : 1;

    for (int restart = 0; restart < restarts; restart++) {

        EnigmaKey current = initial;

        /*
         * Перший запуск починається з порожнього
         * плагборду. Наступні — з випадкових пар.
         */
        if (restart > 0 && max_plugs > 0) {

            int pairs = restart < max_plugs
                ? restart
                : max_plugs;

            for (int p = 0; p < pairs; p++) {
                int a = rand() % ALPHA;
                int b = rand() % ALPHA;

                if (a != b &&
                    current.plug[a] == a &&
                    current.plug[b] == b) {
                    plug_add(&current, a, b);
                }
            }
        }

        /*
         * Послідовна оптимізація:
         * 0 — IoC
         * 1 — біграми
         * 2 — триграми
         */
        for (int score_type = 0;
             score_type < 3;
             score_type++) {

            double current_score = plug_score(
                ct, len, &current, decoded, score_type
            );

            int improved = 1;

            while (improved) {
                improved = 0;

                EnigmaKey next_key = current;
                double next_score = current_score;

                /*
                 * Додавання або заміна пари.
                 */
                for (int a = 0; a < ALPHA; a++) {
                    for (int b = a + 1; b < ALPHA; b++) {

                        EnigmaKey candidate = current;

                        plug_add(&candidate, a, b);

                        if (plug_count(&candidate) >
                            max_plugs) {
                            continue;
                        }

                        double score = plug_score(
                            ct, len, &candidate,
                            decoded, score_type
                        );

                        if (score > next_score) {
                            next_score = score;
                            next_key = candidate;
                            improved = 1;
                        }
                    }
                }

                /*
                 * Видалення наявних пар.
                 */
                for (int a = 0; a < ALPHA; a++) {

                    if (current.plug[a] <= a) {
                        continue;
                    }

                    EnigmaKey candidate = current;

                    plug_remove(&candidate, a);

                    double score = plug_score(
                        ct, len, &candidate,
                        decoded, score_type
                    );

                    if (score > next_score) {
                        next_score = score;
                        next_key = candidate;
                        improved = 1;
                    }
                }

                if (improved) {
                    current = next_key;
                    current_score = next_score;
                }
            }
        }

        double final_score = plug_score(
            ct, len, &current, decoded, 2
        );

        if (final_score > best_score) {
            best_score = final_score;
            best_key = current;
        }

        if (opt->verbose) {
            fprintf(
                stderr,
                "plugboard restart %d/%d: "
                "score=%.6f, pairs=%d\n",
                restart + 1,
                restarts,
                final_score,
                plug_count(&current)
            );
        }
    }

    *key = best_key;

    free(decoded);

    return best_score;
}


/* ---------- Головна функція зламу ---------- */

double crack(const int *ct, int len,
             const CrackOptions *opt, EnigmaKey *best)
{
    if (best == NULL) {
        return 0.0;
    }

    key_init(best);

    if (ct == NULL || opt == NULL ||
        len <= 0 || opt->top_n <= 0) {
        return 0.0;
    }

    Candidate *top = malloc(
        (size_t)opt->top_n * sizeof *top
    );

    if (top == NULL) {
        fprintf(stderr, "crack: out of memory\n");
        return 0.0;
    }

    int n = stage1_rotors(
        ct, len, opt, top, opt->top_n
    );

    if (opt->verbose) {
        fprintf(
            stderr,
            "stage 1: %d candidates selected\n",
            n
        );
    }

    double best_score = 0.0;
    int found = 0;

    for (int i = 0; i < n; i++) {

        stage2_rings(ct, len, &top[i]);

        double score = stage3_plugboard(
            ct, len, opt, &top[i].key
        );

        if (!found || score > best_score) {
            best_score = score;
            *best = top[i].key;
            found = 1;
        }

        if (opt->verbose) {
            fprintf(
                stderr,
                "candidate %d/%d: score=%.6f\n",
                i + 1,
                n,
                score
            );
        }
    }

    free(top);

    return best_score;
}
