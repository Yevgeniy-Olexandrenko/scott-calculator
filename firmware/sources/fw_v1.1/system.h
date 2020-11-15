// SYSTEM

#define FRAMERATE 10 // Maximal number of screen refreshes per second (>3)
#define SIZES 0x01   // Printing size
#define SIZEM 0x02   // Printing size
#define SIZEL 0x04   // Printing size

// Framing times
static uint8_t eachframemillis, thisframestart, lastframedurationms;

// True if frame was just rendered
static bool justrendered;

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
static void delayshort(uint8_t ms)
{
	long t = millis();
	while ((uint8_t)(millis() - t) < ms);
}

// Print char c with width and height (1, 2 or 4)
static void printc(uint8_t c, uint8_t w, uint8_t h)
{
	uint8_t tmpx = dx;
	for (uint8_t k = 0; k < h; k++)
	{ // One byte, two nibbles or 4 pairs of bits
		if (k > 0)
		{			   // Manage cursor position, if size >1
			dx = tmpx; // Remember x position
			dy++;	   // Increment y position/page
			dsetcursor(dx, dy);
		}
		for (uint8_t j = 0; j < FONTWIDTH; j++)
		{																						// Fontbyte - shifted one pixel down (if applicable)
			uint8_t tmp = pgm_read_byte(&font[FONTWIDTH * (c - FONTOFFSET) + j]) << printbitshift; // Fontbyte
			if (h == SIZEM)
				tmp = expand4bit((tmp >> (k * 4)) & 0x0f); // Expand 0000abcd
			else if (h == SIZEL)
				tmp = expand2bit((tmp >> (k * 2)) & 0x03); // Expand 000000ab
			dsenddatastart();
			for (uint8_t i = 0; i < w; i++)
				dsenddatabyte(tmp);
			dsendstop();
		}
	}
}

// Print sized char c at (x|y)
static void printcat(uint8_t c, uint8_t w, uint8_t h, uint8_t x, uint8_t y)
{
	dsetcursor(x, y);
	printc(c, w, h);
}

// Print string
static void prints(char *s, uint8_t w, uint8_t h)
{
	uint8_t tmpx = dx, tmpy = dy;
	for (uint8_t i = 0; i < strlen(s); i++)
		printcat(s[i], w, h, tmpx + i * (FONTWIDTH + 1) * w, tmpy);
}

// Print sized string s at (x|y)
static void printsat(char *s, uint8_t w, uint8_t h, uint8_t x, uint8_t y)
{
	dsetcursor(x, y);
	prints(s, w, h);
}

// Clear screen
static void cls(void)
{
	dfill(0x00);
}

// Swap GDDRAM and render
static void display(void)
{ 
	dswap();
}

// Calculate frameduration
static void setframerate(uint8_t rate)
{ 
	eachframemillis = 1000 / rate;
}

// External interrupt 0 wakes up ATTINY
ISR(PCINT0_vect)
{ 
}

// Execute sleep mode
static void execsleep(void)
{ 
	sleep_enable();
	sleep_cpu();
	sleep_disable();
	power_all_enable();
	power_timer1_disable(); // Never using timer1
}

// Goto deep sleep mode
static void sleep(void)
{
	doff();
	delayshort(200); // Prevent instant wakeup
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	power_all_disable(); // Power off ADC, timer 0 and 1
	execsleep();
	don();
}

// Idle, while waiting for next frame
static void idle(void)
{ 
	set_sleep_mode(SLEEP_MODE_IDLE);
	power_adc_disable(); // Disable ADC (do not disable timer0; timer1 is disabled from last idle)
	execsleep();
}

// Wait (idle) for next frame
static bool nextframe(void)
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
