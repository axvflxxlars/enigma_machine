#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "$0")"

make

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

key="I-II-III B ABC ABC AB CD EF"

cat > "$tmp/plain.txt" <<'TEXT'
The development of modern computers has changed the way people
communicate and work. Every day millions of people use technology
to solve problems and share information. Computer science helps
us understand how algorithms process data and make decisions.
Cryptography is an important part of this field because it
protects information from unauthorized access. Historical
encryption machines provide interesting examples of how
mathematics and engineering can work together.
TEXT

echo "Original key: $key"
echo
echo "Original text:"
cat "$tmp/plain.txt"

./enigma encrypt --key "$key" \
    < "$tmp/plain.txt" > "$tmp/cipher.txt"

echo
echo "Ciphertext:"
cat "$tmp/cipher.txt"

echo
echo "Cracking Enigma..."

time ./enigma crack --top 200 --stage3-top 50 -v \
    < "$tmp/cipher.txt" > "$tmp/result.txt"

echo
echo "Result:"
cat "$tmp/result.txt"

tr -cd 'A-Za-z' < "$tmp/plain.txt" |
    tr 'a-z' 'A-Z' > "$tmp/expected.txt"

tail -n 1 "$tmp/result.txt" |
    tr -d '\n' > "$tmp/actual.txt"

echo

if cmp -s "$tmp/expected.txt" "$tmp/actual.txt"; then
    echo "SUCCESS: plaintext recovered completely"
else
    echo "FAIL: plaintext does not match"
    exit 1
fi
