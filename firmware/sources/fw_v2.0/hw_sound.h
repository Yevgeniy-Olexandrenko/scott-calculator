////////////////////////////////////////////////////////////////////////////////

#define SOUND_PIN PORTB4
#define SOUND_CLK (((F_CPU / 1000000UL) == 16) ? 4 : ((F_CPU / 1000000UL) == 8) ? 3 : 0)

static const uint8_t scale[] PROGMEM =
{
	239, 226, 213, 201, 190, 179, 169, 160, 151, 142, 134, 127
};

static void SoundPlay(int8_t note, int8_t octave)
{
	int8_t prescaler = 8 + SOUND_CLK - (octave + note / 12);
	if (prescaler < 1 || prescaler > 15 || octave == 0) prescaler = 0;

	DDRB  = (DDRB & ~(1 << SOUND_PIN)) | (prescaler != 0) << SOUND_PIN;
	OCR1C = pgm_read_byte(&scale[note % 12]) - 1;
	GTCCR = (SOUND_PIN == 4) << COM1B0;
	TCCR1 = 1 << CTC1 | (SOUND_PIN == 1) << COM1A0 | prescaler << CS10;
}

static void SoundStop()
{
	SoundPlay(0, 0);
}
