/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Time Circuits Display - DMX-controlled
 * (C) 2024 Thomas Winischhofer (A10001986)
 * All rights reserved.
 * -------------------------------------------------------------------
 */

#ifndef _TC_GLOBAL_H
#define _TC_GLOBAL_H

/*************************************************************************
 ***                          Version Strings                          ***
 *************************************************************************/

// These must not contain any characters other than
// '0'-'9', 'A'-'Z', '(', ')', '.', '_', '-' or space
#define TC_VERSION "V0.10"
#define TC_VERSION_EXTRA "MAR082024"

//#define TC_DBG              // debug output on Serial

// If this is uncommented, the firmware uses channel DMX_VERIFY_CHANNEL
// for packet verification. The value of this channel must, at all times,
// be DMX_VERIFY_VALUE for a packet to be accepted.
//#define DMX_USE_VERIFY

//#define TC_HAVESPEEDO
#define TC_SPEEDO_TYPE 11 //0

/*************************************************************************
 ***                             GPIO pins                             ***
 *************************************************************************/

#define STATUS_LED_PIN     2      // Status LED (on ESP)
#define SECONDS_IN_PIN    15      // SQW Monitor 1Hz from the DS3231
#define ENTER_BUTTON_PIN  16      // enter key
#define WHITE_LED_PIN     17      // white led
#define LEDS_PIN          12      // Red/amber/green LEDs (TCD-Control V1.3+)

// I2S audio pins
#define I2S_BCLK_PIN      26
#define I2S_LRCLK_PIN     25
#define I2S_DIN_PIN       33

// SD Card pins
#define SD_CS_PIN          5
#define SPI_MOSI_PIN      23
#define SPI_MISO_PIN      19
#define SPI_SCK_PIN       18

#define VOLUME_PIN        32      // analog input pin

#define FAKE_POWER_BUTTON_PIN       13  // Fake "power" switch
#define EXTERNAL_TIMETRAVEL_IN_PIN  27  // Externally triggered TT (input)
#define EXTERNAL_TIMETRAVEL_OUT_PIN 14  // TT trigger output

// DMX
#define DMX_TRANSMIT 14
#define DMX_RECEIVE  13
#define DMX_ENABLE   27


/*************************************************************************
 ***             Display IDs (Do not change, used as index)            ***
 *************************************************************************/

#define DISP_DEST     0
#define DISP_PRES     1
#define DISP_LAST     2

// Num of characters on display
#define DISP_LEN      13


#endif
