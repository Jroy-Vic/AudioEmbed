#include <Arduino.h>
#include "receiver.h"
#include "I2S.h"
#include "ESP_COMM.h"
#include "LPF.h"
#include "FIFO.h"
#include "MCP4921.h"
#include <Ticker.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

extern "C" {
    #include "esp_wifi.h"
}


/* Global Variables */
// RTOS Task Handlers
QueueHandle_t xRxQueue = nullptr;
TaskHandle_t xDACOutputTaskHandle = NULL;
TaskHandle_t xPacketProcessTaskHandle = NULL;
// DAC Timer Handlers
hw_timer_t *DAC_timer = NULL;
volatile uint8_t DAC_underrun_flag = 0x0;   // Low on Reset
volatile uint8_t DAC_EN_flag = 0x0;         // Low on Reset
// Interpolation Handlers
volatile uint16_t prev_output = 0;
volatile uint8_t interp_step = 0;
volatile uint8_t interp_delta = 0;


/* External Variables */


/* Functions */
// Handle Incoming Signal Data
void onReceive(const uint8_t *mac, const uint8_t *incomingData, int byte_len) {
    // Toggle LED Alert to Confirm Connection
    static uint32_t DMA_SentPacket_Count;
    if (++DMA_SentPacket_Count == 100) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        DMA_SentPacket_Count = 0;
    }

    // Ensure Samples are Valid
    if (byte_len > RX_QUEUE_ITEM_SIZE) {
        return;
    }

    // Make Copy of Incoming Packet Data and Push to RTOS Queue
    uint8_t *packet = (uint8_t*) heap_caps_malloc(byte_len, MALLOC_CAP_DMA);
    if (packet == nullptr) {
        Serial.println("[FATAL] Failed to allocate memory for incoming packet!");
        while(1);
    }
    memcpy(packet, incomingData, byte_len);

    RXPacket_t packet_data = {
        .data = packet,
        .len = byte_len
    };  

    if (xQueueSend(xRxQueue, &packet_data, portMAX_DELAY) != pdTRUE) {
        Serial.println("[FATAL] Failed to enqueue incoming packet!");
        heap_caps_free(packet);
        return;
    }
}


// Process Incoming Data Packet and Push to FIFO Queue
void packetProcessTask(void *param) {
    RXPacket_t packet_data;

    while(1) {
        // Wait for Incoming Packet, then Process into 16-Bit Samples
        if (xQueueReceive(xRxQueue, &packet_data, portMAX_DELAY) == pdTRUE) {
            for (uint32_t i = 0; i < packet_data.len; i += 2) {
                // Ensure Packet Length is Valid
                if (i + 1 >= packet_data.len) {
                    break;
                }
                
                // Parse Incoming Data into 16-Bit Samples
                uint16_t output_sample = (packet_data.data[(i + 1)] << 0x8) | packet_data.data[i];     // Reconstruct 16-Bit Left-Aligned Frame (Little-Endian)
                output_sample &= 0x0FFF;                                                               // Remove I2S Shift (Upper Four Bits) 
                
                // Smoothen Noise using Exponential Moving Average Filter
                output_sample = (7 * prev_output + output_sample) >> 3;                                // Weight: 87.5% old, 12.5% new
                if (output_sample <= 1) {
                    output_sample = 0;
                }
                interp_delta = output_sample - prev_output;
                prev_output = output_sample;

                // Apply Low-Pass Filter to Remove Harmonics
                static uint16_t filtered_sample;
                filtered_sample = apply_fir_filter(output_sample);                                

                // Store Sample into FIFO Queue if Space is Available (Otherwise, Overrun has Occurred!)
                if (!(isFull())) {
                    if (!(push_Sample(filtered_sample))) {
                        Serial.println("[RX] FIFO Overrun: Sample dropped");
                    }
                }

                // Enable DAC Timer once Sufficient Pre-Buffer Size has been Reached
                if (!DAC_EN_flag && (sampleCount() >= (PREBUFF_PACKET_LEN * I2S_DMA_BUFF_LEN))) {
                    DAC_EN_flag = 0x1;
                    timerAlarmEnable(DAC_timer);
                    Serial.println("Receiver Initialization Complete... outputting Data.");
                }

                // Re-enable DAC Timer once Underrun is Resolved
                if (DAC_underrun_flag && (sampleCount() >= (PREBUFF_OVERRUN_REFRESH * I2S_DMA_BUFF_LEN))) {
                    DAC_underrun_flag = 0x0;
                    timerAlarmEnable(DAC_timer);
                }
            }

            // Once Packet is Processed, Free Memory
            heap_caps_free(packet_data.data);
        }
    }
}


// Output Filtered Sample through DAC at 44.1kHz
void dacOutputTask(void *param) {
    uint16_t sample = 0;

    while(1) {
        // Wait for DAC Timer Interrupt Flag
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

         // Check if there is Data in Queue
         if (pop_Sample(&sample)) {
            writeToMCP4921(sample);

            // Toggle Debug Pin2 to Show DAC Output
            digitalWrite(DEBUG_PIN2, !digitalRead(DEBUG_PIN2));
        } else {
            // Temporarily Stop DAC Timer if FIFO Buffer Underruns; Fill Larger Pre-Buffer
            DAC_underrun_flag = 0x1;
            timerAlarmDisable(DAC_timer);
        }
    }
}


// Output Filtered Sample through DAC (Occurs every 23us / sample = 44.1kHz)
void IRAM_ATTR onDACClock() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(xDACOutputTaskHandle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }

    // Toggle Debug Pin to Show DAC Timer Trigger
    digitalWrite(DEBUG_PIN, !digitalRead(DEBUG_PIN));
}


/* Initialize Serial Monitor, Debug Pins, ESP-Now, RTOS, and DAC */
void setup(void) {
    // 3.095 Second Delay for Debugging
    delay(3095);

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

    // Check if MAC Address Matches Receiver
    Serial.println(WiFi.macAddress());
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
        while(1);
    }

    // Register Receive Function to Handle Input
    esp_now_register_recv_cb(onReceive);
    digitalWrite(LED_PIN, HIGH);
    Serial.println("ESP-NOW Communication Initialized.");

    // Initialize RTOS Task Handlers
    xRxQueue = xQueueCreate(RX_QUEUE_LEN, sizeof(RXPacket_t));
    if (xRxQueue == nullptr) {
        Serial.println("[FATAL] Failed to create xRxQueue!");
        while(1);
    }
    xTaskCreatePinnedToCore(packetProcessTask, "PacketProcessing", 4096, NULL, 3, &xPacketProcessTaskHandle, 1);
    xTaskCreatePinnedToCore(dacOutputTask, "DACOutput", 2048, NULL, 2, &xDACOutputTaskHandle, 1);

    // 3 Second Delay for Debugging
    delay(3000);
    digitalWrite(LED_PIN, LOW);

    // Initialize MCP4921 and DAC Timer (Do Not Enable until Sufficient Pre-Buffered Sample Size)
    setupMCP4921();
    DAC_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(DAC_timer, &onDACClock, true);
    timerAlarmWrite(DAC_timer, DAC_TIMER_INTERVAL_US, true);
}


/* Main Function */
void loop(void) {}