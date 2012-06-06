OUTPUT			= api_test
AUG_DIR			= ../../..
CCAN_DIR		= $(AUG_DIR)/libccan
CCAN_CP			= ./ccan
INCLUDES		= -iquote"$(AUG_DIR)/include" -I. -iquote"$(AUG_DIR)/test" -iquote"$(AUG_DIR)/sandbox"
CXX_FLAGS		= -Wall -Wextra $(INCLUDES) -DWANT_PTHREAD
CXX_CMD			= gcc $(CXX_FLAGS)
SRCS			= $(wildcard ./*.c)
OBJECTS			= $(patsubst %.c, %.o, $(SRCS) ) 

default: all

.PHONY: all 
all: $(OUTPUT).so

$(OUTPUT).so: $(OBJECTS)
	$(CXX_CMD) -shared $+ -o $@ 

%.o: %.c
	$(CXX_CMD) -fPIC -c $< -o $@

$(OUTPUT).c: $(CCAN_CP)

$(CCAN_CP):
	mkdir -p $(CCAN_CP)
	cp -R $(CCAN_DIR)/ccan/compiler $(CCAN_CP)
	cp -R $(CCAN_DIR)/ccan/array_size $(CCAN_CP)
	cp -R $(CCAN_DIR)/ccan/build_assert $(CCAN_CP)
	cp $(CCAN_DIR)/config.h ./

.PHONY: clean
clean:
	rm -f *.o *.so config.h
	rm -rf $(CCAN_CP)

