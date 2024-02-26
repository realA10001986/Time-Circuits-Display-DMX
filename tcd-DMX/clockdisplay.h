/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Time Circuits Display - DMX-controlled
 * (C) 2024 Thomas Winischhofer (A10001986)
 * All rights reserved.
 * -------------------------------------------------------------------
 */

#ifndef _CLOCKDISPLAY_H
#define _CLOCKDISPLAY_H

//#include "rtc.h"

struct dateStruct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
};

#define CD_BUF_SIZE   8  // Buffer size in words (16bit)

// Flags for textDirect() etc (flags)
#define CDT_CLEAR 0x0001
#define CDT_CORR6 0x0002
#define CDT_COLON 0x0004
#define CDT_BLINK 0x0008

// Flags for xxxDirect (dflags)
#define CDD_FORCE24 0x0001
#define CDD_NOLEAD0 0x0002

class clockDisplay {

    public:

        clockDisplay(uint8_t did, uint8_t address);
        void begin();
        void on();
        void onCond();
        void off();
        void onBlink(uint8_t blink);
        #if 0
        void realLampTest();
        #endif
        void lampTest(bool randomize = false);

        void clearBuf();

        uint8_t setBrightness(uint8_t level, bool setInitial = false);
        void    resetBrightness();
        uint8_t setBrightnessDirect(uint8_t level);
        uint8_t getBrightness();

        void set1224(bool hours24);
        bool get1224();

        void setRTC(bool rtc);  // make this an RTC display
        bool isRTC();

        void show();
        void showAnimate1();
        void showAnimate2();

        void setFromStruct(const dateStruct *s); // Set object date & time from struct
        void setFromParms(int year, int month, int day, int hour, int minute);

        void getToParms(int& year, int& month, int& day, int& hour, int& minute);

        void setMonth(int monthNum);
        void setDay(int dayNum);
        void setYear(uint16_t yearNum);
        void setYearDigits(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4);
        void setHour(uint16_t hourNum);
        void setHour12(uint16_t hourNum);
        void setMinute(int minNum);
        void setAMPM(int isPM);

        void setColon(bool col);

        uint8_t  getMonth();
        uint8_t  getDay();
        uint16_t getYear();
        uint8_t  getHour();
        uint8_t  getMinute();

        const char* getMonthString(uint8_t month);

        void showMonthDirect(int monthNum, uint16_t dflags = 0);
        void showDayDirect(int dayNum, uint16_t dflags = 0);
        void showHourDirect(int hourNum, uint16_t dflags = 0);
        void showMinuteDirect(int minuteNum, uint16_t dflags = 0);
        void showYearDirect(int yearNum, uint16_t dflags = 0);

        void showTextDirect(const char *text, uint16_t flags = CDT_CLEAR);

        bool colonBlink = false;
        bool isOn = false;

    private:

        uint8_t  getLED7NumChar(uint8_t value);
        uint8_t  getLED7AlphaChar(uint8_t value);
        uint16_t getLEDAlphaChar(uint8_t value);
        
        uint16_t makeNum(uint8_t num, uint16_t dflags = 0);

        void directCol(int col, int segments);

        void clearDisplay();
        void showInt(bool animate = false, bool Alt = false);

        void colonOn();
        void colonOff();

        void AM();
        void PM();
        void AMPMoff();
        void directAMPM(int val1, int val2);
        void directAM();
        void directPM();
        void directAMPMoff();

        void directCmd(uint8_t val);

        uint8_t  _did = 0;
        uint8_t  _address = 0;
        uint16_t _displayBuffer[CD_BUF_SIZE];
        uint16_t _displayBufferAlt[CD_BUF_SIZE];

        uint16_t _year = 2021;          // keep track of these
        int16_t  _yearoffset = 0;       // Offset for faking years < 2000, > 2098

        int16_t  _lastWrittenLY = -333; // Not to be confused with possible results from loadClockStateData
        int16_t  _lastWrittenYO = -11111;
        bool     _lastWrittenRead = false;

        uint8_t _month = 1;
        uint8_t _day = 1;
        uint8_t _hour = 0;
        uint8_t _minute = 0;
        int     _isPM = 0;
        bool    _colon = false;         // should colon be on?
        bool    _rtc = false;           // will this be displaying real time
        uint8_t _brightness = 15;       // current display brightness
        uint8_t _origBrightness = 15;   // value from settings
        bool    _mode24 = false;        // true = 24 hour mode, false = 12 hour mode
        bool    _nightmode = false;     // true = dest/dept times off
        bool    _NmOff = false;         // true = off during night mode, false = dimmed
        int     _oldnm = -1;
        bool    _corr6 = false;
        bool    _yearDot = false;
        bool    _withColon = false;

        int8_t  _Cache = -1;
        char    _CacheData[10];

        int     _savePending = 0;
};

#endif
