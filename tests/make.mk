TEST_BINARIES := $(patsubst %.c,%,$(wildcard $(TEST_DIR)/test_*.c))
CURDIR := $(shell pwd)

ifeq ($(monkeypatch),yes)

AUX_LIB := $(TEST_DIR)/libaux.so

$(AUX_LIB): $(TEST_DIR)/aux.c
	$(CC) -shared -fPIC $(CFLAGS) $< -o $@

$(TEST_DIR)/test_monkeypatch: $(TEST_DIR)/test_monkeypatch.c $(TEST_DIR)/common.h $(SCR_SHARED_LIBRARY) $(AUX_LIB)
	$(CC) $(CFLAGS) $(SCR_INCLUDE_FLAGS) $< -Wl,-rpath $(CURDIR) -Wl,-rpath $(TEST_DIR) -L$(CURDIR) -L$(TEST_DIR) -lscrutiny -laux -o $@

endif

tests: $(TEST_BINARIES)
	failed=0; for binary in $(TEST_BINARIES); do ./$$binary || failed=$$((failed+1)); done; test $$failed = 0

$(TEST_DIR)/test_%: $(TEST_DIR)/test_%.c $(TEST_DIR)/common.h $(SCR_SHARED_LIBRARY)
	$(CC) $(CFLAGS) $(SCR_INCLUDE_FLAGS) $< -Wl,-rpath $(CURDIR) -L$(CURDIR) -lscrutiny -o $@

test_clean:
	@rm -f $(TEST_BINARIES) $(AUX_LIB)

.PHONY: test_clean
CLEAN_TARGETS += test_clean
