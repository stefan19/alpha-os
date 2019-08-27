TARGET=i686-elf
CROSSDIR=$(HOME)/opt/cross/bin
ROOT=$(HOME)/Documents/OS

CC=$(CROSSDIR)/$(TARGET)-gcc
AR=$(CROSSDIR)/$(TARGET)-ar
CFLAGS = -std=gnu11 -ffreestanding -O2 -Wall -Wextra -I./include -I$(ROOT)/kernel/include
ASFLAGS = -f elf32
OBJDIR = ./obj

OBJS=$(addprefix $(OBJDIR)/, string.o stdio.o)

all: libk.a

clean:
	rm obj/*.o libk.a

libk.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

$(OBJDIR)/%.o: %.s
	nasm $(ASFLAGS) -o $@ $<
	
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<