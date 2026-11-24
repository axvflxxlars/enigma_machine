/*
 * text.c — перетворення тексту.                           Відповідальний: B
 */
#include "text.h"

int text_to_nums(const char *s, int *out, int max)
{
    int n = 0;

    /* Тільки латинські літери: isalpha() від від'ємного char (кирилиця в UTF-8)
     * — невизначена поведінка, і в деяких локалях приймає не-латинські літери. */
    for (; *s != '\0' && n < max; s++) {
        if (*s >= 'A' && *s <= 'Z')
            out[n++] = *s - 'A';
        else if (*s >= 'a' && *s <= 'z')
            out[n++] = *s - 'a';
    }
    return n;
}

void nums_to_text(const int *in, int len, char *out)
{
    for (int i = 0; i < len; i++)
        out[i] = (char)('A' + in[i]);
    out[len] = '\0';
}
