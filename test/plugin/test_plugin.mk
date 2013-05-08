OS_NAME			:= $(shell uname)

AUG_DIR			= ../../..
CCAN_DIR		= $(AUG_DIR)/libccan
INCLUDES		= -iquote"$(AUG_DIR)/include" -I. -iquote"$(AUG_DIR)/test"
INCLUDES		+= -I$(CCAN_DIR)
CXX_FLAGS		= -ggdb -Wall -Wextra $(INCLUDES)
CXX_CMD			= gcc $(CXX_FLAGS)
SRCS			= $(wildcard ./*.c)
OBJECTS			= $(patsubst %.c, %.o, $(SRCS) ) 
DEP_FLAGS		= -MMD -MP -MF $(patsubst %.o, %.d, $@)

ifeq ($(OS_NAME), Darwin)
	SO_FLAGS	= -dynamiclib -Wl,-undefined,dynamic_lookup 
else
	SO_FLAGS	= -shared 
endif

default: all

.PHONY: all 
all: $(OUTPUT).so

$(OUTPUT).so: $(OBJECTS)
	$(CXX_CMD) $(SO_FLAGS) $+ -o $@

define cc-template
$(CXX_CMD) $(DEP_FLAGS) -fPIC -c $< -o $@
endef

%.o: %.c
	$(cc-template)

.PHONY: clean
clean:
	rm -f *.o *.so config.h *.d

-include $(wildcard *.d )
