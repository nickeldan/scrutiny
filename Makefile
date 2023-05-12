CFLAGS := -std=gnu99 -fdiagnostics-color -Wall -Wextra -Werror -fvisibility=hidden -DGEAR_NO_EXPORT
ifeq ($(debug),yes)
    CFLAGS += -O0 -g -DDEBUG
else
    CFLAGS += -O2 -DNDEBUG
endif

ifeq ($(monkeypatch),yes)
CFLAGS += -DSCR_MONKEYPATCH -DREAP_NO_PROC -DREAP_NO_ITERATE_FD -DREAP_NO_ITERATE_NET -DREAP_NO_ITERATE_THREAD -DREAP_NO_EXPORT -DEJ_NO_EXPORT
endif

all: _all

BUILD_DEPS :=
ifeq ($(MAKECMDGOALS),clean)
else ifeq ($(MAKECMDGOALS),format)
else ifeq ($(MAKECMDGOALS),uninstall)
else
    BUILD_DEPS := yes
endif

GEAR_DIR := packages/gear
include $(GEAR_DIR)/make.mk

SCR_SHARED_LIBRARY := libscrutiny.so

SCR_SOURCE_FILES := $(wildcard src/*.c)
SCR_OBJECT_FILES := $(patsubst %.c,%.o,$(SCR_SOURCE_FILES)) $(GEAR_OBJECT_FILES)
SCR_ONLY_HEADER_FILES := $(wildcard include/scrutiny/*.h)
SCR_HEADER_FILES := $(SCR_ONLY_HEADER_FILES) $(GEAR_HEADER_FILES)
SCR_INCLUDE_FLAGS := -Iinclude $(GEAR_INCLUDE_FLAGS)

REAP_DIR := packages/reap
include $(REAP_DIR)/make.mk

EJ_DIR := packages/elfjack
include $(EJ_DIR)/make.mk

ifeq ($(monkeypatch),yes)
SCR_OBJECT_FILES += $(REAP_OBJECT_FILES) $(EJ_OBJECT_FILES)
SCR_HEADER_FILES += $(REAP_HEADER_FILES) $(EJ_HEADER_FILES)
SCR_INCLUDE_FLAGS += $(REAP_INCLUDE_FLAGS) $(EJ_INCLUDE_FLAGS)
endif

SCR_DEPS_FILE := src/deps.mk
DEPS_FILES += $(SCR_DEPS_FILE)

BUILD_DEPS ?= $(if $(MAKECMDGOALS),$(subst clean,,$(MAKECMDGOALS)),yes)

ifneq ($(BUILD_DEPS),)

$(SCR_DEPS_FILE): $(SCR_SOURCE_FILES) $(SCR_HEADER_FILES)
	@mkdir -p $(@D)
	@rm -f $@
	for file in $(SCR_SOURCE_FILES); do \
	    echo "src/`$(CC) $(SCR_INCLUDE_FLAGS) -MM $$file`" >> $@ && \
	    echo '\t$$(CC) $$(CFLAGS) -fpic -ffunction-sections $(SCR_INCLUDE_FLAGS) -c $$< -o $$@' >> $@; \
	done
include $(SCR_DEPS_FILE)

endif

$(SCR_SHARED_LIBRARY): $(SCR_OBJECT_FILES)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -Wl,--gc-sections -shared -o $@ $(filter %.o,$^)

scr_clean:
	@rm -f $(SCR_SHARED_LIBRARY) $(SCR_OBJECT_FILES)

CLEAN_TARGETS += scr_clean

ifneq ($(docker_build),yes)
TEST_DIR := tests
include $(TEST_DIR)/make.mk
endif

.PHONY: all _all format install uninstall clean tests $(CLEAN_TARGETS)

_all: $(SCR_SHARED_LIBRARY)

format:
	find . -path ./packages -prune -o -name '*.[hc]' -print0 | xargs -0 -n 1 clang-format -i

install: /usr/local/lib/$(SCR_SHARED_LIBRARY) $(foreach file,$(SCR_ONLY_HEADER_FILES),/usr/local/include/scrutiny/$(notdir $(file)))
	ldconfig

/usr/local/lib/$(notdir $(SCR_SHARED_LIBRARY)): $(SCR_SHARED_LIBRARY)
	cp $< $@

/usr/local/include/scrutiny/%.h: include/scrutiny/%.h
	@mkdir -p $(@D)
	cp $< $@

uninstall:
	rm -rf /usr/local/include/scrutiny
	rm -f /usr/local/lib/$(SCR_SHARED_LIBRARY)
	ldconfig

clean: $(CLEAN_TARGETS)
	@rm -f $(DEPS_FILES)
