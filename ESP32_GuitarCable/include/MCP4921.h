#ifndef __MCP4921_H
#define __MCP4921_H


#include <Arduino.h>
#include <SPI.h>


/* MACROS */
#define DAC_CS 5    // CS


/* Functions */
// Setup DAC
void setupMCP4921(void);

// DAC Output
void writeToMCP4921(uint16_t sample);

#endif