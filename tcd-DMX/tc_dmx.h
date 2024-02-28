/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Time Circuits Display - DMX-controlled
 * (C) 2024 Thomas Winischhofer (A10001986)
 * All rights reserved.
 * -------------------------------------------------------------------
 */

#ifndef _TC_DMX_H
#define _TC_DMX_H

#include "rtc.h"
#include "clockdisplay.h"

extern unsigned long powerupMillis;

extern clockDisplay destinationTime;
extern clockDisplay presentTime;
extern clockDisplay departedTime;

void dmx_boot();
void dmx_setup();
void dmx_loop();

#endif
