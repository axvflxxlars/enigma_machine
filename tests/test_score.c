/* test_score.c — тести перетворення і оцінки тексту.     Відповідальний: B
 *
 * Запускати з кореня репо (через make test), бо n-грами читаються з data/.
 */
#include <stdlib.h>
#include <string.h>

#include "../src/score.h"
#include "../src/text.h"
#include "test.h"

static const char *ENGLISH =
    "It was the best of times, it was the worst of times, it was the age of wisdom, "
    "it was the age of foolishness, it was the epoch of belief, it was the epoch of "
    "incredulity, it was the season of Light, it was the season of Darkness, it was "
    "the spring of hope, it was the winter of despair, we had everything before us, "
    "we had nothing before us, we were all going direct to Heaven, we were all going "
    "direct the other way.";

static void test_text_conversion(void)
{
    int  t[16];
    char s[17];
    int  len = text_to_nums("Ab, z!", t, 16);
    CHECK(len == 3);
    CHECK(t[0] == 0 && t[1] == 1 && t[2] == 25);
    nums_to_text(t, len, s);
    CHECK(strcmp(s, "ABZ") == 0);
    CHECK(text_to_nums("ABCDEF", t, 4) == 4);           /* не більше max */

    /* Кирилиця й інші не-латинські символи відкидаються. */
    len = text_to_nums("Привіт, AB ç!", t, 16);
    CHECK(len == 2);
    CHECK(t[0] == 0 && t[1] == 1);
}

/* Псевдовипадкові літери з фіксованим seed. */
static void random_letters(int *t, int len)
{
    srand(12345);
    for (int i = 0; i < len; i++)
        t[i] = rand() % 26;
}

static void test_ioc(void)
{
    int t[2000];
    int len = text_to_nums(ENGLISH, t, 2000);
    double e = score_ioc(t, len);
    CHECK(e > 0.058 && e < 0.080);

    random_letters(t, 2000);
    double r = score_ioc(t, 2000);
    CHECK(r > 0.034 && r < 0.042);
}

static void test_ngrams(void)
{
    if (ngram_load("data/english_bigrams.txt", 2) != 0 ||
        ngram_load("data/english_trigrams.txt", 3) != 0) {
        fprintf(stderr, "  SKIP ngrams: немає data/english_bigrams.txt або data/english_trigrams.txt\n");
        CHECK(0);
        return;
    }

    int t[2000], r[2000];
    int len = text_to_nums(ENGLISH, t, 2000);
    random_letters(r, len);

    CHECK(score_bigram(t, len) > score_bigram(r, len));
    CHECK(score_trigram(t, len) > score_trigram(r, len));
}

int main(void)
{
    test_text_conversion();
    test_ioc();
    test_ngrams();
    return TEST_SUMMARY("test_score");
}
