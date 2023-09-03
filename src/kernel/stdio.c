#include <stdint.h>
#include "math.h"
#include "mem.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VGA_BUFFER_ADDRESS 0xB8000

// Possible background and foreground colors
enum colors {
  BLACK,
  BLUE,
  GREEN,
  CYAN,
  RED,	
  MAGENTA,
  BROWN,
  LIGHT_GREY,
  DARK_GREY,
  LIGHT_BLUE,
  LIGHT_GREEN,
  LIGHT_CYAN,
  LIGHT_RED,
  LIGHT_MAGENTA,
  LIGHT_BROWN,
  WHITE,
};

// Current location in the screen
int currX = 0, currY = 0;
uint8_t *textModeVGAPtr;

void hScroll () {
  // We do not need to scroll yet
  if (currY < SCREEN_HEIGHT) return;

  int bytesPerLine = SCREEN_WIDTH * 2;
  int screenBytes = bytesPerLine * SCREEN_HEIGHT;
  uint8_t *secondLine = (uint8_t *)textModeVGAPtr + bytesPerLine;

  memcpy(textModeVGAPtr, secondLine, screenBytes - bytesPerLine);

  // Clear the last line
  uint8_t *lastLine = (uint8_t *)textModeVGAPtr + (SCREEN_HEIGHT - 1) * bytesPerLine;
  memset(lastLine, 0x0, bytesPerLine);

  currY = SCREEN_HEIGHT - 1;
}

void putc (unsigned char c) {
  int offset = ((SCREEN_WIDTH * currY) + currX);
  uint16_t *location = (uint16_t *)textModeVGAPtr + offset;

  uint8_t backcolor = BLUE;
  uint8_t forecolor = WHITE;

  if (c == '\n') {
    currX = 0;
    currY++;
  }
  else {
    // 15            12 11            8 7              0
    // -------------------------------------------------
    // |   Backcolor   |   Forecolor   |   Character   |
    // -------------------------------------------------
    *location = backcolor << 12 | forecolor << 8 | c;
    currX++;
  }

  /* If the cursor has reached the edge of the screen's width, 
   * we insert a new line in there */
  if (currX > SCREEN_WIDTH) {
    currX = 0;
    currY++;
  }

   hScroll();  
}



void cls (void) {
  uint8_t *vga = textModeVGAPtr;
  // TODO: change to memset
  for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 2; i++) 
    *++vga = 0x0;

  // Update virtual cursor
  currX = 0;
  currY = 0;

  // Move hardware cursor
  // TODO: moveCsr();
}

void puts (char *string) {
  while (*string) putc(*string++);
}

void putNumber (int number, int base) {
  char *numbers = "0123456789ABCDEF";
  
  char *prefix;
  switch (base) {
    case 2: {
      prefix = "0b\0";
      break;
    }
    case 16: {
      prefix = "0x\0";
      break;
    }
    case 10:
    default: {
      prefix = "\0";
    }
  }

  if (number < 0) putc('-');
  puts(prefix);
  number = abs(number);

  char *digits = "";

  int i = 0;
  while (number > 0) {
    uint8_t digit = number % base;
    digits[i++] = numbers[digit];
    number /= base;
  }

  while (i--) {
    putc(digits[i]);
  }
}

void initVideo (void) {
  textModeVGAPtr = (uint8_t *)VGA_BUFFER_ADDRESS;
  cls();
}
