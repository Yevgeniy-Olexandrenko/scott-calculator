////////////////////////////////////////////////////////////////////////////////

#define CHAR_SHIFT '='
#define CHAR_REC   '@'
#define CHAR_PLAY  '<'




#define MENU_OPS_PER_LINE 3		// 3 function keys
#define KEY_DUMMY 0xff	    // Needed to enter key-loop and printstring







const char menu_str[] PROGMEM = 
	"\03"
	",X+" "Y;+" "1/X"  // Squareroot, Raise to the power of, Reciprocal
	"EXP" "LN+" "X?+"  // Exponential, Natural logarithm, Gamma function (due to Nemes)
	"R>P" "P>R" "PV+"  // Rectangular to polar coordinates, Polar to rectangular coordinates, Present value (annuity)
	"ND+" "STA" "LR+"  // Normal distribution (CDF/PDF), Statistics, Linear regression
	"SIN" "COS" "TAN"  // Sine, Cosine, Tangent
	"ASN" "ACS" "ATN"  // Inverse sine, Inverse cosine, Inverse tangent
	"SNH" "CSH" "TNH"  // Hyperbolic sine, Hyperbolic cosine,  Hyperbolic tangent
	"ASH" "ACH" "ATH"  // Inverse hyperbolic sine, Inverse hyperbolic cosine, Inverse hyperbolic tangent
	"CST" "CMD" "BRI"  // Set constant key, Set command key, Set display brightness
	"@1+" "@2+" "@3+"  // Record user keys
	"<1+" "<2+" "<3+"; // Play user keys
#define NUM_OF_MENU_OPS 33

#define MSG_ERR 0
#define MSG_INF 1
const char message_str[] PROGMEM =
	"\03" "ERR" "INF";

const char month_str[] PROGMEM = 
	"\03"
	"JAN" "FEB" "MAR" "APR" "MAY" "JUN"
	"JUL" "AUG" "SEP" "OCT" "NOV" "DEC";












void ResetCalculator()
{
	brightness = eeprom_read_byte(&eeprom_brightness);
	for (uint8_t i = 0; i < STACK_SIZE; i++) stack.arr[i] = 0.f;
}




#define DIGITS      (6)
#define DIGIT_WIDTH (FONT_WIDTH * CHAR_SIZE_M + 1)
#define POINT_WIDTH (DIGIT_WIDTH - 6)

#define MODE_CHAR   (128 - (DIGIT_WIDTH - 1))

#define M_SIGN      (0)
#define M_DIGIT_FST (M_SIGN + DIGIT_WIDTH)
#define M_DIGIT_LST (M_DIGIT_FST + ((DIGITS - 1) * DIGIT_WIDTH) + POINT_WIDTH)

#define E_DIGIT2    (MODE_CHAR - DIGIT_WIDTH)
#define E_DIGIT1    (E_DIGIT2 - DIGIT_WIDTH)
#define E_SIGN      (E_DIGIT1 - DIGIT_WIDTH)

#define MODE_CHAR   (128 - (DIGIT_WIDTH - 1))

static void PrintStack(uint8_t i, int8_t d, uint8_t s, uint8_t y)
{
	float f = stack.arr[i];
	PrintCharSize(CHAR_SIZE_M, s);

	if (isnan(f)) { PrintStringAt(FPSTR(message_str), MSG_ERR, M_DIGIT_FST, y);	}
	else
	{
		if (f < 0) { f = -f; PrintCharAt('-', M_SIGN, y); }
		if (isinf(f)) {	PrintStringAt(FPSTR(message_str), MSG_INF, M_DIGIT_FST, y); }
		else
		{
			int8_t e = (int8_t)(log(f) / log(10.f));
			uint32_t m = (uint32_t)(f / MathPow10(e - (DIGITS - 1)) + 0.5f);

			if (m > 0 && m < MathPow10(DIGITS - 1))
			{
				m = (uint32_t)(f / MathPow10(--e - (DIGITS - 1)) + 0.5f);
			}

			int8_t int_dig = 1, lead_z = 0;
			if (_abs(e) < DIGITS)
			{
				if (e >= 0)
				{
					int_dig += e;
				}
				else
				{
					int_dig = 0;
					lead_z -= e;
					for (uint8_t n = lead_z; n--; m /= 10);
				}
				e = 0;
			}
			int8_t fra_dig = DIGITS - lead_z - int_dig;

			uint8_t x = M_DIGIT_LST, nonzero = false;
			for (; fra_dig--; m /= 10, x -= DIGIT_WIDTH)
			{
				uint8_t ones = _ones(m);
				if (ones || nonzero)
				{
					PrintCharAt('0' + ones, x, y);
					nonzero = true;
				}
			}

			if (nonzero)
			{
				for (; --lead_z > 0; x -= DIGIT_WIDTH) PrintCharAt('0', x, y);
				PrintCharAt('.', x, y);
			}
			
			PrintCharAt('0', x -= POINT_WIDTH, y);
			for (; int_dig--; m /= 10, x -= DIGIT_WIDTH) PrintCharAt('0' + _ones(m), x, y);

			if (e)
			{
				PrintCharSize(CHAR_SIZE_M, s >> 1);
				if (e < 0) { e = -e; PrintCharAt('-', E_SIGN, y); }
				PrintTwoDigitAt(e, E_DIGIT1, y);
			}
		}
	}
}

static void PrintClock()
{
	DisplayClear();

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_L);
	PrintCharAt(':', 20, 0);
	PrintCharAt(':', 47, 0);
	PrintTwoDigitAt(rtc_hours, 0, 0);
	PrintTwoDigitAt(rtc_minutes, 27, 0);
	PrintTwoDigitAt(rtc_seconds, 54, 0);

	PrintCharSize(CHAR_SIZE_S, CHAR_SIZE_S);
	PrintStringAt(FPSTR(month_str), rtc_month - 1, 85, 0);

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_S);
	PrintTwoDigitAt(rtc_date, 107, 0);
	PrintTwoDigitAt(20, 85, 1);
	PrintTwoDigitAt(rtc_year, 107, 1);

	DisplayRefresh();
}

static void PrintCalculator()
{
	DisplayClear();

	PrintCharSize(CHAR_SIZE_M, CHAR_SIZE_M);
	if (isTypeRecording) PrintCharAt(CHAR_REC, MODE_CHAR, 2);
	if (isTypePlaying) PrintCharAt(CHAR_PLAY, MODE_CHAR, 2);
	if (isShift) PrintCharAt(CHAR_SHIFT, MODE_CHAR, 0);

	uint8_t d = isNewNumber ? 0 : decimals;
	
	if (isMenu)
	{
		
		for (uint8_t i = 0; i < MENU_OPS_PER_LINE; ++i)
		{
			PrintStringAt(FPSTR(menu_str), select * MENU_OPS_PER_LINE + i, 48 * i, 2);
		}
		PrintStack(Stack::X, d, CHAR_SIZE_M, 0);
	}
	else if (isShift)
	{
		PrintStack(Stack::Y, 0, CHAR_SIZE_M, 0);
		PrintStack(Stack::X, d, CHAR_SIZE_M, 2);
	}
	else
	{
		PrintStack(Stack::X, d, CHAR_SIZE_L, 0);
	}

	DisplayRefresh();
}

////////////////////////////////////////////////////////////////////////////////

int main() 
{
	sei();
	ADCInit();
	I2CBusInit();
	DisplayInit();
	KeyboardInit();

	DisplayTurnOn();
	FrameSyncEnable();
	ResetCalculator();

#if 0
	SetTime();
	SetDate();
#endif

	for (;;)
	{
		FrameSyncWait();

		if (isfirstrun)
		{
			RTCWrite();
			isfirstrun = false;
			key = KEY_DUMMY;
		}
		else
		{
			key = KeyboardRead();
			if (key == oldkey) key = NULL; else oldkey = key;
		}

		if (key)
		{
			ResetFrameCounter();
			inCalcMode = true;
		}

		if (frameCounter >= POWEROFF_FRAMES)
		{
			DeepSleep();
			oldkey = KeyboardRead();
			inCalcMode = !RTCRead();
		}

		DisplayBrightness(frameCounter < DIMOUT_FRAMES ? brightness : 0);

		if (inCalcMode)
		{
			if (isPlayString)
			{
				key = playbuf[select];
				if (key == NULL)
				{
					LoadStackFromShadowBuffer(restore);
					isPlayString = false;
					isNewNumber = true;
					key = KEY_DUMMY;
				}
				else
				{
					if (key <= KEY_C3_D && ((select == 0) || (select > 0 && playbuf[select - 1] > KEY_C3_D)))
					{
						isNewNumber = true;
						ispushed = false;
					}
					select++;
				}
			}

			else if (isTypeRecording || isTypePlaying)
			{
				if (isTypeRecording)
				{
					if (key && recIndex < TYPEREC_STEPS)
					{
						eeprom_write_byte(&eeprom_typerecord[recSlot][recIndex++], key);
					}
				}
				else
				{
					if (key == KEY_A3_C)
					{
						isTypePlaying = false;
						key = KEY_DUMMY;
					}
					key = eeprom_read_byte(&eeprom_typerecord[recSlot][recIndex++]);
				}
				if (key == KEY_A3_C || recIndex >= TYPEREC_STEPS)
				{
					isTypePlaying = isTypeRecording = false;
					key = KEY_DUMMY;
				}
			}

			if (key == KEY_A0_F)
			{
				isShift ^= true;
				key = KEY_DUMMY;
			}

			if (key)
			{
				if (key != KEY_DUMMY)
				{
					if (isMenu)
					{
						uint8_t limit = NUM_OF_MENU_OPS / MENU_OPS_PER_LINE - 1;
						if (key == KEY_A1_E)
						{
							if (select > 0) select--; else select = limit;
						}
						else if (key == KEY_A2_S)
						{
							if (select < limit) select++; else select = 0;
						}
						else if (key == KEY_A3_C)
						{
							isMenu = false;
						}
						else if (key >= KEY_B2_1 && key <= KEY_D2_3)
						{
							uint8_t index = select * MENU_OPS_PER_LINE + (key - KEY_B2_1);
							Dispatch(21 + index);
							isNewNumber = true;
							isMenu = false;
						}
					}

					else if (isShift)
					{
						Dispatch(5 + key - KEY_B3_0);
						isShift = ispushed = false;
						isNewNumber = true;
					}

					else
					{
						if (key > KEY_D0_9) 
							Dispatch(key - KEY_C3_D);
						else
							EnterDigit(key - KEY_B3_0);
					}
				}
				PrintCalculator();
			}
		}
		else
		{
			RTCRead();
			PrintClock();
		}
	}
	return 0;
}
