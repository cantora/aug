.SECONDARY:

DEFAULT_CMD		= '{"/bin/sh", NULL}'
DEFAULT_TERM	= \"screen\"
DEFINES			= -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED=1 -D_BSD_SOURCE -DAUG_DEFAULT_TERM=$(DEFAULT_TERM) -DAUG_DEFAULT_ARGV=$(DEFAULT_CMD)
OUTPUT			= aug
BUILD			= ./build
MKBUILD			:= $(shell mkdir -p $(BUILD) )
LIBVTERM		= ./libvterm/.libs/libvterm.a
CCAN_DIR		= ./libccan
LIBCCAN			= $(CCAN_DIR)/libccan.a
LIB 			= -lutil -lncursesw $(LIBVTERM) $(LIBCCAN)
INCLUDES		= -iquote"./libvterm/include" -iquote"./libvterm/src" -iquote"./src" -iquote"./include"
OPTIMIZE		= -ggdb #-O3
CXX_FLAGS		= $(OPTIMIZE) -Wall -Wextra $(INCLUDES) $(DEFINES)
CXX_CMD			= gcc $(CXX_FLAGS)

SRCS			= $(notdir $(filter-out ./src/$(OUTPUT).c, $(wildcard ./src/*.c) ) $(BUILD)/vterm_ansi_colors.c )
OBJECTS			= $(patsubst %.c, $(BUILD)/%.o, $(SRCS) ) 

TESTS 			= $(notdir $(patsubst %.c, %, $(wildcard ./test/*.c) ) )
TEST_OUTPUTS	= $(foreach test, $(TESTS), $(BUILD)/$(test))
TEST_OBJECTS	= $(OBJECTS)

default: all

.PHONY: all
all: $(OUTPUT)

$(LIBVTERM): ./libvterm
	$(MAKE) $(MFLAGS) -C ./libvterm

./libvterm:
	bzr checkout lp:libvterm

$(CCAN_DIR):
	git clone 'https://github.com/rustyrussell/ccan.git' $(CCAN_DIR)

$(LIBCCAN): $(CCAN_DIR)
	cd $(CCAN_DIR) && $(MAKE) $(MFLAGS) -f ./tools/Makefile tools/configurator/configurator
	$(CCAN_DIR)/tools/configurator/configurator > $(CCAN_DIR)/config.h
	cd $(CCAN_DIR) && $(MAKE) $(MFLAGS)

$(BUILD)/vterm_ansi_colors.c: $(LIBVTERM)
	{ \
	echo '#include "vterm.h"'; \
	awk '/static const VTermColor ansi_colors\[\].*/, /};/' ./libvterm/src/pen.c \
		| sed 's/ansi_colors/vterm_ansi_colors/' \
		| sed 's/^static const/const/'; \
	} > $@

$(OUTPUT): $(BUILD)/$(OUTPUT).o $(OBJECTS)
	$(CXX_CMD) $+ $(LIB) -o $@

$(BUILD)/$(OUTPUT).o: ./src/$(OUTPUT).c $(LIBVTERM) $(LIBCCAN)
	$(CXX_CMD) -c $< -o $@

$(BUILD)/screen.o: ./src/screen.c ./src/vterm_ansi_colors.h
	$(CXX_CMD) -c $< -o $@

$(BUILD)/%.o: $(BUILD)/%.c
	$(CXX_CMD) -c $< -o $@

$(BUILD)/%.o: ./src/%.c ./src/%.h
	$(CXX_CMD) -c $< -o $@

$(BUILD)/%.o: ./src/%.c
	$(CXX_CMD) -c $< -o $@

$(BUILD)/%.o: ./test/%.c
	$(CXX_CMD) -c $< -o $@

define test-template
$$(BUILD)/$(1): $$(BUILD)/$(1).o $$(TEST_OBJECTS)
	$(CXX_CMD) $$+ $$(LIB) -o $$@

$(1): $$(BUILD)/$(1) 
#	$(BUILD)/$(1)
endef

.PHONY: $(TESTS) 
$(foreach test, $(TESTS), $(eval $(call test-template,$(test)) ) )

.PHONY: clean 
clean: 
	rm -rf $(BUILD)
	rm -f $(OUTPUT)

.PHONY: libclean
libclean: clean
	rm -rf ./libvterm
	rm -rf $(CCAN_DIR)
