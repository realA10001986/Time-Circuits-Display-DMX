
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

/* First, lets define the hardware pins that we are using with our ESP32. We
  need to define which pin is transmitting data and which pin is receiving data.
  DMX circuits also often need to be told when we are transmitting and when we
  are receiving data. We can do this by defining an enable pin. */
int transmitPin = DMX_TRANSMIT;
int receivePin = DMX_RECEIVE;
int enablePin = DMX_ENABLE;

/* Make sure to double-check that these pins are compatible with your ESP32!
  Some ESP32s, such as the ESP32-WROVER series, do not allow you to read or
  write data on pins 16 or 17, so it's always good to read the manuals. */

/* Next, lets decide which DMX port to use. The ESP32 has either 2 or 3 ports.
  Port 0 is typically used to transmit serial data back to your Serial Monitor,
  so we shouldn't use that port. Lets use port 1! */
dmx_port_t dmxPort = 1;

dmx_packet_t packet;

/* Now we want somewhere to store our DMX data. Since a single packet of DMX
  data can be up to 513 bytes long, we want our array to be at least that long.
  This library knows that the max DMX packet size is 513, so we can fill in the
  array size with `DMX_PACKET_SIZE`. */
byte data[DMX_PACKET_SIZE];

#define DT_BASE 1
#define PT_BASE 10
#define LT_BASE 20

unsigned long powerupMillis;

static bool          dmxIsConnected = false;
static unsigned long lastDMXpacket;

// For tracking second changes
static bool          x = false;  
static bool          y = false;

static void setDisplay(clockDisplay *display, int base);

static void startDisplays()
{
    presentTime.begin();
    destinationTime.begin();
    departedTime.begin();
}


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

    // Switch LEDs on
    // give user some feedback that the unit is powered
    pinMode(LEDS_PIN, OUTPUT);
    digitalWrite(LEDS_PIN, HIGH);

    

    //destinationTime.showTextDirect("WELCOME");
}    





void dmx_setup() 
{
    // Pin for monitoring seconds from RTC
    pinMode(SECONDS_IN_PIN, INPUT_PULLDOWN);

    // RTC setup
    if(!rtc.begin(powerupMillis)) {
        Serial.println("RTC not found!");
    }
    if(rtc.lostPower()) {
        // Lost power and battery didn't keep time, so set current time to 
        // default time 1/1/2024, 0:0
        rtc.adjust(0, 0, 0, 1, 1, 1, 24);
    }

    // Turn on the RTC's 1Hz clock output
    rtc.clockOutEnable();
  
  /* Now we will install the DMX driver! We'll tell it which DMX port to use,
    what device configuration to use, and what DMX personalities it should have.
    If you aren't sure which configuration to use, you can use the macros
    DMX_CONFIG_DEFAULT to set the configuration to its default settings.
    This device is being setup as a DMX responder so it is likely that it should
    respond to DMX commands. It will need at least one DMX personality. Since
    this is an example, we will use a default personality which only uses 1 DMX
    slot in its footprint. */
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
        {3*10, "TCD Personality"}
    };
    int personality_count = 1;
    dmx_driver_install(dmxPort, &config, personalities, personality_count);

    /* Now set the DMX hardware pins to the pins that we want to use and setup
      will be complete! */
    dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin);
}





void dmx_loop() 
{
    bool newData = false;    
    
    if(dmx_receive(dmxPort, &packet, 0)) {
        
        lastDMXpacket = millis();
    
        if (!packet.err) {

            if(!dmxIsConnected) {
                Serial.println("DMX is connected!");
                dmxIsConnected = true;
            }
      
            dmx_read(dmxPort, data, packet.size);
      
            if(!data[0]) {
                setDisplay(&destinationTime, DT_BASE);
                setDisplay(&presentTime, PT_BASE);
                setDisplay(&departedTime, LT_BASE);
                newData = true;
            } else {
                Serial.printf("Unrecognized start code %d (0x%02x)", data[0]);
            }
          
        } else {
            /* Oops! A DMX error occurred! Don't worry, this can happen when you first
              connect or disconnect your DMX devices. If you are consistently getting
              DMX errors, then something may have gone wrong with your code or
              something is seriously wrong with your DMX transmitter. */
            Serial.println("A DMX error occurred.");
        }
        
    } 

    y = digitalRead(SECONDS_IN_PIN);
    if(y != x) {
        destinationTime.setColon(!y);
        presentTime.setColon(!y);
        departedTime.setColon(!y);
        x = y;
        newData = true;
    }

    if(newData) {
        destinationTime.show();
        presentTime.show();
        departedTime.show();
    }

    if(millis() - lastDMXpacket > 1250) {
        Serial.println("DMX was disconnected.");
        dmxIsConnected = false;
    }
}


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
 * 8 = ch9  - Sets AM/PM (0 = AM, 1 = PM)
 * 9 = ch10 - Master Intensity (0-16)
 */
static void setDisplay(clockDisplay *display, int base)
{  
      int year = (data[base + 2] * 1000) +
                 (data[base + 3] * 100) +
                 (data[base + 4] * 10) +
                 data[base + 5];
      display->setYear(year);
      display->setMonth(data[base + 0]);
      display->setDay(data[base + 1]);
      display->setMinute(data[base + 7]);
      int hour = data[base + 6];
      if(data[base + 8]) {   // PM
          if(hour < 12) {
              hour += 12;
          }
      } else {               // AM
          if(hour == 12) {
              hour = 0;
          }
      }
      display->setHour(hour);

      display->show();

      if(data[base + 9]) {
          display->setBrightness(data[base + 9] - 1);
          display->on();
      } else {
          display->off();
      }
}
