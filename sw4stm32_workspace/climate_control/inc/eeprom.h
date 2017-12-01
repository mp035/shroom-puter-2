#include <stdint.h>

// these functions take an ID (use it like an address) between 0 and 255 inclusive
uint32_t eeprom_read(uint16_t id);
uint32_t eeprom_write(uint16_t id, uint32_t data);
