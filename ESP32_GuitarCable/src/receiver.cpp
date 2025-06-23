#include <Arduino.h>
#include "receiver.h"
#include "I2S.h"
#include "ESP_COMM.h"
#include "LPF.h"
#include "FIFO.h"
#include "MCP4921.h"
#include "esp_task_wdt.h"
#include <Ticker.h>

extern "C" {
    #include "esp_wifi.h"
}


/* Global Variables */
// DAC Timer Handlers
hw_timer_t *DAC_timer = NULL;
volatile uint8_t DAC_overrun_flag = 0x0;    // Low on Reset
volatile uint8_t DAC_underrun_flag = 0x0;   // Low on Reset
static uint8_t DAC_timer_flag = 0x0;        // Low on Reset
// Interpolation Handlers
static uint16_t prev_output = 0;
static uint8_t interp_step = 0;
static uint8_t interp_delta = 0;

/* External Variables */


/* Functions */
// Handle Incoming Signal Data
void onReceive(const uint8_t *mac, const uint8_t *incomingData, int byte_len) {
    static uint32_t packet_cnt = 0;

    // Ensure Samples are Valid and Even in Number
    if (byte_len % 2 != 0) return;

    // Convert 16-Bit Data Samples to 12-Bit Output Data (Account for I2S Shift)
    for (size_t i = 0; i < (byte_len - 1); i += 2) {
        uint16_t output_sample = (incomingData[(i + 1)] << 0x8) | incomingData[i];     // Reconstruct 16-Bit Left-Aligned Frame (Little-Endian)
        output_sample &= 0x0FFF;                                                       // Remove I2S Shift (Upper Four Bits)   
        printf("Packet #%d: DAC[%d] = 0x%04X\n", packet_cnt, (i / 2), output_sample);

        // Smoothen Noise using Exponential Moving Average Filter
        output_sample = (7 * prev_output + output_sample) >> 3;                         // Weight: 87.5% old, 12.5% new
        if (output_sample <= 1) {
            output_sample = 0;
        }
        interp_delta = output_sample - prev_output;
        prev_output = output_sample;

        // Apply Low-Pass Filter to Remove Harmonics
        static uint16_t filtered_sample = apply_fir_filter(output_sample);

        // Store Sample into FIFO Queue if Space is Available
        if (!DAC_overrun_flag) 
            push_Sample(filtered_sample);

        // Enable DAC Timer once Sufficient Pre-Buffer Size has been Reached
        if (!DAC_overrun_flag && (sampleCount() >= (PREBUFF_PACKET_LEN * I2S_DMA_BUFF_LEN))) {
            DAC_overrun_flag = 0x1;
            timerAlarmEnable(DAC_timer);
        }

        // Re-enable DAC Timer once Underrun is Resolved
        if (DAC_underrun_flag && (sampleCount() >= (PREBUFF_OVERRUN_REFRESH * I2S_DMA_BUFF_LEN))) {
            DAC_underrun_flag = 0x0;
            timerAlarmEnable(DAC_timer);
        }
    }
    packet_cnt++;
}


// Output Filtered Sample through DAC (Occurs every 23us / sample = 44.1kHz)
void IRAM_ATTR onDACClock() {
    // Toggle Flag
    DAC_timer_flag = 0x1;
}


/* Initial Setup*/
void setup(void) {
    Serial.begin(115200);
    Serial.println("Receiver initialized.");
    
    // DEBUG PINS
    pinMode(DEBUG_PIN, OUTPUT);
    pinMode(DEBUG_PIN2, OUTPUT);
    digitalWrite(DEBUG_PIN, LOW);
    digitalWrite(DEBUG_PIN2, LOW);

    // Setup LED Alert to Confirm Connection
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Serial.println(WiFi.macAddress());

    // Check if MAC Address Matches Receiver
    if (WiFi.macAddress() == "8C:4F:00:16:9E:34") {
        Serial.println("Receiver MAC match confirmed.");
    } else {
        Serial.println("Receiver MAC does not match!");
    }

    // Initialize ESP NOW Protocol for Receiver
    WiFi.mode(WIFI_STA);
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_LR);   // Enable Long-Range Mode
    esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);  // Use Channel 6, No Secondary
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed!");
        return;
    }
    esp_task_wdt_init(5, true);  // 5 second timeout, panic=true
    esp_task_wdt_add(NULL);      // Add current task to WDT

    // Register Receive Function to Handle Input
    esp_now_register_recv_cb(onReceive);
    //digitalWrite(LED_PIN, HIGH);
    Serial.println("Receiver ready for data.");

    // Initialize MCP4921 and DAC Timer (Do Not Enable until Sufficient Pre-Buffered Sample Size)
    setupMCP4921();
    DAC_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(DAC_timer, &onDACClock, true);
    timerAlarmWrite(DAC_timer, DAC_TIMER_INTERVAL_US, true);  // ~44.1kHz
    Serial.println("Receiver Initialization Complete... outputting Data.");
}


/* Main Function */
void loop(void) {
    esp_task_wdt_reset();  // Feed the watchdog


    /* ----------------------------------- */


    if (DAC_overrun_flag) {
        digitalWrite(DEBUG_PIN, HIGH);
    } else {
        digitalWrite(DEBUG_PIN, LOW);
    }

    if (DAC_underrun_flag) {
        digitalWrite(DEBUG_PIN2, HIGH);
    } else {
        digitalWrite(DEBUG_PIN2, LOW);
    }


    /* ----------------------------------- */


    // Handle DAC Timer ISR
    if (DAC_timer_flag) {
        static uint16_t sample;
        // Check if there is Data in Queue
        if (pop_Sample(&sample)) {
            writeToMCP4921(sample);
            analogWrite(LED_PIN, sample);
        }
        // Temporarily Stop Data Pushing if FIFO Buffer Overruns; Interpolate Lost Data
        else if (isFull()) {
            DAC_overrun_flag = 0x1;
            interp_step++;

            // Perform 2-step Linear Interpolation
            if (interp_step <= 2) {
                uint8_t interpolated = prev_output + ((interp_step * interp_delta) / 2);
                writeToMCP4921(interpolated);
            } else if (!isEmpty()) {
                // Recovery: if Data Returns, Stop Interpolating
                DAC_overrun_flag = 0x0;
            }
        } 
        // Temporarily Stop DAC Timer if FIFO Buffer Underruns; Fill Larger Pre-Buffer
        else {
            DAC_underrun_flag = 0x1;
            timerAlarmDisable(DAC_timer);
        }

        // Toggle DAC Timer Flag
        DAC_timer_flag = 0x0;
    }


    /* ----------------------------------- */


    
}