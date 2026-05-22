// Redirect Stream.h to our HAL-compatible Stream shim so TMCStepper's
// __has_include(<Stream.h>) check succeeds without Arduino.h.
#include "HalStream.hpp"
