#define KPIN 3

// Keyboard layout on PCB:
// A0[?] B0[7] C0[8] D0[9]
// A1[<] B1[4] C1[5] D1[6]
// A2[>] B2[1] C2[2] D2[3]
// A3[;] B3[0] C3[:] D3[=]

// Keyboard ordered by key ASCII code:
#define KEY_B3_0 '0'
#define KEY_B2_1 '1'
#define KEY_C2_2 '2'
#define KEY_D2_3 '3'
#define KEY_B1_4 '4'
#define KEY_C1_5 '5'
#define KEY_D1_6 '6'
#define KEY_B0_7 '7' 
#define KEY_C0_8 '8' 
#define KEY_D0_9 '9'
#define KEY_C3_D ':' // DOT
#define KEY_A3_C ';' // CLEAR
#define KEY_A1_E '<' // EXPONENT
#define KEY_D3_P '=' // PUSH
#define KEY_A2_S '>' // SIGN
#define KEY_A0_F '?' // SHIFT   

// Keyboard ordered by voltage level:
const uint8_t key_code[] PROGMEM = 
{
	KEY_D3_P, KEY_C3_D, KEY_B3_0, KEY_A3_C,
	KEY_D2_3, KEY_C2_2, KEY_B2_1, KEY_A2_S,
	KEY_D1_6, KEY_C1_5, KEY_B1_4, KEY_A1_E,
	KEY_D0_9, KEY_C0_8, KEY_B0_7, KEY_A0_F 
};
const uint16_t key_adc[] PROGMEM =
{
	465, 483, 502, 524,
	547, 573, 601, 630,
	665, 704, 744, 792,
	846, 909, 978, 1024
};

static uint8_t getkeycode()
{
	uint16_t adcVal = analogRead(KPIN);
	if (adcVal > 450)
	{
		for (uint8_t i = 0; i < 16; ++i)
		{
			uint16_t adcMax = pgm_read_word(&key_adc[i]);
			if (adcVal < adcMax) return pgm_read_byte(&key_code[i]);
		}
	}
	return NULL;
}
