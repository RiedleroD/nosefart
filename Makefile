CC = gcc
CFLAGS = -std=c99
LDFLAGS = -lm -lSDL2
PREFIX = /usr
WANT_DEBUG=TRUE

NAME = nosefart
VERSION = 3.4

BUILDTOP = nsfobj
BUILDDIR = $(BUILDTOP)/build
SRCDIR = src

CFLAGS += -DNSF_PLAYER

ifeq "$(WANT_DEBUG)" "TRUE"
	CFLAGS += -ggdb
else
	CFLAGS += -O2 -fomit-frame-pointer -ffast-math -funroll-loops
	DEBUG_OBJECTS =
endif

CFLAGS +=\
 -I$(SRCDIR)\
 -I$(SRCDIR)/linux\
 -I$(SRCDIR)/sndhrdw\
 -I$(SRCDIR)/machine\
 -I$(SRCDIR)/cpu/nes6502\
 -I$(BUILDTOP)\
 -I/usr/local/include/

NSFINFO_CFLAGS = $(CFLAGS)

FILES =\
 log\
 memguard\
 cpu/nes6502/nes6502\
 cpu/nes6502/dis6502\
 machine/nsf\
 sndhrdw/nes_apu\
 sndhrdw/vrcvisnd\
 sndhrdw/fmopl\
 sndhrdw/vrc7_snd\
 sndhrdw/mmc5_snd\
 sndhrdw/fds_snd

SRCS = $(addsuffix .c, $(FILES) linux/main_linux nsfinfo)
SOURCES = $(addprefix $(SRCDIR)/, $(SRCS))
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

ALL_OBJECTS = $(OBJECTS)
ALL_TARGETS = $(BUILDTOP)/$(NAME)

all: $(ALL_TARGETS)

$(BUILDDIR):
	mkdir -p $(sort $(dir $(ALL_OBJECTS)))

$(BUILDTOP)/config.h: $(BUILDDIR)
	echo "[$@]"
	echo "#define VERSION \"$(VERSION)\"" > $@
	echo "#define NAME \"$(NAME)\"" >> $@

$(BUILDDIR)/dep: $(BUILDTOP)/config.h
	$(CC) $(NSFINFO_CFLAGS) $(CFLAGS) -M $(SOURCES) > $@

-include $(BUILDDIR)/dep/

install: all
	mkdir -p $(PREFIX)/bin
	cp $(ALL_TARGETS) $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/$(NAME)

clean:
	rm -rf nsfobj compile_commands.json

$(BUILDTOP)/$(NAME): $(OBJECTS)
	mkdir -p $(sort $(dir $(ALL_OBJECTS)))
	$(CC) $(NSFINFO_CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(BUILDTOP)/config.h
	mkdir -p $(sort $(dir $(ALL_OBJECTS)))
	$(CC) $(NSFINFO_CFLAGS) -o $@ -c $<

# from https://sarcasm.github.io/notes/dev/compilation-database.html#clang
compile_commands.json: CC = clang
compile_commands.json: CFLAGS += -MJ $@.json
compile_commands.json: $(ALL_TARGETS)
	sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $(shell find $(BUILDTOP) -iname *.o.json) > compile_commands.json
