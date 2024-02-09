#pragma once
#include <stdint.h>

class IRQHandler {
  public:
    virtual void handleIRQ() = 0;

    uint8_t irqNumber() { return _irq_number; }

    void enableIRQ();
    void disableIRQ();

protected:
   IRQHandler(uint8_t irq);

  private:
    uint8_t _irq_number { 0 };
}; 
