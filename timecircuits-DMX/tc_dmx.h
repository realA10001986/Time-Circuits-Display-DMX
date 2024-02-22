/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Time Circuits Display - DMX version
 * (C) 2024 Thomas Winischhofer (A10001986)
 * All rights reserved. 
 * 
 * -------------------------------------------------------------------
 */

#ifndef _TC_DMX_H
#define _TC_DMX_H

#include "rtc.h"
#include "clockdisplay.h"

extern unsigned long powerupMillis;

void dmx_boot();
void dmx_setup();
void dmx_loop();

int  daysInMonth(int month, int year);

#endif
