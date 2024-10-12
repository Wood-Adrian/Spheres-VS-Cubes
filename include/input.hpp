#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include <ogcsys.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

//button scheme to be designed around (ontop of 2 sticks and 2 triggers)
#define BUTTON_SELECT 0x0004
#define BUTTON_DESELECT 0x0008
#define BUTTON_UP 0x0010
#define BUTTON_DOWN 0x0020
#define BUTTON_LEFT 0x0040
#define BUTTON_RIGHT 0x0080
#define BUTTON_PAUSE 0x0100
//extra buttons that only pro/gc controller has (for shortcuts ig)
#define BUTTON_EX_X 0x1000;
#define BUTTON_EX_Y 0x2000;

struct Stick {
	float x;
	float y;
};

//calls (W)PAD_ScanPads(), run each frame
void RefreshPads();

//get all freshly pressed buttons from all controllers in specific channel
u32 GetButtonsDown(int chan);

//get all held down buttons from all controllers in specific channel
u32 GetButtonsHeld(int chan);

//get all released buttons from all controllers in specific channel
u32 GetButtonsUp(int chan);

//get current value of left stick (/nunchuck)
Stick GetLeftStick(int chan);

//get current value of right stick (/dpad)
Stick GetRightStick(int chan);

//get current value of left trigger (/c button)
float GetLeftTrigger(int chan);

//get current value of right trigger (/z button)
float GetRightTrigger(int chan);