# QuickCache Build System
CC = gcc
CFLAGS = -Wall -O2 -g -std=c11
LDFLAGS = -lpthread

# Use pkg-config to get correct flags for dependencies
PKG_CONFIG ?= pkg-config

# Check for libraries and add flags
HAS_OPENSSL = $(shell $(PKG_CONFIG) --exists openssl && echo yes)
HAS_LIBCURL = $(shell $(PKG_CONFIG) --exists libcurl && echo yes)
HAS_ZSTD = $(shell $(PKG_CONFIG) --exists libzstd && echo yes)
HAS_SQLITE3 = $(shell $(PKG_CONFIG) --exists sqlite3 && echo yes)

ifeq ($(HAS_OPENSSL),yes)
CFLAGS += $(shell $(PKG_CONFIG) --cflags openssl)
LDFLAGS += $(shell $(PKG_CONFIG) --libs openssl)
else
$(error openssl development libraries not found via pkg-config)
endif

ifeq ($(HAS_LIBCURL),yes)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libcurl)
LDFLAGS += $(shell $(PKG_CONFIG) --libs libcurl)
else
$(error libcurl development libraries not found via pkg-config)
endif

ifeq ($(HAS_ZSTD),yes)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libzstd)
LDFLAGS += $(shell $(PKG_CONFIG) --libs libzstd)
else
$(warning libzstd not found via pkg-config, trying -lzstd)
LDFLAGS += -lzstd
endif

ifeq ($(HAS_SQLITE3),yes)
CFLAGS += $(shell $(PKG_CONFIG) --cflags sqlite3)
LDFLAGS += $(shell $(PKG_CONFIG) --libs sqlite3)
else
$(warning sqlite3 not found via pkg-config, trying -lsqlite3)
LDFLAGS += -lsqlite3
endif

# Targets and Objects
TARGET = buildcache
SRCDIR = src
OBJDIR = obj
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall
