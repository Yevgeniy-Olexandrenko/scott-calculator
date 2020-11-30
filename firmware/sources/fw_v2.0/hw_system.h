static void DisplayTurnOn();
static void DisplayTurnOff();

////////////////////////////////////////////////////////////////////////////////

static void ADCInit()
{
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
}

static uint16_t ADCRead(uint8_t channel, uint8_t delay)
{
	ADMUX = channel & 0xF;
	while (delay--) _delay_ms(1);
	set_bit(ADCSRA, ADSC);
	while (isb_set(ADCSRA, ADSC));
	uint8_t adcl = ADCL;
	uint8_t adch = ADCH;
	return (adcl | adch << 8);
}

///////////////////////////////////////////////////////////////////////////////

#define WDT_MODE_DISABLED  0x00
#define WDT_MODE_RES       0x08 // To reset the CPU if there is a timeout
#define WDT_MODE_INT       0x40 // Timeout will cause an interrupt
#define WDT_MODE_INT_RES   0x48 // First time-out interrupt , the second time out - reset

#define WDT_TIMEOUT_16MS   0x00 // (16 ± 1.6) ms
#define WDT_TIMEOUT_32MS   0x01 // (32 ± 3.2) ms
#define WDT_TIMEOUT_64MS   0x02 // (64 ± 6.4) ms
#define WDT_TIMEOUT_125MS  0x03 // (128 ± 12.8) ms
#define WDT_TIMEOUT_250MS  0x04 // (256 ± 25.6) ms
#define WDT_TIMEOUT_500MS  0x05 // (512 ± 51.2) ms
#define WDT_TIMEOUT_1S     0x06 // (1024 ± 102.4) ms
#define WDT_TIMEOUT_2S     0x07 // (2048 ± 204.8) ms
#define WDT_TIMEOUT_4S     0x08 // (4096 ± 409.6) ms
#define WDT_TIMEOUT_8S     0x09 // (8192 ± 819.2) ms

static volatile uint8_t  isWaitingForNextFrame;
static volatile uint16_t frameCounter;

static void wdt_init(uint8_t mode, uint8_t prescaler)
{
	// does not change global interrupts enable flag
	uint8_t wdtr = mode | ((prescaler > 7) ? 0x20 | (prescaler - 8) : prescaler);
	uint8_t sreg = SREG;
	cli();
	WDTCR = ((1 << WDCE) | (1 << WDE));
	WDTCR = wdtr;
	SREG  = sreg;
}

ISR(WDT_vect)
{
	isWaitingForNextFrame = false;
	frameCounter++;
}

static void ResetFrameCounter()
{
	frameCounter = 0;
}

static void EnableFrameSync()
{
	// frame rate is about 15 FPS
	wdt_init(WDT_MODE_INT, WDT_TIMEOUT_64MS);
	ResetFrameCounter();
}

static void DisableFrameSync()
{
	wdt_init(WDT_MODE_DISABLED, 0);
}

////////////////////////////////////////////////////////////////////////////////

static void execute_sleep(uint8_t mode)
{ 	
	power_all_disable();
	set_sleep_mode(mode);
	sleep_enable();
	sleep_cpu();

	// sleeping here...

	sleep_disable();
	power_usi_enable(); // Power On USI for I2C communication
	power_adc_enable(); // Power On ADC for analog keyboard reading
}

static void DeepSleep()
{
	_delay_ms(250);

	DisplayTurnOff();
	DisableFrameSync();
	
	execute_sleep(SLEEP_MODE_PWR_DOWN);

	EnableFrameSync();
	DisplayTurnOn();
}

static void IdleSleep()
{ 
	execute_sleep(SLEEP_MODE_IDLE);
}

////////////////////////////////////////////////////////////////////////////////

static void WaitForNextFrame()
{
	isWaitingForNextFrame = true;
	while (isWaitingForNextFrame) IdleSleep();
}
