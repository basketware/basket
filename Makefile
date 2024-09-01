TARGET ?= native

CC ?= gcc

PKG-CONFIG ?= pkg-config
LIBS ?= $(shell $(PKG-CONFIG) --libs sdl2)

OUT = out/$(TARGET)

SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,$(OUT)/%.o,$(SOURCES))

DYN-EXT ?= .so
STATIC = $(OUT)/libbasket.a
DYNAMIC = $(OUT)/libbasket$(DYN-EXT)

# Compiler flags
CFLAGS ?= -Wall -Wextra -pedantic
CFLAGS += $(shell $(PKG-CONFIG) --cflags sdl2)

# Include directories
INCLUDES = -Iinclude

# Default target
all: static

static: $(STATIC)
dynamic: $(DYNAMIC)

# Rule for static library
$(STATIC): $(OBJECTS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^

# Rule for dynamic library
$(DYNAMIC): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) -shared -o $@ $^ $(LIBS)

# Rule for object files
$(OUT)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean rule
clean:
	rm -rf $(OUT)

# Phony targets
.PHONY: all clean

# Include dependencies
-include $(OBJECTS:.o=.d)

# Rule to generate dependency files
$(OUT)/%.d: %.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(INCLUDES) -MM -MT '$(OUT)/$*.o $@' $< > $@
