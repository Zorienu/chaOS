#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

// #define PRINT_HEX

//TODO
int main() {
  printf("Creating chaOS image\n");

  int bootloaderFD = open("build/bin/bootloader", O_RDONLY);
  int kernelFD = open("build/bin/kernel", O_RDONLY);
  int imgFD = open("build/bin/image.img", O_WRONLY | O_CREAT | O_TRUNC, 0777);

  if (imgFD < 0) { printf("Error creating image.img, error: %s\n", strerror(errno)); return 1; }

  printf("Bootloader FD: %d\n", bootloaderFD);
  printf("Kernel FD: %d\n", kernelFD);
  printf("Image FD: %d\n", imgFD);

  char buffer[512];
  memset(buffer, 0x0, sizeof(buffer));
  int nread = read(bootloaderFD, buffer, sizeof(buffer));

  for (int i = 0; i < sizeof(buffer); i++) {
#ifdef PRINT_HEX
    printf("%x ", buffer[i] & 0xFF);

    if (i % 16 == 0) printf("\n");
#endif
     
    int nwrite = write(imgFD, &buffer[i], sizeof(buffer[i]));
    if (nwrite != sizeof(buffer[i])) {
      printf("Error writing buffer[i]: %x to image, error: %s", buffer[i], strerror(errno));
      return 1;
    }
  }
  printf("\n");


  close(bootloaderFD);
  close(kernelFD);
  close(imgFD);

  return 0;
}
