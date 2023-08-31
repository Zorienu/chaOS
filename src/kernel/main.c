#include <stdint.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VGA_BUFFER_ADDRESS 0xB8000

void OSStart() {
  char nombre[] = "JASYD OS";

  uint8_t *vga = (uint8_t *)VGA_BUFFER_ADDRESS;
  for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i++) 
    *++vga = 0x0;

  int center = (SCREEN_WIDTH * 12 + 40 - 4) * 2 ;
  vga = (uint8_t *)VGA_BUFFER_ADDRESS + center;
  for (int i = 0; i < 8; i++) {
    *vga = nombre[i];
    vga++;
    *vga = 0x12;
    vga++;
  }
}
