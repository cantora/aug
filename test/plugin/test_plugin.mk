AUG_DIR			= ../../..
CCAN_DIR		= $(AUG_DIR)/libccan
INCLUDES		= -iquote"$(AUG_DIR)/include" -I. -iquote"$(AUG_DIR)/test"
INCLUDES		+= -I$(CCAN_DIR)
CXX_FLAGS		= -ggdb -Wall -Wextra $(INCLUDES)
CXX_CMD			= gcc $(CXX_FLAGS)
SRCS			= $(wildcard ./*.c)
OBJECTS			= $(patsubst %.c, %.o, $(SRCS) ) 
DEP_FLAGS		= -MMD -MP -MF $(patsubst %.o, %.d, $@)

default: all

.PHONY: all 
all: $(OUTPUT).so

$(OUTPUT).so: $(OBJECTS)
	$(CXX_CMD) -shared $+ -o $@ 

define cc-template
$(CXX_CMD) $(DEP_FLAGS) -fPIC -c $< -o $@
endef

%.o: %.c
	$(cc-template)

.PHONY: clean
clean:
	rm -f *.o *.so config.h *.d

-include $(wildcard *.d )
