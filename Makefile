ENABLE_DEBUG := 1
ENABLE_LOGGING := 1

CC     := $(if $(shell which ccache),ccache) clang
CFLAGS := \
	-Wall -Wextra \
	-std=c17 \
	-O0 \
	-I. \
	$(if $(ENABLE_LOGGING),-DENABLE_LOGGING) \
	$(if $(ENABLE_DEBUG),-g -DENABLE_DEBUG,)

LD       := $(CC)
LDFLAGS := -lm

BUILD_PATH   := build
OBJECTS_PATH := $(BUILD_PATH)/.objects
PATHS        := $(BUILD_PATH) $(OBJECTS_PATH)

TARGET  := $(BUILD_PATH)/buffer
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

large_file:
	dd if=/dev/zero of=large.txt bs=1048576 count=32
	

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJECTS)
