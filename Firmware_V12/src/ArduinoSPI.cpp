// Definition of the global SPI instance declared in Arduino.h.
// TMCStepper's SPI-based driver source files (TMC2130, TMC2660) reference
// this object; those drivers are not instantiated in this firmware so the
// methods are no-ops.
#include "Arduino.h"

SPIClass SPI;
