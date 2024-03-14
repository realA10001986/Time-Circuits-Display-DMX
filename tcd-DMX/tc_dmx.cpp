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
#ifdef TC_HAVESPEEDO
#include "speeddisplay.h"
#endif

#define DEST_TIME_ADDR 0x71 // TC displays
#define PRES_TIME_ADDR 0x72
#define DEPT_TIME_ADDR 0x74

#define SPEEDO_ADDR    0x70 // speedo display

#define DS3231_ADDR    0x68 // DS3231 RTC
#define PCF2129_ADDR   0x51 // PCF2129 RTC

// The TC display objects
clockDisplay destinationTime(DISP_DEST, DEST_TIME_ADDR);
clockDisplay presentTime(DISP_PRES, PRES_TIME_ADDR);
clockDisplay departedTime(DISP_LAST, DEPT_TIME_ADDR);

// The speedo object
#ifdef TC_HAVESPEEDO
speedDisplay speedo(SPEEDO_ADDR);
#endif

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

/* We have 3 ports (0-2). Port 0 is for the Serial Monitor. */
dmx_port_t dmxPort = 1;

dmx_packet_t packet;

uint8_t data[DMX_PACKET_SIZE];

#define DMX_ADDRESS               1
#define DMX_CHANNELS_PER_DISPLAY 11
#define DMX_CHANNELS (3 * DMX_CHANNELS_PER_DISPLAY)

#define DMX_SPEEDO_CHANNEL       57
#define DMX_SPEEDO_CHANNELS       2

#define DMX_VERIFY_CHANNEL       46    // must be set to DMX_VERIFY_VALUE
#define DMX_VERIFY_VALUE        100  

#if defined(DMX_USE_VERIFY) && (DMX_ADDRESS < DMX_VERIFY_CHANNEL)
#define DMX_SLOTS_TO_RECEIVE (DMX_VERIFY_CHANNEL + 1)
#else
#define DMX_SLOTS_TO_RECEIVE (DMX_ADDRESS + DMX_CHANNELS)
#endif

#ifdef TC_HAVESPEEDO
#define DMX_SLOTS_TO_RECEIVE_SP (DMX_SPEEDO_CHANNEL + DMX_SPEEDO_CHANNELS)
#endif

int dmx_slots_to_receive = DMX_SLOTS_TO_RECEIVE;

uint8_t cachedt[DMX_CHANNELS_PER_DISPLAY];
uint8_t cachept[DMX_CHANNELS_PER_DISPLAY];
uint8_t cachelt[DMX_CHANNELS_PER_DISPLAY];
#ifdef TC_HAVESPEEDO
uint8_t cachesp[DMX_SPEEDO_CHANNELS];
#endif

// DMX addresses for the displays
#define DT_BASE DMX_ADDRESS
#define PT_BASE (DT_BASE+DMX_CHANNELS_PER_DISPLAY)
#define LT_BASE (PT_BASE+DMX_CHANNELS_PER_DISPLAY)

#define SP_BASE DMX_SPEEDO_CHANNEL

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

unsigned long        powerupMillis;

static bool          dmxIsConnected = false;
static unsigned long lastDMXpacket;

// For tracking second changes
static bool          x = false;  
static bool          y = false;

static int           kpleds = 0;

#ifdef TC_HAVESPEEDO
static bool          useSpeedo = true;
#endif

// Forward declarations
static void setDisplay(clockDisplay *display, int base, int kpbit);
#ifdef TC_HAVESPEEDO
static void setSpeedoDisplay(speedDisplay *display, int base);
#endif

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

    #ifdef TC_HAVESPEEDO
    for(int i = 0; i < DMX_SPEEDO_CHANNELS; i++) {
        cachesp[i] = rand() % 255;
    }
    #endif
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

    #ifdef TC_HAVESPEEDO
    if(!speedo.begin(TC_SPEEDO_TYPE)) {
        useSpeedo = false;
        #ifdef TC_DBG
        Serial.println("Speedo not found");
        #endif
    } else {
        speedo.setDot(true);
        if(dmx_slots_to_receive < DMX_SLOTS_TO_RECEIVE_SP) {
            dmx_slots_to_receive = DMX_SLOTS_TO_RECEIVE_SP;
        }
    }
    #endif
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
        {DMX_CHANNELS, "TCD Personality"}
    };
    int personality_count = 1;

    Serial.println(F("Time Circuits Display DMX version " TC_VERSION " " TC_VERSION_EXTRA));
    Serial.println(F("(C) 2024 Thomas Winischhofer (A10001986)"));
    #ifdef DMX_USE_VERIFY
    Serial.printf("Verification is enabled; checking channel %d for value %d", DMX_VERIFY_CHANNEL, DMX_VERIFY_VALUE);
    #else
    Serial.println("Verification is disabled");
    #endif
    #ifdef TC_HAVESPEEDO
    Serial.println("Speedo support is enabled");
    #else
    Serial.println("Speedo support is disabled");
    #endif

    // Pin for monitoring seconds from RTC
    pinMode(SECONDS_IN_PIN, INPUT_PULLDOWN);

    // RTC setup
    if(!rtc.begin(powerupMillis)) {
        Serial.println("RTC not found, no 1Hz oscillator available");
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

    if(dmx_receive_num(dmxPort, &packet, dmx_slots_to_receive, 0)) {
        
        lastDMXpacket = millis();
    
        if(!packet.err) {

            if(!dmxIsConnected) {
                Serial.println("DMX is connected");
                dmxIsConnected = true;
            }
      
            dmx_read(dmxPort, data, packet.size);
      
            if(!data[0]) {

                #ifdef DMX_USE_VERIFY
                if(data[DMX_VERIFY_CHANNEL] == DMX_VERIFY_VALUE) {
                #endif
    
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

                    #ifdef TC_HAVESPEEDO
                    if(useSpeedo) {
                        if(memcmp(cachesp, data + SP_BASE, DMX_SPEEDO_CHANNELS)) {
                            setSpeedoDisplay(&speedo, SP_BASE);
                            memcpy(cachesp, data + SP_BASE, DMX_SPEEDO_CHANNELS);
                        }
                    }
                    #endif

                #ifdef DMX_USE_VERIFY
                } else {

                    Serial.printf("Bad verification value on channel %d: %d (should be %d)\n", 
                          DMX_VERIFY_CHANNEL, data[DMX_VERIFY_CHANNEL], DMX_VERIFY_VALUE);
                  
                }
                #endif

            } else {
              
                Serial.printf("Unrecognized start code %d (0x%02x)", data[0], data[0]);
                
            }
          
        } else {
            
            Serial.printf("DMX error: %d\n", packet.err);
            
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
 * DMX translation
 *
 *********************************************************************************/

/*
 * 0 = ch1  - Sets the month
 * 1 = ch2  - Sets the day
 * 2 = ch3  - Sets 1000’s digit for year
 * 3 = ch4  - Sets 100’s digit for year
 * 4 = ch5  - Sets 10’s digit for year
 * 5 = ch6  - Sets 1’s digit for year
 * 6 = ch7  - Sets Hour 1-12
 * 7 = ch8  - Sets Minute 00-59
 * 8 = ch9  - Sets AM/PM (0-127=AM, 128-255=PM)
 * 9 = ch10 - colon      (0-85=off; 86-170=on; >=171 blink) 
 * 10 = ch11 - Master Intensity (0-255)
 * 
 * Keypad LEDs are off if all displays' Intensity is 0
 * otherwise on
 */

static void setDisplay(clockDisplay *display, int base, int kpbit)
{
      int mbri;

      #ifdef TC_DBG
      for(int i = 0; i < 11; i++) {
          Serial.printf("%02x ", data[base + i]);
      }
      Serial.println(" ");
      #endif

      display->setMonth(monthRanges[data[base + 0]]);

      display->setDay(data[base + 1] / 8);

      display->setYearDigits(yearRanges[data[base + 2]], yearRanges[data[base + 3]],
                             yearRanges[data[base + 4]], yearRanges[data[base + 5]]);

      display->setHour12(hourRanges[data[base + 6]]);
      
      display->setMinute(minRanges[data[base + 7]]);

      #if 0
      if(data[base + 8] <= 85)        display->setAMPM(-1); // off
      else if(data[base + 8] <= 170)  display->setAMPM(1);  // PM  
      else                            display->setAMPM(0);  // AM
      #else
      if(data[base + 8] <= 127) display->setAMPM(1);  // PM
      else                      display->setAMPM(0);  // AM  
      // no off?                display->setAMPM(-1); // off
      #endif

      if(data[base + 9] <= 85) {
          display->setColon(false);
          display->colonBlink = false;
      } else if(data[base + 9] <= 170) {
          display->setColon(true);
          display->colonBlink = false;
      } else {
          display->colonBlink = true;
      }

      mbri = data[base + 10];   // Brightness: 0=off; 1-255:darkest->brightest

      if(mbri) {
          mbri /= 16; 
          display->setBrightness(mbri);
          display->isOn = true;     // off immediately, on after show in loop()
          kpleds |= kpbit;
      } else {
          display->off();           // off immediately, on after show in loop()
          display->isOn = false;
          kpleds &= ~kpbit;
      }
}

/*
 * Speedo fixture:
 * 0 = ch1 - Sets the speed (0-255 = 0-88mph)
 * 1 = ch2 - Master Intensity (0-255; 0=off; 1-255 = darkest-brightest)
 */
#ifdef TC_HAVESPEEDO
static void setSpeedoDisplay(speedDisplay *display, int base)
{
      int mbri = data[base + 1];   // Brightness: 0=off; 1-255:darkest->brightest

      if(mbri) {
          display->setSpeed((int8_t)((float)data[base] / 2.87));
          display->show();
          display->setBrightness(mbri / 16);
          display->on();
      } else {
          display->off();
      }
}
#endif
