#
# INFO: If you want to perform benchmarks, uncomment the following line.
#DO_BENCHMARK :=

#
# INFO: `BUILD_MODE` can be either release (default) or debug.
BUILD_MODE := release

CC := $(if $(shell which ccache),ccache) clang++
CFLAGS := \
	-Wall -Wextra -pedantic \
	-std=c++20 \
	-O3 \
	-I. \
	$(ifdef DO_BENCHMARK,-DBENCHMARK)

ifeq ($(BUILD_MODE),debug)
CFLAGS += -g
else
CFLAGS += -DNDEBUG
endif

LD := $(CC)
LDFLAGS := $(ifdef DO_BENCHMARK,-lbenchmark)

BUILD_PATH := build
OBJECTS_PATH := $(BUILD_PATH)/.objects
PATHS := $(BUILD_PATH) $(OBJECTS_PATH)

TARGET := $(BUILD_PATH)/buffer
SOURCES := $(wildcard source/*.cc)
OBJECTS := $(patsubst source/%.cc,$(OBJECTS_PATH)/%.o,$(SOURCES))

.PHONY: default
default: $(PATHS) $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJECTS_PATH)/%.o: source/%.cc
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
