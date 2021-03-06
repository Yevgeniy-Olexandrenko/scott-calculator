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

// Figure out build date and time (Example __DATE__ : "Jul 27 2012" and __TIME__ : "21:06:19")
#define COMPUTE_BUILD_YEAR ((__DATE__[ 9] - '0') *   10 + (__DATE__[10] - '0'))
#define COMPUTE_BUILD_DAY  (((__DATE__[4] >= '0') ? (__DATE__[4] - '0') * 10 : 0) + (__DATE__[5] - '0'))
#define BUILD_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_MONTH_IS_DEC (__DATE__[0] == 'D')
#define COMPUTE_BUILD_MONTH \
		( \
			(BUILD_MONTH_IS_JAN) ?  1 : \
			(BUILD_MONTH_IS_FEB) ?  2 : \
			(BUILD_MONTH_IS_MAR) ?  3 : \
			(BUILD_MONTH_IS_APR) ?  4 : \
			(BUILD_MONTH_IS_MAY) ?  5 : \
			(BUILD_MONTH_IS_JUN) ?  6 : \
			(BUILD_MONTH_IS_JUL) ?  7 : \
			(BUILD_MONTH_IS_AUG) ?  8 : \
			(BUILD_MONTH_IS_SEP) ?  9 : \
			(BUILD_MONTH_IS_OCT) ? 10 : \
			(BUILD_MONTH_IS_NOV) ? 11 : \
			(BUILD_MONTH_IS_DEC) ? 12 : \
			/* error default */  99 \
		)
#define COMPUTE_BUILD_HOUR ((__TIME__[0] - '0') * 10 + __TIME__[1] - '0')
#define COMPUTE_BUILD_MIN  ((__TIME__[3] - '0') * 10 + __TIME__[4] - '0')
#define COMPUTE_BUILD_SEC  ((__TIME__[6] - '0') * 10 + __TIME__[7] - '0')

#define BUILD_DATE_IS_BAD (__DATE__[0] == '?')
#define BUILD_YEAR        ((BUILD_DATE_IS_BAD) ? 0 : COMPUTE_BUILD_YEAR)
#define BUILD_MONTH       ((BUILD_DATE_IS_BAD) ? 1 : COMPUTE_BUILD_MONTH)
#define BUILD_DAY         ((BUILD_DATE_IS_BAD) ? 1 : COMPUTE_BUILD_DAY)

#define BUILD_TIME_IS_BAD (__TIME__[0] == '?')
#define BUILD_HOUR        ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_HOUR)
#define BUILD_MIN         ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_MIN)
#define BUILD_SEC         ((BUILD_TIME_IS_BAD) ? 99 :  COMPUTE_BUILD_SEC)

static uint8_t rtc_seconds = BUILD_SEC;   // 0 - 59
static uint8_t rtc_minutes = BUILD_MIN;   // 0 - 59
static uint8_t rtc_hours   = BUILD_HOUR;  // 0 - 23
static uint8_t rtc_date    = BUILD_DAY;   // 1 - 31
static uint8_t rtc_month   = BUILD_MONTH; // 1 - 12
static uint8_t rtc_year    = BUILD_YEAR;  // 0 - 99

static uint8_t decode_bcd(uint8_t num)
{
  return (num / 16 * 10) + (num % 16);
}

static uint8_t encode_bcd(uint8_t num)
{
  return (num / 10 * 16) + (num % 10);
}

static bool RTCRead()
{
	// Day of the week is not used!
	// Century flag is not supported!
	if (I2CBusStart(DS1337_ADDR, 0))
	{
		I2CBusWrite(DS1337_REG_SECONDS);
		I2CBusRestart(DS1337_ADDR, 7);
		rtc_seconds = decode_bcd(I2CBusRead());
		rtc_minutes = decode_bcd(I2CBusRead());
		uint8_t tmp = I2CBusRead();
		if (tmp & DS1337_HOUR_12) 
			rtc_hours = ((tmp >> 4) & 0x01) * 12 + ((tmp >> 5) & 0x01) * 12;
		else 
			rtc_hours = decode_bcd(tmp);
		tmp = I2CBusRead();
		rtc_date  = decode_bcd(I2CBusRead());
		rtc_month = decode_bcd(I2CBusRead() & 0x1F);
		rtc_year  = decode_bcd(I2CBusRead() % 100);
		I2CBusStop();
		return true;
	}
	return false;
}

static void RTCWrite()
{
	// Time always stored in 24-hour format!
	// Day of the week is not used!
	// Century flag is not supported!
	if(I2CBusStart(DS1337_ADDR, 0))
	{
		I2CBusWrite(DS1337_REG_SECONDS);
		I2CBusWrite(encode_bcd(rtc_seconds));
		I2CBusWrite(encode_bcd(rtc_minutes));
		I2CBusWrite(encode_bcd(rtc_hours));
		I2CBusWrite(1);
		I2CBusWrite(encode_bcd(rtc_date));
		I2CBusWrite(encode_bcd(rtc_month));
		I2CBusWrite(encode_bcd(rtc_year));
		I2CBusStop();
	}
}
