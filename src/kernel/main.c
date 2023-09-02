#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "mem.h"


void OSStart() {
  char string[] = "Lore ipsum";
  char* string2 = "Ipsum lore"; 

  initVideo();
  putNumber(255, 10);
  puts("\n");
  putNumber(511, 16);
  puts("\n");
  putNumber(255, 2);
  puts("\n");
  putNumber(255, 16);

  puts("\n");
  puts(string);
  puts("\n");
  puts(string2);
  puts("\n");
  memset(&string, 'H', 4);
  puts("\n");
  puts(string);
  puts("\n");
  puts(string2);
  puts("\n");
  memcpy(string2, string, strlen(string));
  puts("\n");
  puts(string);
  puts("\n");
  puts(string2);
}
