// KEYBOARD
#define KPIN   3  // Pin to read analog keyboard  (H2 = D3)
#define BITEXP 0x01

// Keychars
#define KEY_A0_F '?' // SHIFT    1-?  2-7  3-8  4-9
#define KEY_B0_7 '7' //          5-<  6-4  7-5  8-6
#define KEY_C0_8 '8' //          9-> 10-1 11-2 12-3
#define KEY_D0_9 '9' //         13-; 14-0 15-: 16-=
#define KEY_A1_E '<' // EE
#define KEY_B1_4 '4'
#define KEY_C1_5 '5'
#define KEY_D1_6 '6'
#define KEY_A2_S '>' // CHS
#define KEY_B2_1 '1'
#define KEY_C2_2 '2'
#define KEY_D2_3 '3'
#define KEY_A3_C ';' // CLX
#define KEY_B3_0 '0'
#define KEY_C3_D ':' // DOT
#define KEY_D3_E '=' // ENTER

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
	const uint8_t keys[] = { KEY_D3_E, KEY_C3_D, KEY_B3_0, KEY_A3_C, KEY_D2_3, KEY_C2_2, KEY_B2_1, KEY_A2_S, KEY_D1_6, KEY_C1_5, KEY_B1_4, KEY_A1_E, KEY_D0_9, KEY_C0_8, KEY_B0_7, KEY_A0_F };
	if (b < 231)
		return (NULL); // No key pressed
	else
		return (keys[(uint8_t)(_exp_sin_asin(0.5 * log(b / 2 - 235), BITEXP) - 1)]); // Approximate keys with quadratic function
#else
  if (b < 231) return (NULL); // No key pressed
  else if (b < 470) return (KEY_D3_E);
  else if (b < 488) return (KEY_C3_D);
  else if (b < 508) return (KEY_B3_0);
  else if (b < 529) return (KEY_A3_C);
  else if (b < 552) return (KEY_D2_3);
  else if (b < 578) return (KEY_C2_2);
  else if (b < 606) return (KEY_B2_1);
  else if (b < 637) return (KEY_A2_S);
  else if (b < 661) return (KEY_D1_6);
  else if (b < 708) return (KEY_C1_5);
  else if (b < 751) return (KEY_B1_4);
  else if (b < 799) return (KEY_A1_E);
  else if (b < 853) return (KEY_D0_9);
  else if (b < 915) return (KEY_C0_8);
  else if (b < 986) return (KEY_B0_7);
  else return (KEY_A0_F);
#endif   
}
