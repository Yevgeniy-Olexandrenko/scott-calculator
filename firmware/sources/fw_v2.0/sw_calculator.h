////////////////////////////////////////////////////////////////////////////////

#define CHARSPACE ':'	    // Character for space symbol
#define CHARSHIFT '='	    // Character for shift symbol
#define CHARREC   '@'       // Character for recording symbol

#define DIMOUT_FRAMES 156   // Frames before display dim out (about 10 sec)
#define POWEROFF_FRAMES 469 // Frames before power off (about 30 sec)

#define STACK_SIZE 5		// Size of floatstack
#define TINYNUMBER 1e-7	    // Number for rounding to 0
#define MAXITERATE 100	    // Maximal number of Taylor series loops to iterate
#define FKEYNR 3		    // 3 function keys
#define KEY_DUMMY 0xff	    // Needed to enter key-loop and printstring

#define EECONTRAST 0	    // EEPROM address of brightness (1 byte)
#define EESTACK 1		    // EEPROM address of stack ((4+1)*4 bytes) (not used now!!!)
#define EECMDKEY 21		    // EEPROM address of command keys (10 bytes)
#define EECONST 31		    // EEPROM address of constants (10*4 bytes)
#define EEREC 71		    // EEPROM address Starting EE-address for saving "type recorder"
#define MAXREC 146		    // Number of record steps per slot
#define MAXRECSLOT 3	    // Maximal slots to record

#define BITEXP 1		    // Bit for exp()
#define BITSIN 2		    // Bit for sin()
#define BITASIN 4		    // Bit for asin

#define _min(a,b) ((a) < (b) ? (a) : (b))
#define _max(a,b) ((a) > (b) ? (a) : (b))
#define _abs(x)    ((x < 0) ? (-x) : (x)) // abs()-substitute macro
#define _ones(x)   ((x) % 10)             // Calculates ones unit
#define _tens(x)   (((x) / 10) % 10)      // Calculates tens unit
#define _huns(x)   (((x) / 100) % 10)     // Calculates hundreds unit
#define _tsds(x)   (((x) / 1000) % 10)    // Calculates thousands unit
#define _to_rad(x) ((x) * (PI / 180))
#define _to_deg(x) ((x) * (180 / PI))

static uint8_t key;                                // Holds entered key
static uint8_t oldkey;                             // Old key - use for debouncing
static float   stack[STACK_SIZE];                  // Float stack (XYZT) and memory
static uint8_t isNewNumber = true;                 // True if stack has to be lifted before entering a new number
static uint8_t ispushed = false;                   // True if stack was already pushed by ENTER
static uint8_t isShowStack = false;
static uint8_t decimals = 0;                       // Number of decimals entered - used for input after decimal dot
static uint8_t isdot = false;                      // True if dot was pressed and decimals will be entered
static uint8_t isShift = false;                        // true if f is pressed
static uint8_t isMenu = false;                     // True if menu demanded
static uint8_t select = 0;                         // Selection number or playstring position
static uint8_t isPlayString = false;               // True if string should be played
static uint8_t brightness;                         // Contrast
static uint8_t isfirstrun = true;                  // Allows first run of loop and printscreen without key query
static uint16_t recptr = 0;                         // Pointer to recording step
static uint8_t recslot = 0;                        // Slot number for recording to EEPROM
static uint8_t isrec = false, isplay = false;      // True, if "Type Recorder" records or plays
static float   sum[STACK_SIZE] = { 0.f };	       // Memory to save statistic sums
static float   shadow[STACK_SIZE] = { 0.f };        // Shadow memory (buffer) for stack
static uint8_t restore;                            // Position of stack salvation (including mem)

const char c00[] PROGMEM = ",X";  // Squareroot
const char c01[] PROGMEM = "Y;";  // Raise to the power of
const char c02[] PROGMEM = "1/X"; // Reciprocal

const char c03[] PROGMEM = "EXP"; // Exponential
const char c04[] PROGMEM = "LN";  // Natural logarithm
const char c05[] PROGMEM = "X?";   // Gamma function (due to Nemes)

const char c06[] PROGMEM = ">P";  // Rectangular to polar coordinates
const char c07[] PROGMEM = ">R";  // Polar to rectangular coordinates
const char c08[] PROGMEM = "PV";  // Present value (annuity)

const char c09[] PROGMEM = "ND";  // Normal distribution (CDF/PDF)
const char c10[] PROGMEM = "STA"; // Statistics
const char c11[] PROGMEM = "LR";  // Linear regression

const char c12[] PROGMEM = "SIN"; // Sine
const char c13[] PROGMEM = "COS"; // Cosine
const char c14[] PROGMEM = "TAN"; // Tangent

const char c15[] PROGMEM = "ASN"; // Inverse sine
const char c16[] PROGMEM = "ACS"; // Inverse cosine
const char c17[] PROGMEM = "ATN"; // Inverse tangent

const char c18[] PROGMEM = "SNH"; // Hyperbolic sine
const char c19[] PROGMEM = "CSH"; // Hyperbolic cosine
const char c20[] PROGMEM = "TNH"; // Hyperbolic tangent

const char c21[] PROGMEM = "ASH"; // Inverse hyperbolic sine
const char c22[] PROGMEM = "ACH"; // Inverse hyperbolic cosine
const char c23[] PROGMEM = "ATH"; // Inverse hyperbolic tangent

const char c24[] PROGMEM = "CST"; // Set constant key
const char c25[] PROGMEM = "CMD"; // Set command key
const char c26[] PROGMEM = "LIT"; // Set contrast/brightness

const char c27[] PROGMEM = "@1";  //  ... Record user keys
const char c28[] PROGMEM = "@2";  //
const char c29[] PROGMEM = "@3";  //

const char c30[] PROGMEM = "<1";  //  ... Play user keys
const char c31[] PROGMEM = "<2";  //
const char c32[] PROGMEM = "<3";  //

const char * const cmd[] PROGMEM = 
{
	c00, c01, c02,
	c03, c04, c05,
	c06, c07, c08,
	c09, c10, c11,
	c12, c13, c14,
	c15, c16, c17,
	c18, c19, c20,
	c21, c22, c23,
	c24, c25, c26,
	c27, c28, c29,
	c30, c31, c32
};
#define numberofcommands (sizeof(cmd) / sizeof(const char *))












// PLAYSTRINGS (Don't play a playstring!)
#define PSCOS 0
#define PSTAN 1
#define PSACOS 2
#define PSATAN 3
#define PSPV 4
#define PSGAMMA 5
#define PSSINH 6
#define PSCOSH 7
#define PSTANH 8
#define PSASINH 9
#define PSACOSH 10
#define PSATANH 11
#define PSSUM 12
#define RESTORE2 13 // Leave stack[0] and stack[1] unchanged when salvaging from shadow[]
#define PSND 13
#define PSR2P 14
#define PSP2R 15
#define PSSTAT 16
#define PSLR 17

const char s00[] PROGMEM = "90L[";                                 // COS
const char s01[] PROGMEM = "[==E>1LOH";                            // TAN
const char s02[] PROGMEM = "^>90L";                                // ACOS
const char s03[] PROGMEM = "===E1LOQE^";                           // ATAN
const char s04[] PROGMEM = ">I=1LIMIP>1LKH";                       // PV
const char s05[] PROGMEM = "1L===12EI10EQBQL1RHIP2:506628KOHE";    // GAMMA
const char s06[] PROGMEM = "R=Q>L2H";                              // SINH
const char s07[] PROGMEM = "R=QL2H";                               // COSH
const char s08[] PROGMEM = "2ER1B=2LH";                            // TANH
const char s09[] PROGMEM = "==E1LOLS";                             // ASINH
const char s10[] PROGMEM = "==E1BOLS";                             // ACOSH
const char s11[] PROGMEM = "==1LI>1LHOS";                          // ATANH
const char s12[] PROGMEM = "1AM==EMEMIOp";                         // SUM+
const char s13[] PROGMEM = "===EE:07E>I1:6EBR1LQI=E>2HR:3989423E"; // ND
const char s14[] PROGMEM = "=EI==MELO=MH^K";                       // R2P
const char s15[] PROGMEM = "I[==E>1LOMEMEIM";                      // P2R
const char s16[] PROGMEM = "IM=@H=ME>L@1BHOI";                     // STAT
const char s17[] PROGMEM = "qEI@EK@EKrq=EsIMBMBKH===rqtKEB@HI";    // L.R.

const char *const pstable[] PROGMEM = { s00, s01, s02, s03, s04, s05, s06, s07, s08, s09, s10, s11, s12, s13, s14, s15, s16, s17 };
char playbuf[40]; // Holds sii[]




void ResetStack()
{
	for (uint8_t i = 0; i < STACK_SIZE; i++) stack[i] = 0.f;
}

// Save stack to shadow buffer (including mem)
void SaveStackToShadowBuffer()
{ 
	memcpy(shadow, stack, STACK_SIZE * sizeof(float));
}

// Load higher stack from shadow buffer
void LoadStackFromShadowBuffer(uint8_t pos)
{ 
	memcpy(&stack[pos], &shadow[pos], (STACK_SIZE - pos) * sizeof(float));
}

 // PUSH
void StackPush()
{
	// stack[4] -> not changed
	// stack[2] -> stack[3]
	// stack[1] -> stack[2]
	// stack[0] -> stack[1]
	memmove(&stack[1], &stack[0], (STACK_SIZE - 2) * sizeof(float));
}

// PULL
void StackPull()
{
	// stack[1] -> stack[0]
	// stack[2] -> stack[1]
	// stack[3] -> stack[2]
	// stack[4] -> not changed
	memcpy(&stack[0], &stack[1], (STACK_SIZE - 2) * sizeof(float));
}

// PULLUPPER
void StackPullUpper()
{
	// stack[0] -> not changed
	// stack[2] -> stack[1]
	// stack[3] -> stack[2]
	// stack[4] -> not changed
	memcpy(&stack[1], &stack[2], (STACK_SIZE - 3) * sizeof(float));
}

void Store()
{
	stack[STACK_SIZE - 1] = stack[0];
}

void Recall()
{
	StackPush();
	stack[0] = stack[STACK_SIZE - 1];
}

// Checks, if stack[0] is between 0 and 9
static uint8_t IsStackXFrom0to9()
{ 
	return (stack[0] >= 0 && stack[0] <= 9);
}

// PLAYSTRING
void PlayString(uint8_t slot)
{
	restore = slot >= RESTORE2 ? 2 : 1;
	SaveStackToShadowBuffer();
	strcpy_P(playbuf, (char *)pgm_read_word(&(pstable[slot])));
	select = 0;
	isNewNumber = isPlayString = true;
	ispushed = isdot = false;
}

void ReadBattery()
{
	// Set voltage bits for ATTINY85
	ADMUX = _BV(MUX3) | _BV(MUX2);
	_delay_ms(10);

	 // Measuring
	ADCSRA |= _BV(ADSC);
	while (bit_is_set(ADCSRA, ADSC));
	
	// Calculate Vcc in V - 1125.3 = 1.1 * 1023
	StackPush();
	uint8_t high = ADCH;
	stack[0] = 1125.3f / ((high << 8) | ADCL);
}

static void PowerOff()
{
	// simulate automatic Power Off
	frameCounter = POWEROFF_FRAMES;
}

void X_Is_X_Add_Y()
{
	stack[0] += stack[1];
	StackPullUpper();
}

void X_Is_Y_Sub_X()
{
	stack[0] = stack[1] - stack[0];
	StackPullUpper();
}

void X_Is_X_Mul_Y()
{
	stack[0] *= stack[1];
	StackPullUpper();
}

void X_Is_Y_Div_X()
{
	stack[0] = stack[1] / stack[0];
	StackPullUpper();
}

// Math
static float Pow10(int8_t e)
{
	float f = 1.f;
	if (e > 0)
		while (e--) f *= 10.f;
	else
		while (e++) f /= 10.f;
	return f;
}

// Calculate exp, sin or asin
static float _exp_sin_asin(float f, uint8_t nr)
{								 // Calculate exp, sin or asin
	float result = f, frac = f; // Start values for sin or asin
	if (nr == BITEXP)
		result = frac = 1.0; // Start values for exp
	for (int i = 1; _abs(frac) > TINYNUMBER && i < MAXITERATE; i++)
	{
		int i2 = 2 * i, i2p = 2 * i + 1, i2m = 2 * i - 1, i2m2 = i2m * i2m;
		float ffi2i2p = f * f / (i2 * i2p);
		if (nr == BITEXP)
			frac *= f / i; // Fraction for exp
		else if (nr == BITSIN)
			frac *= -ffi2i2p; // Fraction for sin
		else
			frac *= ffi2i2p * i2m2; // Fraction for asin
		result += frac;
	}
	return (result);
}

// Function pointer array subroutines
void _acos();
void _acosh();
void _asin();
void _asinh();
void _atan();
void _atanh();
void _ce();
void _ceclx();
void ChangeSign();
void ClearX();
void _cmdkey();
void _const();
void _contrast();
void _cos();
void _cosh();
void _dot();
void EnterExp();
void _enter();
void _exp();
void _gamma();
void _inv();
void _ln();
void _lr();
void _menu();
void _nd();
void _newnumber();
void _nop();
void _numinput();
void _p2r();
void _recplay();
void _play();
static float _pow();
void _pv();
void _r2p();
void _rec();
void RotateStackUp();
void RotateStackDown();
void _setcmdkey();
void _setconst();
void _shadowload1();
void _shadowload2();
void _sin();
void _sinh();
void _sqrt();
void _stat();
void _sum();
void _sum1();
void _sum2stack();
void SwapStackXY();
void _tan();
void _tanh();

void (*dispatch[])() = 
{
	&_numinput,
	&_nop,
	&ClearX,
	&EnterExp,
	&_enter,
	&ChangeSign,
	&ReadBattery,

	&Recall,
	&Store,
	&X_Is_Y_Sub_X,
	&_const,
	&_cmdkey,
	&X_Is_X_Mul_Y,
	&_menu,
	&_sum,
	&X_Is_Y_Div_X,
	&SwapStackXY,
	&PowerOff,
	&RotateStackUp,
	&X_Is_X_Add_Y,
	&RotateStackDown,
	&_nop,

	// MENU
	&_sqrt,
	&_pow,
	&_inv,
	&_exp,
	&_ln,
	&_gamma,
	&_r2p,
	&_p2r,
	&_pv,
	&_nd,
	&_stat,
	&_lr,
	&_sin,
	&_cos,
	&_tan,
	&_asin,
	&_acos,
	&_atan,
	&_sinh,
	&_cosh,
	&_tanh,
	&_asinh,
	&_acosh,
	&_atanh,
	&_setconst,
	&_setcmdkey,
	&_contrast,
	&_rec,
	&_rec,
	&_rec,
	&_play,
	&_play,
	&_play,

	// Hidden links
	&_sum1,
	&_sum2stack,
	&SaveStackToShadowBuffer,
	&_shadowload1,
	&_shadowload2
};

// Function pointer array subroutines

void _acos()
{ // ACOS
	PlayString(PSACOS);
}
void _acosh()
{ // ACOSH
	PlayString(PSACOSH);
}
void _asin()
{ // ASIN
	stack[0] = _to_deg(_exp_sin_asin(stack[0], BITASIN));
}
void _asinh()
{ // ASINH
	PlayString(PSASINH);
}
void _atan()
{ // ATAN
	PlayString(PSATAN);
}
void _atanh()
{ // ATANH
	PlayString(PSATANH);
}

void ChangeSign()
{ // CHS
	stack[0] = -stack[0];
	isNewNumber = true;
}

void ClearX()
{ // CLX
	stack[0] = 0.f;
}

void _cmdkey()
{ // CMDKEY
	if (IsStackXFrom0to9)
	{
		uint8_t tmp = stack[0];
		StackPull();
		(*dispatch[EEPROM.read(EECMDKEY + tmp)])();
	}
}
void _const()
{ // CONST
	if (IsStackXFrom0to9)
		EEPROM.get(EECONST + (uint8_t)stack[0], stack[0]);
}

void _contrast()
{
	brightness = stack[0];
	DisplayBrightness(brightness);
	EEPROM[EECONTRAST] = brightness;
}

void _cos()
{ // COS
	PlayString(PSCOS);
}
void _cosh()
{ // COSH
	PlayString(PSCOSH);
}

void _dot()
{ // DOT .
	if (!isShift)
	{
		_newnumber();
		isdot = true;
	}
}
void EnterExp()
{ // EE
	stack[0] = Pow10(stack[0]);
	X_Is_X_Mul_Y();
	isNewNumber = true;
}
void _enter()
{ // ENTER
	StackPush();
	ispushed = isNewNumber = true;
}
void _exp()
{ // EXP
	stack[0] = _exp_sin_asin(stack[0], BITEXP);
}
void _gamma()
{ // GAMMA
	PlayString(PSGAMMA);
}
void _inv()
{ // INV
	stack[0] = 1 / stack[0];
}
void _ln()
{ // LN
	stack[0] = log(stack[0]);
}
void _lr()
{ // L.R.
	PlayString(PSLR);
}
void _menu()
{ // MENU
	isMenu = true;
	select = 0;
}

void _nd()
{ // ND
	PlayString(PSND);
}
void _newnumber()
{ // NEW number
	if (isNewNumber)
	{ // New number
		//if (ispushed) ispushed = false;
		//else _push();
		if (!ispushed)
			StackPush();
		stack[0] = 0.0;
		decimals = 0;
		isdot = isNewNumber = false;
	}
}

void _nop() {} // NOP - no operation

void _numinput()
{ // NUM Numeric input (0...9)
	_newnumber();
	if (isdot)
		stack[0] += (key - KEY_B3_0) / Pow10(++decimals); // Append decimal to number
	else
	{ // Append digit to number
		stack[0] *= 10;
		stack[0] += key - KEY_B3_0;
	}
}
void _p2r()
{ // P2R
	PlayString(PSP2R);
}
void _recplay()
{ // Prepare variables for REC or PLAY
	recslot = key - KEY_B0_7;
	recptr = EEREC + recslot * MAXREC;
}
void _play()
{ // PLAY
	_recplay();
	isplay = isNewNumber = true;
}



static float _pow()
{ // POW
	SwapStackXY();
	_ln();
	X_Is_X_Mul_Y();
	_exp();
}





void _pv()
{ // PV
	PlayString(PSPV);
}

void _r2p()
{ // R2P
	PlayString(PSR2P);
}

void _rec()
{ // REC
	_recplay();
	isrec = true;
}
void RotateStackUp()
{ // ROTup
	for (uint8_t i = 0; i < STACK_SIZE - 2; i++)
		RotateStackDown();
	
}
void RotateStackDown()
{ // ROT
	float tmp = stack[0];
	StackPull();
	stack[STACK_SIZE - 2] = tmp;
	isShowStack = true;
}
void _setcmdkey()
{ // SETCMDKEY
	if (IsStackXFrom0to9)
		EEPROM.write(EECMDKEY + (uint8_t)stack[0], (uint8_t)stack[1]);
}
void _setconst()
{ // SETCONST
	if (IsStackXFrom0to9)
		EEPROM.put(EECONST + (uint8_t)stack[0], stack[1]);
}

void _shadowload1()
{ // Load stack from shadow buffer from pos 1
	LoadStackFromShadowBuffer(1);
}
void _shadowload2()
{ // Load stack from shadow buffer from pos 2
	LoadStackFromShadowBuffer(2);
}

void _sin()
{ // SIN
	stack[0] = _exp_sin_asin(_to_rad(stack[0]), BITSIN);
}
void _sinh()
{ // SINH
	PlayString(PSSINH);
}

void _sqrt()
{ 
	//if (stack[0] != 0.0)
	//{
		_ln();
		stack[0] *= 0.5;
		_exp();
	//}
}
void _stat()
{ // STAT
	_sum2stack();
	PlayString(PSSTAT);
}


void _sum()
{ // SUM
	PlayString(PSSUM);
}
void _sum1()
{ // SUM addon
	for (uint8_t i = 0; i < STACK_SIZE; i++)
		sum[i] += stack[i];
	_sum2stack(); // Show n
	Recall();
}
void _sum2stack()
{ // Copies sum[] to stack[] (including mem)
	memmove(stack, sum, STACK_SIZE * sizeof(float));
}
void SwapStackXY()
{ // SWAP
	float tmp = stack[0];
	stack[0] = stack[1];
	stack[1] = tmp;
	isShowStack = true;
}
void _tan()
{ // TAN
	PlayString(PSTAN);
}
void _tanh()
{ // TANH
	PlayString(PSTANH);
}



#define DIGIT_WIDTH (FONT_WIDTH * CHAR_SIZE_M + 1)

#define M_SIGN      (0)
#define M_DIGIT1    (M_SIGN + DIGIT_WIDTH)
#define M_POINT     (M_DIGIT1 + DIGIT_WIDTH - 2)
#define M_DIGIT2    (M_POINT + DIGIT_WIDTH - 4)

#define E_DIGIT2    (128 - (DIGIT_WIDTH - 1))
#define E_DIGIT1    (E_DIGIT2 - DIGIT_WIDTH)
#define E_SIGN      (E_DIGIT1 - DIGIT_WIDTH)

void PrintFloat(float f, uint8_t h, uint8_t y)
{
	if (isnan(f))
	{
		PrintStringAt(F("ERROR"), CHAR_SIZE_M, h, M_DIGIT1, y);
	}
	else
	{
		if (f < 0)
		{
			f = -f;
			PrintCharAt('-', CHAR_SIZE_M, h, M_SIGN, y);
		}

		if (isinf(f))
		{
			PrintStringAt(F("INFINITY"), CHAR_SIZE_M, h, M_DIGIT1, y);
		}
		else
		{
			int8_t e; uint32_t m;
			e = (int8_t)(log(f) / log(10.f));
			m = (uint32_t)((f / Pow10(e - 5)) + 0.5f);
			if (m > 0 && m < 1e5)
			{
				m = (uint32_t)((f / Pow10(--e - 5)) + 0.5f);
			}

			if (e)
			{
				uint8_t s = _min(CHAR_SIZE_M, h);
				if (e < 0)
				{
					e = -e;
					PrintCharAt('-', CHAR_SIZE_M, s, E_SIGN, y);
				}
				PrintCharAt('0' + _tens(e), CHAR_SIZE_M, s, E_DIGIT1, y);
				PrintCharAt('0' + _ones(e), CHAR_SIZE_M, s, E_DIGIT2, y);
			}

			PrintCharAt('.', CHAR_SIZE_M, h, M_POINT, y);
			uint8_t nonzero = false;
			for (int8_t i = 4; i >= 0; --i, m /= 10)
			{
				uint8_t ones = _ones(m);
				if (ones || nonzero)
				{
					PrintCharAt('0' + ones, CHAR_SIZE_M, h, M_DIGIT2 + i * DIGIT_WIDTH, y);
					nonzero = true;
				}
			}
			PrintCharAt('0' + _ones(m), CHAR_SIZE_M, h, M_DIGIT1, y);
		}
	}
}

void PrintScreen()
{
	DisplayFill(0x00);

	uint8_t i;
	uint8_t h = CHAR_SIZE_M;

	printbitshift = 1;
	if (isPlayString || isplay)
	{
		PrintStringAt(F("RUN"), CHAR_SIZE_M, CHAR_SIZE_M, 0, 2);
	}
	else if (isMenu)
	{
		for (uint8_t i = 0; i < FKEYNR; i++)
		{
			void * ptr = pgm_read_word(&cmd[select * FKEYNR + i]);
			PrintStringAt(FPSTR(ptr), CHAR_SIZE_M, CHAR_SIZE_M, 48 * i, 2);
		}
	}
	else
	{
		if (isrec)
			PrintCharAt(CHARREC, CHAR_SIZE_M, CHAR_SIZE_M, 106, 2);
		if (isShift)
			PrintCharAt(CHARSHIFT, CHAR_SIZE_M, CHAR_SIZE_M, 106 + 11, 2);
		h = CHAR_SIZE_L;
	}

	printbitshift = 0;
	if (!isPlayString && !isplay)
	{
		if (isShowStack)
		{
			for (i = 0; i <= STACK_SIZE - 2; ++i)
				PrintFloat(stack[STACK_SIZE - 2 - i], CHAR_SIZE_S, i);
		}
		else
		{
			PrintFloat(stack[0], h, 0);
		}
	}

	DisplayRefresh();
}

////////////////////////////////////////////////////////////////////////////////

void setup()
{
	I2CBusInit();
	DisplayInit();

	// INIT WAKEUP (with pin change interrupt) ... same pin as keyboard!
	pinMode(KPIN, INPUT); // Wakeup pin
	PCMSK |= bit(KPIN);	  // want pin D3 / H2
	GIFR  |= bit(PCIF);	  // Clear any outstanding interrupts
	GIMSK |= bit(PCIE);	  // Enable pin change interrupts

	// INIT SYSTEM
	DisplayTurnOn();
	DisplayBrightness(brightness = EEPROM[EECONTRAST]);
	EnableFrameSync();

	// START
	ResetStack();
}

////////////////////////////////////////////////////////////////////////////////

void loop()
{
	WaitForNextFrame(); 

	if (isfirstrun)
	{
		isfirstrun = false;
		key = KEY_DUMMY;
	}
	else
	{
		key = getkeycode();
		if (key == oldkey) key = NULL; else oldkey = key;
	}

	if (key)
	{
		ResetFrameCounter();
	}

	if (frameCounter >= POWEROFF_FRAMES)
	{
		DeepSleep();
		oldkey = getkeycode();
	}

	DisplayBrightness(frameCounter < DIMOUT_FRAMES ? brightness : 0);

	if (isPlayString)
	{ // ### Play string
		key = playbuf[select];
		if (key == NULL)
		{						  // Stop playstring
			LoadStackFromShadowBuffer(restore); // Restore upper part of stack
			isPlayString = false;
			isNewNumber = true;
			key = KEY_DUMMY;
		}
		else
		{ // Go on for dispatching
			if (key <= KEY_C3_D && ((select == 0) || (select > 0 && playbuf[select - 1] > KEY_C3_D)))
			{ // New number (0-9,.)
				isNewNumber = true;
				ispushed = false;
			}
			select++;
		}
	}

	else if (isrec || isplay)
	{ // ### Type recorder (else: playstring works inside play)
		uint16_t maxptr = EEREC + (recslot + 1) * MAXREC;
		if (isrec)
		{ // Record keys and write to EEPPROM
			if (key && recptr < maxptr)
				EEPROM[recptr++] = key;
		}
		else
		{ // Read/play key from EEPROM
			if (key == KEY_A3_C)
			{ // Stop execution
				isplay = false;
				key = KEY_DUMMY;
			}
			key = EEPROM[recptr++];
		}
		if (key == KEY_A3_C || recptr >= maxptr)
		{
			isplay = isrec = false;
			key = KEY_DUMMY;
		}
	}

	if (key == KEY_A0_F)
	{
		isShift ^= true;
		key = KEY_DUMMY;
	}
	else if (key == KEY_C3_D)
	{
		_dot();
	}

	if (key)
	{
		isShowStack = false;
		if (key != KEY_DUMMY)
		{
			if (isMenu)
			{
				uint8_t limit = numberofcommands / FKEYNR - 1;
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
					uint8_t index = select * FKEYNR + (key - KEY_B2_1);
					(*dispatch[22 + index])();
					isNewNumber = true;
					isMenu = false;
				}
			}

			else if (isShift)
			{
				(*dispatch[6 + key - KEY_B3_0])();
				isShift = ispushed = false;
				isNewNumber = true;
			}

			else
			{
				if (key <= KEY_D0_9)
					(*dispatch[0])(); // Dispatch number 0(0...9)
				else
					(*dispatch[key - KEY_D0_9])(); // Dispatch key (1d: 2c; 3e< 4x= 5s> 6f? + other) due to function table
			}
		}
		PrintScreen(); // Print screen every keypress (or if key == KEY_DUMMY)
	}
}
