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
		dcontrast(brightness);
		don();
	}

	uint8_t pot = (millis() - timestamp) / 1000L;
	if (pot > POWEROFFTIME)
	{
		PowerOff();
	}
	else if (pot > DIMTIME)
	{
		dcontrast(0x00);
	}

	if (isplaystring)
	{ // ### Play string
		key = playbuf[select];
		if (key == NULL)
		{						  // Stop playstring
			LoadStackFromShadowBuffer(restore); // Restore upper part of stack
			isplaystring = false;
			isnewnumber = true;
			key = KEY_DUMMY;
		}
		else
		{ // Go on for dispatching
			if (key <= KEY_15 && ((select == 0) || (select > 0 && playbuf[select - 1] > KEY_15)))
			{ // New number (0-9,.)
				isnewnumber = true;
				ispushed = false;
			}
			select++;
		}
	}

	else if (isrec || isplay)
	{ // ### Type recorder (else: playstring works inside play)
		int maxptr = EEREC + (recslot + 1) * MAXREC;
		if (isrec)
		{ // Record keys and write to EEPPROM
			if (key && recptr < maxptr)
				EEPROM[recptr++] = key;
		}
		else
		{ // Read/play key from EEPROM
			if (key == KEY_13)
			{ // Stop execution
				isplay = false;
				key = KEY_DUMMY;
			}
			key = EEPROM[recptr++];
		}
		if (key == KEY_13 || recptr >= maxptr)
		{
			isplay = isrec = false;
			key = KEY_DUMMY;
		}
	}

	if (key == KEY_1)
	{							  // ### SHIFT pressed
		isf = isf ? false : true; // Toggle shift key
		key = KEY_DUMMY;
	}
	else if (key == KEY_15)
		_dot(); // ### Entering decimals demanded

	if (key)
	{ // ### Execute key
		if (key != KEY_DUMMY)
		{ // Printscreen only
			if (ismenu)
			{ // Select and dispatch MENU
				uint8_t limit = numberofcommands / FKEYNR - 1;
				if (key == KEY_5)
				{ // Cursor up
					if (select > 0)
						select--;
					else
						select = limit;
				}
				else if (key == KEY_9)
				{ // Cursor down
					if (select < limit)
						select++;
					else
						select = 0;
				}
				else if (key == KEY_13)
				{ // ESC
					ismenu = false;
				}
				else if (key >= KEY_2 && key <= KEY_4)
				{													   // F-KEY "7 8 9"
					(*dispatch[22 + select * FKEYNR + key - KEY_2])(); // Dispatch with menu offset
					isnewnumber = true;
					ismenu = false;
				}
			}
			else if (isf)
			{ // Dispatch shifted key
				(*dispatch[key - KEY_14 + 6])();
				isf = ispushed = false;
				isnewnumber = true;
			}
			else
			{ // Dispatch anything else
				if (key <= KEY_4)
					(*dispatch[0])(); // Dispatch number 0(0...9)
				else
					(*dispatch[key - KEY_4])(); // Dispatch key (1d: 2c; 3e< 4x= 5s> 6f? + other) due to function table
			}
		}
		printscreen(); // Print screen every keypress (or if key == KEY_DUMMY)
	}
}