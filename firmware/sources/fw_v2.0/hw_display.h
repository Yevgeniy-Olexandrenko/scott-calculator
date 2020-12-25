////////////////////////////////////////////////////////////////////////////////

#define DISPLAY_ADDRESS 0x3C
#define DISPLAY_COMMAND 0x00
#define DISPLAY_DATA    0x40
#define DISPLAY_WIDTH   128 
#define DISPLAY_PAGES   4

static uint8_t renderRAM = 0xB4, drawRAM = 0x40;

static const uint8_t ssd1306_init_sequence[] PROGMEM =
{
	0xC8,       // Set scan direction (C0 scan from COM0 to COM[N-1] or C8 mirroring)
	0xA1,       // Set segment remap (A0 regular or A1 flip)
	0xA8, 0x1F, // Set mux ratio (N+1) where: 14<N<64 ... 3F or 1F
	0xDA, 0x02, // COM config pin mapping:
	            //                  right/left left/right
	            //      sequential      02        22
	            //      alternate       12        32
	0x20, 0x00, // Horizontal addressing mode (line feed after EOL)
	0x8D, 0x14 // Charge pump (0x14 enable or 0x10 disable)
};

////////////////////////////////////////////////////////////////////////////////

static void ssd1306_send_start()
{ 
	I2CBusStart(DISPLAY_ADDRESS, 0);
}

static uint8_t ssd1306_send_byte(uint8_t b)
{ 
	return I2CBusWrite(b);
}

static void ssd1306_send_stop()
{ 
	I2CBusStop();
}

static void ssd1306_command_start()
{ 
	ssd1306_send_start();
	ssd1306_send_byte(DISPLAY_COMMAND);
}

static void ssd1306_data_start()
{ 
	ssd1306_send_start();
	ssd1306_send_byte(DISPLAY_DATA);
}

static void ssd1306_send_command(uint8_t cmd)
{ 
	ssd1306_command_start();
	ssd1306_send_byte(cmd);
	ssd1306_send_stop();
}

static void ssd1306_send_data(uint8_t b)
{ 
	if (!ssd1306_send_byte(b))
	{
		ssd1306_send_stop();
		ssd1306_data_start();
		ssd1306_send_byte(b);
	}
}

////////////////////////////////////////////////////////////////////////////////

static void DisplayInit()
{
	ssd1306_command_start();
	for (uint8_t i = 0; i < sizeof(ssd1306_init_sequence); i++)
	{
		ssd1306_send_byte(pgm_read_byte(&ssd1306_init_sequence[i]));
	}
	ssd1306_send_stop();
}

static void DisplayTurnOn()
{
	ssd1306_send_command(0xAF);
}

static void DisplayTurnOff()
{ 
	ssd1306_send_command(0xAE);
}

static void DisplayBrightness(uint8_t brightness)
{ 
	ssd1306_command_start();
	ssd1306_send_byte(0x81);
	ssd1306_send_byte(brightness);
	ssd1306_send_stop();
}

static void DisplayPosition(uint8_t x, uint8_t y)
{ 
	ssd1306_command_start();
	ssd1306_send_byte(renderRAM | (y & 0x07));
	ssd1306_send_byte(0x10 | (x >> 4));
	ssd1306_send_byte(x & 0x0f);
	ssd1306_send_stop();
}

static void DisplayWrite(uint8_t b, uint8_t s)
{
	ssd1306_data_start();
	while (s--) ssd1306_send_data(b);
	ssd1306_send_stop();
}

static void DisplayClear()
{
	DisplayPosition(0, 0);
	for (uint8_t i = DISPLAY_PAGES; i > 0; --i)
	{
		DisplayWrite(0x00, DISPLAY_WIDTH);
	}
}

static void DisplayRefresh()
{
	drawRAM ^= 0x20;
	ssd1306_send_command(drawRAM);
	renderRAM ^= 0x04;
}
