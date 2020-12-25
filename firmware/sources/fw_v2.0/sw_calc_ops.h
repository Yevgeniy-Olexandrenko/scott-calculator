#define DIMOUT_FRAMES 156   // Frames before display dim out (about 10 sec)
#define POWEROFF_FRAMES 469 // Frames before power off (about 30 sec)

#define TYPEREC_STEPS 70	// Number of record steps per slot
#define TYPEREC_SLOTS 3	    // Number of slots for recording

static uint8_t  inCalcMode;
static uint8_t  key;                   // Holds entered key
static uint8_t  oldkey;                // Old key - use for debouncing
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
static uint8_t  restore;               // Position of stack salvation (including mem)
char playbuf[40];

static uint8_t EEMEM eeprom_brightness = 0xFF;
static uint8_t EEMEM eeprom_comandkey[10];
static float   EEMEM eeprom_constant[10];
static uint8_t EEMEM eeprom_typerecord[TYPEREC_SLOTS][TYPEREC_STEPS];

#define OP_DOT  ":"
#define OP_CLRX ";"
#define OP_EEXP "<"
#define OP_PUSH "="
#define OP_CHS  ">"

#define OP_BAT  "?"
#define OP_RCL  "@"
#define OP_STO  "A"
#define OP_SUB  "B"
#define OP_CST  "C"
#define OP_CMD  "D"
#define OP_MUL  "E"
#define OP_MENU "F"
#define OP_SUM  "G"
#define OP_DIV  "H"
#define OP_SWAP "I"
#define OP_POFF "J"
#define OP_ROTU "K"
#define OP_ADD  "L"
#define OP_ROTD "M"
#define OP_NOP  "N"

#define OP_SQRT "O"
#define OP_POW  "P"
#define OP_INV  "Q"
#define OP_EXP  "R"
#define OP_LN   "S"
#define OP_GAMM "T"
#define OP_R2P  "U"
#define OP_P2R  "V"
#define OP_PV   "W"
#define OP_ND   "X"
#define OP_STAT "Y"
#define OP_LR   "Z"
#define OP_SIN  "["
#define OP_COS  "\\"
#define OP_TAN  "]"
#define OP_ASIN "^"
#define OP_ACOS "_"
#define OP_ATAN "`"
#define OP_SINH "a"
#define OP_COSH "b"
#define OP_TANH "c"
#define OP_ASNH "d"
#define OP_ACSH "e"
#define OP_ATNH "f"
//

const char PROG_COS[] PROGMEM = 
	"90"
	OP_ADD
	OP_SIN;

const char PROG_TAN[] PROGMEM = 
	OP_SIN
	OP_PUSH
	OP_PUSH
	OP_MUL
	OP_CHS
	"1"
	OP_ADD
	OP_SQRT
	OP_DIV;

const char PROG_ACOS[] PROGMEM = 
	OP_ASIN
	OP_CHS
	"90"
	OP_ADD;

const char PROG_ATAN[] PROGMEM = 
	OP_PUSH
	OP_PUSH
	OP_PUSH
	OP_MUL
	"1"
	OP_ADD
	OP_SQRT
	OP_INV
	OP_MUL
	OP_ASIN;

const char PROG_PV[] PROGMEM = 
	OP_CHS
	OP_SWAP
	OP_PUSH
	"1"
	OP_ADD
	OP_SWAP
	OP_ROTD
	OP_SWAP
	OP_POW
	OP_CHS
	"1"
	OP_ADD
	OP_ROTU
	OP_DIV;

const char PROG_GAMMA[] PROGMEM = 
	"1"
	OP_ADD
	OP_PUSH
	OP_PUSH
	OP_PUSH
	"12"
	OP_MUL
	OP_SWAP
	"10"
	OP_MUL
	OP_INV
	OP_SUB
	OP_INV
	OP_ADD
	"1"
	OP_EXP
	OP_DIV
	OP_SWAP
	OP_POW
	"2" OP_DOT "506628"
	OP_ROTU
	OP_SQRT
	OP_DIV
	OP_MUL;

const char PROG_SINH[] PROGMEM = 
	OP_EXP
	OP_PUSH
	OP_INV
	OP_CHS
	OP_ADD
	"2"
	OP_DIV;

const char PROG_COSH[] PROGMEM = 
	OP_EXP
	OP_PUSH
	OP_INV
	OP_ADD
	"2"
	OP_DIV; 

const char PROG_TANH[] PROGMEM = 
	"2"
	OP_MUL
	OP_EXP
	"1"
	OP_SUB
	OP_PUSH
	"2"
	OP_ADD
	OP_DIV;

const char PROG_ASINH[] PROGMEM = 
	OP_PUSH
	OP_PUSH
	OP_MUL
	"1"
	OP_ADD
	OP_SQRT
	OP_ADD
	OP_LN;

const char PROG_ACOSH[] PROGMEM = 
	OP_PUSH
	OP_PUSH
	OP_MUL
	"1"
	OP_SUB
	OP_SQRT
	OP_ADD
	OP_LN;

const char PROG_ATANH[] PROGMEM = 
	OP_PUSH
	OP_PUSH
	"1"
	OP_ADD
	OP_SWAP
	OP_CHS
	"1"
	OP_ADD
	OP_DIV
	OP_SQRT
	OP_LN;

const char s12[] PROGMEM = "1AM==EMEMIOp";                         // SUM+
const char s13[] PROGMEM = "===EE:07E>I1:6EBR1LQI=E>2HR:3989423E"; // ND
const char s14[] PROGMEM = "=EI==MELO=MH^K";                       // R2P
const char s15[] PROGMEM = "I[==E>1LOMEMEIM";                      // P2R
const char s16[] PROGMEM = "IM=@H=ME>L@1BHOI";                     // STAT
const char s17[] PROGMEM = "qEI@EK@EKrq=EsIMBMBKH===rqtKEB@HI";    // L.R.

const char *const pstable[] PROGMEM = 
{
	PROG_COS, PROG_TAN, PROG_ACOS, PROG_ATAN, PROG_PV, PROG_GAMMA,
	PROG_SINH, PROG_COSH, PROG_TANH, PROG_ASINH, PROG_ACOSH, PROG_ATANH,
	s12, s13, s14, s15, s16, s17 
};

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

////////////////////////////////////////////////////////////////////////////////

static void Dispatch(uint8_t operation);

static bool GetIfStackRegInRange(uint8_t i, uint8_t min, uint8_t max, uint8_t & dest)
{
	if (stack.arr[i] >= min && stack.arr[i] <= max)
	{
		dest = (uint8_t)stack.arr[i];
		return true;
	}
	stack.arr[i] = NAN;
	return false;
}

static void SetBrightness()
{
	if (GetIfStackRegInRange(Stack::X, 0, 255, brightness))
	{
		eeprom_write_byte(&eeprom_brightness, brightness);
	}
}

static void SetTime()
{
	if (RTCRead())
	{
		if (GetIfStackRegInRange(Stack::Z, 0, 23, rtc_hours) &&
			GetIfStackRegInRange(Stack::Y, 0, 59, rtc_minutes) &&
			GetIfStackRegInRange(Stack::X, 0, 59, rtc_seconds))
		{
			RTCWrite();
		}
	}
}

static void SetDate()
{
	if (RTCRead())
	{
		if (GetIfStackRegInRange(Stack::Z, 1, 31, rtc_date) &&
			GetIfStackRegInRange(Stack::Y, 1, 12, rtc_month) &&
			GetIfStackRegInRange(Stack::X, 0, 99, rtc_year))
		{
			RTCWrite();
		}
	}
}

static void SetCommandKey()
{
	uint8_t index;
	if (GetIfStackRegInRange(Stack::X, 0, 9, index))
	{
		eeprom_write_byte(&eeprom_comandkey[index], (uint8_t)stack.reg.Y);
	}
}

static void SetConstant()
{
	uint8_t index;
	if (GetIfStackRegInRange(Stack::X, 0, 9, index))
	{
		eeprom_write_float(&eeprom_constant[index], stack.reg.Y);
	}
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

// PULLUPPER
void StackPullUpper()
{
	// stack[0] -> not changed
	// stack[2] -> stack[1]
	// stack[3] -> stack[2]
	// stack[4] -> not changed
	memcpy(&stack.reg.Y, &stack.reg.Z, (STACK_SIZE - 3) * sizeof(float));
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

void OpCLRX();
void CheckForNewInput()
{
	if (isNewNumber)
	{
		if (!ispushed) StackPush();
		OpCLRX();
	}
}

void _sum2stack()
{ // Copies sum[] to stack[] (including mem)
	memmove(stack.arr, sum, sizeof(Stack));
}

void _recplay()
{
	recSlot = key - KEY_B2_1;
	recIndex = 0;
}

void _play()
{
	_recplay();
	isTypePlaying = isNewNumber = true;
}

void _rec()
{
	_recplay();
	isTypeRecording = true;
}

void _shadowload1()
{ // Load stack from shadow buffer from pos 1
	LoadStackFromShadowBuffer(1);
}
void _shadowload2()
{ // Load stack from shadow buffer from pos 2
	LoadStackFromShadowBuffer(2);
}

void OpRCL();
void _sum1()
{
	for (uint8_t i = 0; i < STACK_SIZE; i++) sum[i] += stack.arr[i];
	_sum2stack(); // Show n
	OpRCL();
}

////////////////////////////////////////////////////////////////////////////////

static void OpCLRX()
{
	stack.reg.X = 0.f;
	isNewNumber = false;
	decimals = 0;
}

static void OpCHS()
{
	stack.reg.X = -stack.reg.X;
	isNewNumber = true;
}

static void OpDOT()
{
	if (!decimals)
	{
		CheckForNewInput();
		decimals = 1;
	}
}

static void OpMUL();
static void OpEEXP()
{
	stack.reg.X = MathPow10(stack.reg.X);
	OpMUL();
	isNewNumber = true;
}

static void EnterDigit(float digit)
{
	CheckForNewInput();
	if (decimals)
		digit /= MathPow10(decimals++);
	else
		stack.reg.X *= 10;
	stack.reg.X += digit;
}

////////////////////////////////////////////////////////////////////////////////

static void OpPUSH()
{
	StackPush();
	ispushed = true;
	isNewNumber = true;
}

static void OpSWAP()
{
	float t_reg = stack.reg.X;
	stack.reg.X = stack.reg.Y;
	stack.reg.Y = t_reg;
}

static void OpROTD()
{
	float t_reg = stack.reg.X;
	StackPull();
	stack.reg.T = t_reg;
}

static void OpROTU()
{
	for (uint8_t i = 0; i < STACK_SIZE - 2; i++)
	{
		OpROTD();
	}
}

static void OpSTO()
{
	stack.reg.M = stack.reg.X;
}

static void OpRCL()
{
	StackPush();
	stack.reg.X = stack.reg.M;
}

////////////////////////////////////////////////////////////////////////////////

static void OpADD()
{
	stack.reg.X += stack.reg.Y;
	StackPullUpper();
}

static void OpSUB()
{
	stack.reg.X = stack.reg.Y - stack.reg.X;
	StackPullUpper();
}

static void OpMUL()
{
	stack.reg.X *= stack.reg.Y;
	StackPullUpper();
}

static void OpDIV()
{
	stack.reg.X = stack.reg.Y / stack.reg.X;
	StackPullUpper();
}

////////////////////////////////////////////////////////////////////////////////

static void OpLN()  { stack.reg.X = log(stack.reg.X); }
static void OpEXP() { stack.reg.X = MathExpSinAsin(stack.reg.X, BITEXP); }
static void OpINV() { stack.reg.X = 1.f / stack.reg.X; }

static void OpPOW()
{
	OpSWAP();
	OpLN();
	OpMUL();
	OpEXP();
}

static void OpSQRT()
{ 
	if (stack.reg.X != 0)
	{
		OpLN();
		stack.reg.X *= 0.5f;
		OpEXP();
	}
}

static void OpSTAT() 
{
	_sum2stack();
	PlayString(PSSTAT);
}

static void OpGAMM() { PlayString(PSGAMMA); }
static void OpR2P()  { PlayString(PSR2P); }
static void OpP2R()  { PlayString(PSP2R); }
static void OpPV()   { PlayString(PSPV); }
static void OpND()   { PlayString(PSND); }
static void OpLR()   { PlayString(PSLR); }

////////////////////////////////////////////////////////////////////////////////

static void OpSIN() { stack.reg.X = MathExpSinAsin(_to_rad(stack.reg.X), BITSIN); }
static void OpCOS() { PlayString(PSCOS); }
static void OpTAN() { PlayString(PSTAN); }

static void OpASIN() { stack.reg.X = _to_deg(MathExpSinAsin(stack.reg.X, BITASIN)); }
static void OpACOS() { PlayString(PSACOS); }
static void OpATAN() { PlayString(PSATAN); }

static void OpSINH() { PlayString(PSSINH); }
static void OpCOSH() { PlayString(PSCOSH); }
static void OpTANH() { PlayString(PSTANH); }

static void OpASNH() { PlayString(PSASINH); }
static void OpACSH() { PlayString(PSACOSH); }
static void OpATNH() { PlayString(PSATANH); }

////////////////////////////////////////////////////////////////////////////////

static void OpPOFF()
{
	// simulate automatic Power Off
	frameCounter = POWEROFF_FRAMES;
}

static void OpMENU()
{
	isMenu = true;
	select = 0;
}

static void OpBAT()
{
	StackPush();
	uint16_t adc = ADCRead(_BV(MUX3) | _BV(MUX2), 10);
	stack.reg.X = 1125.3f / adc;
}

static void OpCST()
{
	uint8_t index;
	if (GetIfStackRegInRange(Stack::X, 0, 9, index))
	{
		stack.reg.X = eeprom_read_float(&eeprom_constant[index]);
	}
}

static void OpCMD()
{
	uint8_t index;
	if (GetIfStackRegInRange(Stack::X, 0, 9, index))
	{
		StackPull();
		Dispatch(eeprom_read_byte(&eeprom_comandkey[index]));
	}
}

static void OpSUM() { PlayString(PSSUM); }

////////////////////////////////////////////////////////////////////////////////

static void OpNOP() {}

static void Dispatch(uint8_t operation)
{
	static void (*dispatch[])() = 
	{
		/* [:] */ &OpDOT,
		/* [;] */ &OpCLRX,
		/* [<] */ &OpEEXP,
		/* [=] */ &OpPUSH,
		/* [>] */ &OpCHS,

		// SHIFT + KEY operations:
		/* [?] [0] */ &OpBAT,
		/* [@] [1] */ &OpRCL,
		/* [A] [2] */ &OpSTO,
		/* [B] [3] */ &OpSUB,
		/* [C] [4] */ &OpCST,
		/* [D] [5] */ &OpCMD,
		/* [E] [6] */ &OpMUL,
		/* [F] [7] */ &OpMENU,
		/* [G] [8] */ &OpSUM,
		/* [H] [9] */ &OpDIV,
		/* [I] [:] */ &OpSWAP,
		/* [J] [;] */ &OpPOFF,
		/* [K] [<] */ &OpROTU,
		/* [L] [=] */ &OpADD,
		/* [M] [>] */ &OpROTD,
		/* [N] [?] */ &OpNOP,

		// MENU operations:
		/* [O] */ &OpSQRT,
		/* [P] */ &OpPOW,
		/* [Q] */ &OpINV,
		/* [R] */ &OpEXP,
		/* [S] */ &OpLN,
		/* [T] */ &OpGAMM,
		/* [U] */ &OpR2P,
		/* [V] */ &OpP2R,
		/* [W] */ &OpPV,
		/* [X] */ &OpND,
		/* [Y] */ &OpSTAT,
		/* [Z] */ &OpLR,
		/* [[] */ &OpSIN,
		/* [\] */ &OpCOS,
		/* []] */ &OpTAN,
		/* [^] */ &OpASIN,
		/* [_] */ &OpACOS,
		/* [`] */ &OpATAN,
		/* [a] */ &OpSINH,
		/* [b] */ &OpCOSH,
		/* [c] */ &OpTANH,
		/* [d] */ &OpASNH,
		/* [e] */ &OpACSH,
		/* [f] */ &OpATNH,
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

	(*dispatch[operation])();
}