#
# INFO: If you want to enable logs, set this to 1.
ENABLE_LOGGING := 1

#
# INFO: `BUILD_MODE` can be either release (default) or debug.
BUILD_MODE := debug

CC := $(if $(shell which ccache),ccache) clang
CFLAGS := \
	-Wall -Wextra -pedantic \
	-Wno-c11-extensions \
	-std=c17 \
	-O3 \
	-I. \
	$(if $(ENABLE_LOGGING),-DENABLE_LOGGING)

ifeq ($(BUILD_MODE),debug)
CFLAGS += -g
else
CFLAGS += -DNDEBUG
endif

LD := $(CC)
LDFLAGS := -lm

BUILD_PATH := build
OBJECTS_PATH := $(BUILD_PATH)/.objects
PATHS := $(BUILD_PATH) $(OBJECTS_PATH)

TARGET := $(BUILD_PATH)/buffer
SOURCES := $(wildcard source/*.c)
OBJECTS := $(patsubst source/%.c,$(OBJECTS_PATH)/%.o,$(SOURCES))

.PHONY: default
default: $(PATHS) $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJECTS_PATH)/%.o: source/%.c
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

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)
