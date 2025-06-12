#include <Arduino.h>
#include "receiver.h"
#include "I2S.h"
#include "ESP_COMM.h"
#include "LPF.h"
#include "FIFO.h"
#include <Ticker.h>

extern "C" {
    #include "esp_wifi.h"
}


/* Global Variables */
hw_timer_t *DAC_timer = NULL;
volatile uint8_t Prebuff_flag = 0x0;        // Low on Reset
volatile uint8_t DAC_overrun_flag = 0x0;    // Low on Reset
static uint8_t last_sample = 0;
static uint8_t interp_step = 0;
static uint8_t interp_delta = 0;

/* External Variables */
const uint8_t Receiver_pins[] = RECEIVER_PINS;


/* Functions */
// Handle Incoming Signal Data
void onReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
    // Ensure Samples are Valid and Even in Number
    if (len % 2 != 0)
        return;

    // Convert 16-Bit Data Samples to 8-Bit Output Data (Account for I2S Shift)
    for (size_t i = 0; i < len; i += 2) {
        uint16_t output_sample = (incomingData[(i + 1)] << 0x8) | incomingData[i];     // Reconstruct 16-Bit Left-Aligned Frame (Little-Endian)
        output_sample &= 0x0FFF;                                                       // Remove I2S Shift (Upper Four Bits)   

        // Smoothen Noise using Exponential Moving Average Filter
        static uint16_t prev_output = 0;
        output_sample = (7 * prev_output + output_sample) >> 3;                         // Weight: 87.5% old, 12.5% new
        if (output_sample <= 1) {
            output_sample = 0;
        }
        prev_output = output_sample;

        // Apply Low-Pass Filter to Remove Harmonics
        uint8_t filtered_sample = apply_fir_filter((uint8_t)output_sample);

        // Store Sample into FIFO Queue
        push_Sample(filtered_sample);

        // Enable DAC Timer once Sufficient Pre-Buffer Size has been Reached
        if (!Prebuff_flag && (sampleCount() >= (PREBUFF_PACKET_LEN * I2S_DMA_BUFF_LEN))) {
            Prebuff_flag = 0x1;
            timerAlarmEnable(DAC_timer);
            Serial.println("Pre-buffering complete...starting DAC output.");
        }

        // Re-enable DAC Timer once Overrun is Resolved
        if (DAC_overrun_flag && (sampleCount() >= (PREBUFF_OVERRUN_REFRESH * I2S_DMA_BUFF_LEN))) {
            DAC_overrun_flag = 0x0;
            timerAlarmEnable(DAC_timer);
            Serial.println("Pre-buffering resolving from overrun...starting DAC output.");
        }
    }
}


// Output Filtered Sample through DAC (Occurs every 23us / sample = 44.1kHz)
void IRAM_ATTR onDACClock() {
    // Blink LED to Indicate Transmission
    digitalWrite(DEBUG_PIN, !digitalRead(DEBUG_PIN));  

    // Hold if Pre-Buffer is not yet Filled
    if (!Prebuff_flag)
        return;

    uint8_t sample;

    // Check if there is Data in Queue
    if (pop_Sample(&sample)) {
        dacWrite(Receiver_pins[0], sample);

        // Blink LED to Indicate Transmission
        digitalWrite(22, !digitalRead(22));  
    } 
    // Temporarily Disable DAC Timer if FIFO Buffer Becomes Full After Running
    else if (Prebuff_flag) {
        DAC_overrun_flag = 0x1;
        timerAlarmDisable(DAC_timer);
        interp_step++;

        // Perform 2-step linear interpolation only
        if (interp_step <= 2) {
            uint8_t interpolated = last_sample + interp_step * interp_delta;
            dacWrite(Receiver_pins[0], interpolated);
        } else if (!isEmpty()) {
            // Recovery: if data returns, stop interpolating
            DAC_overrun_flag = 0x0;
            timerAlarmEnable(DAC_timer);     // Optional if already running
        }
    }
}


/* Initial Setup*/
void setup(void) {
    // LED Used for Debugging
    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, LOW);
    pinMode(22, OUTPUT);
    digitalWrite(22, LOW);

    Serial.begin(115200);
    Serial.println("Receiver initialized.");

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

    // Register Receive Function to Handle Input
    esp_now_register_recv_cb(onReceive);
    Serial.println("Receiver ready for data.");

    // Initialize DAC Timer (Do Not Enable until Sufficient Pre-Buffered Sample Size)
    DAC_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(DAC_timer, &onDACClock, true);
    timerAlarmWrite(DAC_timer, DAC_TIMER_INTERVAL_US, true);  // ~44.1kHz
}


/* Main Function */
void loop(void) {

    static uint32_t underruns = 0;
if (DAC_overrun_flag) {
    underruns++;
    Serial.printf("Underrun #%lu at %lu ms\n", underruns, millis());
    delay(100);  // debounce message
}


}