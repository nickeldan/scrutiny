ifndef SCR_MK
SCR_MK :=

GEAR_DIR := $(SCR_DIR)/packages/gear
include $(GEAR_DIR)/make.mk

SCR_LIB_DIR ?= $(SCR_DIR)
SCR_OBJ_DIR ?= $(SCR_DIR)/src

SCR_SHARED_LIBRARY := $(SCR_LIB_DIR)/libscrutiny.so
SCR_STATIC_LIBRARY := $(SCR_LIB_DIR)/libscrutiny.a

SCR_SOURCE_FILES := $(wildcard $(SCR_DIR)/src/*.c)
SCR_OBJECT_FILES := $(patsubst $(SCR_DIR)/src/%.c,$(SCR_OBJ_DIR)/%.o,$(SCR_SOURCE_FILES)) $(GEAR_OBJECT_FILES)
SCR_ONLY_HEADER_FILES := $(wildcard $(SCR_DIR)/include/scrutiny/*.h)
SCR_HEADER_FILES := $(SCR_ONLY_HEADER_FILES) $(GEAR_HEADER_FILES)
SCR_INCLUDE_FLAGS := -I$(SCR_DIR)/include $(GEAR_INCLUDE_FLAGS)

ifeq ($(monkeypatch),yes)
CFLAGS += -DSCR_MONKEYPATCH -DREAP_NO_PROC -DREAP_NO_ITERATE_FD -DREAP_NO_ITERATE_NET -DREAP_NO_ITERATE_THREAD
endif

REAP_DIR := $(SCR_DIR)/packages/reap
include $(REAP_DIR)/make.mk

EJ_DIR := $(SCR_DIR)/packages/elfjack
include $(EJ_DIR)/make.mk

ifeq ($(monkeypatch),yes)

SCR_OBJECT_FILES += $(REAP_OBJECT_FILES) $(EJ_OBJECT_FILES)
SCR_HEADER_FILES += $(REAP_HEADER_FILES) $(EJ_HEADER_FILES)
SCR_INCLUDE_FLAGS += $(REAP_INCLUDE_FLAGS) $(EJ_INCLUDE_FLAGS)

endif

SCR_DEPS_FILE := $(SCR_OBJ_DIR)/deps.mk
DEPS_FILES += $(SCR_DEPS_FILE)

BUILD_DEPS ?= $(if $(MAKECMDGOALS),$(subst clean,,$(MAKECMDGOALS)),yes)

ifneq ($(BUILD_DEPS),)

$(SCR_DEPS_FILE): $(SCR_SOURCE_FILES) $(SCR_HEADER_FILES)
	@mkdir -p $(@D)
	@rm -f $@
	for file in $(SCR_SOURCE_FILES); do \
	    echo "$(SCR_OBJ_DIR)/`$(CC) $(SCR_INCLUDE_FLAGS) -MM $$file`" >> $@ && \
	    echo '\t$$(CC) $$(CFLAGS) -fpic -ffunction-sections $(SCR_INCLUDE_FLAGS) -c $$< -o $$@' >> $@; \
	done
include $(SCR_DEPS_FILE)

endif

$(SCR_SHARED_LIBRARY): $(SCR_OBJECT_FILES) $(SCR_DIR)/src/vis.map
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -Wl,--version-script,$(SCR_DIR)/src/vis.map -Wl,--gc-sections -shared -o $@ $(filter %.o,$^)

$(SCR_STATIC_LIBRARY): $(SCR_OBJECT_FILES)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^

scr_clean:
	@rm -f $(SCR_SHARED_LIBRARY) $(SCR_STATIC_LIBRARY) $(SCR_OBJECT_FILES)

CLEAN_TARGETS += scr_clean

endif
