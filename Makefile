CC ?= gcc

PKG_CONFIG ?= pkg-config

OUT ?= out/

SOURCES := $(wildcard *.c) $(wildcard lib/*.c)
OBJECTS := $(patsubst %.c,$(OUT)/%.o,$(SOURCES))

DYN-EXT ?= .so
STATIC = $(OUT)/libbasket.a
DYNAMIC = $(OUT)/libbasket$(DYN-EXT)

# Compiler flags
CFLAGS ?= $(shell $(PKG_CONFIG) --cflags sdl2) -Wall -Wextra -pedantic
LDFLAGS += $(shell $(PKG_CONFIG) --libs sdl2)

ifndef IGNORE_LIBM
	LDFLAGS += -lm
endif

ifdef DEBUG
CFLAGS += -ggdb -fsanitize=address
else
CFLAGS += -Ofast -ffast-math
endif

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
	$(CC) $(LDFLAGS) -shared -o $@ $^ $(LIBS)

# Rule for object files
$(OUT)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -fPIC -c $< -o $@

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
