#include <stdint.h>

uint8_t read6502(uint16_t address);
void write6502(uint16_t address, uint8_t value);
void irq6502();
