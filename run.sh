# qemu-system-i386 -fda build/bin/boot.img
qemu-system-i386 -monitor stdio -drive format=raw,file=build/bin/boot.img,if=ide,index=0,media=disk # -d int -no-reboot
