.SECONDARY:

DEFAULT_CMD		= '{"/bin/sh", NULL}'
DEFAULT_TERM	= \"screen\"

DEFINES			= -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED=1 -D_BSD_SOURCE
DEFINES			+= -DAUG_DEFAULT_TERM=$(DEFAULT_TERM)
DEFINES			+= -DAUG_DEFAULT_ARGV=$(DEFAULT_CMD)

OUTPUT			= aug
BUILD			= ./build
MKBUILD			:= $(shell mkdir -p $(BUILD) )
LIBVTERM		= ./libvterm/.libs/libvterm.a
CCAN_DIR		= ./libccan
LIBCCAN			= $(CCAN_DIR)/libccan.a
LIB 			= -lutil -lpanel -lncursesw $(LIBVTERM) $(LIBCCAN)

INCLUDES		= -iquote"./libvterm/include" -I$(CCAN_DIR)
INCLUDES		+= -iquote"./src" -iquote"./include"

OPTIMIZE		= -ggdb #-O3
CXX_FLAGS		= $(OPTIMIZE) -Wall -Wextra $(INCLUDES) $(DEFINES) 
CXX_CMD			= gcc $(CXX_FLAGS)

SRCS			= $(notdir $(filter-out ./src/$(OUTPUT).c, $(wildcard ./src/*.c) ) $(BUILD)/vterm_ansi_colors.c )
OBJECTS			= $(patsubst %.c, $(BUILD)/%.o, $(SRCS) ) 

#PLUGIN_DIRS		= $(notdir $(shell find ./plugin -maxdepth 1 -mindepth 1 -type d) )
#PLUGIN_OBJECTS	= $(foreach dir, $(PLUGIN_DIRS), ./plugin/$(dir)/$(dir).so )
PLUGIN_DIRS		= $(shell find ./plugin -maxdepth 1 -mindepth 1 -type d) $(shell find ./test/plugin -maxdepth 1 -mindepth 1 -type d) 
PLUGIN_OBJECTS	= $(foreach dir, $(PLUGIN_DIRS), $(dir)/$(notdir $(dir) ).so )

TESTS 			= $(notdir $(patsubst %.c, %, $(wildcard ./test/*.c) ) )
TEST_OUTPUTS	= $(foreach test, $(TESTS), $(BUILD)/$(test))

SANDBOX_PGMS	= $(notdir $(patsubst %.c, %, $(wildcard ./sandbox/*.c) ) )
SANDBOX_OUTPUTS	= $(foreach sbox_pgm, $(SANDBOX_PGMS), $(BUILD)/$(sbox_pgm))

default: all

.PHONY: all
all: $(OUTPUT) $(PLUGIN_OBJECTS)

$(LIBVTERM): ./libvterm
	$(MAKE) $(MFLAGS) -C ./libvterm

./libvterm:
	bzr checkout lp:libvterm

CCAN_WARNING_PATCH		= $(CCAN_DIR)/ccan/htable/htable_type.h
$(CCAN_DIR):
	git clone 'https://github.com/rustyrussell/ccan.git' $(CCAN_DIR)
	sed 's/return hashfn(keyof((const type \*)elem));/(void)(priv); return hashfn(keyof((const type *)elem));/' \
		$(CCAN_WARNING_PATCH) > $(CCAN_WARNING_PATCH).tmp && mv $(CCAN_WARNING_PATCH).tmp $(CCAN_WARNING_PATCH)

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
	$(CXX_CMD) -DAUG_CORE -c $< -o $@

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

$(BUILD)/%.o: ./sandbox/%.c
	$(CXX_CMD) -c $< -o $@

./plugin/%.so: 
	$(MAKE) $(MFLAGS) -C ./$(dir $@) 

./test/plugin/%.so: 
	$(MAKE) $(MFLAGS) -C ./$(dir $@)

define aux-program-template
$$(BUILD)/$(1): $$(BUILD)/$(1).o $$(OBJECTS)
	$(CXX_CMD) $$+ $$(LIB) -o $$@

$(1): $$(BUILD)/$(1) 
#	$(BUILD)/$(1) 
endef

.PHONY: $(TESTS) 
$(foreach test, $(filter-out api_test, $(TESTS)), $(eval $(call aux-program-template,$(test)) ) )

$(BUILD)/api_test: $(BUILD)/api_test.o $(filter-out $(BUILD)/screen.o, $(OBJECTS) ) $(PLUGIN_OBJECTS)
	$(CXX_CMD) $(filter-out $(BUILD)/screen.o, $(OBJECTS) ) $(BUILD)/api_test.o $(LIB) -o $@

api_test: $(BUILD)/api_test

.PHONY: $(SANDBOX_PGMS) 
$(foreach thing, $(SANDBOX_PGMS), $(eval $(call aux-program-template,$(thing)) ) )

.PHONY: clean 
clean: 
	rm -rf $(BUILD)
	rm -f $(OUTPUT)
	for i in $(PLUGIN_DIRS); do dir=$$i; echo "clean $$dir"; $(MAKE) $(MFLAGS) -C $$dir clean; done

.PHONY: libclean
libclean: clean
	rm -rf ./libvterm
	rm -rf $(CCAN_DIR)
