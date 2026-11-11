CC      ?= gcc
CFLAGS  ?= -std=gnu11 -Wall -Wextra -O2
LDLIBS  += -lm

BUILD   := build
LIB_SRC := src/enigma.c src/text.c src/score.c src/crack.c
LIB_OBJ := $(LIB_SRC:src/%.c=$(BUILD)/%.o)
TESTS   := $(BUILD)/test_enigma $(BUILD)/test_score
HEADERS := $(wildcard src/*.h) tests/test.h

.PHONY: all test debug clean

all: enigma $(BUILD)/gen_testcase

enigma: $(LIB_OBJ) $(BUILD)/main.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD)/%.o: src/%.c $(HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/test_%: tests/test_%.c $(LIB_OBJ) $(HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIB_OBJ) $(LDLIBS)

$(BUILD)/gen_testcase: tools/gen_testcase.c $(LIB_OBJ) $(HEADERS) | $(BUILD)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIB_OBJ) $(LDLIBS)

$(BUILD):
	mkdir -p $@

# Запускає всі тести, навіть якщо якийсь упав.
test: $(TESTS)
	@status=0; for t in $(TESTS); do ./$$t || status=1; done; exit $$status

# Збірка для налагодження: make clean && make debug
debug: CFLAGS  = -std=gnu11 -Wall -Wextra -g -O0 -fsanitize=address,undefined
debug: LDFLAGS = -fsanitize=address,undefined
debug: all $(TESTS)

clean:
	rm -rf $(BUILD) enigma
