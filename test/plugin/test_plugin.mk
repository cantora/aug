AUG_DIR			= ../../..
CCAN_DIR		= $(AUG_DIR)/libccan
CCAN_CP			= ./ccan
INCLUDES		= -iquote"$(AUG_DIR)/include" -I.
CXX_FLAGS		= -Wall -Wextra $(INCLUDES)
CXX_CMD			= gcc $(CXX_FLAGS)
SRCS			= $(wildcard ./*.c)
OBJECTS			= $(patsubst %.c, %.o, $(SRCS) ) 

default: all

.PHONY: all 
all: $(OUTPUT).so

$(OUTPUT).so: $(OBJECTS) tap.o
	$(CXX_CMD) -shared $+ -o $@ 

%.o: %.c
	$(CXX_CMD) -fPIC -c $< -o $@

tap.o: $(CCAN_CP)/tap/tap.c
	$(CXX_CMD) -fPIC -c $< -o $@

$(OUTPUT).c: $(CCAN_CP)

$(CCAN_CP):
	mkdir -p $(CCAN_CP)
	cp -R $(CCAN_DIR)/ccan/tap $(CCAN_CP)
	cp -R $(CCAN_DIR)/ccan/compiler $(CCAN_CP)
	cp $(CCAN_DIR)/config.h ./

.PHONY: clean
clean:
	rm -f *.o *.so config.h
	rm -rf $(CCAN_CP)

