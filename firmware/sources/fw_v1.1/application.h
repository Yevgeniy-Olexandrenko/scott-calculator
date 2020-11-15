// APPLICATION

#define CHARSPACE ':'	  // Character for space symbol
#define CHARSHIFT '='	  // Character for shift symbol
#define CHARREC '@'		  // Character for recording symbol
#define POWEROFFTIME 30	  // Time for auto poweroff in s
#define DISPLAYOFFTIME 20 // Time for auto displayoff in s
#define DIMTIME 10		  // Time for auto displaydim in s
#define MAXSTRBUF 12	  // Maximal length of string buffer sbuf[]
#define STACKSIZE 5		  // Size of floatstack
#define TINYNUMBER 1e-7	  // Number for rounding to 0
#define RAD 57.29578	  // 180/PI
#define MAXITERATE 100	  // Maximal number of Taylor series loops to iterate
#define FKEYNR 3		  // 3 function keys
#define KEY_DUMMY 0xff	  // Needed to enter key-loop and printstring
#define EECONTRAST 0	  // EEPROM address of brightness (1 byte)
#define EESTACK 1		  // EEPROM address of stack ((4+1)*4 bytes)
#define EECMDKEY 21		  // EEPROM address of command keys (10 bytes)
#define EECONST 31		  // EEPROM address of constants (10*4 bytes)
#define EEREC 71		  // EEPROM address Starting EE-address for saving "type recorder"
#define MAXREC 146		  // Number of record steps per slot
#define MAXRECSLOT 3	  // Maximal slots to record
#define BITEXP 1		  // Bit for exp()
#define BITSIN 2		  // Bit for sin()
#define BITASIN 4		  // Bit for asin

#define _abs(x) ((x < 0) ? (-x) : (x)) // abs()-substitute macro
#define _ones(x) ((x) % 10)			   // Calculates ones unit
#define _tens(x) (((x) / 10) % 10)	   // Calculates tens unit
#define _huns(x) (((x) / 100) % 10)	   // Calculates hundreds unit
#define _tsds(x) (((x) / 1000) % 10)   // Calculates thousands unit

static byte key;											 // Holds entered key
static byte oldkey;											 // Old key - use for debouncing
static char sbuf[MAXSTRBUF];								 // Holds string to print
static float stack[STACKSIZE];								 // Float stack (XYZT) and memory
static bool isnewnumber = true;							 // True if stack has to be lifted before entering a new number
static bool ispushed = false;							 // True if stack was already pushed by ENTER
static byte decimals = 0;									 // Number of decimals entered - used for input after decimal dot
static bool isdot = false;								 // True if dot was pressed and decimals will be entered
static bool isf = false;									 // true if f is pressed
static bool ismenu = false;								 // True if menu demanded
static byte select = 0;										 // Selection number or playstring position
static bool isplaystring = false;						 // True if string should be played
static byte brightness;										 // Contrast
static bool isfirstrun = true;							 // Allows first run of loop and printscreen without key query
static long timestamp = 0;									 // Needed for timing of power manangement
static int recptr = 0;										 // Pointer to recording step
static byte recslot = 0;									 // Slot number for recording to EEPROM
static bool isrec = false, isplay = false;				 // True, if "Type Recorder" records or plays
static float sum[STACKSIZE] = {0.0, 0.0, 0.0, 0.0, 0.0};	 // Memory to save statistic sums
static float shadow[STACKSIZE] = {0.0, 0.0, 0.0, 0.0, 0.0}; // Shadow memory (buffer) for stack
static byte restore;										 // Position of stack salvation (including mem)

const char c0[] PROGMEM = ",X";	  // Squareroot
const char c1[] PROGMEM = "Y;";	  // Raise to the power of
const char c2[] PROGMEM = "1/X";  // Reciprocal
const char c3[] PROGMEM = "EXP";  // Exponential
const char c4[] PROGMEM = "LN";	  // Natural logarithm
const char c5[] PROGMEM = "?";	  // Gamma function (due to Nemes)
const char c6[] PROGMEM = ">P";	  // Rectangular to polar coordinates
const char c7[] PROGMEM = ">R";	  // Polar to rectangular coordinates
const char c8[] PROGMEM = "PV";	  // Present value (annuity)
const char c9[] PROGMEM = "ND";	  // Normal distribution (CDF/PDF)
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
const char *const cmd[] PROGMEM = {
	c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18, c19, c20,
	c21, c22, c23, c24, c25, c26, c27, c28, c29, c30, c31, c32};
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
const char s0[] PROGMEM = "90L[";								   // COS
const char s1[] PROGMEM = "[==E>1LOH";							   // TAN
const char s2[] PROGMEM = "^>90L";								   // ACOS
const char s3[] PROGMEM = "===E1LOQE^";							   // ATAN
const char s4[] PROGMEM = ">I=1LIMIP>1LKH";						   // PV
const char s5[] PROGMEM = "1L===12EI10EQBQL1RHIP2:506628KOHE";	   // GAMMA
const char s6[] PROGMEM = "R=Q>L2H";							   // SINH
const char s7[] PROGMEM = "R=QL2H";								   // COSH
const char s8[] PROGMEM = "2ER1B=2LH";							   // TANH
const char s9[] PROGMEM = "==E1LOLS";							   // ASINH
const char s10[] PROGMEM = "==E1BOLS";							   // ACOSH
const char s11[] PROGMEM = "==1LI>1LHOS";						   // ATANH
const char s12[] PROGMEM = "1AM==EMEMIOp";						   // SUM+
const char s13[] PROGMEM = "===EE:07E>I1:6EBR1LQI=E>2HR:3989423E"; // ND
const char s14[] PROGMEM = "=EI==MELO=MH^K";					   // R2P
const char s15[] PROGMEM = "I[==E>1LOMEMEIM";					   // P2R
const char s16[] PROGMEM = "IM=@H=ME>L@1BHOI";					   // STAT
const char s17[] PROGMEM = "qEI@EK@EKrq=EsIMBMBKH===rqtKEB@HI";	   // L.R.
const char *const pstable[] PROGMEM = {s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17};
char playbuf[40]; // Holds sii[]

// Function pointer array subroutines
static void _add(void);
static void _acos(void);
static void _acosh(void);
static void _asin(void);
static void _asinh(void);
static void _atan(void);
static void _atanh(void);
static void _batt(void);
static void _ce(void);
static void _ceclx(void);
static void _chs(void);
static void _clx(void);
static void _cmdkey(void);
static void _const(void);
static void _contrast(void);
static void _cos(void);
static void _cosh(void);
static void _div(void);
static void _dot(void);
static void _ee(void);
static void _enter(void);
static void _exp(void);
static void _gamma(void);
static void _inv(void);
static void _ln(void);
static void _lr(void);
static void _menu(void);
static void _mult(void);
static void _nd(void);
static void _newnumber(void);
static void _nop(void);
static void _numinput(void);
static void _p2r(void);
static void _recplay(void);
static void _play(void);
static void _playstring(byte slot);
static float _pow(void);
static float _pow10(int8_t e);
static void _pull(void);
static void _pullupper(void);
static void _push(void);
static void _pv(void);
static void _r2p(void);
static void _rcl(void);
static void _rec(void);
static void _rotup(void);
static void _rot(void);
static void _setcmdkey(void);
static void _setconst(void);
static void _shadowsave(void);
static void _shadowload1(void);
static void _shadowload2(void);
static void _shadowload(byte pos);
static void _sin(void);
static void _sinh(void);
static void _sleep(void);
static void _sqrt(void);
static void _stat(void);
static void _sto(void);
static void _sub(void);
static void _sum(void);
static void _sum1(void);
static void _sum2stack(void);
static void _swap(void);
static void _tan(void);
static void _tanh(void);

// Function pointer array/table
static void (*dispatch[])(void) = {
	&_numinput,							  // Normal calculator keys (dispatch 0)     OFFSET: 0
	&_nop, &_ceclx, &_ee, &_enter, &_chs, // 1d:15 2c;13 3e<5 4x=16 5s>9 (no 6f?1)
	&_batt, &_rcl, &_sto, &_sub,		  // Shiftkeys 0 1 2 3  OFFSET: 6
	&_const, &_cmdkey, &_mult, &_menu,	  //         4 5 6 7
	&_sum, &_div, &_swap, &_sleep,		  //             8 9 d c
	&_rotup, &_add, &_rot, &_nop,		  //              e x s f
	&_sqrt, &_pow, &_inv,				  // MENU                          OFFSET: 21 //22
	&_exp, &_ln, &_gamma,				  // Mathematical functions and settings
	&_r2p, &_p2r, &_pv,
	&_nd, &_stat, &_lr,
	&_sin, &_cos, &_tan, // Trigonometrical functions
	&_asin, &_acos, &_atan,
	&_sinh, &_cosh, &_tanh, // Hyperbolical functions
	&_asinh, &_acosh, &_atanh,
	&_setconst, &_setcmdkey, &_contrast, // Settings
	&_rec, &_rec, &_rec,				 // User definable Menukeys       OFFSET: 48 //49
	&_play, &_play, &_play,
	&_sum1, &_sum2stack, &_shadowsave, &_shadowload1, &_shadowload2 // Hidden links
};

static void savestack(void)
{ // Save whole stack to EEPROM
	byte *p = (byte *)stack;
	for (byte i = 0; i < STACKSIZE * sizeof(float); i++)
		EEPROM[EESTACK + i] = *p++;
}
static void loadstack(void)
{ // Load whole stack from EEMPROM
	byte *p = (byte *)stack;
	for (byte i = 0; i < sizeof(float) * STACKSIZE; i++)
		*p++ = EEPROM[EESTACK + i];
}

static bool is09(void)
{ // Checks, if stack[0] is between 0 and 9
	return (stack[0] >= 0 && stack[0] <= 9);
}

// Function pointer array subroutines
static void _add(void)
{ // ADD +
	stack[0] += stack[1];
	_pullupper();
}
static void _acos(void)
{ // ACOS
	_playstring(PSACOS);
}
static void _acosh(void)
{ // ACOSH
	_playstring(PSACOSH);
}
static void _asin(void)
{ // ASIN
	stack[0] = _exp_sin_asin(stack[0], BITASIN) * RAD;
}
static void _asinh(void)
{ // ASINH
	_playstring(PSASINH);
}
static void _atan(void)
{ // ATAN
	_playstring(PSATAN);
}
static void _atanh(void)
{ // ATANH
	_playstring(PSATANH);
}
static void _batt(void)
{ // BATT
	_push();
	ADMUX = _BV(MUX3) | _BV(MUX2); // Set voltage bits for ATTINY85
	delayshort(10);				   // Settle Vref
	ADCSRA |= _BV(ADSC);
	while (bit_is_set(ADCSRA, ADSC))
		; // Measuring
	byte high = ADCH;
	stack[0] = 1125.3 / ((high << 8) | ADCL); // Calculate Vcc in V - 1125.3 = 1.1 * 1023
}
static void _ce(void)
{ // CE
	if (isdot)
	{
		if (stack[0] > TINYNUMBER && decimals > 0)
		{
			decimals--;
			stack[0] = (long)(stack[0] * _pow10(decimals)) / _pow10(decimals);
		}
		else
			isdot = false;
	}
	else
		stack[0] = (long)(stack[0] / 10);
}
static void _ceclx(void)
{ // CE/CLX
	if (isnewnumber)
		_clx();
	else
		_ce();
}
static void _chs(void)
{ // CHS
	stack[0] = -stack[0];
	isnewnumber = true;
}
static void _clx(void)
{ // CLX
	stack[0] = 0.0;
}
static void _cmdkey(void)
{ // CMDKEY
	if (is09)
	{
		byte tmp = stack[0];
		_pull();
		(*dispatch[EEPROM.read(EECMDKEY + tmp)])();
	}
}
static void _const(void)
{ // CONST
	if (is09)
		EEPROM.get(EECONST + (byte)stack[0], stack[0]);
}
static void _contrast(void)
{ // CONTRAST
	brightness = stack[0];
	dcontrast(brightness);
}
static void _cos(void)
{ // COS
	_playstring(PSCOS);
}
static void _cosh(void)
{ // COSH
	_playstring(PSCOSH);
}
static void _div(void)
{ // DIV /
	stack[0] = stack[1] / stack[0];
	_pullupper();
}
static void _dot(void)
{ // DOT .
	if (!isf)
	{
		_newnumber();
		isdot = true;
	}
}
static void _ee(void)
{ // EE
	stack[0] = _pow10(stack[0]);
	_mult();
	isnewnumber = true;
}
static void _enter(void)
{ // ENTER
	_push();
	ispushed = isnewnumber = true;
}
static void _exp(void)
{ // EXP
	stack[0] = _exp_sin_asin(stack[0], BITEXP);
}
static void _gamma(void)
{ // GAMMA
	_playstring(PSGAMMA);
}
static void _inv(void)
{ // INV
	stack[0] = 1 / stack[0];
}
static void _ln(void)
{ // LN
	stack[0] = log(stack[0]);
}
static void _lr(void)
{ // L.R.
	_playstring(PSLR);
}
static void _menu(void)
{ // MENU
	ismenu = true;
	select = 0;
}
static void _mult(void)
{ // MULT *
	stack[0] *= stack[1];
	_pullupper();
}
static void _nd(void)
{ // ND
	_playstring(PSND);
}
static void _newnumber(void)
{ // NEW number
	if (isnewnumber)
	{ // New number
		//if (ispushed) ispushed = false;
		//else _push();
		if (!ispushed)
			_push();
		stack[0] = 0.0;
		decimals = 0;
		isdot = isnewnumber = false;
	}
}

static void _nop(void) {} // NOP - no operation

static void _numinput(void)
{ // NUM Numeric input (0...9)
	_newnumber();
	if (isdot)
		stack[0] += (key - KEY_14) / _pow10(++decimals); // Append decimal to number
	else
	{ // Append digit to number
		stack[0] *= 10;
		stack[0] += key - KEY_14;
	}
}
static void _p2r(void)
{ // P2R
	_playstring(PSP2R);
}
static void _recplay(void)
{ // Prepare variables for REC or PLAY
	recslot = key - KEY_2;
	recptr = EEREC + recslot * MAXREC;
}
static void _play(void)
{ // PLAY
	_recplay();
	isplay = isnewnumber = true;
}
static void _playstring(byte slot)
{ // PLAYSTRING
	restore = slot >= RESTORE2 ? 2 : 1;
	_shadowsave();												// Save stack for salvation
	strcpy_P(playbuf, (char *)pgm_read_word(&(pstable[slot]))); // Copy playstring to buffer
	select = 0;
	isnewnumber = isplaystring = true;
	ispushed = isdot = false;
}
static float _pow(void)
{ // POW
	_swap();
	_ln();
	_mult();
	_exp();
}
static float _pow10(int8_t e)
{ // POW10 10^x
	float f = 1.0;
	if (e > 0)
		while (e--)
			f *= 10;
	else
		while (e++)
			f /= 10;
	return (f);
}
void _pull(void)
{ // PULL
	memcpy(&stack[0], &stack[1], (STACKSIZE - 2) * sizeof(float));
}
static void _pullupper(void)
{ // PULLUPPER
	memcpy(&stack[1], &stack[2], (STACKSIZE - 3) * sizeof(float));
}
static void _push(void)
{ // PUSH
	memmove(&stack[1], &stack[0], (STACKSIZE - 2) * sizeof(float));
}
static void _pv(void)
{ // PV
	_playstring(PSPV);
}

static void _r2p(void)
{ // R2P
	_playstring(PSR2P);
}
static void _rcl(void)
{ // RCL
	_push();
	stack[0] = stack[STACKSIZE - 1];
}
static void _rec(void)
{ // REC
	_recplay();
	isrec = true;
}
static void _rotup(void)
{ // ROTup
	for (byte i = 0; i < STACKSIZE - 2; i++)
		_rot();
}
static void _rot(void)
{ // ROT
	float tmp = stack[0];
	_pull();
	stack[STACKSIZE - 2] = tmp;
}
static void _setcmdkey(void)
{ // SETCMDKEY
	if (is09)
		EEPROM.write(EECMDKEY + (byte)stack[0], (byte)stack[1]);
}
static void _setconst(void)
{ // SETCONST
	if (is09)
		EEPROM.put(EECONST + (byte)stack[0], stack[1]);
}
static void _shadowsave(void)
{ // Save stack to shadow buffer (including mem)
	memcpy(shadow, stack, STACKSIZE * sizeof(float));
}
static void _shadowload1(void)
{ // Load stack from shadow buffer from pos 1
	_shadowload(1);
}
static void _shadowload2(void)
{ // Load stack from shadow buffer from pos 2
	_shadowload(2);
}
static void _shadowload(byte pos)
{ // Load higher stack from shadow buffer
	memcpy(&stack[pos], &shadow[pos], (STACKSIZE - pos) * sizeof(float));
}
static void _sin(void)
{ // SIN
	stack[0] = _exp_sin_asin(stack[0] / RAD, BITSIN);
}
static void _sinh(void)
{ // SINH
	_playstring(PSSINH);
}
static void _sleep(void)
{									 // SLEEP
	EEPROM[EECONTRAST] = brightness; // Save brightness to EEPROM
	savestack();
	sleep();
}
static void _sqrt(void)
{ // SQRT
	if (stack[0] != 0.0)
	{
		_ln();
		stack[0] *= 0.5;
		_exp();
	}
}
static void _stat(void)
{ // STAT
	_sum2stack();
	_playstring(PSSTAT);
}
static void _sto(void)
{ // STO
	stack[STACKSIZE - 1] = stack[0];
}
static void _sub(void)
{ // SUB -
	stack[0] = stack[1] - stack[0];
	_pullupper();
}
static void _sum(void)
{ // SUM
	_playstring(PSSUM);
}
static void _sum1(void)
{ // SUM addon
	for (byte i = 0; i < STACKSIZE; i++)
		sum[i] += stack[i];
	_sum2stack(); // Show n
	_rcl();
}
static void _sum2stack(void)
{ // Copies sum[] to stack[] (including mem)
	memmove(stack, sum, STACKSIZE * sizeof(float));
}
static void _swap(void)
{ // SWAP
	float tmp = stack[0];
	stack[0] = stack[1];
	stack[1] = tmp;
}
static void _tan(void)
{ // TAN
	_playstring(PSTAN);
}
static void _tanh(void)
{ // TANH
	_playstring(PSTANH);
}

static float _exp_sin_asin(float f, byte nr)
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

static void printfloat(float f, byte mh, byte y)
{						 // Print float with mantissa height (mh) at line y
	long m;				 // Mantissa
	int8_t e;			 // Exponent
	sbuf[0] = CHARSPACE; // * Create sign
	if (f < 0.0)
	{
		f = -f;
		sbuf[0] = '-';
	}
	e = log(f) / log(10);				 // * Calculate exponent (without using log10())
	m = (f / _pow10(e - 5)) + 0.5;		 // * Create mantissa
	if (m > 0 && m < 1e5)				 // Change (n-1)-digit-mantissa to n digits
		m = (f / _pow10(--e - 5)) + 0.5; // "New" mantissa
	for (byte i = 6; i > 0; i--)
	{ // Print mantissa
		sbuf[i] = _ones(m) + '0';
		m /= 10;
	}
	sbuf[7] = e < 0 ? '-' : CHARSPACE; // * Create exponent
	if (e < 0)
		e = -e;
	sbuf[8] = e >= 10 ? _tens(e) + '0' : '0';
	sbuf[9] = _ones(e) + '0';
	printcat(sbuf[0], SIZEM, mh, 0, y); // * Print sbuf in float format
	printcat('.', SIZEM, mh, 23, y);
	printcat(sbuf[1], SIZEM, mh, 12, y);

	byte nonzero = false; // Suppress trailing zeros
	for (byte i = 6; i > 1; i--)
		if (sbuf[i] != '0' || nonzero)
		{
			nonzero = true;
			printcat(sbuf[i], SIZEM, mh, 12 * i + 8, y);
		}
	for (byte i = 7; i < 10; i++)
		printcat(sbuf[i], SIZEM, SIZEM, 12 * i + 10, 0);
}

static void printscreen(void)
{					 // Print screen due to state
	byte mh = SIZEM; // Mantissa height
	cls();
	printbitshift = 1; // Shift second line one pixel down
	if (isplaystring || isplay)
		printsat("RUN", SIZEM, SIZEM, 0, 2); // Print running message
	else if (ismenu)
	{ // Print MENU above F-keys (789)
		for (byte i = 0; i < FKEYNR; i++)
		{
			strcpy_P(sbuf, (char *)pgm_read_word(&(cmd[select * FKEYNR + i])));
			printsat(sbuf, SIZEM, SIZEM, 47 * i, 2);
		}
	}
	else
	{
		mh = SIZEL;
		sbuf[2] = NULL; // Print record and/or shift sign
		sbuf[0] = sbuf[1] = CHARSPACE;
		if (isrec)
			sbuf[0] = CHARREC;
		if (isf)
			sbuf[1] = CHARSHIFT;
		printsat(sbuf, SIZEM, SIZEM, 106, 2);
	}
	printbitshift = 0;
	if (!isplaystring && !isplay)
		printfloat(stack[0], mh, 0); // Print stack[0]
	display();
}