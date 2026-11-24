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

/* Середній і правий ротори одночасно на нотчі: середній крокує лише раз. */
static void test_double_step_both_notches(void)
{
    int rotor[3] = { 0, 1, 2 };                     /* I-II-III */
    int pos[3]   = { 0, 4, 21 };                    /* A E V */

    enigma_step(rotor, pos);
    CHECK(pos[0] == 1 && pos[1] == 5 && pos[2] == 22);   /* BFW */
    enigma_step(rotor, pos);
    CHECK(pos[0] == 1 && pos[1] == 5 && pos[2] == 23);   /* BFX */
}

/* Кожен ротор I–V праворуч і посередині штовхає сусіда на своєму нотчі. */
static void test_all_notches(void)
{
    for (int r = 0; r < NUM_ROTORS; r++) {
        int notch = ROTOR_NOTCH[r] - 'A';
        int next  = (notch + 1) % ALPHA;
        int other1 = (r + 1) % NUM_ROTORS;
        int other2 = (r + 2) % NUM_ROTORS;

        /* Правий ротор на нотчі: середній крокує, лівий ні. */
        int rotor_right[3] = { other1, other2, r };
        int pos_right[3]   = { 0, 0, notch };       /* A A <нотч> */
        enigma_step(rotor_right, pos_right);
        CHECK(pos_right[0] == 0 && pos_right[1] == 1 && pos_right[2] == next);

        /* Середній ротор на нотчі: double-step, крокують усі три. */
        int rotor_mid[3] = { other1, r, other2 };
        int right_pos    = (ROTOR_NOTCH[other2] - 'A' + 5) % ALPHA;   /* не на нотчі */
        int pos_mid[3]   = { 0, notch, right_pos };
        enigma_step(rotor_mid, pos_mid);
        CHECK(pos_mid[0] == 1 && pos_mid[1] == next && pos_mid[2] == (right_pos + 1) % ALPHA);
    }
}

/* Лівий ротор переходить через Z → A. */
static void test_wraparound(void)
{
    int rotor[3] = { 0, 1, 2 };                     /* I-II-III */
    int pos[3]   = { 25, 4, 0 };                    /* Z E A */

    enigma_step(rotor, pos);
    CHECK(pos[0] == 0 && pos[1] == 5 && pos[2] == 1);    /* A F B */
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

/* key_parse: пробіли й '\n' як роздільники, некоректні рядки відкидаються. */
static void test_key_parse_edge_cases(void)
{
    EnigmaKey k;

    /* Ключ, прочитаний з файлу, закінчується '\n'. */
    CHECK(key_parse("I-II-III B AAA AAA AB CD\n", &k) == 0);
    CHECK(k.plug[2] == 3 && k.plug[3] == 2);
    CHECK(key_parse("I-II-III  B\tAAA AAA", &k) == 0);

    CHECK(key_parse("I-II-IIIX B AAA AAA", &k) != 0);    /* хвіст після ротора */
    CHECK(key_parse("I-II-III-IV B AAA AAA", &k) != 0);  /* чотири ротори */
    CHECK(key_parse("I-II B AAA AAA", &k) != 0);         /* два ротори */
    CHECK(key_parse("I-II-VI B AAA AAA", &k) != 0);      /* ротора VI немає */
    CHECK(key_parse("I-II-III A AAA AAA", &k) != 0);     /* рефлектора A немає */
    CHECK(key_parse("I-II-III B AA AAA", &k) != 0);      /* два ринги */
    CHECK(key_parse("I-II-III B AAA AAA AB C", &k) != 0); /* неповна пара */
    CHECK(key_parse("I-II-III B AAA AAA AA", &k) != 0);  /* літера з'єднана сама з собою */
    CHECK(key_parse("", &k) != 0);

    /* Рядок довший за внутрішній буфер не обрізається мовчки. */
    char longkey[300];
    snprintf(longkey, sizeof longkey, "I-II-III B AAA AAA%200s", "ZZ");
    CHECK(key_parse(longkey, &k) != 0);
}

/* Таблиця шляху дає той самий результат, що й enigma_crypt. */
static void test_path_matches_crypt(void)
{
    static const char *const keys[] = {
        "IV-II-V C KTR QWE AB CD EF GH IJ KL MN OP QR ST",   /* ринги, UKW C, 10 пар */
        "I-II-III B AAA ADU",                                 /* double-step на початку */
        "V-III-I B ZQM XEZ AZ BY CX",                         /* нотчі Z і V */
        "II-V-IV C MMM JZE AQ BJ CM DX EV FL GO HU IZ KP",    /* нотч J, 10 пар */
    };
    enum { LEN = 400 };   /* довгий текст: double-step і поворот лівого ротора */
    static int path[LEN][ALPHA];
    int a[LEN], b[LEN], c[LEN];

    for (int i = 0; i < LEN; i++)
        a[i] = (i * 7 + 3) % ALPHA;

    for (size_t n = 0; n < sizeof keys / sizeof keys[0]; n++) {
        EnigmaKey k;
        CHECK(key_parse(keys[n], &k) == 0);

        enigma_crypt(&k, a, b, LEN);
        enigma_build_path(&k, LEN, path);
        enigma_crypt_path((const int (*)[ALPHA])path, k.plug, a, c, LEN);

        CHECK(memcmp(b, c, sizeof b) == 0);
    }
}

int main(void)
{
    test_basic_vector();
    test_ring_vector();
    test_double_step();
    test_double_step_both_notches();
    test_all_notches();
    test_wraparound();
    test_roundtrip_with_plugs();
    test_no_letter_to_itself();
    test_key_parse_print();
    test_key_parse_edge_cases();
    test_path_matches_crypt();
    return TEST_SUMMARY("test_enigma");
}
