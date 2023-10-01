#include <stdint.h>
#include <stdbool.h>
#include "math.h"
#include "mem.h"
#include "stdio.h"
#include "x86.h"


// Current location in the screen
int currX = 0, currY = 0;
uint8_t *textModeVGAPtr;

// Current background and foreground color
uint8_t backgroundColor = BLUE;
uint8_t foregroundColor = WHITE;

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

  if (c == '\n') {
    currX = 0;
    currY++;
  }
  else {
    // 15            12 11            8 7              0
    // -------------------------------------------------
    // |   Backcolor   |   Forecolor   |   Character   |
    // -------------------------------------------------
    *location = backgroundColor << 12 | foregroundColor << 8 | c;
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

void setForegroundColor (enum colors color) {
  foregroundColor = color;
}

void setBackgroundColor (enum colors color) {
  backgroundColor = color;
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

void putHex (uint32_t address) {
  char *numbers = "0123456789ABCDEF";

  puts("0x");

  if (address == 0) {
    putc('0');
    return;
  }

  char digits[50] = {};

  int i = 0;
  while (address > 0) {
    uint8_t digit = address % 16;
    digits[i++] = numbers[digit];
    address /= 16;
  }

  while (i--) 
    putc(digits[i]);
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

  if (number == 0) {
    putc('0');
    return;
  }

  char digits[50] = {};

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

#define PRINTF_STATE_NORMAL         0
#define PRINTF_STATE_LENGTH         1
#define PRINTF_STATE_LENGTH_SHORT   2
#define PRINTF_STATE_LENGTH_LONG    3
#define PRINTF_STATE_SPEC           4

#define PRINTF_LENGTH_DEFAULT       0
#define PRINTF_LENGTH_SHORT_SHORT   1
#define PRINTF_LENGTH_SHORT         2
#define PRINTF_LENGTH_LONG          3
#define PRINTF_LENGTH_LONG_LONG     4

int *printfNumber(int *argp, int length, bool sign, int radix);

void printf(char *format, ...) {
  int *argp = (int *)&format;
  argp++;
  
  int state = PRINTF_STATE_NORMAL;
  int length = PRINTF_LENGTH_DEFAULT;
  int radix = 10;
  bool sign = false;

  while (*format) {
    switch (state) {
      case PRINTF_STATE_NORMAL:
        switch (*format) {
          case '%': state = PRINTF_STATE_LENGTH;
                    break;
          default: putc(*format);
                   break;
        }
        break;

      case PRINTF_STATE_LENGTH:
        switch (*format) {
          case 'h': length = PRINTF_LENGTH_SHORT;
                    state = PRINTF_STATE_LENGTH_SHORT;
                    break;
          case 'l': length = PRINTF_LENGTH_LONG;
                    state = PRINTF_STATE_LENGTH_LONG;
                    break;
          default:  goto PRINTF_STATE_SPEC_;
        }
        break;

      case PRINTF_STATE_LENGTH_SHORT:
        if (*format == 'h') {
          length = PRINTF_LENGTH_SHORT_SHORT;
          state = PRINTF_STATE_SPEC;
        }
        else goto PRINTF_STATE_SPEC_;
        break;

      case PRINTF_STATE_LENGTH_LONG:
        if (*format == 'l') {
          length = PRINTF_LENGTH_LONG_LONG;
          state = PRINTF_STATE_SPEC;
        }
        else goto PRINTF_STATE_SPEC_;
        break;

      case PRINTF_STATE_SPEC:
      PRINTF_STATE_SPEC_:
        switch (*format) {
          // Specifier characters
          case 'c': putc((char)*argp);
                    argp++;
                    break;

          case 's': puts(*(char **)argp);
                    argp++;
                    break;

          case '%': putc('%');
                    break;

          // Signed decimal number
          case 'd': 
          case 'i': radix = 10; sign = true;
                    argp = printfNumber(argp, length, sign, radix);
                    break;

          // Unsigned decimal number
          case 'u': radix = 10; sign = false;
                    argp = printfNumber(argp, length, sign, radix);
                    break;

          // Unsigned hexadecimal number
          case 'X':  
          case 'x':  
          case 'p': radix = 16; sign = false;
                    argp = printfNumber(argp, length, sign, radix);
                    break;
          
          // Unsigned octal number
          case 'o': radix = 8; sign = false;
                    argp = printfNumber(argp, length, sign, radix);
                    break;

          // Ignore invalid characters
          default:  break;
        }

        // Return to normal state
        state = PRINTF_STATE_NORMAL; 
        length = PRINTF_LENGTH_DEFAULT;
        radix = 10;
        sign = false;
    }

    format++;
  }
}

char *g_HexChars = "0123456789ABCDEF";

int *printfNumber(int *argp, int length, bool sign, int radix) {
  char buffer[32] = { 0 };
  unsigned long long number = 0;
  int number_sign = 1;

  // Process length
  switch (length) {
    case PRINTF_LENGTH_SHORT_SHORT:
    case PRINTF_LENGTH_SHORT:
    case PRINTF_LENGTH_DEFAULT:
    case PRINTF_LENGTH_LONG:
      if (sign) {
        int n = *argp;
        if (n < 0) {
          n = -n;
          number_sign = -1;
        }
        number = (unsigned long long)n;
      }
      else {
        number = *(unsigned int*)argp;
      }

      argp++;
      break;

    case PRINTF_LENGTH_LONG_LONG:
      if (sign) {
        long long int n = *(long long int *)argp;
        if (n < 0) {
          n = -n;
          number_sign = -1;
        }
        number = (unsigned long long)n;
      }
      else {
        number = *(unsigned long long int*)argp;
      }

      argp += 2;
      break;

  }

  // Convert number to ASCII
  int pos = 0;
  do {
    uint32_t remainder;
    x86_div64_32(number, radix, &number, &remainder);
    buffer[pos++] = g_HexChars[remainder];
  } while (number > 0);

  // Add sign
  if (sign && number_sign < 0) {
    buffer[pos++] = '-';
  }

  // Print number in reverse order
  while (--pos >= 0) {
    putc(buffer[pos]);
  }

  return argp;
}
