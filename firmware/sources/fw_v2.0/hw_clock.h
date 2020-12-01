////////////////////////////////////////////////////////////////////////////////

// Device address
#define DS1337_ADDR            0x68

// Registers
#define DS1337_REG_SECONDS     0x00
#define DS1337_REG_MINUTES     0x01
#define DS1337_REG_HOURS       0x02
#define DS1337_REG_DAY         0x03
#define DS1337_REG_DATE        0x04
#define DS1337_REG_MONTH       0x05
#define DS1337_REG_YEAR        0x06
#define DS1337_REG_A1_SECONDS  0x07
#define DS1337_REG_A1_MINUTES  0x08
#define DS1337_REG_A1_HOUR     0x09
#define DS1337_REG_A1_DAY_DATE 0x0A
#define DS1337_REG_A2_MINUTES  0x0B
#define DS1337_REG_A2_HOUR     0x0C
#define DS1337_REG_A2_DAY_DATE 0x0D
#define DS1337_REG_CONTROL     0x0E
#define DS1337_REG_STATUS      0x0F

// Flags
#define DS1337_HOUR_12         (0x01 << 6)

static uint8_t rtc_available;  // 0 - 1
static uint8_t rtc_seconds;    // 0 - 59
static uint8_t rtc_minutes;    // 0 - 59
static uint8_t rtc_hours;      // 0 - 23
static uint8_t rtc_date;       // 1 - 31
static uint8_t rtc_month;      // 1 - 12
static uint8_t rtc_year;       // 0 - 199

static uint8_t decode_bcd(uint8_t x)
{
	return ((x >> 4) * 10 + (x & 0x0F));
}

static uint8_t encode_bcd(uint8_t x)
{
	return ((((x / 10) & 0x0F) << 4) + (x % 10));
}

static void RTCRead()
{
	rtc_available = false;
	if (I2CBusStart(DS1337_ADDR, 0))
	{
		uint8_t tmp;
		rtc_available = true;
		I2CBusWrite(DS1337_REG_SECONDS);
		I2CBusRestart(DS1337_ADDR, 7);
		rtc_seconds = decode_bcd(I2CBusRead());
		rtc_minutes = decode_bcd(I2CBusRead());
		tmp = I2CBusRead();
		if (tmp & DS1337_HOUR_12) 
			rtc_hours = ((tmp >> 4) & 0x01) * 12 + ((tmp >> 5) & 0x01) * 12;
		else 
			rtc_hours = decode_bcd(tmp);
		tmp = I2CBusRead();
		rtc_date = decode_bcd(I2CBusRead());
		tmp = I2CBusRead();
		rtc_month = decode_bcd(tmp & 0x1F);
		rtc_year = 100 * ((tmp >> 7) & 0x01) + decode_bcd(I2CBusRead());
		I2CBusStop();
	}
}

static void RTCWrite()
{
	// Time always stored in 24-hour format!
	// Day of the week is not used!
	if(I2CBusStart(DS1337_ADDR, 0))
	{
		I2CBusWrite(DS1337_REG_SECONDS);
		I2CBusWrite(encode_bcd(rtc_seconds));
		I2CBusWrite(encode_bcd(rtc_minutes));
		I2CBusWrite(encode_bcd(rtc_hours));
		I2CBusWrite(1);
		I2CBusWrite(encode_bcd(rtc_date));
		I2CBusWrite(((rtc_year / 100) << 7) | encode_bcd(rtc_month));
		I2CBusWrite(encode_bcd((rtc_year) % 100));
		I2CBusStop();
	}
}
