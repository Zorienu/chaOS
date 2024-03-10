#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#define SECTOR_SIZE 512
// #define PRINT_HEX

void writeBinaryToImg(int imgFD, int binaryFD) {
  int nread = 0;
  char buffer[SECTOR_SIZE];

  while ((nread = read(binaryFD, buffer, sizeof(buffer))) > 0) {
    int nwrite = write(imgFD, buffer, sizeof(buffer));

    // Clear buffer
    memset(buffer, 0x0, sizeof(buffer));

    if (nwrite < nread) {
      printf("Error writing buffer to image, error: %s", strerror(errno));
    }
  } 
}

int main() {
  printf("Creating chaOS image\n");

  int bootsectorFD = open("build/bin/bootsector", O_RDONLY);
  int prekernelFD = open("build/bin/prekernel", O_RDONLY);
  int kernelFD = open("build/bin/kernel", O_RDONLY);
  int imgFD = open("build/bin/image.img", O_WRONLY | O_CREAT | O_TRUNC, 0777);

  if (imgFD < 0) { printf("Error creating image.img, error: %s\n", strerror(errno)); return 1; }

  printf("Bootsector FD: %d\n", bootsectorFD);
  printf("Prekernel FD: %d\n", prekernelFD);
  printf("Kernel FD: %d\n", kernelFD);
  printf("Image FD: %d\n", imgFD);

  int bootsectorSize = lseek(bootsectorFD, 0, SEEK_END);
  int prekernelSize = lseek(prekernelFD, 0, SEEK_END);
  int kernelSize = lseek(kernelFD, 0, SEEK_END);

  printf("\nBinaries size:\n");
  printf("Bootsector size in bytes: %d\n", bootsectorSize);
  printf("Prekernel size in bytes: %d\n", prekernelSize);
  printf("Kernel size in bytes: %d\n", kernelSize);

  lseek(bootsectorFD, 0, SEEK_SET);
  lseek(prekernelFD, 0, SEEK_SET);
  lseek(kernelFD, 0, SEEK_SET);

  char buffer[512];
  memset(buffer, 0x0, sizeof(buffer));
  int nread = 0;

  printf("Writing bootsector to img...\n");
  writeBinaryToImg(imgFD, bootsectorFD);
  printf("Writing prekernel to img...\n");
  writeBinaryToImg(imgFD, prekernelFD);

  lseek(imgFD, 0x200 * 100, SEEK_SET);
  printf("Writing kernel to img...\n");
  writeBinaryToImg(imgFD, kernelFD);

  close(bootsectorFD);
  close(prekernelFD);
  close(kernelFD);
  close(imgFD);

  printf("Finished creating img...\n");

  return 0;
}
