
#define TINYNUMBER 1e-7	    // Number for rounding to 0
#define MAXITERATE 100	    // Maximal number of Taylor series loops to iterate

#define BITEXP  1		    // Bit for exp()
#define BITSIN  2		    // Bit for sin()
#define BITASIN 4		    // Bit for asin

#define _to_rad(x) ((x) * (PI / 180))
#define _to_deg(x) ((x) * (180 / PI))

static float MathPow10(int8_t e)
{
	float f = 1.f;
	if (e > 0)
		while (e--) f *= 10.f;
	else
		while (e++) f /= 10.f;
	return f;
}

static float MathExpSinAsin(float f, uint8_t op)
{
	float result = f, frac = f;
	if (op == BITEXP)
		result = frac = 1.0;
	for (int i = 1; _abs(frac) > TINYNUMBER && i < MAXITERATE; i++)
	{
		int i2 = 2 * i, i2p = 2 * i + 1, i2m = 2 * i - 1, i2m2 = i2m * i2m;
		float ffi2i2p = f * f / (i2 * i2p);
		if (op == BITEXP)
			frac *= f / i;
		else if (op == BITSIN)
			frac *= -ffi2i2p;
		else
			frac *= ffi2i2p * i2m2;
		result += frac;
	}
	return (result);
}