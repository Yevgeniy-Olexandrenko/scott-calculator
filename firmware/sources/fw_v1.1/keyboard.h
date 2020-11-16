// KEYBOARD
#define KPIN   3  // Pin to read analog keyboard  (H2 = D3)
#define BITEXP 0x01

// Keychars
#define KEY_1  '?' // SHIFT    1-?  2-7  3-8  4-9
#define KEY_2  '7' //          5-<  6-4  7-5  8-6
#define KEY_3  '8' //          9-> 10-1 11-2 12-3
#define KEY_4  '9' //         13-; 14-0 15-: 16-=
#define KEY_5  '<' // EE
#define KEY_6  '4'
#define KEY_7  '5'
#define KEY_8  '6'
#define KEY_9  '>' // CHS
#define KEY_10 '1'
#define KEY_11 '2'
#define KEY_12 '3'
#define KEY_13 ';' // CLX
#define KEY_14 '0'
#define KEY_15 ':' // DOT
#define KEY_16 '=' // ENTER

// Calculate exp, sin or asin
static float _exp_sin_asin(float f, uint8_t nr);

// Returns analog value measured on keyboard pin
static int getbutton()
{
	return (analogRead(KPIN));
}

// Returns key character due to analog value
static uint8_t getkeycode()
{
	int b = getbutton();
#if 0  
	const uint8_t keys[] = { KEY_16, KEY_15, KEY_14, KEY_13, KEY_12, KEY_11, KEY_10, KEY_9, KEY_8, KEY_7, KEY_6, KEY_5, KEY_4, KEY_3, KEY_2, KEY_1 };
	if (b < 231)
		return (NULL); // No key pressed
	else
		return (keys[(uint8_t)(_exp_sin_asin(0.5 * log(b / 2 - 235), BITEXP) - 1)]); // Approximate keys with quadratic function
#else
  if (b < 231) return (NULL); // No key pressed
  else if (b < 470) return (KEY_16);
  else if (b < 488) return (KEY_15);
  else if (b < 508) return (KEY_14);
  else if (b < 529) return (KEY_13);
  else if (b < 552) return (KEY_12);
  else if (b < 578) return (KEY_11);
  else if (b < 606) return (KEY_10);
  else if (b < 637) return (KEY_9);
  else if (b < 661) return (KEY_8);
  else if (b < 708) return (KEY_7);
  else if (b < 751) return (KEY_6);
  else if (b < 799) return (KEY_5);
  else if (b < 853) return (KEY_4);
  else if (b < 915) return (KEY_3);
  else if (b < 986) return (KEY_2);
  else return (KEY_1);
#endif   
}
