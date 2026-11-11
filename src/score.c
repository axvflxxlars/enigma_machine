/*
 * score.c — оцінка тексту.                                Відповідальний: B
 */
#include "score.h"

#include "enigma.h"   /* ALPHA */

/* Таблиці log10-ймовірностей. Індекс біграми: a*26 + b, триграми: (a*26 + b)*26 + c. */
static double bigram[ALPHA * ALPHA];
static double trigram[ALPHA * ALPHA * ALPHA];

double score_ioc(const int *t, int len)
{
    /* TODO(B): порахувати частоти 26 літер і формулу IoC. Для len < 2 повернути 0. */
    (void)t;
    (void)len;
    return 0.0;
}

int ngram_load(const char *path, int n)
{
    /* TODO(B): відкрити файл, прочитати рядки "XY count", підсумувати total,
     * заповнити bigram[] або trigram[] значеннями log10(count / total). */
    (void)path;
    (void)n;
    (void)bigram;
    (void)trigram;
    return -1;
}

double score_bigram(const int *t, int len)
{
    /* TODO(B): sum bigram[t[i]*26 + t[i+1]] */
    (void)t;
    (void)len;
    return 0.0;
}

double score_trigram(const int *t, int len)
{
    /* TODO(B): sum trigram[(t[i]*26 + t[i+1])*26 + t[i+2]] */
    (void)t;
    (void)len;
    return 0.0;
}
