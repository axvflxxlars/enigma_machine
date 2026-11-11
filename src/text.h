/*
 * text.h — перетворення тексту.                           Відповідальний: B
 */
#ifndef TEXT_H
#define TEXT_H
#include <stddef.h>


/*
 * Перетворити рядок у числа 0..25: літери a-z/A-Z -> 0..25, усе інше відкинути.
 * Записує не більше max чисел. Повертає кількість записаних.
 */
int text_to_nums(const char *s, int *out, int max);

/* Числа 0..25 -> великі літери. out має вміщати len + 1 символ (з '\0'). */
void nums_to_text(const int *in, int len, char *out);

#endif
