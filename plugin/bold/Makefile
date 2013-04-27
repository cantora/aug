OS_NAME			:= $(shell uname)

AUG_DIR			= ../..
OUTPUT			= bold.so
INCLUDES		= -iquote"$(AUG_DIR)/include"
CXX_FLAGS		= -Wall -Wextra $(INCLUDES)
CXX_CMD			= gcc $(CXX_FLAGS)
SRCS			= $(wildcard ./*.c)
OBJECTS			= $(patsubst %.c, %.o, $(SRCS) ) 

ifeq ($(OS_NAME), Darwin)
	SO_FLAGS	= -dynamiclib -Wl,-undefined,dynamic_lookup 
else
	SO_FLAGS	= -shared 
endif

default: all

.PHONY: all
all: $(OUTPUT) 

$(OUTPUT): $(OBJECTS)
	$(CXX_CMD) $(SO_FLAGS) $+ -o $@

%.o: %.c
	$(CXX_CMD) -fPIC -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o *.so