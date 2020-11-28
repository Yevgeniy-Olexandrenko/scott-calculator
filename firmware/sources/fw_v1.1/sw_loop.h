// LOOP

void loop()
{
	if (!(nextframe())) return;

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
		timestamp = millis();
		DisplayBrightness(brightness);
		DisplayTurnOn();
	}

	uint8_t elapsedTime = (millis() - timestamp) / 1000L;
	if (elapsedTime > POWEROFFTIME)
	{
		PowerOff();
	}
	else if (elapsedTime > DIMTIME)
	{
		DisplayBrightness(0x00);
	}

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