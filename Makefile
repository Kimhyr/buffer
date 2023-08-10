BUILD_MODE := debug

CCWRAPPER := $(if $(shell which sccache),sccache,$(if $(shell which ccache),ccache))
CC := $(CCWRAPPER) clang++
CFLAGS := \
	-Wall -Wextra \
	-std=c++20 \
	-I. \
	-fno-exceptions

ifeq ($(BUILD_MODE),debug)
CFLAGS += -DBUILD_MODE=0 -g -O0
else
CFLAGS += -DBUILD_MODE=1 -O3
endif

LD      := $(CC)
LDFLAGS := -lm

BUILD_PATH   := build
OBJECTS_PATH := $(BUILD_PATH)/.objects
PATHS        := $(BUILD_PATH) $(OBJECTS_PATH)

TARGET  := $(BUILD_PATH)/buffer
SOURCES := $(wildcard source/*.cpp)
OBJECTS := $(patsubst source/%.cpp,$(OBJECTS_PATH)/%.o,$(SOURCES))

.PHONY: default
default: $(PATHS) $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJECTS_PATH)/%.o: source/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $^

$(PATHS):
	mkdir -p $@ $(OBJECTS_PATH)

EMPTY := 
SPACE := $(EMPTY) $(EMPTY)

compile_flags:
	@rm -f $@.txt && touch $@.txt
	@for FLAG in $(CFLAGS); do \
		echo $$FLAG >> $@.txt; \
	done

aligned_file:
	dd if=/dev/zero of=aligned_file bs=4096 count=1
	

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)
