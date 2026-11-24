#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "../src/enigma.h"
#include "../src/crack.h"
#include "../src/score.h"

int main(void)
{
    CrackOptions opt;
    crack_default_options(&opt);

    CHECK(opt.top_n == 200);
    CHECK(opt.stage3_top == 50);
    CHECK(opt.restarts == 5);
    CHECK(opt.max_plugs == 10);
    CHECK(opt.both_reflectors == 0);
    CHECK(opt.verbose == 0);

    EnigmaKey recovered;
    int invalid[] = {0, 26, 2};
    int valid[] = {0, 1, 2};

    CHECK(crack(NULL, 3, &opt, &recovered) == -INFINITY);
    CHECK(crack(valid, 0, &opt, &recovered) == -INFINITY);
    CHECK(crack(valid, 3, NULL, &recovered) == -INFINITY);
    CHECK(crack(valid, 3, &opt, NULL) == -INFINITY);
    CHECK(crack(invalid, 3, &opt, &recovered) == -INFINITY);

    int bg = ngram_load("data/english_bigrams.txt", 2);
    int tg = ngram_load("data/english_trigrams.txt", 3);

    CHECK(bg >= 0);
    CHECK(tg >= 0);

    if (bg < 0 || tg < 0)
        return TEST_SUMMARY("test_crack");

    const char *message =
        "THEDEVELOPMENTOFMODERNCOMPUTERSHASCHANGEDTHEWAYPEOPLE"
        "COMMUNICATEANDWORKEVERYDAYMILLIONSOFPEOPLEUSETECHNOLOGY"
        "TOSOLVEPROBLEMSANDSHAREINFORMATIONCOMPUTERSCIENCEHELPS"
        "USUNDERSTANDHOWALGORITHMSPROCESSDATAANDMAKEDECISIONS"
        "CRYPTOGRAPHYISANIMPORTANTPARTOFTHISFIELDBECAUSEIT"
        "PROTECTSINFORMATIONFROMUNAUTHORIZEDACCESSHISTORICAL"
        "ENCRYPTIONMACHINESPROVIDEINTERESTINGEXAMPLESOFHOW"
        "MATHEMATICSANDENGINEERINGCANWORKTOGETHER";

    int len = (int)strlen(message);
    int plain[len];
    int cipher[len];
    int decoded[len];

    for (int i = 0; i < len; i++)
        plain[i] = message[i] - 'A';

    EnigmaKey key;
    key_init(&key);

    int parsed = key_parse(
        "I-II-III B ABC ABC AB CD EF", &key
    );

    CHECK(parsed == 0);

    if (parsed != 0)
        return TEST_SUMMARY("test_crack");

    enigma_crypt(&key, plain, cipher, len);

    opt.top_n = 10;
    opt.stage3_top = 3;
    opt.restarts = 1;
    opt.max_plugs = 3;

    srand(1);

    double score = crack(cipher, len, &opt, &recovered);

    CHECK(isfinite(score));

    if (isfinite(score)) {
        enigma_crypt(&recovered, cipher, decoded, len);

        CHECK(memcmp(plain, decoded, sizeof plain) == 0);
        CHECK(recovered.reflector == key.reflector);
        CHECK(memcmp(recovered.rotor, key.rotor,
                     sizeof key.rotor) == 0);
        CHECK(memcmp(recovered.plug, key.plug,
                     sizeof key.plug) == 0);
    }

    return TEST_SUMMARY("test_crack");
}
