/*
 * score.c — оцінка тексту.                                Відповідальний: B
 */
#include "score.h"

#include "enigma.h"   /* ALPHA */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* Таблиці log10-ймовірностей. Індекс біграми: a*26 + b, триграми: (a*26 + b)*26 + c. */
static double bigram[ALPHA * ALPHA];
static double trigram[ALPHA * ALPHA * ALPHA];

double score_ioc(const int *t, int len)
{
    if (len < 2)
        return 0.0;

    int freq[ALPHA] = {0};

    for (int i = 0; i < len; i++)
    {
        freq[t[i]]++;
    }

    double sum = 0.0;

    for (int i = 0; i < ALPHA; i++)
    {
        sum += (double)freq[i] * (freq[i] - 1);
    }

    return sum / ((double)len * (len - 1));
}

int ngram_load(const char *path, int n)
{
    if (n != 2 && n != 3)
        return -1;

    FILE *file = fopen(path, "r");
    if (file == NULL)
        return -1;

    char letters[4];
    double count;
    double total = 0.0;

    while (fscanf(file, "%3s %lf", letters, &count) == 2)
{
    if (strlen(letters) != (size_t)n || !isfinite(count) || count <= 0.0)
        continue;

    int valid = 1;

    for (int i = 0; i < n; i++)
    {
        if (letters[i] < 'A' || letters[i] > 'Z')
        {
            valid = 0;
            break;
        }
    }

    if (valid)
        total += count;
}

    if (total <= 0.0)
    {
        fclose(file);
        return -1;
    }

double fallback = log10(0.01 / total);

    if (n == 2)
    {
        for (int i = 0; i < ALPHA * ALPHA; i++)
            bigram[i] = fallback;
    }
    else
    {
        for (int i = 0; i < ALPHA * ALPHA * ALPHA; i++)
            trigram[i] = fallback;
    }

    rewind(file);

    while (fscanf(file, "%3s %lf", letters, &count) == 2)
    {

 if (strlen(letters) != (size_t)n || !isfinite(count) || count <= 0.0)
            continue;

        int valid = 1;

        for (int i = 0; i < n; i++)
        {
            if (letters[i] < 'A' || letters[i] > 'Z')
            {
                valid = 0;
                break;
            }
        }

        if (!valid)
            continue;

        int a = letters[0] - 'A';
        int b = letters[1] - 'A';

        if (n == 2)
        {
            int index = a * ALPHA + b;
            bigram[index] = log10(count / total);
        }
        else
        {
            int c = letters[2] - 'A';
            int index = (a * ALPHA + b) * ALPHA + c;
            trigram[index] = log10(count / total);
        }
    }

    fclose(file);
    return 0;
}

double score_bigram(const int *t, int len)
{
    if (len < 2)
        return 0.0;

    double sum = 0.0;

    for (int i = 0; i < len - 1; i++)
    {
        int index = t[i] * ALPHA + t[i + 1];
        sum += bigram[index];
    }

    return sum;
}

double score_trigram(const int *t, int len)
{
    if (len < 3)
        return 0.0;

    double sum = 0.0;

    for (int i = 0; i < len - 2; i++)
    {
        int index = (t[i] * ALPHA + t[i + 1]) * ALPHA + t[i + 2];
        sum += trigram[index];
    }

    return sum;
}
