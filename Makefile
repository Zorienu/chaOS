TARGET=i686-elf
ASM=nasm
GCC=$(TARGET)-gcc
GPP=$(TARGET)-g++
LD=$(TARGET)-ld

SRC_DIR=src
BUILD_DIR=build
BUILD_DIR_IMG=$(BUILD_DIR)/bin

CFLAGS=-mgeneral-regs-only -nostdlib -ffreestanding #-Wall -Wextra -Wno-pointer-sign -Wno-interrupt-service-routine

# Get crtbegin.o and crtend.o paths
CRTBEGIN_OBJ:=$(shell $(GPP) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(GPP) $(CFLAGS) -print-file-name=crtend.o)

CRTI_OBJ=$(BUILD_DIR)/crti.o
CRTN_OBJ=$(BUILD_DIR)/crtn.o

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

img: clean link_bootloader link_kernel
	dd if=/dev/zero of=$(BUILD_DIR_IMG)/boot.img count=10000
	dd if=$(BUILD_DIR_IMG)/bootloader of=$(BUILD_DIR_IMG)/boot.img conv=notrunc
	# seek=7  -> 0xE00 (0x200 * 7)
	# seek=50 -> 0x6400 (0x200 * 50)
	# seek=100 -> 0xC800 (0x200 * 100)
	dd if=$(BUILD_DIR_IMG)/kernel of=$(BUILD_DIR_IMG)/boot.img conv=notrunc seek=100

#
# Link bootloader (everything inside src/bootloader)
#
link_bootloader: bootloader
	# TODO: if watching weird behavior, link crti.o and crtn.o in order
	$(GPP) $(CFLAGS) $(CRTBEGIN_OBJ) $(BUILD_DIR)/*.o $(CRTEND_OBJ) -T bootloader.ld -o $(BUILD_DIR_IMG)/bootloader
	rm ./$(BUILD_DIR)/*.o

#
# Bootloader: rules for building the bootloader
#
bootloader: $(BUILD_DIR)/bootloader
# Build the bootloader
$(BUILD_DIR)/bootloader: always
	$(ASM) $(SRC_DIR)/bootloader/boot.asm -f elf32 -o $(BUILD_DIR)/bootloader.o
	$(ASM) $(SRC_DIR)/bootloader/stage2.asm -f elf32 -o $(BUILD_DIR)/stage2.o
	$(ASM) $(SRC_DIR)/bootloader/crti.asm -f elf32 -o $(CRTI_OBJ)
	$(ASM) $(SRC_DIR)/bootloader/crtn.asm -f elf32 -o $(CRTN_OBJ)
	$(ASM) $(SRC_DIR)/include/x86/x86.asm -f elf32 -o $(BUILD_DIR)/x86.o
	$(GPP) -c $(CFLAGS) src/bootloader/*.cpp
	$(GPP) -c $(CFLAGS) src/include/*/*.cpp
	mv *.o ./$(BUILD_DIR)

# 
# Link kernel (everything inside src/kernel)
#
link_kernel: kernel
	# TODO: if watching weird behavior, link crti.o and crtn.o in order
	$(GPP) $(CFLAGS) $(CRTBEGIN_OBJ) $(BUILD_DIR)/*.o $(CRTEND_OBJ) -T kernel.ld -o $(BUILD_DIR_IMG)/kernel
	rm ./$(BUILD_DIR)/*.o

#
# Kernel
# 
kernel: $(BUILD_DIR)/kernel
# Build the kernel
$(BUILD_DIR)/kernel: always
	$(ASM) $(SRC_DIR)/kernel/entry.asm -f elf32 -o $(BUILD_DIR)/entry.o
	$(ASM) $(SRC_DIR)/kernel/crti.asm -f elf32 -o $(CRTI_OBJ)
	$(ASM) $(SRC_DIR)/kernel/crtn.asm -f elf32 -o $(CRTN_OBJ)
	$(ASM) $(SRC_DIR)/include/x86/x86.asm -f elf32 -o $(BUILD_DIR)/x86.o
	$(GPP) -c $(CFLAGS) src/kernel/*.cpp
	$(GPP) -c $(CFLAGS) src/include/*/*.cpp
	mv *.o ./$(BUILD_DIR)

# 
# Always: this target will be used to create the build directory 
# if it does not exist
#
always: 
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR_IMG)

#
# Clean: this target cleans the build directory
#
clean: 
	rm -rf $(BUILD_DIR)/*
