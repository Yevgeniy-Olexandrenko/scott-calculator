////////////////////////////////////////////////////////////////////////////////

#define FONT_OFFSET ','
#define FONT_WIDTH  5

const uint8_t font[] PROGMEM =
{
	// No ascii signs below 44 (',') to save memory (flash)
	0b00010000, // , squareroot
	0b00100000,
	0b01111111,
	0b00000001,
	0b00000001,
	
	0x08, 0x08, 0x08, 0x08, 0x08, // -
	0x00, 0x60, 0x60, 0x00, 0x00, // .
	0x20, 0x10, 0x08, 0x04, 0x02, // /
	0x7f, 0x41, 0x41, 0x41, 0x7f, // 0
	0x00, 0x00, 0x02, 0x7f, 0x00, // 1
	0x79, 0x49, 0x49, 0x49, 0x4f, // 2
	0x41, 0x49, 0x49, 0x49, 0x7f, // 3
	0x0f, 0x08, 0x08, 0x08, 0x7f, // 4
	0x4f, 0x49, 0x49, 0x49, 0x79, // 5
	0x7f, 0x49, 0x49, 0x49, 0x79, // 6
	0x03, 0x01, 0x01, 0x01, 0x7f, // 7
	0x7f, 0x49, 0x49, 0x49, 0x7f, // 8
	0x4f, 0x49, 0x49, 0x49, 0x7f, // 9
	0x00, 0x00, 0x00, 0x00, 0x00, // : space
	0x00, 0x1b, 0x04, 0x1b, 0x00, // ; raised x
	0x00, 0x7f, 0x3e, 0x1c, 0x08, // < play
	0x04, 0xbe, 0xbf, 0xbe, 0x04, // = shift sign
	0x08, 0x08, 0x3e, 0x1c, 0x08, // > arrow to right
	//
	0x00, 0x00, 0x2f, 0x00, 0x00, // ? !
	//
	0x1c, 0x3e, 0x3e, 0x3e, 0x1c, // @ record
	0x7f, 0x09, 0x09, 0x09, 0x7f, // A
	0x7f, 0x49, 0x49, 0x4f, 0x78, // B
	0x7f, 0x41, 0x41, 0x41, 0x40, // C

	0b01111111, // D
	0b01000001,
	0b01000001,
	0b01000001,
	0b00111110,

	0x7F, 0x49, 0x49, 0x49, 0x41, // E
	0x7F, 0x09, 0x09, 0x09, 0x01, // F
	0x7f, 0x41, 0x49, 0x49, 0x79, // G
	0x7F, 0x08, 0x08, 0x08, 0x7F, // H
	0x00, 0x41, 0x7F, 0x41, 0x00, // I
	0x40, 0x40, 0x40, 0x40, 0x7f, // J
	0x7F, 0x08, 0x08, 0x0f, 0x78, // K
	0x7F, 0x40, 0x40, 0x40, 0x40, // L

	0b01111111, // M
	0b00000100,
	0b00001000,
	0b00000100,
	0b01111111,

	0b01111111, // N
	0b00000100,
	0b00001000,
	0b00010000,
	0b01111111,

	0x7f, 0x41, 0x41, 0x41, 0x7f, // O
	0x7F, 0x09, 0x09, 0x09, 0x0f, // P
	0x7f, 0x41, 0x71, 0x41, 0x7f, // Q

	0b01111111, // R
	0b00001001,
	0b00011001,
	0b00101001,
	0b01001111,

	0x4f, 0x49, 0x49, 0x49, 0x78, // S
	0x01, 0x01, 0x7F, 0x01, 0x01, // T
	0x7F, 0x40, 0x40, 0x40, 0x7F, // U

	0b00011111, // V
	0b00100000,
	0b01000000,
	0b00100000,
	0b00011111,

	0x7F, 0x40, 0x70, 0x40, 0x7F, // W

	0b01100011, // X
	0b00010100,
	0b00001000,
	0b00010100,
	0b01100011,

	0b00000011, // Y
	0b00000100,
	0b01111000,
	0b00000100,
	0b00000011,

	0x71, 0x51, 0x5d, 0x45, 0x47, // Z
	// No low case letters to save flash memory
	// 47 * 5 = 235 bytes
};

////////////////////////////////////////////////////////////////////////////////

#define CHAR_SIZE_S 0x01
#define CHAR_SIZE_M 0x02
#define CHAR_SIZE_L 0x04

static uint8_t printbitshift = 0;

static uint8_t expand4bit(uint8_t b)
{	
	// 0000abcd -> aabbccdd
	b = (b | (b << 2)) & 0x33;
	b = (b | (b << 1)) & 0x55;
	return b | (b << 1);
}

static uint8_t expand2bit(uint8_t b)
{							   
	// 000000ab -> aaaabbbb
	b = (b | (b << 3)) & 0x11;
	for (uint8_t i = 3; i > 0; --i) b |= (b << 1);
	return b;
}

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

////////////////////////////////////////////////////////////////////////////////

#define FRAMERATE 10

static uint8_t eachframemillis, thisframestart, lastframedurationms;

static uint8_t justrendered;

void delayshort(uint8_t ms)
{
	long t = millis();
	while ((uint8_t)(millis() - t) < ms);
}

void setframerate(uint8_t rate)
{ 
	eachframemillis = 1000 / rate;
}

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
