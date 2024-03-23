TARGET=i686-elf
ASM=nasm
GCC=$(TARGET)-gcc
GPP=$(TARGET)-g++
LD=$(TARGET)-ld

SRC_DIR=src
BUILD_DIR=build
BUILD_DIR_IMG=$(BUILD_DIR)/bin

# Linker scripts
BOOTSECTOR_LINKER_SCRIPT=src/bootsector/bootsector.ld
PREKERNEL_LINKER_SCRIPT=src/prekernel/prekernel.ld
KERNEL_LINKER_SCRIPT=src/kernel/kernel.ld

CFLAGS=-mgeneral-regs-only \
			 -nostdlib -ffreestanding \
			 -fno-rtti -Wno-write-strings \
			 -I ./src/include \
			 -I ./src/include/c \
			 -I ./src/include/io \
			 -I ./src/include/mem \
			 -I ./src/include/elf \
			 -I ./src/include/sys \
			 -I ./src \
#-Wall -Wextra -Wno-pointer-sign -Wno-interrupt-service-routine

# Get crtbegin.o and crtend.o paths
CRTBEGIN_OBJ:=$(shell $(GPP) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(GPP) $(CFLAGS) -print-file-name=crtend.o)

CRTI_OBJ=$(BUILD_DIR)/crti.o
CRTN_OBJ=$(BUILD_DIR)/crtn.o

LIBRARY_OBJECTS = \
	build/objects/include/c/stdio.o \
	build/objects/include/c/stdlib.o \
	build/objects/include/c/string.o \
	build/objects/include/elf/elf.o \
	build/objects/include/mem/MemoryManager.o \
	build/objects/include/mem/malloc.o \
	build/objects/include/mem/mem.o \
	build/objects/include/mem/virtualMem.o \
	build/objects/include/mem/memLayout.o \
	build/objects/include/sys/syscallWrappers.o \
	build/objects/include/x86/x86.o \

KERNEL_OBJECTS = \
	build/objects/kernel/devices/CharacterDevice.o \
	build/objects/kernel/devices/Device.o \
	build/objects/kernel/devices/KeyboardDevice.o \
	build/objects/kernel/fileSystem/File.o \
	build/objects/kernel/fileSystem/FileDescription.o \
	build/objects/kernel/heap/kmalloc.o \
	build/objects/kernel/interrupts/IRQHandler.o \
	build/objects/kernel/interrupts/idt.o \
	build/objects/kernel/interrupts/pic.o \
	build/objects/kernel/main.o \
	build/objects/kernel/syscalls/syscalls.o \
	build/objects/kernel/test.o \
	build/objects/kernel/tty/TTY.o \
	build/objects/kernel/tty/VirtualConsole.o \
	build/objects/kernel/utils/Assertions.o \
	build/objects/kernel/utils/kprintf.o \
	build/objects/kernel/crti.o \
	build/objects/kernel/crtn.o \
	build/objects/kernel/entry.o \

PREKERNEL_OBJECTS = \
  build/objects/prekernel/prekernel.o \
  build/objects/prekernel/crti.o \
  build/objects/prekernel/crtn.o \

BOOTSECTOR_OBJECTS = \
	build/objects/bootsector/boot.o \
  build/objects/bootsector/stage2.o \

ALL_OBJECTS = $(LIBRARY_OBJECTS) $(KERNEL_OBJECTS) $(PREKERNEL_OBJECTS) $(BOOTSECTOR_OBJECTS)

# Keep our make file cleaner by refering to varios modules 
# using their names rather than their output file names
.PHONY: all floppy_image kernel bootloader clean always

# 
# Floppy image, add rule to tell make that the phony floppy img target 
# depends on the actual file (main_floppy.img)
#
floppy_image: $(BUILD_DIR)/main_floppy.img

# The floppy img dependencies are the bootloader and the kernel targets
# Build the floppy img
$(BUILD_DIR)/main_floppy.img: bootloader kernel
	# Generate an empty 1.44 MB file
	# dd: dataset definition
	# if: input file
	# /dev/zero: special file in unix that provides as many null characters as are read from it
	# of: output file
	# bs: block size (512 bytes)
	# count: block count (2880 * 512 bytes = 1.44 MB)
	dd if=/dev/zero of=$(BUILD_DIR)/main_floppy.img bs=512 count=2880
	# Create filesystem
	# -F 12: tells it to use FAT12
	#  -n "CHAOS": label
	mkfs.fat -F 32 -n "CHAOS" $(BUILD_DIR)/main_floppy.img
	# Put the bootloader into the first sector of the disk
	# conv=notrunc: tells dd to not truncate the file, otherwise we will lose the rest of the image
	# removing it will cause the img to be 512 bytes (the size of the bootloader)
	dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/main_floppy.img conv=notrunc
	# Now we have a filesystem, we can copy the files to the img
	# mtools can manipulate FAT images directly without having to mount them
	# -i: input disk img
	# "::kernel.bin": specifies the destination path within the disk img, in this case in /kernel.bin (root dir)
	mcopy -i $(BUILD_DIR)/main_floppy.img $(BUILD_DIR)/kernel.bin "::kernel.bin"

binary_files: $(BUILD_DIR_IMG)/bootsector $(BUILD_DIR_IMG)/prekernel $(BUILD_DIR_IMG)/kernel 
img: clean always binary_files
	@#gcc -D PRINT_HEX makeImage.c -o makeImage && ./makeImage
	gcc makeImage.c -o makeImage
	./makeImage

# 
# Always: this target will be used to create the build directory 
# if it does not exist
# NOTE: the "@" avoids the command being printed to the screen
#
always: 
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR_IMG)

#
# Clean: this target cleans the build directory
#
clean: 
	@echo "Cleaning $(BUILD_DIR) directory...\n"
	@rm -rf $(BUILD_DIR)/*

#
# Compile all cpp files
#
build/objects/%.o: src/%.cpp
	@echo "Compiling $< --> $@"
	@mkdir -p $(@D)
	@$(GPP) $(CFLAGS) -o $@ -c $<

#
# Compile all asm files
#
build/objects/%.o: src/%.asm
	@echo "Compiling $< --> $@"
	@mkdir -p $(@D)
	@$(ASM) $< -f elf32 -o $@

#
# Compile all files
#
compile: $(ALL_OBJECTS)

#
# Link kernel binary
#
$(BUILD_DIR_IMG)/kernel: link_kernel
link_kernel: compile
	@echo "Linking kernel..."
	@# REVIEW: removing crtbegin and crtend does not affect?
	@$(GPP) $(CFLAGS) $(CRTBEGIN_OBJ) $(LIBRARY_OBJECTS) $(KERNEL_OBJECTS) $(CRTEND_OBJ) -T $(KERNEL_LINKER_SCRIPT) -o $(BUILD_DIR_IMG)/kernel
	@echo "Finished linking kernel..."

#
# Link prekernel binary
#
$(BUILD_DIR_IMG)/prekernel: link_prekernel
link_prekernel: compile
	@echo "Linking prekernel..."
	@$(GPP) $(CFLAGS) $(PREKERNEL_OBJECTS) $(LIBRARY_OBJECTS) -T $(PREKERNEL_LINKER_SCRIPT) -o $(BUILD_DIR_IMG)/prekernel
	@echo "Finished linking prekernel..."

#
# Link bootsector binary
#
$(BUILD_DIR_IMG)/bootsector: link_bootsector
link_bootsector: compile
	@echo "Linking bootsector..."
	@$(GPP) $(CFLAGS) $(BOOTSECTOR_OBJECTS) -T $(BOOTSECTOR_LINKER_SCRIPT) -o $(BUILD_DIR_IMG)/bootsector
