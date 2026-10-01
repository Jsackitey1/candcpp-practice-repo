CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
SANITIZERS = address,undefined

.PHONY: all test sanitize clean
all: CPUSim

CPUSim: CPUSim.c
	$(CC) $(CFLAGS) CPUSim.c -o CPUSim

test: CPUSim
	python3 tests/test_sim.py ./CPUSim

sanitize:
	$(CC) -std=c11 -Wall -Wextra -Wpedantic -g -fsanitize=$(SANITIZERS) CPUSim.c -o CPUSim-sanitize
	python3 tests/test_sim.py ./CPUSim-sanitize

clean:
	rm -f CPUSim CPUSim-sanitize
