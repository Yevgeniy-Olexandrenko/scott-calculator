#include <TinyWireM.h> // I2C wire communication with display
#include <avr/power.h> // Needed for power management
#include <avr/sleep.h> // Needed for sleeping
#include <EEPROM.h>    // For saving data to EEPROM

#include "font.h"
#include "display.h"
#include "keyboard.h"
#include "system.h"
#include "application.h"
#include "setup.h"
#include "loop.h"
