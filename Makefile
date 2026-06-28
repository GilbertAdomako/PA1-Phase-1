CC = gcc
CFLAGS = -Wall -Wextra -g

.PHONY: all clean test

all: logger counter

logger: logger.c logger.h
	$(CC) $(CFLAGS) -o logger logger.c

counter: counter.c logger.h
	$(CC) $(CFLAGS) -o counter counter.c

test: all
	@echo "========================================"
	@echo " Running Testcase 1: Empty Directory"
	@echo "========================================"
	./logger testcases/testcase1
	@echo "\n========================================"
	@echo " Running Testcase 2: Single Clean File"
	@echo "========================================"
	./logger testcases/testcase2
	@echo "\n========================================"
	@echo " Running Testcase 3: Empty & Corrupted"
	@echo "========================================"
	./logger testcases/testcase3
	@echo "\n========================================"
	@echo " Running Testcase 4: 5 Mixed Files"
	@echo "========================================"
	./logger testcases/testcase4
	@echo "\n========================================"
	@echo " Running Testcase 5: 15 Mixed Files"
	@echo "========================================"
	./logger testcases/testcase5
	@echo "\nTesting complete."

clean:
	rm -f logger counter
	find . -type f -name "*.log" -delete