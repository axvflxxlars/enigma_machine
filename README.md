# Enigma M3 і брейкер на C

Симулятор Enigma M3 і програма, яка зламує англійський шифротекст без кріпу, тобто без відомого фрагмента відкритого тексту. Злам іде через індекс збігу (IoC) і n-грами (метод Gillogly / Weierud–Sullivan).

Дедлайн: **25 листопада**.

## Збірка і запуск

```bash
make            # ./enigma і build/gen_testcase
make test       # усі тести
make clean && make debug   # з -g і AddressSanitizer
```

```bash
echo "HELLO WORLD" | ./enigma encrypt --key "I-II-III B AAA AAA AB CD EF"
./enigma crack --ngrams data -v < tests/testset/t01.cipher.txt
```

Формат ключа: `ротори рефлектор ринги позиції [пари плагборду]`, наприклад `IV-II-V C KTR QWE AB CD EF`.

## Структура

```
src/
  enigma.c/h   симулятор M3                                   A
  text.c/h     текст <-> числа 0..25                          B
  score.c/h    IoC, біграми, триграми                         B
  crack.c/h    злам: ротори -> ринги -> плагборд              C
  main.c       CLI (encrypt / crack)                          C
tests/
  test_enigma.c  еталонні вектори, double-step, roundtrip     A
  test_score.c   IoC, n-грами                                 B
  testset/       згенеровані шифротексти і ключі              B
tools/
  gen_testcase.c генератор тестів                             B
data/
  english_bigrams.txt, english_trigrams.txt                   B
```

Місця, які треба реалізувати, позначено в коді `TODO(A)`, `TODO(B)`, `TODO(C)`:

```bash
grep -rn "TODO(A)" src tools tests
```

## Ролі

| | Роль | Файли |
|---|---|---|
| **A** | Машина | `enigma.c`, `tests/test_enigma.c`, потім оптимізація |
| **B** | Оцінка тексту | `text.c`, `score.c`, `tools/gen_testcase.c`, `data/`, тест-сет, цей README |
| **C** | Брейкер | `crack.c`, `main.c`, `Makefile` |

## Правила

- Літери всередині програми завжди числа 0..25.
- Інтерфейс у `src/*.h` заморожено 12.11. Змінювати його тільки після обговорення з усіма.
- Кожен працює у своїй гілці, у `main` зливати через pull request, який переглянув ще хтось один.
- Перед злиттям перевірити, що `make` збирається без попереджень.

## Контрольні точки

- [ ] **14.11** базова машина без рингів і плагборду (A)
- [ ] **17.11** `test_enigma` і `test_score` проходять; брейкер знаходить ротори на тексті без плагборду
- [ ] **21.11** брейкер ламає текст на 300 літер із 10 парами плагборду
- [ ] **24.11** README з таблицею результатів, демо
- [ ] **25.11** здача

## Результати

_Заповнює B: успішність зламу за довжиною тексту і час роботи._

| Довжина | Тестів | Зламано | Середній час |
|---|---|---|---|
| 100 | | | |
| 200 | | | |
| 300 | | | |
| 500 | | | |
