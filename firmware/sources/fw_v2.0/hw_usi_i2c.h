// Defines /////////////////////////////////////////////////////////////////////
#define DDR_USI       DDRB
#define PORT_USI      PORTB
#define PIN_USI       PINB

#define PORT_USI_SDA  PORTB0
#define PORT_USI_SCL  PORTB2
#define PIN_USI_SDA   PINB0
#define PIN_USI_SCL   PINB2

#define DDR_USI_CL    DDR_USI
#define PORT_USI_CL   PORT_USI
#define PIN_USI_CL    PIN_USI

#define TWI_FAST_MODE
#ifdef  TWI_FAST_MODE                // TWI FAST mode timing limits. SCL = 100-400kHz
#define DELAY_T2TWI (_delay_us(2))   // >1.3us
#define DELAY_T4TWI (_delay_us(1))   // >0.6us
#else                                // TWI STANDARD mode timing limits. SCL <= 100kHz
#define DELAY_T2TWI (_delay_us(5))   // >4.7us
#define DELAY_T4TWI (_delay_us(4))   // >4.0us
#endif

#define TWI_NACK_BIT 0               // Bit position for (N)ACK bit.

// Constants ///////////////////////////////////////////////////////////////////
// Prepare register value to: Clear flags, and set USI to shift 8 bits i.e. count 16 clock edges.
const unsigned char USISR_8bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0x0<<USICNT0;

// Prepare register value to: Clear flags, and set USI to shift 1 bit i.e. count 2 clock edges.
const unsigned char USISR_1bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0xE<<USICNT0;

// Variables ///////////////////////////////////////////////////////////////////
static int I2Ccount;

// USI I2C Routines ////////////////////////////////////////////////////////////
static uint8_t i2c_transfer(uint8_t data) 
{
	USISR = data;                                // Set USISR according to data.
												 // Prepare clocking.
	data  = 0<<USISIE | 0<<USIOIE |              // Interrupts disabled
			1<<USIWM1 | 0<<USIWM0 |              // Set USI in Two-wire mode.
			1<<USICS1 | 0<<USICS0 | 1<<USICLK |  // Software clock strobe as source.
			1<<USITC;                            // Toggle Clock Port.
	do {
		DELAY_T2TWI;
		USICR = data;                            // Generate positive SCL edge.
		while (!(PIN_USI_CL & 1<<PIN_USI_SCL));  // Wait for SCL to go high.
		DELAY_T4TWI;
		USICR = data;                            // Generate negative SCL edge.
	} while (!(USISR & 1<<USIOIF));              // Check for transfer complete.

	DELAY_T2TWI;
	data = USIDR;                                // Read out data.
	USIDR = 0xFF;                                // Release SDA.
	DDR_USI |= (1<<PIN_USI_SDA);                 // Enable SDA as output.

	return data;                                 // Return the data from the USIDR
}

static void I2CBusInit()
{
	PORT_USI |= 1<<PIN_USI_SDA;                  // Enable pullup on SDA.
	PORT_USI_CL |= 1<<PIN_USI_SCL;               // Enable pullup on SCL.

	DDR_USI_CL |= 1<<PIN_USI_SCL;                // Enable SCL as output.
	DDR_USI |= 1<<PIN_USI_SDA;                   // Enable SDA as output.

	USIDR = 0xFF;                                // Preload data register with "released level" data.
	USICR = 0<<USISIE | 0<<USIOIE |              // Disable Interrupts.
			1<<USIWM1 | 0<<USIWM0 |              // Set USI in Two-wire mode.
			1<<USICS1 | 0<<USICS0 | 1<<USICLK |  // Software stobe as counter clock source
			0<<USITC;
	USISR = 1<<USISIF | 1<<USIOIF | 1<<USIPF |   // Clear flags,
			1<<USIDC  |	0<<USICNT0;              // and reset counter.
}

static uint8_t I2CBusRead()
{
	if ((I2Ccount != 0) && (I2Ccount != -1)) I2Ccount--;

	/* Read a byte */
	DDR_USI &= ~(1<<PIN_USI_SDA);                // Enable SDA as input.
	uint8_t data = i2c_transfer(USISR_8bit);

	/* Prepare to generate ACK (or NACK in case of End Of Transmission) */
	if (I2Ccount == 0) USIDR = 0xFF; else USIDR = 0x00;
	i2c_transfer(USISR_1bit);                    // Generate ACK/NACK.

	return data;                                 // Read successfully completed
}

static uint8_t I2CBusReadLast()
{
	I2Ccount = 0;
	return I2CBusRead();
}

static bool I2CBusWrite(uint8_t data)
{
	/* Write a byte */
	PORT_USI_CL &= ~(1<<PIN_USI_SCL);            // Pull SCL LOW.
	USIDR = data;                                // Setup data.
	i2c_transfer(USISR_8bit);                    // Send 8 bits on bus.

	/* Clock and verify (N)ACK from slave */
	DDR_USI &= ~(1<<PIN_USI_SDA);                // Enable SDA as input.
	if (i2c_transfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false;

	return true;                                 // Write successfully completed
}

static bool I2CBusStart(uint8_t address, int readcount)
{
	/* Start transmission by sending address */
	if (readcount != 0) { I2Ccount = readcount; readcount = 1; }
	uint8_t addressRW = address<<1 | readcount;

	/* Release SCL to ensure that (repeated) Start can be performed */
	PORT_USI_CL |= 1<<PIN_USI_SCL;               // Release SCL.
	while (!(PIN_USI_CL & 1<<PIN_USI_SCL));      // Verify that SCL becomes high.
#ifdef TWI_FAST_MODE
	DELAY_T4TWI;
#else
	DELAY_T2TWI;
#endif

	/* Generate Start Condition */
	PORT_USI &= ~(1<<PIN_USI_SDA);               // Force SDA LOW.
	DELAY_T4TWI;
	PORT_USI_CL &= ~(1<<PIN_USI_SCL);            // Pull SCL LOW.
	PORT_USI |= 1<<PIN_USI_SDA;                  // Release SDA.

	if (!(USISR & 1<<USISIF)) return false;

	/*Write address */
	PORT_USI_CL &= ~(1<<PIN_USI_SCL);            // Pull SCL LOW.
	USIDR = addressRW;                           // Setup data.
	i2c_transfer(USISR_8bit);                    // Send 8 bits on bus.

	/* Clock and verify (N)ACK from slave */
	DDR_USI &= ~(1<<PIN_USI_SDA);                // Enable SDA as input.
	if (i2c_transfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false; // No ACK

	return true;                                 // Start successfully completed
}

static bool I2CBusRestart(uint8_t address, int readcount) 
{
	return I2CBusStart(address, readcount);
}

static void I2CBusStop() 
{
	PORT_USI &= ~(1<<PIN_USI_SDA);               // Pull SDA low.
	PORT_USI_CL |= 1<<PIN_USI_SCL;               // Release SCL.
	while (!(PIN_USI_CL & 1<<PIN_USI_SCL));      // Wait for SCL to go high.
	DELAY_T4TWI;
	PORT_USI |= 1<<PIN_USI_SDA;                  // Release SDA.
	DELAY_T2TWI;
}
