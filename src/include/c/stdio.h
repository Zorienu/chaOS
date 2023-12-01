#pragma once
#include <stdint.h>
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VGA_BUFFER_ADDRESS 0xB8000

// Possible background and foreground colors in text-mode in text-mode
enum colors {
  BLACK = 0,
  BLUE = 1,
  GREEN = 2,
  CYAN = 3,
  RED = 4,	
  MAGENTA = 5,
  BROWN = 6,
  LIGHT_GREY = 7,
  DARK_GREY = 8,
  LIGHT_BLUE = 9,
  LIGHT_GREEN = 10,
  LIGHT_CYAN = 11,
  LIGHT_RED = 12,
  LIGHT_MAGENTA = 13,
  LIGHT_BROWN = 14,
  WHITE = 15,
};

/*
 * Clear the whole screen
 */
void cls(void);

/*
 * Set the foreground color to use for next characters in text-mode
 */
void setForegroundColor(enum colors color);

/*
 * Set the background color to use for next characters in text-mode
 */
void setBackgroundColor(enum colors color);

/*
 * Set the text-mode VGA pointer, then clears the screen
 */
void initVideo(void);

/*
 * Implementation of the same printf you have in C std
 */
void printf(char *format, ...);
