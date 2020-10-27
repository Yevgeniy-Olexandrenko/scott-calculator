// SYSTEM

#define FRAMERATE 10 // Maximal number of screen refreshes per second (>3)
#define SIZES 0x01   // Printing size
#define SIZEM 0x02   // Printing size
#define SIZEL 0x04   // Printing size

// Framing times
static byte eachframemillis, thisframestart, lastframedurationms;

// True if frame was just rendered
static boolean justrendered;

// Shifts printed character down
static byte printbitshift = 0;

// Expand 4 bits (lower nibble)
static byte expand4bit(byte b)
{							   // 0000abcd
	b = (b | (b << 2)) & 0x33; // 00ab00cd
	b = (b | (b << 1)) & 0x55; // 0a0b0c0d
	return (b | (b << 1));	   // aabbccdd
}

// Expand 2 bits
static byte expand2bit(byte b)
{							   // 000000ab
	b = (b | (b << 3)) & 0x11; // 000a000b
	for (byte i = 0; i < 3; i++)
		b |= (b << 1); // aaaabbbb
	return (b);
}

// Delay (with timer) in ms with 8 bit duration
static void delayshort(byte ms)
{
	long t = millis();
	while ((byte)(millis() - t) < ms);
}

// Print char c with width and height (1, 2 or 4)
static void printc(byte c, byte w, byte h)
{
	byte tmpx = dx;
	for (byte k = 0; k < h; k++)
	{ // One byte, two nibbles or 4 pairs of bits
		if (k > 0)
		{			   // Manage cursor position, if size >1
			dx = tmpx; // Remember x position
			dy++;	   // Increment y position/page
			dsetcursor(dx, dy);
		}
		for (byte j = 0; j < FONTWIDTH; j++)
		{																						// Fontbyte - shifted one pixel down (if applicable)
			byte tmp = pgm_read_byte(&font[FONTWIDTH * (c - FONTOFFSET) + j]) << printbitshift; // Fontbyte
			if (h == SIZEM)
				tmp = expand4bit((tmp >> (k * 4)) & 0x0f); // Expand 0000abcd
			else if (h == SIZEL)
				tmp = expand2bit((tmp >> (k * 2)) & 0x03); // Expand 000000ab
			dsenddatastart();
			for (byte i = 0; i < w; i++)
				dsenddatabyte(tmp);
			dsendstop();
		}
	}
}

// Print sized char c at (x|y)
static void printcat(byte c, byte w, byte h, byte x, byte y)
{
	dsetcursor(x, y);
	printc(c, w, h);
}

// Print string
static void prints(char *s, byte w, byte h)
{
	byte tmpx = dx, tmpy = dy;
	for (byte i = 0; i < strlen(s); i++)
		printcat(s[i], w, h, tmpx + i * (FONTWIDTH + 1) * w, tmpy);
}

// Print sized string s at (x|y)
static void printsat(char *s, byte w, byte h, byte x, byte y)
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
static void setframerate(byte rate)
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
static boolean nextframe(void)
{
	byte now = (byte)millis(), framedurationms = now - thisframestart;
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
