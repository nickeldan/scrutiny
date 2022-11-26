CC ?= gcc
debug ?= no

CFLAGS := -std=gnu11 -fdiagnostics-color -Wall -Wextra
ifeq ($(debug),yes)
    CFLAGS += -O0 -g -DDEBUG
else
    CFLAGS += -O2 -DNDEBUG
endif

all: _all

BUILD_DEPS :=
ifeq ($(MAKECMDGOALS),clean)
else ifeq ($(MAKECMDGOALS),format)
else
    BUILD_DEPS := yes
endif

SCR_DIR := .
include make.mk

TEST_DIR := tests
include $(TEST_DIR)/make.mk

.PHONY: all _all format install uninstall clean tests $(CLEAN_TARGETS)

_all: $(SCR_SHARED_LIBRARY) $(SCR_STATIC_LIBRARY)

format:
	find . -path ./packages -prune -o -name '*.[hc]' -print0 | xargs -0 -n 1 clang-format -i

install: /usr/local/lib/$(notdir $(SCR_SHARED_LIBRARY)) $(foreach file,$(SCR_HEADER_FILES),/usr/local/include/scrutiny/$(notdir $(file)))

/usr/local/lib/$(notdir $(SCR_SHARED_LIBRARY)): $(SCR_SHARED_LIBRARY)
	cp $< $@

/usr/local/include/scrutiny/%.h: $(SCR_INCLUDE_DIR)/scrutiny/%.h /usr/local/include/scrutiny
	cp $< $@

/usr/local/include/scrutiny:
	mkdir -p $@

uninstall:
	rm -rf /usr/local/include/scrutiny
	rm -f /usr/local/lib/$(notdir $(SCR_SHARED_LIBRARY))

clean: $(CLEAN_TARGETS)
	@rm -f $(DEPS_FILES)
