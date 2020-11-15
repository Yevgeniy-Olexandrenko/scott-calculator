// DISPLAY
#define DISPLAY_ADDRESS 0x3C  // I2C slave address
#define DISPLAY_PAGES   4     // Lines of screen
#define DISPLAY_COMMAND 0x00  // Command byte
#define DISPLAY_DATA    0x40  // Data byte
#define SCREENWIDTH     128   // Screen width in pixel

// Display cursor coordinates
static uint8_t dx = 0, dy = 0;

// Masks to address GDDRAM of display
static uint8_t renderram = 0xB0, drawram = 0x40;

// Initialization sequence
static const uint8_t inits[] PROGMEM =
{
	0xC8,		// Set scan direction (C0 scan from COM0 to COM[N-1] or C8 mirroring)
	0xA1,		// Set segment remap (A0 regular or A1 flip)
	0xA8, 0x1F, // Set mux ratio (N+1) where: 14<N<64 ... 3F or 1F
	0xDA, 0x02, // COM config pin mapping:
				//					right/left left/right
				//		sequential      02        22
				//		alternate       12        32
	0x20, 0x00, // Horizontal addressing mode (line feed after EOL)
	0x8D, 0x14	// Charge pump (0x14 enable or 0x10 disable)
};

// Initialize communication
static void dbegin(void)
{
	TinyWireM.begin();
}

// Start communication
static void dsendstart(void)
{ 
	TinyWireM.beginTransmission(DISPLAY_ADDRESS);
}

// Send byte
static bool dsendbyte(uint8_t b)
{ 
	return (TinyWireM.write(b));
}

// Stop communication
static void dsendstop(void)
{ 
	TinyWireM.endTransmission();
}

// Start data transfer
static void dsenddatastart(void)
{ 
	dsendstart();
	dsendbyte(DISPLAY_DATA);
}

// Send data byte
static void dsenddatabyte(uint8_t b)
{ 
	if (!dsendbyte(b))
	{
		dsendstop();
		dsenddatastart();
		dsendbyte(b);
	}
}

// Start command transfer
static void dsendcmdstart(void)
{ 
	dsendstart();
	dsendbyte(DISPLAY_COMMAND);
}

// Send command
static void dsendcmd(uint8_t cmd)
{ 
	dsendcmdstart();
	dsendbyte(cmd);
	dsendstop();
}

// Render current half of GDDRAM to oled display
static void drender(void)
{
	renderram ^= 0x04;
}

// Swap GDDRAM to other half and render
static void dswap(void)
{
	drawram ^= 0x20;
	dsendcmd(drawram);
	drender();
}

// Run initialization sequence
static void dinit(void)
{
	dbegin();
	dsendstart();
	dsendbyte(DISPLAY_COMMAND);
	for (uint8_t i = 0; i < sizeof(inits); i++)
		dsendbyte(pgm_read_byte(&inits[i]));
	dsendstop();
}

// Display on
static void don(void)
{
	dsendcmd(0xAF);
}

// Display off
static void doff(void)
{ 
	dsendcmd(0xAE);
}

// Set contrast
void dcontrast(uint8_t contrast)
{ 
	dsendcmdstart();
	dsendbyte(0x81);
	dsendbyte(contrast);
	dsendstop();
}

// Set cursor to position (x|y)
static void dsetcursor(uint8_t x, uint8_t y)
{ 
	dsendcmdstart();
	dsendbyte(renderram | (y & 0x07));
	dsendbyte(0x10 | (x >> 4));
	dsendbyte(x & 0x0f);
	dsendstop();
	dx = x;
	dy = y;
}

// Fill screen with byte/pattern b
static void dfill(uint8_t b)
{
	dsetcursor(0, 0);
	dsenddatastart();
	for (int i = 0; i < SCREENWIDTH * DISPLAY_PAGES; i++)
		dsenddatabyte(b);
	dsendstop();
}
