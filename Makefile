CFLAGS := -std=gnu99 -fdiagnostics-color -Wall -Wextra -Wshadow -Werror
ifeq ($(debug),yes)
    CFLAGS += -O0 -g -DDEBUG
else
    CFLAGS += -O2 -DNDEBUG
endif
LDFLAGS := -Wl,--version-script,src/vis.map

all: _all

BUILD_DEPS :=
ifeq ($(MAKECMDGOALS),clean)
else ifeq ($(MAKECMDGOALS),format)
else
    BUILD_DEPS := yes
endif

GEAR_DIR := packages/gear
include $(GEAR_DIR)/make.mk

SCR_SHARED_LIBRARY := libscrutiny.so
SCR_STATIC_LIBRARY := libscrutiny.a

SCR_SOURCE_FILES := $(wildcard src/*.c)
SCR_OBJECT_FILES := $(patsubst %.c,%.o,$(SCR_SOURCE_FILES))
SCR_HEADER_FILES := $(wildcard include/scrutiny/*.h)
SCR_INCLUDE_FLAGS := -Iinclude $(GEAR_INCLUDE_FLAGS)

SCR_DEPS_FILE := src/deps.mk
DEPS_FILES += $(SCR_DEPS_FILE)

ifneq ($(BUILD_DEPS),)

$(SCR_DEPS_FILE): $(SCR_SOURCE_FILES) $(GEAR_HEADER_FILES) $(SCR_HEADER_FILES) $(wildcard src/*.h)
	@rm -f $@
	for file in $(SCR_SOURCE_FILES); do \
	    echo "src/`$(CC) $(SCR_INCLUDE_FLAGS) -MM $$file`" >> $@ && \
	    echo '\t$$(CC) $$(CFLAGS) -fpic -ffunction-sections $(SCR_INCLUDE_FLAGS) -c $$< -o $$@' >> $@; \
	done
include $(SCR_DEPS_FILE)

endif

$(SCR_SHARED_LIBRARY): $(SCR_OBJECT_FILES) $(GEAR_OBJECT_FILES)
	$(CC) $(LDFLAGS) -shared -o $@ $^

$(SCR_STATIC_LIBRARY): $(SCR_OBJECT_FILES) $(GEAR_OBJECT_FILES)
	$(AR) rcs $@ $^

TEST_DIR := tests
include $(TEST_DIR)/make.mk

.PHONY: all _all format install uninstall clean tests $(CLEAN_TARGETS)

_all: $(SCR_SHARED_LIBRARY) $(SCR_STATIC_LIBRARY)

format:
	find . -path ./packages -prune -o -name '*.[hc]' -print0 | xargs -0 -n 1 clang-format -i

install: /usr/local/lib/$(SCR_SHARED_LIBRARY) $(foreach file,$(SCR_HEADER_FILES),/usr/local/include/scrutiny/$(notdir $(file)))

/usr/local/lib/$(SCR_SHARED_LIBRARY): $(SCR_SHARED_LIBRARY)
	cp $< $@

/usr/local/include/scrutiny/%.h: include/scrutiny/%.h
	@mkdir -p $(@D)
	cp $< $@

uninstall:
	rm -rf /usr/local/include/scrutiny
	rm -f /usr/local/lib/$(SCR_SHARED_LIBRARY)

clean: $(CLEAN_TARGETS)
	@rm -f $(SCR_SHARED_LIBRARY) $(SCR_STATIC_LIBRARY) $(SCR_OBJECT_FILES) $(DEPS_FILES)
