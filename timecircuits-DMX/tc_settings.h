/*
 * -------------------------------------------------------------------
 * CircuitSetup.us Time Circuits Display - DMX-controlled
 * (C) 2024 Thomas Winischhofer (A10001986)
 * All rights reserved.
 * -------------------------------------------------------------------
 */

#ifndef _TC_SETTINGS_H
#define _TC_SETTINGS_H

extern bool haveFS;
extern bool haveSD;

void settings_setup();

void unmount_fs();

void formatFlashFS();


#if 0
bool readFileFromSD(const char *fn, uint8_t *buf, int len);
bool writeFileToSD(const char *fn, uint8_t *buf, int len);
bool readFileFromFS(const char *fn, uint8_t *buf, int len);
bool writeFileToFS(const char *fn, uint8_t *buf, int len);

#include <FS.h>
bool   openACFile(File& file);
size_t writeACFile(File& file, uint8_t *buf, size_t len);
void   closeACFile(File& file);
void   removeACFile();

#endif
#endif
