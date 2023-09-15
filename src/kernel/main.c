#include <stdint.h>
#include "stdio.h"
#include "string.h"
#include "mem.h"
#include "mmu.h"

void OSStart() {
  initVideo();
  for (int i = 0; i < 100; i ++) {
    setBackgroundColor(i % 16);
    putNumber(i, 10);
    puts("\n");
  }
}
