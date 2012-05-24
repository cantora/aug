
AUG_DIR			= ../..
OUTPUT			= hello.so
INCLUDES		= -iquote"$(AUG_DIR)/include"
CXX_FLAGS		= -Wall -Wextra $(INCLUDES)
CXX_CMD			= gcc $(CXX_FLAGS)
SRCS			= $(wildcard ./*.c)
OBJECTS			= $(patsubst %.c, %.o, $(SRCS) ) 

default: all

.PHONY: all
all: $(OUTPUT) 

$(OUTPUT): $(OBJECTS)
	$(CXX_CMD) -shared $+ -o $@

%.o: %.c
	$(CXX_CMD) -fPIC -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o *.so