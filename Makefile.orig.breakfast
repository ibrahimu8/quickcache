CC = gcc
CFLAGS = -Wall -O2 -g
LDFLAGS = -lcrypto -lzstd -lsqlite3 -lpthread -lcurl

TARGET = buildcache
SRCDIR = src
OBJDIR = obj
SRCS = $(SRCDIR)/main.c $(SRCDIR)/hash.c $(SRCDIR)/cache.c $(SRCDIR)/exec.c $(SRCDIR)/utils.c $(SRCDIR)/stats.c $(SRCDIR)/metadata.c $(SRCDIR)/compress.c $(SRCDIR)/clean.c $(SRCDIR)/config.c $(SRCDIR)/network.c
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
mkdir -p $(OBJDIR)

clean:
rm -rf $(OBJDIR) $(TARGET)

install: $(TARGET)
install -m 755 $(TARGET) /usr/local/bin/

test: $(TARGET)
./tests/test_basic.sh

.PHONY: all clean install test
