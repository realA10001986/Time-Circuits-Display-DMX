/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Time Circuits Display - DMX-controlled
 * (C) 2024 Thomas Winischhofer (A10001986)
 * All rights reserved.
 * -------------------------------------------------------------------
 */

#include "tc_global.h"

#include <Arduino.h>
#include <Wire.h>

#include "clockdisplay.h"
#include "tc_font.h"

#define CD_MONTH_POS  0
#define CD_MONTH_SIZE 3     //      number of words
#define CD_MONTH_DIGS 3     //      number of digits/letters
#define CD_DAY_POS    3
#define CD_YEAR_POS   4
#define CD_HOUR_POS   6
#define CD_MIN_POS    7

#define CD_AMPM_POS   CD_DAY_POS
#define CD_COLON_POS  CD_YEAR_POS

static const char months[12][4] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

static const char *nullStr = "";

/*
 * ClockDisplay class
 */

// Store i2c address and display ID
clockDisplay::clockDisplay(uint8_t did, uint8_t address)
{
    _did = did;
    _address = address;
}

// Start the display
void clockDisplay::begin()
{
    directCmd(0x20 | 1); // turn on oscillator

    clearBuf();          // clear buffer
    setBrightness(15);   // setup initial brightness
    clearDisplay();      // clear display RAM
    on();                // turn it on
}

// Turn on the display
void clockDisplay::on()
{
    directCmd(0x80 | 1);
}

// Turn on the display unless off due to night mode
void clockDisplay::onCond()
{
    if(!_nightmode || !_NmOff) {
        directCmd(0x80 | 1);
    }
}

// Turn off the display
void clockDisplay::off()
{
    directCmd(0x80);
}

void clockDisplay::onBlink(uint8_t blink)
{
    directCmd(0x80 | 1 | ((blink & 0x03) << 1)); 
}

// Turn on some LEDs
// Used for effects and brightness keypad menu
void clockDisplay::lampTest(bool randomize)
{
    Wire.beginTransmission(_address);
    Wire.write(0x00);  // start address

    uint32_t rnd = esp_random();

    for(int i = 0; i < CD_BUF_SIZE; i++) {
        Wire.write(randomize ? ((rand() % 0x7f) ^ rnd) & 0x7f : 0xaa);
        Wire.write(randomize ? (((rand() % 0x7f) ^ (rnd >> 8))) & 0x77 : 0x55);
    }
    
    Wire.endTransmission();
}

// Clear the buffer
void clockDisplay::clearBuf()
{
    for(int i = 0; i < CD_BUF_SIZE; i++) {
        _displayBuffer[i] = 0;
    }
}

// Set display brightness
// Valid brighness levels are 0 to 15.
// 255 sets it to previous level
uint8_t clockDisplay::setBrightness(uint8_t level, bool setInitial)
{
    if(level == 255)
        level = _brightness;    // restore to old val

    _brightness = setBrightnessDirect(level);

    if(setInitial) _origBrightness = _brightness;

    return _brightness;
}

void clockDisplay::resetBrightness()
{
    _brightness = setBrightnessDirect(_origBrightness);
}

uint8_t clockDisplay::setBrightnessDirect(uint8_t level)
{
    if(level > 15)
        level = 15;

    directCmd(0xe0 | level);

    return level;
}

uint8_t clockDisplay::getBrightness()
{
    return _brightness;
}

void clockDisplay::set1224(bool hours24)
{
    _mode24 = hours24;
}

bool clockDisplay::get1224(void)
{
    return _mode24;
}

// Track if this is will be holding real time.
void clockDisplay::setRTC(bool rtc)
{
    _rtc = rtc;
}

bool clockDisplay::isRTC()
{
    return _rtc;
}


// Setup date in buffer --------------------------------------------------------


// Set YEAR, MONTH, DAY, HOUR, MIN from structure
void clockDisplay::setFromStruct(const dateStruct *s)
{    
    setYear(s->year);
    setMonth(s->month);
    setDay(s->day);
    setHour(s->hour);
    setMinute(s->minute);
}

void clockDisplay::setFromParms(int year, int month, int day, int hour, int minute)
{
    setYear(year);
    setMonth(month);
    setDay(day);
    setHour(hour);
    setMinute(minute);
}

void clockDisplay::getToParms(int& year, int& month, int& day, int& hour, int& minute)
{
    year = getYear();
    month = getMonth();
    day = getDay();
    hour = getHour();
    minute = getMinute();
}


// Show data in display --------------------------------------------------------


// Show the buffer
void clockDisplay::show()
{
    showInt(false);
}

// Show all but month
void clockDisplay::showAnimate1()
{
    showInt(true);
}

// Show month, assumes showAnimate1() was called before
void clockDisplay::showAnimate2()
{
    Wire.beginTransmission(_address);
    Wire.write(0x00);
    for(int i = 0; i < CD_BUF_SIZE; i++) {
        Wire.write(_displayBuffer[i] & 0xff);
        Wire.write(_displayBuffer[i] >> 8);
    }
    Wire.endTransmission();
}

// Set fields in buffer --------------------------------------------------------


void clockDisplay::setMonth(int monthNum)
{
    if(!monthNum) {
        _displayBuffer[CD_MONTH_POS]     = 0;
        _displayBuffer[CD_MONTH_POS + 1] = 0;
        _displayBuffer[CD_MONTH_POS + 2] = 0;
        return;
    }
    
    if(monthNum < 1 || monthNum > 12) {
        monthNum = (monthNum > 12) ? 12 : 1;
    }

    _month = monthNum;

    monthNum--;
    _displayBuffer[CD_MONTH_POS]     = getLEDAlphaChar(months[monthNum][0]);
    _displayBuffer[CD_MONTH_POS + 1] = getLEDAlphaChar(months[monthNum][1]);
    _displayBuffer[CD_MONTH_POS + 2] = getLEDAlphaChar(months[monthNum][2]);
}

void clockDisplay::setDay(int dayNum)
{
    if(!dayNum) {
        _displayBuffer[CD_DAY_POS] = 0;
        return;
    }

    if(dayNum < 1 || dayNum > 31) {
        dayNum = (dayNum < 1) ? 1 : 31;
    }

    _day = dayNum;

    _displayBuffer[CD_DAY_POS] = makeNum(dayNum);
}

void clockDisplay::setYear(uint16_t yearNum)
{
    _year = yearNum;

    while(yearNum >= 10000)
        yearNum -= 10000;

    _displayBuffer[CD_YEAR_POS]     = makeNum(yearNum / 100);
    _displayBuffer[CD_YEAR_POS + 1] = makeNum(yearNum % 100);
}

void clockDisplay::setYearDigits(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4)
{
    uint16_t seg = 0;

    // Each position holds two digits
    // MSB = 1s, LSB = 10s

    seg = d2 ? getLED7NumChar(d2 - 1) << 8 : 0;
    seg |= d1 ? getLED7NumChar(d1 - 1) : 0;
    _displayBuffer[CD_YEAR_POS]     = seg;
    
    seg = d4 ? getLED7NumChar(d4 - 1) << 8 : 0;
    seg |= d3 ? getLED7NumChar(d3 - 1) : 0;
    _displayBuffer[CD_YEAR_POS + 1] = seg;
}

void clockDisplay::setHour(uint16_t hourNum)
{
    uint16_t seg = 0;
    
    if(!hourNum) {
        _displayBuffer[CD_HOUR_POS] = 0;
        return;
    }

    _hour = hourNum;

    if(!_mode24) {
        if(hourNum == 0) {
            hourNum = 12;
        } else if(hourNum > 12) {
            hourNum -= 12; 
        }
    }

    _displayBuffer[CD_HOUR_POS] = makeNum(hourNum);

    // AM/PM will be set on show() to avoid being overwritten
}

void clockDisplay::setHour12(uint16_t hourNum)
{
    if(!hourNum) {
        _displayBuffer[CD_HOUR_POS] = 0;
        return;
    }

    _displayBuffer[CD_HOUR_POS] = makeNum(hourNum - 1);
}

void clockDisplay::setMinute(int minNum)
{
    if(!minNum) {
        _displayBuffer[CD_MIN_POS] = 0;
        return;
    }

    minNum--;
    
    if(minNum < 0 || minNum > 59) {
        minNum = (minNum > 59) ? 59 : 0;
    }

    _minute = minNum;

    _displayBuffer[CD_MIN_POS] = makeNum(minNum);
}

void clockDisplay::setAMPM(int isPM)
{
    // -1 = off
    // 0  = am
    // 1  = pm
    _isPM = isPM;
}

void clockDisplay::setColon(bool col)
{
    // set true to turn it on
    // colon is on in night mode
    
    _colon = _nightmode ? true : col;
}

// Query data ------------------------------------------------------------------


uint8_t clockDisplay::getMonth()
{
    return _month;
}

uint8_t clockDisplay::getDay()
{
    return _day;
}

uint16_t clockDisplay::getYear()
{
    return _year;
}

uint8_t clockDisplay::getHour()
{
    return _hour;
}

uint8_t clockDisplay::getMinute()
{
    return _minute;
}

const char * clockDisplay::getMonthString(uint8_t mon)
{
    if(mon >= 1 && mon <= 12)
        return months[mon-1];
    else
        return nullStr;
}

// Put data directly on display (bypass buffer) --------------------------------


void clockDisplay::showMonthDirect(int monthNum, uint16_t dflags)
{
    clearDisplay();

    if(monthNum > 12)
        monthNum = 12;

    if(monthNum > 0) {
        monthNum--;
        directCol(CD_MONTH_POS,     getLEDAlphaChar(months[monthNum][0]));
        directCol(CD_MONTH_POS + 1, getLEDAlphaChar(months[monthNum][1]));
        directCol(CD_MONTH_POS + 2, getLEDAlphaChar(months[monthNum][2]));
    } else {
        directCol(CD_MONTH_POS,     0);
        directCol(CD_MONTH_POS + 1, 0);
        directCol(CD_MONTH_POS + 2, getLEDAlphaChar('_'));
    }
}

void clockDisplay::showDayDirect(int dayNum, uint16_t dflags)
{
    clearDisplay();

    directCol(CD_DAY_POS, makeNum(dayNum, dflags));
}

void clockDisplay::showYearDirect(int yearNum, uint16_t dflags)
{
    uint16_t seg = 0;
    int y100;

    clearDisplay();

    while(yearNum >= 10000) 
        yearNum -= 10000;

    y100 = yearNum / 100;

    if(!(dflags & CDD_NOLEAD0) || y100) {
        seg = makeNum(y100, dflags);
        if(y100) dflags &= ~CDD_NOLEAD0;
    }
    directCol(CD_YEAR_POS, seg);
    directCol(CD_YEAR_POS + 1, makeNum(yearNum % 100, dflags));
}

void clockDisplay::showHourDirect(int hourNum, uint16_t dflags)
{
    clearDisplay();

    // This assumes that CD_HOUR_POS is different to
    // CD_AMPM_POS

    if(!_mode24 && !(dflags & CDD_FORCE24)) {

        (hourNum > 11) ? directPM() : directAM();

        if(hourNum == 0) {
            hourNum = 12;
        } else if(hourNum > 12) {
            hourNum -= 12;
        }

    } else {

        directAMPMoff();

    }

    directCol(CD_HOUR_POS, makeNum(hourNum, dflags));
}

void clockDisplay::showMinuteDirect(int minuteNum, uint16_t dflags)
{
    clearDisplay();

    directCol(CD_MIN_POS, makeNum(minuteNum, dflags));
}


// Special purpose -------------------------------------------------------------


// Show the given text
void clockDisplay::showTextDirect(const char *text, uint16_t flags)
{
    int idx = 0, pos = CD_MONTH_POS;
    int temp = 0;

    _corr6 = (flags & CDT_CORR6) ? true : false;
    _withColon = (flags & CDT_COLON) ? true : false;

    while(text[idx] && pos < (CD_MONTH_POS+CD_MONTH_SIZE)) {
        directCol(pos++, getLEDAlphaChar(text[idx++]));
    }

    while(pos < CD_DAY_POS) {
        directCol(pos++, 0);
    }
    
    pos = CD_DAY_POS;
    while(text[idx] && pos <= CD_MIN_POS) {
        temp = getLED7AlphaChar(text[idx++]);
        if(text[idx]) {
            temp |= (getLED7AlphaChar(text[idx++]) << 8);
        }
        directCol(pos++, temp);
    }

    if(flags & CDT_CLEAR) {
        while(pos <= CD_MIN_POS) {
            directCol(pos++, 0);
        }
    }
    
    _corr6 = _withColon = false;
}


// Private functions ###########################################################

/*
 * Segment helpers
 */

// Returns bit pattern for provided value 0-9 or number provided as a char '0'-'9'
// for display on 7 segment display
uint8_t clockDisplay::getLED7NumChar(uint8_t value)
{
    if(value >= '0' && value <= '9') {
        return numDigs[value - 32];
    } else if(value <= 9) {
        return numDigs[value + '0' - 32];
    }
    return 0;
}
  
// Returns bit pattern for provided character for display on 7 segment display
uint8_t clockDisplay::getLED7AlphaChar(uint8_t value)
{
    if(value < 32 || value >= 127 + 2)
        return 0;
    
    if(value == '6' && _corr6)
        return numDigs[value - 32] | 0x01;
    
    return numDigs[value - 32];
}

// Returns bit pattern for provided character for display on 14 segment display
uint16_t clockDisplay::getLEDAlphaChar(uint8_t value)
{
    if(value < 32 || value >= 127)
        return 0;

    // For text, use common "6" pattern to conform with 7-seg-part
    if(value == '6' && _corr6)
        return alphaChars['6' - 32] | 0x0001;

    return alphaChars[value - 32];
}

// Make a 2 digit number from the array and return the segment data
// (makes leading 0s)
uint16_t clockDisplay::makeNum(uint8_t num, uint16_t dflags)
{
    uint16_t segments = 0;

    // Each position holds two digits
    // MSB = 1s, LSB = 10s

    segments = getLED7NumChar(num % 10) << 8;     
    if(!(dflags & CDD_NOLEAD0) || (num / 10)) {
        segments |= getLED7NumChar(num / 10);   
    }

    return segments;
}

// Directly write to a column with supplied segments
// (leave buffer intact, directly write to display)
void clockDisplay::directCol(int col, int segments)
{
    if((col == CD_YEAR_POS + 1) && _yearDot) {
        segments |= 0x8000;
    } else if((col == CD_YEAR_POS) && _withColon) {
        segments |= 0x8080;
    }
    Wire.beginTransmission(_address);
    Wire.write(col * 2);
    Wire.write(segments & 0xff);
    Wire.write(segments >> 8);
    Wire.endTransmission();
}

// Directly clear the display
void clockDisplay::clearDisplay()
{
    Wire.beginTransmission(_address);
    Wire.write(0x00);

    for(int i = 0; i < CD_BUF_SIZE*2; i++) {
        Wire.write(0x00);
    }

    Wire.endTransmission();
}

// Show the buffer
void clockDisplay::showInt(bool animate, bool Alt)
{
    int i = 0;
    uint16_t *db = _displayBuffer;

    if(animate) off();

    if(_isPM > 0)   PM();
    else if(!_isPM) AM();
    else            AMPMoff();

    (_colon) ? colonOn() : colonOff();

    Wire.beginTransmission(_address);
    Wire.write(0x00);

    if(animate) {
        for(i = 0; i < CD_DAY_POS; i++) {
            Wire.write(0x00);  // blank month
            Wire.write(0x00);
        }
    }

    for(; i < CD_BUF_SIZE; i++) {
        Wire.write(db[i] & 0xff);
        Wire.write(db[i] >> 8);
    }

    Wire.endTransmission();

    if(animate) on();
}

void clockDisplay::colonOn()
{
    _displayBuffer[CD_COLON_POS] |= 0x8080;
}

void clockDisplay::colonOff()
{
    _displayBuffer[CD_COLON_POS] &= 0x7F7F;
}

void clockDisplay::AM()
{
#ifndef REV_AMPM
    _displayBuffer[CD_AMPM_POS] |= 0x0080;
    _displayBuffer[CD_AMPM_POS] &= 0x7FFF;
#else
    _displayBuffer[CD_AMPM_POS] |= 0x8000;
    _displayBuffer[CD_AMPM_POS] &= 0xFF7F;
#endif
}

void clockDisplay::PM()
{
#ifndef REV_AMPM  
    _displayBuffer[CD_AMPM_POS] |= 0x8000;
    _displayBuffer[CD_AMPM_POS] &= 0xFF7F;
#else
    _displayBuffer[CD_AMPM_POS] |= 0x0080;
    _displayBuffer[CD_AMPM_POS] &= 0x7FFF;
#endif    
}

void clockDisplay::AMPMoff()
{
    _displayBuffer[CD_AMPM_POS] &= 0x7F7F;
}

void clockDisplay::directAMPM(int val1, int val2)
{
    Wire.beginTransmission(_address);
    Wire.write(CD_AMPM_POS * 2);
    Wire.write(val1 & 0xff);
    Wire.write(val2 & 0xff);
    Wire.endTransmission();
}

void clockDisplay::directAM()
{
#ifndef REV_AMPM  
    directAMPM(0x80, 0x00);
#else
    directAMPM(0x00, 0x80);
#endif    
}

void clockDisplay::directPM()
{
#ifndef REV_AMPM  
    directAMPM(0x00, 0x80);
#else
    directAMPM(0x80, 0x00);
#endif
}

void clockDisplay::directAMPMoff()
{
    directAMPM(0x00, 0x00);
}

void clockDisplay::directCmd(uint8_t val)
{
    Wire.beginTransmission(_address);
    Wire.write(val);
    Wire.endTransmission();
}
