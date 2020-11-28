// Defines
#define DDR_USI             DDRB
#define PORT_USI            PORTB
#define PIN_USI             PINB
#define PORT_USI_SDA        PORTB0
#define PORT_USI_SCL        PORTB2
#define PIN_USI_SDA         PINB0
#define PIN_USI_SCL         PINB2
#define DDR_USI_CL          DDR_USI
#define PORT_USI_CL         PORT_USI
#define PIN_USI_CL          PIN_USI
#define TWI_FAST_MODE

#ifdef TWI_FAST_MODE                 // TWI FAST mode timing limits. SCL = 100-400kHz
#define DELAY_T2TWI (_delay_us(2))   // >1.3us
#define DELAY_T4TWI (_delay_us(1))   // >0.6us
#else                                // TWI STANDARD mode timing limits. SCL <= 100kHz
#define DELAY_T2TWI (_delay_us(5))   // >4.7us
#define DELAY_T4TWI (_delay_us(4))   // >4.0us
#endif

#define TWI_NACK_BIT 0 // Bit position for (N)ACK bit.

// Constants
// Prepare register value to: Clear flags, and set USI to shift 8 bits i.e. count 16 clock edges.
const unsigned char USISR_8bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0x0<<USICNT0;
// Prepare register value to: Clear flags, and set USI to shift 1 bit i.e. count 2 clock edges.
const unsigned char USISR_1bit = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | 0xE<<USICNT0;

class TinyI2CMaster {

    public:
	TinyI2CMaster();
	void init(void);
	uint8_t read(void);
	uint8_t readLast(void);
	bool write(uint8_t data);
	bool start(uint8_t address, int readcount);
	bool restart(uint8_t address, int readcount);
	void stop(void);

    private:
	int I2Ccount;
	uint8_t transfer(uint8_t data);
};

TinyI2CMaster::TinyI2CMaster() 
{
}

// Minimal Tiny I2C Routines **********************************************

uint8_t TinyI2CMaster::transfer (uint8_t data) {
  USISR = data;                               // Set USISR according to data.
                                              // Prepare clocking.
  data = 0<<USISIE | 0<<USIOIE |              // Interrupts disabled
         1<<USIWM1 | 0<<USIWM0 |              // Set USI in Two-wire mode.
         1<<USICS1 | 0<<USICS0 | 1<<USICLK |  // Software clock strobe as source.
         1<<USITC;                            // Toggle Clock Port.
  do {
    DELAY_T2TWI;
    USICR = data;                             // Generate positive SCL edge.
    while (!(PIN_USI_CL & 1<<PIN_USI_SCL));   // Wait for SCL to go high.
    DELAY_T4TWI;
    USICR = data;                             // Generate negative SCL edge.
  } while (!(USISR & 1<<USIOIF));             // Check for transfer complete.

  DELAY_T2TWI;
  data = USIDR;                               // Read out data.
  USIDR = 0xFF;                               // Release SDA.
  DDR_USI |= (1<<PIN_USI_SDA);                // Enable SDA as output.

  return data;                                // Return the data from the USIDR
}

void TinyI2CMaster::init () {
  PORT_USI |= 1<<PIN_USI_SDA;                 // Enable pullup on SDA.
  PORT_USI_CL |= 1<<PIN_USI_SCL;              // Enable pullup on SCL.

  DDR_USI_CL |= 1<<PIN_USI_SCL;               // Enable SCL as output.
  DDR_USI |= 1<<PIN_USI_SDA;                  // Enable SDA as output.

  USIDR = 0xFF;                               // Preload data register with "released level" data.
  USICR = 0<<USISIE | 0<<USIOIE |             // Disable Interrupts.
          1<<USIWM1 | 0<<USIWM0 |             // Set USI in Two-wire mode.
          1<<USICS1 | 0<<USICS0 | 1<<USICLK | // Software stobe as counter clock source
          0<<USITC;
  USISR = 1<<USISIF | 1<<USIOIF | 1<<USIPF | 1<<USIDC | // Clear flags,
          0x0<<USICNT0;                       // and reset counter.
}

uint8_t TinyI2CMaster::read (void) {
  if ((I2Ccount != 0) && (I2Ccount != -1)) I2Ccount--;
  
  /* Read a byte */
  DDR_USI &= ~(1<<PIN_USI_SDA);               // Enable SDA as input.
  uint8_t data = TinyI2CMaster::transfer(USISR_8bit);

  /* Prepare to generate ACK (or NACK in case of End Of Transmission) */
  if (I2Ccount == 0) USIDR = 0xFF; else USIDR = 0x00;
  TinyI2CMaster::transfer(USISR_1bit);                 // Generate ACK/NACK.

  return data;                                // Read successfully completed
}

uint8_t TinyI2CMaster::readLast (void) {
  I2Ccount = 0;
  return TinyI2CMaster::read();
}

bool TinyI2CMaster::write (uint8_t data) {
  /* Write a byte */
  PORT_USI_CL &= ~(1<<PIN_USI_SCL);           // Pull SCL LOW.
  USIDR = data;                               // Setup data.
  TinyI2CMaster::transfer(USISR_8bit);                 // Send 8 bits on bus.

  /* Clock and verify (N)ACK from slave */
  DDR_USI &= ~(1<<PIN_USI_SDA);               // Enable SDA as input.
  if (TinyI2CMaster::transfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false;

  return true;                                // Write successfully completed
}

// Start transmission by sending address
bool TinyI2CMaster::start (uint8_t address, int readcount) {
  if (readcount != 0) { I2Ccount = readcount; readcount = 1; }
  uint8_t addressRW = address<<1 | readcount;

  /* Release SCL to ensure that (repeated) Start can be performed */
  PORT_USI_CL |= 1<<PIN_USI_SCL;              // Release SCL.
  while (!(PIN_USI_CL & 1<<PIN_USI_SCL));     // Verify that SCL becomes high.
#ifdef TWI_FAST_MODE
  DELAY_T4TWI;
#else
  DELAY_T2TWI;
#endif

  /* Generate Start Condition */
  PORT_USI &= ~(1<<PIN_USI_SDA);              // Force SDA LOW.
  DELAY_T4TWI;
  PORT_USI_CL &= ~(1<<PIN_USI_SCL);           // Pull SCL LOW.
  PORT_USI |= 1<<PIN_USI_SDA;                 // Release SDA.

  if (!(USISR & 1<<USISIF)) return false;

  /*Write address */
  PORT_USI_CL &= ~(1<<PIN_USI_SCL);           // Pull SCL LOW.
  USIDR = addressRW;                          // Setup data.
  TinyI2CMaster::transfer(USISR_8bit);                 // Send 8 bits on bus.

  /* Clock and verify (N)ACK from slave */
  DDR_USI &= ~(1<<PIN_USI_SDA);               // Enable SDA as input.
  if (TinyI2CMaster::transfer(USISR_1bit) & 1<<TWI_NACK_BIT) return false; // No ACK

  return true;                                // Start successfully completed
}

bool TinyI2CMaster::restart(uint8_t address, int readcount) {
  return TinyI2CMaster::start(address, readcount);
}

void TinyI2CMaster::stop (void) {
  PORT_USI &= ~(1<<PIN_USI_SDA);              // Pull SDA low.
  PORT_USI_CL |= 1<<PIN_USI_SCL;              // Release SCL.
  while (!(PIN_USI_CL & 1<<PIN_USI_SCL));     // Wait for SCL to go high.
  DELAY_T4TWI;
  PORT_USI |= 1<<PIN_USI_SDA;                 // Release SDA.
  DELAY_T2TWI;
}

static TinyI2CMaster TinyI2C;