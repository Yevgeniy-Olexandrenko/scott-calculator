// SETUP

void setup()
{
	// INIT DISPLAY
	dinit();   // Init display
	drender(); // Render current half of GDDRAM

	// INIT WAKEUP (with pin change interrupt) ... same pin as keyboard!
	pinMode(KPIN, INPUT); // Wakeup pin
	PCMSK |= bit(KPIN);	  // want pin D3 / H2
	GIFR  |= bit(PCIF);	  // Clear any outstanding interrupts
	GIMSK |= bit(PCIE);	  // Enable pin change interrupts

	// INIT SYSTEM
	setframerate(FRAMERATE);
	dcontrast(brightness = EEPROM[EECONTRAST]);

	// START
	LoadStack(); // Read stack from EEPROM (saved with sleep/OFF)
}
