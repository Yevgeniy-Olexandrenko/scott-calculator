// SYSTEM

#define FRAMERATE 10 // Maximal number of screen refreshes per second (>3)
#define CHAR_SIZE_S 0x01   // Printing size
#define CHAR_SIZE_M 0x02   // Printing size
#define CHAR_SIZE_L 0x04   // Printing size

// Framing times
static uint8_t eachframemillis, thisframestart, lastframedurationms;

// True if frame was just rendered
static uint8_t justrendered;

// Shifts printed character down
static uint8_t printbitshift = 0;

// Expand 4 bits (lower nibble)
static uint8_t expand4bit(uint8_t b)
{							   // 0000abcd
	b = (b | (b << 2)) & 0x33; // 00ab00cd
	b = (b | (b << 1)) & 0x55; // 0a0b0c0d
	return (b | (b << 1));	   // aabbccdd
}

// Expand 2 bits
static uint8_t expand2bit(uint8_t b)
{							   // 000000ab
	b = (b | (b << 3)) & 0x11; // 000a000b
	for (uint8_t i = 0; i < 3; i++)
		b |= (b << 1); // aaaabbbb
	return (b);
}

// Delay (with timer) in ms with 8 bit duration
void delayshort(uint8_t ms)
{
	long t = millis();
	while ((uint8_t)(millis() - t) < ms);
}

// Print char c with width and height (1, 2 or 4)
void PrintChar(uint8_t c, uint8_t w, uint8_t h)
{
	uint8_t tx = dx;
	for (uint8_t k = 0; k < h; k++)
	{
		if (k > 0) DisplayPosition(dx = tx, ++dy);

		for (uint8_t j = 0; j < FONT_WIDTH; j++)
		{
			uint8_t bitmap = pgm_read_byte(&font[FONT_WIDTH * (c - FONT_OFFSET) + j]) << printbitshift;
			if (h == CHAR_SIZE_M)
				bitmap = expand4bit((bitmap >> (k * 4)) & 0x0f); // Expand 0000abcd
			else if (h == CHAR_SIZE_L)
				bitmap = expand2bit((bitmap >> (k * 2)) & 0x03); // Expand 000000ab
			DisplayWrite(bitmap, w);
		}
	}
}

void PrintCharAt(uint8_t c, uint8_t w, uint8_t h, uint8_t x, uint8_t y)
{
	DisplayPosition(x, y);
	PrintChar(c, w, h);
}

void PrintStringAt(const __FlashStringHelper* s, uint8_t w, uint8_t h, uint8_t x, uint8_t y)
{
	DisplayPosition(x, y);
	const char* ptr = (const char*)s;
	uint8_t ww = FONT_WIDTH * w + 1;
	while(uint8_t ch = pgm_read_byte(ptr++))
	{
		PrintCharAt(ch, w, h, x, y);
		x += ww;
	}
}

// Calculate frameduration
void setframerate(uint8_t rate)
{ 
	eachframemillis = 1000 / rate;
}

// External interrupt 0 wakes up ATTINY
ISR(PCINT0_vect)
{ 
}

// Execute sleep mode
void execsleep()
{ 
	sleep_enable();
	sleep_cpu();
	sleep_disable();
	power_all_enable();
	power_timer1_disable(); // Never using timer1
}

// Goto deep sleep mode
void sleep()
{
	DisplayTurnOff();
	delayshort(200); // Prevent instant wakeup
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	power_all_disable(); // Power off ADC, timer 0 and 1
	execsleep();
	DisplayTurnOn();
}

// Idle, while waiting for next frame
void idle()
{ 
	set_sleep_mode(SLEEP_MODE_IDLE);
	power_adc_disable(); // Disable ADC (do not disable timer0; timer1 is disabled from last idle)
	execsleep();
}

// Wait (idle) for next frame
static uint8_t nextframe()
{
	uint8_t now = (uint8_t)millis(), framedurationms = now - thisframestart;
	if (justrendered)
	{
		lastframedurationms = framedurationms;
		justrendered = false;
		return false;
	}
	else if (framedurationms < eachframemillis)
	{
		if (++framedurationms < eachframemillis)
			idle();
		return false;
	}
	justrendered = true;
	thisframestart = now;
	return true;
}
