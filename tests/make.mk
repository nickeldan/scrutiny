TEST_BINARIES := $(patsubst %.c,%,$(wildcard $(TEST_DIR)/test_*.c))

tests: $(TEST_BINARIES)
	failed=0; for binary in $(TEST_BINARIES); do ./$$binary || failed=$$((failed+1)); done; test $$failed = 0

$(TEST_DIR)/test_%: $(TEST_DIR)/test_%.c $(SCR_STATIC_LIBRARY) $(GEAR_STATIC_LIBRARY) $(SCR_HEADER_FILES)
	$(CC) $(CFLAGS) $(SCR_INCLUDE_FLAGS) $(filter-out %.h,$^) -o $@

test_clean:
	@rm -f $(TEST_BINARIES)

CLEAN_TARGETS += test_clean
