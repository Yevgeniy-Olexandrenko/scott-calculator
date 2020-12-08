////////////////////////////////////////////////////////////////////////////////

#define CHAR_SHIFT '='
#define CHAR_REC   '@'
#define CHAR_PLAY  '<'

#define DIMOUT_FRAMES 156   // Frames before display dim out (about 10 sec)
#define POWEROFF_FRAMES 469 // Frames before power off (about 30 sec)

#define TINYNUMBER 1e-7	    // Number for rounding to 0
#define MAXITERATE 100	    // Maximal number of Taylor series loops to iterate
#define FUN_PER_LINE 3		// 3 function keys
#define KEY_DUMMY 0xff	    // Needed to enter key-loop and printstring

#define TYPEREC_STEPS 70	// Number of record steps per slot
#define TYPEREC_SLOTS 3	    // Number of slots for recording

#define BITEXP  1		    // Bit for exp()
#define BITSIN  2		    // Bit for sin()
#define BITASIN 4		    // Bit for asin

#define _to_rad(x) ((x) * (PI / 180))
#define _to_deg(x) ((x) * (180 / PI))

static uint8_t EEMEM eeprom_brightness = 0xFF;
static uint8_t EEMEM eeprom_comandkey[10];
static float   EEMEM eeprom_constant[10];
static uint8_t EEMEM eeprom_typerecord[TYPEREC_SLOTS][TYPEREC_STEPS];

typedef struct 
{
    union
	{
		struct { float X, Y, Z, T, M; } reg;
		float arr[5];
	};
} Stack;

//#define STACK_SIZE (sizeof(Stack) / sizeof(float))
#define STACK_SIZE 5

static uint8_t  inCalcMode;
static uint8_t  key;                   // Holds entered key
static uint8_t  oldkey;                // Old key - use for debouncing
static Stack    stack;                 // Float stack (XYZT) and memory
static uint8_t  isNewNumber = true;    // True if stack has to be lifted before entering a new number
static uint8_t  ispushed;              // True if stack was already pushed by ENTER
static uint8_t  decimals;              // Number of decimals entered - used for input after decimal dot
static uint8_t  isShift;               // true if f is pressed
static uint8_t  isMenu;                // True if menu demanded
static uint8_t  select;                // Selection number or playstring position
static uint8_t  isPlayString;          // True if string should be played
static uint8_t  brightness;            // Contrast
static uint8_t  isfirstrun = true;     // Allows first run of loop and printscreen without key query
static uint8_t  recSlot;               // Slot number for recording
static uint8_t  recIndex;              // Index of recording step
static uint8_t  isTypeRecording;
static uint8_t  isTypePlaying;                
static float    sum[STACK_SIZE];	   // Memory to save statistic sums
static Stack    shadow;                // Shadow memory (buffer) for stack
static uint8_t  restore;               // Position of stack salvation (including mem)

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
#define numberofcommands 33

const char message_str[] PROGMEM =
	"\03" "ERR" "INF";
#define MSG_ERR 0
#define MSG_INF 1

const char month_str[] PROGMEM = 
	"\03"
	"JAN" "FEB" "MAR" "APR" "MAY" "JUN"
	"JUL" "AUG" "SEP" "OCT" "NOV" "DEC";

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




void ResetCalculator()
{
	brightness = eeprom_read_byte(&eeprom_brightness);
	for (uint8_t i = 0; i < STACK_SIZE; i++) stack.arr[i] = 0.f;
}

// Save stack to shadow buffer (including mem)
void SaveStackToShadowBuffer()
{ 
	memcpy(&shadow, &stack, sizeof(Stack));
}

// Load higher stack from shadow buffer
void LoadStackFromShadowBuffer(uint8_t pos)
{ 
	memcpy(&stack.arr[pos], &shadow.arr[pos], (STACK_SIZE - pos) * sizeof(float));
}

 // PUSH
void StackPush()
{
	// stack[4] -> not changed
	// stack[2] -> stack[3]
	// stack[1] -> stack[2]
	// stack[0] -> stack[1]
	memmove(&stack.reg.Y, &stack.reg.X, (STACK_SIZE - 2) * sizeof(float));
}

// PULL
void StackPull()
{
	// stack[1] -> stack[0]
	// stack[2] -> stack[1]
	// stack[3] -> stack[2]
	// stack[4] -> not changed
	memcpy(&stack.reg.X, &stack.reg.Y, (STACK_SIZE - 2) * sizeof(float));
}

// PULLUPPER
void StackPullUpper()
{
	// stack[0] -> not changed
	// stack[2] -> stack[1]
	// stack[3] -> stack[2]
	// stack[4] -> not changed
	memcpy(&stack.reg.Y, &stack.reg.Z, (STACK_SIZE - 3) * sizeof(float));
}

void Store()
{
	stack.reg.M = stack.reg.X;
}

void Recall()
{
	StackPush();
	stack.reg.X = stack.reg.M;
}

static bool IsStackXInRange(float min, float max)
{ 
	return (stack.reg.X >= min && stack.reg.X <= max);
}

// Checks, if stack[0] is between 0 and 9
static bool IsStackXInRange0to9()
{
	return IsStackXInRange(0, 9);
}

// PLAYSTRING
void PlayString(uint8_t slot)
{
	restore = slot >= RESTORE2 ? 2 : 1;
	SaveStackToShadowBuffer();
	strcpy_P(playbuf, (char *)pgm_read_word(&(pstable[slot])));
	select = 0;
	isNewNumber = isPlayString = true;
	ispushed = false;
	decimals = 0;
}

static void ReadBattery()
{
	StackPush();
	uint16_t adc = ADCRead(_BV(MUX3) | _BV(MUX2), 10);
	stack.reg.X = 1125.3f / adc;
}

static void PowerOff()
{
	// simulate automatic Power Off
	frameCounter = POWEROFF_FRAMES;
}

void AddXY()
{
	stack.reg.X += stack.reg.Y;
	StackPullUpper();
}

void SubYX()
{
	stack.reg.X = stack.reg.Y - stack.reg.X;
	StackPullUpper();
}

void MulXY()
{
	stack.reg.X *= stack.reg.Y;
	StackPullUpper();
}

void DivYX()
{
	stack.reg.X = stack.reg.Y / stack.reg.X;
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
void UseCommandKey();
void GetConstant();
void SetBrightness();
void _cos();
void _cosh();
void _dot();
void EnterExponent();
void PushX();
void _exp();
void _gamma();
void _inv();
void _ln();
void _lr();
void _menu();
void _nd();
void _newnumber();
void _nop();
void EnterDigit();
void _p2r();
void _recplay();
void _play();
static float _pow();
void _pv();
void _r2p();
void _rec();
void RotateStackUp();
void RotateStackDown();
void SetCommandKey();
void SetConstant();
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
	/* [ ] */ &EnterDigit,
	/* [:] */ &_dot,
	/* [;] */ &ClearX,
	/* [<] */ &EnterExponent,
	/* [=] */ &PushX,
	/* [>] */ &ChangeSign,

	// SHIFT + KEY operations:
	/* [?] [0] */ &ReadBattery,
	/* [@] [1] */ &Recall,
	/* [A] [2] */ &Store,
	/* [B] [3] */ &SubYX,
	/* [C] [4] */ &GetConstant,
	/* [D] [5] */ &UseCommandKey,
	/* [E] [6] */ &MulXY,
	/* [F] [7] */ &_menu,
	/* [G] [8] */ &_sum,
	/* [H] [9] */ &DivYX,
	/* [I] [:] */ &SwapStackXY,
	/* [J] [;] */ &PowerOff,
	/* [K] [<] */ &RotateStackUp,
	/* [L] [=] */ &AddXY,
	/* [M] [>] */ &RotateStackDown,
	/* [N] [?] */ &_nop,

	// MENU operations:
	/* [O] */ &_sqrt,
	/* [P] */ &_pow,
	/* [Q] */ &_inv,
	/* [R] */ &_exp,
	/* [S] */ &_ln,
	/* [T] */ &_gamma,
	/* [U] */ &_r2p,
	/* [V] */ &_p2r,
	/* [W] */ &_pv,
	/* [X] */ &_nd,
	/* [Y] */ &_stat,
	/* [Z] */ &_lr,
	/* [[] */ &_sin,
	/* [\] */ &_cos,
	/* []] */ &_tan,
	/* [^] */ &_asin,
	/* [_] */ &_acos,
	/* [`] */ &_atan,
	/* [a] */ &_sinh,
	/* [b] */ &_cosh,
	/* [c] */ &_tanh,
	/* [d] */ &_asinh,
	/* [e] */ &_acosh,
	/* [f] */ &_atanh,
	/* [g] */ &SetConstant,
	/* [h] */ &SetCommandKey,
	/* [i] */ &SetBrightness,
	/* [j] */ &_rec,
	/* [k] */ &_rec,
	/* [l] */ &_rec,
	/* [m] */ &_play,
	/* [n] */ &_play,
	/* [o] */ &_play,

	// Hidden operations:
	/* [p] */ &_sum1,
	/* [q] */ &_sum2stack,
	/* [r] */ &SaveStackToShadowBuffer,
	/* [s] */ &_shadowload1,
	/* [t] */ &_shadowload2
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
	stack.reg.X = _to_deg(_exp_sin_asin(stack.reg.X, BITASIN));
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
	stack.reg.X = -stack.reg.X;
	isNewNumber = true;
}

void ClearX()
{
	stack.reg.X = 0.f;
	isNewNumber = false;
	decimals = 0;
}

static void UseCommandKey()
{
	if (IsStackXInRange0to9())
	{
		uint8_t index = stack.reg.X;
		StackPull();
		(*dispatch[eeprom_read_byte(&eeprom_comandkey[index])])();
	}
}

static void GetConstant()
{
	if (IsStackXInRange0to9())
	{
		stack.reg.X = eeprom_read_float(&eeprom_constant[(uint8_t)stack.reg.X]);
	}
}

void SetBrightness()
{
	if (IsStackXInRange(0, 255))
	{
		brightness = (uint8_t)stack.reg.X;
		eeprom_write_byte(&eeprom_brightness, brightness);
	}
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
{
	if(!decimals)
	{
		_newnumber();
		decimals = 1;
	}
}
void EnterExponent()
{ // EE
	stack.reg.X = Pow10(stack.reg.X);
	MulXY();
	isNewNumber = true;
}
void PushX()
{ // ENTER
	StackPush();
	ispushed = isNewNumber = true;
}
void _exp()
{ // EXP
	stack.reg.X = _exp_sin_asin(stack.reg.X, BITEXP);
}
void _gamma()
{ // GAMMA
	PlayString(PSGAMMA);
}
void _inv()
{ // INV
	stack.reg.X = 1 / stack.reg.X;
}
void _ln()
{ // LN
	stack.reg.X = log(stack.reg.X);
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
{
	if (isNewNumber)
	{
		if (!ispushed) StackPush();
		ClearX();
	}
}

void _nop() {} // NOP - no operation

void EnterDigit()
{ // NUM Numeric input (0...9)
	_newnumber();
	if (decimals)
		stack.reg.X += (key - KEY_B3_0) / Pow10(decimals++); // Append decimal to number
	else
	{ // Append digit to number
		stack.reg.X *= 10;
		stack.reg.X += key - KEY_B3_0;
	}
}
void _p2r()
{ // P2R
	PlayString(PSP2R);
}

void _recplay()
{ // Prepare variables for REC or PLAY
	recSlot = key - KEY_B2_1;
	recIndex = 0;
}

void _play()
{ // PLAY
	_recplay();
	isTypePlaying = isNewNumber = true;
}



static float _pow()
{ // POW
	SwapStackXY();
	_ln();
	MulXY();
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
	isTypeRecording = true;
}

static void RotateStackUp()
{
	for (uint8_t i = 0; i < STACK_SIZE - 2; i++)
	{
		RotateStackDown();
	}
}

static void RotateStackDown()
{
	float tmp = stack.reg.X;
	StackPull();
	stack.reg.T = tmp;
}

static void SetCommandKey()
{
	if (IsStackXInRange0to9())
	{
		eeprom_write_byte(&eeprom_comandkey[(uint8_t)stack.reg.X], (uint8_t)stack.reg.Y);
	}
}

static void SetConstant()
{
	if (IsStackXInRange0to9())
	{
		eeprom_write_float(&eeprom_constant[(uint8_t)stack.reg.X], stack.reg.Y);
	}
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
	stack.reg.X = _exp_sin_asin(_to_rad(stack.reg.X), BITSIN);
}
void _sinh()
{ // SINH
	PlayString(PSSINH);
}

void _sqrt()
{ 
	//if (stack.reg.X != 0.0)
	//{
		_ln();
		stack.reg.X *= 0.5;
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
	for (uint8_t i = 0; i < STACK_SIZE; i++) sum[i] += stack.arr[i];
	_sum2stack(); // Show n
	Recall();
}
void _sum2stack()
{ // Copies sum[] to stack[] (including mem)
	memmove(stack.arr, sum, sizeof(Stack));
}
void SwapStackXY()
{ // SWAP
	float tmp = stack.reg.X;
	stack.reg.X = stack.reg.Y;
	stack.reg.Y = tmp;
}
void _tan()
{ // TAN
	PlayString(PSTAN);
}
void _tanh()
{ // TANH
	PlayString(PSTANH);
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

static void PrintStack(uint8_t i, uint8_t s, uint8_t y)
{
	float f = stack.arr[i];
	PrintCharSize(CHAR_SIZE_M, s);

	if (isnan(f))
	{
		PrintStringAt(FPSTR(message_str), MSG_ERR, M_DIGIT_FST, y);
	}
	else
	{
		if (f < 0)
		{
			f = -f;
			PrintCharAt('-', M_SIGN, y);
		}

		if (isinf(f))
		{
			PrintStringAt(FPSTR(message_str), MSG_INF, M_DIGIT_FST, y);
		}
		else
		{
			int8_t e = (int8_t)(log(f) / log(10.f));
			uint32_t m = (uint32_t)(f / Pow10(e - (DIGITS - 1)) + 0.5f);

			if (m > 0 && m < Pow10(DIGITS - 1))
			{
				m = (uint32_t)(f / Pow10(--e - (DIGITS - 1)) + 0.5f);
			}

			int8_t int_dig = 0, fra_dig, lead_z = 0;
			if (abs(e) >= DIGITS)
			{
				int_dig = 1;
			}
			else if (e >= 0)
			{
				int_dig = 1 + e; e = 0;
			}
			else // e < 0
			{
				int_dig = 0; lead_z = -e; e = 0;
				for (uint8_t n = lead_z; n--; m /= 10);
			}

			i = M_DIGIT_LST; uint8_t nonzero = false;
			for (fra_dig = DIGITS - lead_z - int_dig; fra_dig--; m /= 10, i -= DIGIT_WIDTH)
			{
				uint8_t ones = _ones(m);
				if (ones || nonzero)
				{
					PrintCharAt('0' + ones, i, y);
					nonzero = true;
				}
			}

			if (nonzero)
			{
				for (; --lead_z > 0; i -= DIGIT_WIDTH) PrintCharAt('0', i, y);
				PrintCharAt('.', i, y);
			}
			
			PrintCharAt('0', i -= POINT_WIDTH, y);
			for (; int_dig--; m /= 10, i -= DIGIT_WIDTH) PrintCharAt('0' + _ones(m), i, y);

			if (e)
			{
				PrintCharSize(CHAR_SIZE_M, s >> 1);
				if (e < 0)
				{
					e = -e;
					PrintCharAt('-', E_SIGN, y);
				}
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

	if (isMenu)
	{
		PrintStack(0, CHAR_SIZE_M, 0);
		for (uint8_t i = 0; i < FUN_PER_LINE; ++i)
		{
			PrintStringAt(FPSTR(menu_str), select * FUN_PER_LINE + i, 48 * i, 2);
		}
	}
	else if (isShift)
	{
		PrintCharAt(CHAR_SHIFT, MODE_CHAR, 0);
		PrintStack(1, CHAR_SIZE_M, 0);
		PrintStack(0, CHAR_SIZE_M, 2);
	}
	else
	{
		PrintStack(0, CHAR_SIZE_L, 0);
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

			else if (isTypeRecording || isTypePlaying)
			{ // ### Type recorder (else: playstring works inside play)
				if (isTypeRecording)
				{ // Record keys and write to EEPPROM
					if (key && recIndex < TYPEREC_STEPS)
					{
						eeprom_write_byte(&eeprom_typerecord[recSlot][recIndex++], key);
					}
				}
				else
				{
					if (key == KEY_A3_C)
					{ // Stop execution
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
						uint8_t limit = numberofcommands / FUN_PER_LINE - 1;
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
							uint8_t index = select * FUN_PER_LINE + (key - KEY_B2_1);
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
						if (key > KEY_D0_9) 
							(*dispatch[key - KEY_D0_9])();
						else
							(*dispatch[0])();
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
