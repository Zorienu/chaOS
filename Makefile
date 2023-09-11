TARGET=i686-elf
ASM=nasm
GCC=$(TARGET)-gcc
LD=$(TARGET)-ld

SRC_DIR=src
BUILD_DIR=build
BUILD_DIR_IMG=$(BUILD_DIR)/bin

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
	dd if=$(BUILD_DIR_IMG)/kernel of=$(BUILD_DIR_IMG)/boot.img conv=notrunc seek=3

#
# Link bootloader (everything inside src/bootloader)
#
link_bootloader: bootloader
	$(LD) $(BUILD_DIR)/*.o -T bootloader.ld -o $(BUILD_DIR_IMG)/bootloader
	rm ./$(BUILD_DIR)/*.o

#
# Bootloader: rules for building the bootloader
#
bootloader: $(BUILD_DIR)/bootloader
# Build the bootloader
$(BUILD_DIR)/bootloader: always
	#$(ASM) $(SRC_DIR)/bootloader/boot.asm -f bin -o $(BUILD_DIR)/bootloader.bin
	$(ASM) $(SRC_DIR)/bootloader/boot.asm -f elf32 -o $(BUILD_DIR)/bootloader.o
	$(GCC) -c src/bootloader/*.c -nostdlib -ffreestanding
	mv *.o ./$(BUILD_DIR)

# 
# Link kernel (everything inside src/kernel)
#
link_kernel: kernel
	$(LD) $(BUILD_DIR)/*.o -T kernel.ld -o $(BUILD_DIR_IMG)/kernel
	rm ./$(BUILD_DIR)/*.o

#
# Kernel
# 
kernel: $(BUILD_DIR)/kernel
# Build the kernel
$(BUILD_DIR)/kernel: always
	$(GCC) -c src/kernel/*.c -nostdlib -ffreestanding
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
