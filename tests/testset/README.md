# Тестовий набір

20 шифротекстів: 5 англійських текстів × 4 довжини, 5 пар плагборду, рефлектор B.

| Файли | Довжина |
|---|---|
| `t01`–`t05` | 100 |
| `t06`–`t10` | 200 |
| `t11`–`t15` | 300 |
| `t16`–`t20` | 500 |

У кожній групі тексти йдуть по черзі: `gpl3`, `mpl2`, `artistic`, `apache2`, `gfdl`
(уривки англійських ліцензій із `texts/`). Номер тесту — це і seed генератора.

Відтворити набір (Linux, glibc: `rand()` на інших системах дає інші ключі):

```bash
make
n=0
for len in 100 200 300 500; do
  for t in gpl3 mpl2 artistic apache2 gfdl; do
    n=$((n + 1))
    ./build/gen_testcase tests/testset/texts/$t.txt tests/testset/t$(printf %02d $n) 5 $len $n
  done
done
```
