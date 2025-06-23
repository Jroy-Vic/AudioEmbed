#include <Arduino.h>
#include "MCP4921.h"
#include <SPI.h>


/* Global Variables */
uint8_t SPI_rdy_flag = 0x0;     // Low on Reset


/* Functions */
// Setup DAC
void setupMCP4921(void) {
    // Initialize SPI with VSPI Pins on MCU
    SPI.begin();
    SPI_rdy_flag = 0x1;

    // Initialize Chip Select to Active Low
    pinMode(DAC_CS, OUTPUT);
    digitalWrite(DAC_CS, HIGH);

    Serial.println("MCP4921 DAC Initialized.");
}


// DAC Output -
/* Configuration Bits (Bits 15 - 12): 0101 
    Bit 15: Write to DAC A (0)
    Bit 14: Buffered (1)
    Bit 13: 2x Output Gain (0)
    Bit 12: Output Power Down On (1) */
void writeToMCP4921(uint16_t sample) {
    // Prevent Early Crash Before SPI is Ready
    if (!SPI_rdy_flag) return;

    // Handle Data Frame
    const uint16_t config_frame = 0x5000;
    uint16_t output_frame = config_frame | sample;

    // Pull Chip Select Low (Activate)
    digitalWrite(DAC_CS, LOW);

    // Handle SPI Communication to MCP4921
    SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    SPI.transfer16(output_frame);
    SPI.endTransaction();

    // Pull Chip Select High (De-Activate)
    digitalWrite(DAC_CS, HIGH);
}