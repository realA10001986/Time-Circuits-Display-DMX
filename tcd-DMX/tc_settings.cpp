/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Time Circuits Display - DMX-controlled
 * (C) 2024 Thomas Winischhofer (A10001986)
 * All rights reserved.
 * -------------------------------------------------------------------
 */

#include "tc_global.h"

#include <SD.h>
#include <SPI.h>
#include <FS.h>

#include <Update.h>

#include "tc_settings.h"
#include "tc_dmx.h" 

static const char *fwfn = "/tcdfw.bin";     //"/tcd-DMX.ino.nodemcu-32s.bin";
static const char *fwfnold = "/tcdfw.old";  //"/tcd-DMX.ino.nodemcu-32s.old";

static bool haveSD = false;

static bool firmware_update();
static void unmount_fs();

/*
 * settings_setup()
 * 
 * Mount SD (if available) and update firmware if available
 * 
 */
void settings_setup()
{
    const char *funcName = "settings_setup";
    bool SDres = false;
    
    // Set up SD card
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);

    haveSD = false;
    
    #ifdef TC_DBG
    Serial.printf("%s: Mounting SD... ", funcName);
    #endif

    if(!(SDres = SD.begin(SD_CS_PIN, SPI, 16000000))) {
        #ifdef TC_DBG
        Serial.printf("Retrying at 25Mhz... ");
        #endif
        SDres = SD.begin(SD_CS_PIN, SPI, 25000000);
    }

    if(SDres) {

        #ifdef TC_DBG
        Serial.println(F("ok"));
        #endif

        uint8_t cardType = SD.cardType();
       
        #ifdef TC_DBG
        const char *sdTypes[5] = { "No card", "MMC", "SD", "SDHC", "unknown (SD not usable)" };
        Serial.printf("SD card type: %s\n", sdTypes[cardType > 4 ? 4 : cardType]);
        #endif

        haveSD = ((cardType != CARD_NONE) && (cardType != CARD_UNKNOWN));

    } else {

        Serial.println(F("No SD card found"));

    }

    if(haveSD) {
        if(SD.exists(fwfn)) {
            destinationTime.showTextDirect("UPDATING");
            if(!firmware_update()) {
                destinationTime.showTextDirect("ERROR");
                delay(5000);
            }
        }

        unmount_fs();
    }
}


static bool firmware_update()
{
    uint32_t maxSketchSpace = UPDATE_SIZE_UNKNOWN;
    uint8_t  buf[1024];
    bool     error = false;
    size_t   s;
    
    File myFile = SD.open(fwfn, FILE_READ);
    
    if(!myFile) {
        Serial.println("Failed to open firmware file");
        return false;
    }
    
    if(!Update.begin(maxSketchSpace)) {
        
        Serial.printf("Firmware update error %d\n", Update.getError());
        
        Update.end();
        return false;
    }

    while((s = myFile.read(buf, 1024))) {
        if(Update.write(buf, s) != s) {
            Serial.printf("Firmware update write error %d\n", Update.getError());
            error = true;
            break;
        }
    }

    if(!error) {
        Update.end(true);
        if(!Update.hasError()) {
            SD.remove(fwfnold);
            SD.rename(fwfn, fwfnold);
            unmount_fs();
            destinationTime.showTextDirect("DONE");
            delay(3000);
            ESP.restart();
        } else {
            Serial.printf("Firmware update error %d\n", Update.getError());
        }
    } else {
        Update.abort();
    }

    myFile.close();

    return false;
}    

static void unmount_fs()
{
    if(haveSD) {
        SD.end();
        #ifdef TC_DBG
        Serial.println(F("Unmounted SD card"));
        #endif
        haveSD = false;
    }
}
