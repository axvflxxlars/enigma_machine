/*
 * text.c — перетворення тексту.                           Відповідальний: B
 */
#include "text.h"

int text_to_nums(const char *s, int *out, int max)
{
    /* TODO(B): пройти по s, брати тільки латинські літери, A/a -> 0 ... Z/z -> 25. */
    (void)s;
    (void)out;
    (void)max;
    return 0;
}

void nums_to_text(const int *in, int len, char *out)
{
    /* TODO(B): 0 -> 'A' ... 25 -> 'Z', в кінці '\0'. */
    (void)in;
    (void)len;
    out[0] = '\0';
}
