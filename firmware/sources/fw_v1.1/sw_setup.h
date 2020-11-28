// SETUP

void setup()
{
	TinyI2C.init();
	DisplayInit();

	// INIT WAKEUP (with pin change interrupt) ... same pin as keyboard!
	pinMode(KPIN, INPUT); // Wakeup pin
	PCMSK |= bit(KPIN);	  // want pin D3 / H2
	GIFR  |= bit(PCIF);	  // Clear any outstanding interrupts
	GIMSK |= bit(PCIE);	  // Enable pin change interrupts

	// INIT SYSTEM
	setframerate(FRAMERATE);
	DisplayBrightness(brightness = EEPROM[EECONTRAST]);

	// START
	ResetStack();
}
