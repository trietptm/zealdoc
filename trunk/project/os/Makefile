
######################
# Makefile for Tinix #
######################

# Entry point of Tinix
# It must be as same as 'KernelEntryPointPhyAddr' in load.inc!!!
ENTRYPOINT	= 0x30400

# Offset of entry point in kernel file
# It depends on ENTRYPOINT
ENTRYOFFSET	=   0x400

ASM		= nasm
DASM		= ndisasm
CC		= gcc
LD		= ld
# for boot and loader
ASMBFLAGS	= -I include/boot/ 
# for kernel and lib
ASMKFLAGS	= -I include -f elf
CFLAGS		= -I include -c -fno-builtin
LDFLAGS		= -s -Ttext $(ENTRYPOINT)
DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

TOPDIR		= .
OUTPUT 	= $(TOPDIR)/bin

BOOTDIR		= $(TOPDIR)/boot
KERNELDIR	= $(TOPDIR)/kernel
# All binary
TINIXBOOT	= $(BOOTDIR)/boot.bin $(BOOTDIR)/loader.bin
TINIXKERNEL	= $(KERNELDIR)/kernel.bin

LIBDIR		= $(TOPDIR)/lib

objs :=
include $(KERNELDIR)/Makefile
include $(LIBDIR)/Makefile

# All Phony Targets
.PHONY : all clean

# Default starting position
all : $(TINIXKERNEL) $(TINIXBOOT) output

%.bin : %.asm
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(TINIXKERNEL) : $(objs)
	$(LD) $(LDFLAGS) -o $@ $(objs)

%.o : %.c
	$(CC) $(CFLAGS) -o $@ $<

# kernel.asm and klib.asm use ASMKFLAGS flags
%.o : %.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

output:
	mkdir -p $(OUTPUT);
	mv $(TINIXBOOT) $(TINIXKERNEL) $(OUTPUT);

clean:
	rm -rf $(objs) $(OUTPUT)
