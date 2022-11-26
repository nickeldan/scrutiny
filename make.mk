ifndef SCR_MK
SCR_MK :=

GEAR_DIR := $(SCR_DIR)/packages/gear
include $(GEAR_DIR)/make.mk

SCR_LIB_DIR ?= $(SCR_DIR)
SCR_OBJ_DIR ?= $(SCR_DIR)/src

SCR_SHARED_LIBRARY := $(SCR_LIB_DIR)/libscrutiny.so
SCR_STATIC_LIBRARY := $(SCR_LIB_DIR)/libscrutiny.a

SCR_SOURCE_FILES := $(wildcard $(SCR_DIR)/src/*.c)
SCR_OBJECT_FILES := $(patsubst $(SCR_DIR)/src/%.c,$(SCR_OBJ_DIR)/%.o,$(SCR_SOURCE_FILES))
SCR_HEADER_FILES := $(wildcard $(SCR_DIR)/include/scrutiny/*.h) $(GEAR_HEADER_FILES)
SCR_INCLUDE_FLAGS := -I$(SCR_DIR)/include $(GEAR_INCLUDE_FLAGS)

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

$(SCR_SHARED_LIBRARY): $(SCR_OBJECT_FILES)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -shared -o $@ $^

$(SCR_STATIC_LIBRARY): $(SCR_OBJECT_FILES)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^

scr_clean:
	@rm -f $(SCR_SHARED_LIBRARY) $(SCR_STATIC_LIBRARY) $(SCR_OBJECT_FILES)

CLEAN_TARGETS += scr_clean

endif
