TARGET := i686-elf
CROSSDIR := $(HOME)/opt/cross/bin
ROOT := $(HOME)/Documents/OS
LIBDIR := $(ROOT)/libk

CC := $(CROSSDIR)/$(TARGET)-gcc
LINKER := $(CROSSDIR)/$(TARGET)-gcc

CFLAGS := -std=gnu11 -ffreestanding -O2 -Wall -Wextra -I./include -I$(LIBDIR)/include
LDFLAGS := -T link.ld -ffreestanding -O2 -nostdlib -L$(LIBDIR)
ASFLAGS := -f elf32
OBJDIR := obj
SOURCES := $(shell find . -name '*.c')
ASMSOURCES := $(shell find . -name '*.s')

OBJECTNAMES := $(SOURCES:.c=.o) $(ASMSOURCES:.s=.o)
OBJECTS := $(addprefix $(OBJDIR)/, $(OBJECTNAMES))

all: $(OBJECTS) link

clean:
	find obj/ -type f -name '*.o' -delete

link:
	$(LINKER) $(LDFLAGS) -o $(ROOT)/i386-kernel $(OBJECTS) -lgcc -l:libk.a

$(OBJDIR)/%.o: %.s
	nasm $(ASFLAGS) -o $@ $<
	
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
