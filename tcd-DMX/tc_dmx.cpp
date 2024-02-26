/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Time Circuits Display - DMX-controlled
 * (C) 2024 Thomas Winischhofer (A10001986)
 * All rights reserved.
 * -------------------------------------------------------------------
 */

#include "tc_global.h"

#include <Arduino.h>
#include <esp_dmx.h>

#include "tc_dmx.h"

#define DEST_TIME_ADDR 0x71 // TC displays
#define PRES_TIME_ADDR 0x72
#define DEPT_TIME_ADDR 0x74

#define DS3231_ADDR    0x68 // DS3231 RTC
#define PCF2129_ADDR   0x51 // PCF2129 RTC

// The TC display objects
clockDisplay destinationTime(DISP_DEST, DEST_TIME_ADDR);
clockDisplay presentTime(DISP_PRES, PRES_TIME_ADDR);
clockDisplay departedTime(DISP_LAST, DEPT_TIME_ADDR);

// The RTC object
tcRTC rtc(2, (uint8_t[2*2]){ PCF2129_ADDR, RTCT_PCF2129, 
                         DS3231_ADDR,  RTCT_DS3231 });

static const uint8_t monthDays[] =
{
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const dateStruct defDestinationTime = {1985, 10, 26,  1, 21};
const dateStruct defPresentTime     = {1985, 10, 26,  1, 22};
const dateStruct defDepartedTime    = {1985, 10, 26,  1, 20};

int transmitPin = DMX_TRANSMIT;
int receivePin = DMX_RECEIVE;
int enablePin = DMX_ENABLE;

/* Next, lets decide which DMX port to use. The ESP32 has either 2 or 3 ports.
   Port 0 is typically used to transmit serial data back to your Serial Monitor,
   so we shouldn't use that port. Lets use port 1! */
dmx_port_t dmxPort = 1;

dmx_packet_t packet;

uint8_t data[DMX_PACKET_SIZE];

#define DMX_ADDRESS 1
#define DMX_CHANNELS_PER_DISPLAY 11

uint8_t cachedt[DMX_CHANNELS_PER_DISPLAY];
uint8_t cachept[DMX_CHANNELS_PER_DISPLAY];
uint8_t cachelt[DMX_CHANNELS_PER_DISPLAY];

// DMX addresses for the displays
#define DT_BASE DMX_ADDRESS
#define PT_BASE (DT_BASE+DMX_CHANNELS_PER_DISPLAY)
#define LT_BASE (PT_BASE+DMX_CHANNELS_PER_DISPLAY)

static const uint8_t monthRanges[256] = {
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
     2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
     3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
     4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
     5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
     6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6, 
     7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
     8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
     9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12
};    
static const uint8_t yearRanges[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 0-23
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,      // 24-46
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,      // 47-69
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,   // 70-93
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,      // 94-116
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,      // 117-139
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,      // 140-162
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,   // 163-186
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,      // 187-209
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,      // 210-232
   10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10       // 233-255
};

static const uint8_t hourRanges[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        // 0-18
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,           // 19-36
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,           // 37-54
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,           // 55-72
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,           // 73-90
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,           // 91-108
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,           // 109-126
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,           // 127-144
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,           // 145-162
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,           // 163-180
   10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,           // 181-198
   11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,           // 199-216
   12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,           // 217-234
   13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13   // 235-255
};

static const uint8_t minRanges[256] = {
    0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,  // 0-16
    4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,  // 17-33
    8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,10,11,11,11,11,  // 34-50
   12,12,12,12,13,13,13,13,14,14,14,14,15,15,15,15,15,  // 51-67
   16,16,16,16,17,17,17,17,18,18,18,18,19,19,19,19,     // 68-83
   20,20,20,20,20,21,21,21,21,22,22,22,22,23,23,23,23,  // 84-100
   24,24,24,24,25,25,25,25,25,26,26,26,26,27,27,27,27,  // 101-117
   28,28,28,28,29,29,29,29,30,30,30,30,30,31,31,31,31,  // 118-134
   32,32,32,32,33,33,33,33,34,34,34,34,35,35,35,35,35,  // 135-151
   36,36,36,36,37,37,37,37,38,38,38,38,39,39,39,39,     // 152-167
   40,40,40,40,40,41,41,41,41,42,42,42,42,43,43,43,43,  // 168-184
   44,44,44,44,45,45,45,45,45,46,46,46,46,47,47,47,47,  // 185-201
   48,48,48,48,49,49,49,49,50,50,50,50,50,51,51,51,51,  // 202-218
   52,52,52,52,53,53,53,53,54,54,54,54,55,55,55,55,55,  // 219-235
   56,56,56,56,57,57,57,57,58,58,58,58,59,59,59,59,     // 236-251
   60,60,60,60                                          // 252-255
};

unsigned long powerupMillis;

static bool          dmxIsConnected = false;
static unsigned long lastDMXpacket;

// For tracking second changes
static bool          x = false;  
static bool          y = false;

static int           kpleds = 0;

// Forward declarations
static void setDisplay(clockDisplay *display, int base, int kpbit);


static void startDisplays()
{
    presentTime.begin();
    destinationTime.begin();
    departedTime.begin();
}

static void invalidateCache()
{
    for(int i = 0; i < DMX_CHANNELS_PER_DISPLAY; i++) {
        cachedt[i] = cachept[i] = cachelt[i] = rand() % 255;
    }
}


/*********************************************************************************
 * 
 * boot
 *
 *********************************************************************************/

void dmx_boot() 
{
    // Start the displays early to clear them
    startDisplays();

    presentTime.set1224(false);
    destinationTime.set1224(false);
    departedTime.set1224(false);

    destinationTime.setFromStruct(&defDestinationTime);
    presentTime.setFromStruct(&defPresentTime);
    departedTime.setFromStruct(&defDepartedTime);

    // Init color LEDs, keep them off
    pinMode(LEDS_PIN, OUTPUT);
    digitalWrite(LEDS_PIN, LOW);

    // Init white LED, keep it off
    pinMode(WHITE_LED_PIN, OUTPUT);
    digitalWrite(WHITE_LED_PIN, LOW);
}    



/*********************************************************************************
 * 
 * setup
 *
 *********************************************************************************/


void dmx_setup() 
{
    dmx_config_t config = {
      .interrupt_flags = DMX_INTR_FLAGS_DEFAULT,
      .root_device_parameter_count = 32,
      .sub_device_parameter_count = 0,
      .model_id = 0,
      .product_category = RDM_PRODUCT_CATEGORY_FIXTURE,
      .software_version_id = 1,
      .software_version_label = "TCD-DMXv1",
      .queue_size_max = 32
    };
    dmx_personality_t personalities[] = {
        {3*DMX_CHANNELS_PER_DISPLAY, "TCD Personality"}
    };
    int personality_count = 1;

    Serial.println(F("Time Circuits Display DMX version " TC_VERSION " " TC_VERSION_EXTRA));

    // Pin for monitoring seconds from RTC
    pinMode(SECONDS_IN_PIN, INPUT_PULLDOWN);

    // RTC setup
    if(!rtc.begin(powerupMillis)) {
        Serial.println("RTC not found!");
    }
    if(rtc.lostPower()) {
        // Lost power and battery didn't keep time, so set some default time
        rtc.adjust(0, 0, 0, 1, 1, 1, 24);
    }

    // Turn on the RTC's 1Hz clock output
    rtc.clockOutEnable();

    invalidateCache();
  
    // Start the DMX stuff
    dmx_driver_install(dmxPort, &config, personalities, personality_count);
    dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin);
}


/*********************************************************************************
 * 
 * loop
 *
 *********************************************************************************/

void dmx_loop() 
{
    bool newDataDT = false;
    bool newDataPT = false;
    bool newDataLT = false;
    
    if(dmx_receive(dmxPort, &packet, 0)) {
        
        lastDMXpacket = millis();
    
        if(!packet.err) {

            if(!dmxIsConnected) {
                Serial.println("DMX is connected");
                dmxIsConnected = true;
            }
      
            dmx_read(dmxPort, data, packet.size);
      
            if(!data[0]) {
                if(memcmp(cachedt, data + DT_BASE, DMX_CHANNELS_PER_DISPLAY)) {
                    setDisplay(&destinationTime, DT_BASE, 1);
                    newDataDT = true;
                    memcpy(cachedt, data + DT_BASE, DMX_CHANNELS_PER_DISPLAY);
                }
                if(memcmp(cachept, data + PT_BASE, DMX_CHANNELS_PER_DISPLAY)) {
                    setDisplay(&presentTime, PT_BASE, 2);
                    newDataPT = true;
                    memcpy(cachept, data + PT_BASE, DMX_CHANNELS_PER_DISPLAY);
                }
                if(memcmp(cachelt, data + LT_BASE, DMX_CHANNELS_PER_DISPLAY)) {
                    setDisplay(&departedTime, LT_BASE, 4);
                    newDataLT = true;
                    memcpy(cachelt, data + LT_BASE, DMX_CHANNELS_PER_DISPLAY);
                }
            } else {
                Serial.printf("Unrecognized start code %d (0x%02x)", data[0], data[0]);
            }
          
        } else {
            
            Serial.println("A DMX error occurred");
            
        }
        
    }

    y = digitalRead(SECONDS_IN_PIN);
    if(y != x) {
        if(destinationTime.colonBlink) {
            destinationTime.setColon(!y);
            newDataDT = true;
        }
        if(presentTime.colonBlink) {
            presentTime.setColon(!y);
            newDataPT = true;
        }
        if(departedTime.colonBlink) {
            departedTime.setColon(!y);
            newDataLT = true;
        }
        x = y;
    }

    if(newDataDT) {
        destinationTime.show();
        if(destinationTime.isOn) destinationTime.on();
    }
    if(newDataPT) {
        presentTime.show();
        if(presentTime.isOn) presentTime.on();
    }
    if(newDataLT) {
        departedTime.show();
        if(departedTime.isOn) departedTime.on();
    }
    if(newDataDT || newDataPT || newDataLT) {
        if(kpleds) {
            digitalWrite(LEDS_PIN, HIGH);
        } else {
            digitalWrite(LEDS_PIN, LOW);
        }
    }

    if(dmxIsConnected && (millis() - lastDMXpacket > 1250)) {
        Serial.println("DMX was disconnected");
        dmxIsConnected = false;
        invalidateCache();
    }
}



/*********************************************************************************
 * 
 * helpers
 *
 *********************************************************************************/

static bool isLeapYear(int year)
{
    if((year & 3) == 0) { 
        if((year % 100) == 0) {
            if((year % 400) == 0) {
                return true;
            } else {
                return false;
            }
        } else {
            return true;
        }
    } else {
        return false;
    }
}

/* 
 * Find number of days in a month 
 */
int daysInMonth(int month, int year)
{
    if(month == 2 && isLeapYear(year)) {
        return 29;
    }
    return monthDays[month - 1];
}


/*
 * 0 = ch1  - Sets the month
 * 1 = ch2  - Sets the day
 * 2 = ch3  - Sets 1000’s digit for year
 * 3 = ch4  - Sets 100’s digit for year
 * 4 = ch5  - Sets 10’s digit for year
 * 5 = ch6  - Sets 1’s digit for year
 * 6 = ch7  - Sets Hour 1-12
 * 7 = ch8  - Sets Minute 00-59
 * 8 = ch9  - Sets AM/PM (0 = AM, 1 = PM)     (fs: 0-127=AM, 128-255=PM)
 * 9 = ch10 - colon (0-85=off; 86-170=on; >171 blink) 
 * 10 = ch11 - Master Intensity (0-255)
 * 
 * Keypad LEDs are off if all displays' Intensity is 0
 * otherwise on
 */


static void setDisplay(clockDisplay *display, int base, int kpbit)
{
      int mbri;

      display->setMonth(monthRanges[data[base + 0]]);

      display->setDay(data[base + 1] / 8);

      display->setYearDigits(yearRanges[data[base + 2]], yearRanges[data[base + 3]],
                             yearRanges[data[base + 4]], yearRanges[data[base + 5]]);

      display->setHour12(hourRanges[data[base + 6]]);
      
      display->setMinute(minRanges[data[base + 7]]);

      if(data[base + 8] <= 127) display->setAMPM(1);  // PM
      else                      display->setAMPM(0);  // AM  
      // no off?                display->setAMPM(-1); // off

      if(data[base + 9] <= 85) {
          display->setColon(false);
          display->colonBlink = false;
      } else if(data[base + 9] <= 170) {
          display->setColon(true);
          display->colonBlink = false;
      } else {
          display->colonBlink = true;
      }

      mbri = data[base + 10] /= 15; // Brightness
      if(mbri > 16) mbri = 16;

      if(mbri) {
          display->setBrightness(mbri - 1);
          display->isOn = true;     // off immediately, on after show in loop()
          kpleds |= kpbit;
      } else {
          display->off();           // off immediately, on after show in loop()
          display->isOn = false;
          kpleds &= ~kpbit;
      }
}
