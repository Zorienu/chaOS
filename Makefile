ASM=nasm

SRC_DIR=src
BUILD_DIR=build

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
	cp $(BUILD_DIR)/boot.bin $(BUILD_DIR)/main_floppy.img
	truncate -s 1440k $(BUILD_DIR)/main_floppy.img

#
# Bootloader: rules for building the bootloader
#
bootloader: $(BUILD_DIR)/bootloader.bin
# Build the bootloader
$(BUILD_DIR)/bootloader.bin: always
	$(ASM) $(SRC_DIR)/bootloader/boot.asm -f bin -o $(BUILD_DIR)/boot.bin

#
# Kernel
# 
kernel: $(BUILD_DIR)/kernel/main.bin
# Build the kernel
$(BUILD_DIR)/kernel/main.bin: always
	$(ASM) $(SRC_DIR)/kernel/main.asm -f bin -o $(BUILD_DIR)/kernel.bin

# 
# Always: this target will be used to create the build directory 
# if it does not exist
#
always:
	mkdir -p $(BUILD_DIR)

#
# Clean: this target cleans the build directory
#
clean: 
	rm -rf $(BUILD_DIR)/*
