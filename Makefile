BUILDMODE := debug

CC := clang++
CFLAGS := \
	-Wall -Wextra -pedantic \
	-std=c++20 \
	-O2 \
	-I.

ifeq ($(BUILDMODE),debug)
CFLAGS += -g
else
CFLAGS += -DNDEBUG
endif

LD := $(CC)
LDFLAGS :=

BUILDPATH := build
OBJECTSPATH := $(BUILDPATH)/.objects
PATHS := $(BUILDPATH) $(OBJECTSPATH)

TARGET := $(BUILDPATH)/buffer
SOURCES := $(wildcard source/*.cc)
OBJECTS := $(patsubst source/%.cc,$(OBJECTSPATH)/%.o,$(SOURCES))

.PHONY: all
all: $(PATHS) $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ 

$(OBJECTSPATH)/%.o: source/%.cc
	$(CC) $(CFLAGS) -o $@ -c $^

$(PATHS):
	mkdir -p $@ $(OBJECTSPATH)

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
