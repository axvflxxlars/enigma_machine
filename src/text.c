/*
 * text.c — перетворення тексту.                           Відповідальний: B
 */
#include "text.h"
#include <ctype.h>

int text_to_nums(const char *s, int *out, int max)
{
    /* TODO(B): пройти по s, брати тільки латинські літери, A/a -> 0 ... Z/z -> 25. */
    int j = 0;
    for (int i = 0; s[i] != '\0'; i++)
    {   if (j == max) return j;
        if(isalpha(s[i])){
            out[j] = toupper(s[i]) - 'A';
            j++;
        }
    }
    
    return j;
}

void nums_to_text(const int *in, int len, char *out)
{
    /* TODO(B): 0 -> 'A' ... 25 -> 'Z', в кінці '\0'. */
    for (int i = 0; i != len; i++)
    {
        out[i] = in[i] + 'A';
    }
    out[len] = '\0';
}
