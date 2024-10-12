#include "input.hpp"


//formats buttons from wiimote and gamepad to use a unified button system
static u32 ButtonFormat(u32 wpadValue, u16 padValue) {
	u32 returnValue = 0;
	//add/channge this as necessary, current support
	if ((wpadValue & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)) || (padValue & PAD_BUTTON_A)) {
		returnValue |= BUTTON_SELECT;
	}
	if ((wpadValue & (WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B)) || (padValue & PAD_BUTTON_B)) {
		returnValue |= BUTTON_DESELECT;
	}
	if ((wpadValue & (WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_UP)) || (padValue & PAD_BUTTON_UP)) {
		returnValue |= BUTTON_UP;
	}
	if ((wpadValue & (WPAD_BUTTON_2 | WPAD_CLASSIC_BUTTON_DOWN)) || (padValue & PAD_BUTTON_DOWN)) {
		returnValue |= BUTTON_DOWN;
	}
	if ((wpadValue & (WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_LEFT)) || (padValue & PAD_BUTTON_LEFT)) {
		returnValue |= BUTTON_LEFT;
	}
	if ((wpadValue & (WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_RIGHT)) || (padValue & PAD_BUTTON_RIGHT)) {
		returnValue |= BUTTON_RIGHT;
	}
	if ((wpadValue & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)) || (padValue & PAD_BUTTON_START)) {
		returnValue |= BUTTON_PAUSE;
	}

	if ((wpadValue & (WPAD_CLASSIC_BUTTON_X)) or (padValue & PAD_BUTTON_X)) {
		returnValue |= BUTTON_EX_X;
	}
	if ((wpadValue & (WPAD_CLASSIC_BUTTON_Y)) or (padValue & PAD_BUTTON_Y)) {
		returnValue |= BUTTON_EX_Y;
	}

	return returnValue;
	
}

//used for filtering nunchuk buttons out if used
static bool UsingNunchuk(int chan) {
	u32 expType = 0;
	WPAD_Probe(chan, &expType);
	if (expType == WPAD_EXP_NUNCHUK) return true;
	return false;
}

void RefreshPads() {
	WPAD_ScanPads();
	PAD_ScanPads();
}

u32 GetButtonsDown(int chan) {
	return ButtonFormat(((UsingNunchuk(chan)) ? 0x0000ffff : 0xffffffff) & WPAD_ButtonsDown(chan), PAD_ButtonsDown(chan));
}
u32 GetButtonsHeld(int chan) {
	return ButtonFormat(((UsingNunchuk(chan)) ? 0x0000ffff : 0xffffffff) & WPAD_ButtonsHeld(chan), PAD_ButtonsHeld(chan));
}
u32 GetButtonsUp(int chan) {
	return ButtonFormat(((UsingNunchuk(chan)) ? 0x0000ffff : 0xffffffff) & WPAD_ButtonsUp(chan), PAD_ButtonsUp(chan));
}

Stick GetLeftStick(int chan) {
	u32 expType;
	Stick returnStick;

	//wpad_probe == 0 checks wiimote is connected (will be <0 if not)
	if (WPAD_Probe(chan, &expType) == 0) {
		if (expType) {
			expansion_t expansion;
			WPAD_Expansion(chan, &expansion);

			switch (expType) {

			case WPAD_EXP_NUNCHUK:
				returnStick = { (float)(expansion.nunchuk.js.mag * sinf(DegToRad(expansion.nunchuk.js.ang))),
								(float)(expansion.nunchuk.js.mag * cosf(DegToRad(expansion.nunchuk.js.ang))) };
				break;
			case WPAD_EXP_CLASSIC:
				returnStick = { (float)(expansion.classic.ljs.mag * sinf(DegToRad(expansion.classic.ljs.ang))),
								(float)(expansion.classic.ljs.mag * cosf(DegToRad(expansion.classic.ljs.ang))) };
				break;
			case WPAD_EXP_GUITARHERO3:
				returnStick = { (float)(expansion.gh3.js.mag * sinf(DegToRad(expansion.gh3.js.ang))),
								(float)(expansion.gh3.js.mag * cosf(DegToRad(expansion.gh3.js.ang))) };
				break;
			default:
				returnStick = { 0,0 };
			}

		}
	}
	else {
		//TODO: add stick support for gamecube controllers (hard to test without a controller)
		returnStick = { 0,0 };
	}

	if (fabsf(returnStick.x) < 0.1f) returnStick.x = 0.0f;
	if (fabsf(returnStick.y) < 0.1f) returnStick.y = 0.0f;
	return returnStick;
}

Stick GetRightStick(int chan) {
	u32 expType;
	Stick returnStick;
	bool useExpStick = false;

	//wpad_probe == 0 checks wiimote is connected (will be <0 if not)
	if (WPAD_Probe(chan, &expType) == 0) {
		if (expType) {
			expansion_t expansion;
			WPAD_Expansion(chan, &expansion);
			switch (expType) {
			case WPAD_EXP_CLASSIC:
				returnStick = { (float)(expansion.classic.rjs.mag * sinf(DegToRad(expansion.classic.rjs.ang))),
								(float)(expansion.classic.rjs.mag * cosf(DegToRad(expansion.classic.rjs.ang))) };
				useExpStick = true;
				break;
			}
		}

		//use wiimote dpad if no other stick (i.e. classic controller)
		if (!useExpStick) {
			u32 buttonsHeld = WPAD_ButtonsHeld(chan);
			int up = (buttonsHeld & WPAD_BUTTON_UP) ? 1 : 0;
			int down = (buttonsHeld & WPAD_BUTTON_DOWN) ? 1 : 0;
			int left = (buttonsHeld & WPAD_BUTTON_LEFT) ? 1 : 0;
			int right = (buttonsHeld & WPAD_BUTTON_RIGHT) ? 1 : 0;


			if (up) {
				if (right) {
					returnStick = { 0.71875f, 0.71875f };
				}
				else if (left) {
					returnStick = { -0.71875f, 0.71875f };
				}
				else {
					returnStick = { 0, 1 };
				}
			}
			else if (down) {
				if (right) {
					returnStick = { 0.71875f, -0.71875f };
				}
				else if (left) {
					returnStick = { -0.71875f, -0.71875f };
				}
				else {
					returnStick = { 0, -1 };
				}
			}
			else if (right) {
				returnStick = { 1, 0 };
			}
			else if (left) {
				returnStick = { -1, 0 };
			}
			else {
				returnStick = { 0,0 };
			}
		}
	}
	else {
		//todo: gc controller
		returnStick = { 0,0 };
	}

	if (fabsf(returnStick.x) < 0.1f) returnStick.x = 0.0f;
	if (fabsf(returnStick.y) < 0.1f) returnStick.y = 0.0f;
	return returnStick;
}


float GetLeftTrigger(int chan) {
	u32 expType;
	float returnValue = 0;

	//wpad_probe == 0 checks wiimote is connected (will be <0 if not)
	if (WPAD_Probe(chan, &expType) == 0) {
		if (expType) {
			expansion_t expansion;
			WPAD_Expansion(chan, &expansion);

			switch (expType) {
			case WPAD_EXP_NUNCHUK:
				returnValue = ((WPAD_ButtonsHeld(chan) & WPAD_NUNCHUK_BUTTON_C) > 0) ? 1 : 0;
				break;
			case WPAD_EXP_CLASSIC:
				returnValue = expansion.classic.l_shoulder;
				break;
			case WPAD_EXP_GUITARHERO3:
				//todo: (probably never tbh but) ig find control scheme for guitar controller?
				returnValue = 0;
				break;
			default:
				returnValue = 0;
			}
		}
	}
	else {
		//todo: gc
		returnValue = 0;
	}

	if (returnValue < 0.1f) returnValue = 0.0f;
	return returnValue;
}

float GetRightTrigger(int chan) {
	u32 expType;
	float returnValue = 0;

	if (WPAD_Probe(chan, &expType) == 0) {
		if (expType) {
			expansion_t expansion;
			WPAD_Expansion(chan, &expansion);

			switch (expType) {
			case WPAD_EXP_NUNCHUK:
				returnValue = ((WPAD_ButtonsHeld(chan) & WPAD_NUNCHUK_BUTTON_Z) > 0) ? 1 : 0;
				break;
			case WPAD_EXP_CLASSIC:
				returnValue = expansion.classic.r_shoulder;
				break;
			case WPAD_EXP_GUITARHERO3:
				//todo: (probably never tbh but) ig find control scheme for guitar controller?
				returnValue = 0;
				break;
			default:
				returnValue = 0;
			}
		}
	}
	else {
		//todo: gc
		returnValue = 0;
	}
	if (returnValue < 0.1f) returnValue = 0.0f;
	return returnValue;
}