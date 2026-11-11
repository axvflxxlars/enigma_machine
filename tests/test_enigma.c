/* test_enigma.c — тести симулятора.                      Відповідальний: A */
#include <string.h>

#include "../src/enigma.h"
#include "../src/text.h"
#include "test.h"

/* Зашифрувати рядок ключем і порівняти з очікуваним. */
static int encrypts_to(const EnigmaKey *key, const char *plain, const char *expected)
{
    int  t[256];
    char out[257];
    int  len = text_to_nums(plain, t, 256);
    enigma_crypt(key, t, t, len);
    nums_to_text(t, len, out);
    return strcmp(out, expected) == 0;
}

static void test_basic_vector(void)
{
    EnigmaKey k;
    key_init(&k);                                   /* I-II-III B AAA AAA */
    CHECK(encrypts_to(&k, "AAAAA", "BDZGO"));
}

static void test_ring_vector(void)
{
    EnigmaKey k;
    key_init(&k);
    k.ring[0] = k.ring[1] = k.ring[2] = 1;          /* ринги BBB */
    CHECK(encrypts_to(&k, "AAAAA", "EWTYX"));
}

static void test_double_step(void)
{
    int rotor[3] = { 0, 1, 2 };                     /* I-II-III */
    int pos[3]   = { 0, 3, 20 };                    /* A D U */

    enigma_step(rotor, pos);
    CHECK(pos[0] == 0 && pos[1] == 3 && pos[2] == 21);   /* ADV */
    enigma_step(rotor, pos);
    CHECK(pos[0] == 0 && pos[1] == 4 && pos[2] == 22);   /* AEW */
    enigma_step(rotor, pos);
    CHECK(pos[0] == 1 && pos[1] == 5 && pos[2] == 23);   /* BFX */
}

static void test_roundtrip_with_plugs(void)
{
    EnigmaKey k;
    CHECK(key_parse("IV-II-V C KTR QWE AB CD EF GH IJ KL MN OP QR ST", &k) == 0);

    const char *plain = "THEQUICKBROWNFOXJUMPSOVERTHELAZYDOG";
    int  a[64], b[64];
    int  len = text_to_nums(plain, a, 64);
    enigma_crypt(&k, a, b, len);
    enigma_crypt(&k, b, b, len);
    CHECK(memcmp(a, b, sizeof(int) * (size_t)len) == 0);
}

static void test_no_letter_to_itself(void)
{
    EnigmaKey k;
    key_init(&k);
    int a[ALPHA * 4], b[ALPHA * 4];
    for (int i = 0; i < ALPHA * 4; i++)
        a[i] = i % ALPHA;
    enigma_crypt(&k, a, b, ALPHA * 4);
    int ok = 1;
    for (int i = 0; i < ALPHA * 4; i++)
        if (a[i] == b[i])
            ok = 0;
    CHECK(ok);
}

static void test_key_parse_print(void)
{
    const char *s = "V-I-III C BCD XYZ AZ BY";
    EnigmaKey k;
    CHECK(key_parse(s, &k) == 0);
    CHECK(k.rotor[0] == 4 && k.rotor[1] == 0 && k.rotor[2] == 2);
    CHECK(k.reflector == REFL_C);
    CHECK(k.ring[0] == 1 && k.pos[2] == 25);
    CHECK(k.plug[0] == 25 && k.plug[25] == 0 && k.plug[2] == 2);

    char buf[128];
    FILE *f = fmemopen(buf, sizeof buf, "w");
    key_print(&k, f);
    fclose(f);
    CHECK(strcmp(buf, s) == 0);

    CHECK(key_parse("I-I-III B AAA AAA", &k) != 0);      /* однакові ротори */
    CHECK(key_parse("I-II-III B AAA AAA AB AC", &k) != 0); /* A двічі в плагборді */
}

int main(void)
{
    test_basic_vector();
    test_ring_vector();
    test_double_step();
    test_roundtrip_with_plugs();
    test_no_letter_to_itself();
    test_key_parse_print();
    return TEST_SUMMARY("test_enigma");
}
