#include <Arduino.h>
#include "transmitter.h"
#include "I2S.h"
#include "ESP_COMM.h"
#include <Ticker.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>


/* Global Variables */
const uint8_t ESP_Now_Peer_MAC[6] = {0x8C, 0x4F, 0x00, 0x16, 0x9E, 0x34};
const uint8_t Transmitter_pins[] = TRANSMITTER_PINS;
volatile uint8_t DMA_flag = 0x1;        // High on Reset
uint16_t *read_ptr = DMA_ping_buff, *transmit_ptr = DMA_pong_buff;
uint16_t RTOS_buff[I2S_DMA_BUFF_LEN];
volatile uint8_t TaskBuff_flag = 0x0;  // Low on Reset
QueueHandle_t packet_send_queue;
hw_timer_t *Transmit_timer = NULL;
static uint8_t Timer_flag = 0x0;       // Low on Reset


/* Functions */
// Transmit Data Packet to Peer MCU Synchronously (Occurs every 454us / 20 samples = 44.1kHz)
void IRAM_ATTR sendPacket() {
  static uint8_t Transmit_flag = 0;

  // Wait until DMA Buffer is Ready for Transfer
  if (TaskBuff_flag) {
    // Toggle Flag
    TaskBuff_flag = 0x0;

    // Send Task to Queue if Space is Available
    if (uxQueueSpacesAvailable(packet_send_queue) > 0) {
      static uint8_t flag = 0;
      xQueueSendFromISR(packet_send_queue, &flag, NULL);
    }
  }

  digitalWrite(DEBUG_PIN, !digitalRead(DEBUG_PIN));  
}


// ESP-Now Transmission Offloader (ADC Timer ISR cannot handle ESP-Now Tasks)
void packetSendTask(void *param) {
  // Handle Task
  uint8_t Transmit_flag;

  while(1) {
    if (xQueueReceive(packet_send_queue, &Transmit_flag, portMAX_DELAY)) {
      // Copy DMA Buffer to Respective Task Buffer to Avoid Illegal Manipulation
      memcpy(RTOS_buff, transmit_ptr, BYTES_READ);

      // Transmit Data from Task Buffer
      esp_now_send(ESP_Now_Peer_MAC, (uint8_t*) RTOS_buff, BYTES_READ);
      
      // // Block Other Tasks to Allow ESP-Now to Transmit
      // vTaskDelay(1);

      // Blink LED to Indicate Transmission
      // static uint32_t last_toggle_time = 0;
      // static int counter = 0;
      // if (++counter >= 10) {
      //     uint32_t now = micros();
      //     Serial.printf("Toggled at %lu µs, Δ = %lu µs\n", now, now - last_toggle_time);
      //     last_toggle_time = now;

      //     digitalWrite(DEBUG_PIN, !digitalRead(DEBUG_PIN));
      //     counter = 0;
      // }

      

      // for (int i = 0; i < (BYTES_READ / sizeof(uint16_t)); i += 2) {
      //   uint16_t sample = (RTOS_buff[i + 1] << 0x8) | RTOS_buff[i];  // Little-endian
      //   uint16_t adc_value = sample >> 4;                                  // Extract actual 12-bit ADC reading
      //   Serial.printf("Transmit: Raw: 0x%04X | Aligned: 0x%03X\n", sample, adc_value);
      // }
    }
  }
}


// Initialize Serial, FreeRTOS, I2S Communication, ESP-Now Communication, and Hardware Timer
void setup(void) {
  // LED Used for Debugging
  pinMode(DEBUG_PIN, OUTPUT);
  digitalWrite(DEBUG_PIN, LOW);

  Serial.begin(115200);
  Serial.println("Transmitter initialized.");

  packet_send_queue = xQueueCreate(512, sizeof(uint8_t));
  xTaskCreatePinnedToCore(packetSendTask, "ESP_Now Sender", 4096, NULL, 1, NULL, 1);
  Serial.println("FreeRTOS initialized.");

  setupI2S(Transmitter_pins);

  setupESPNow();

  Transmit_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(Transmit_timer, &sendPacket, true);
  timerAlarmWrite(Transmit_timer, ADC_TIMER_INTERVAL_US, true);
}


// Read and Transmit Guitar Signal Data
void loop(void) {
  // Sample Guitar Data and Store to DMA Buffer, then Transmit Data Once Full; Refill
  static size_t bytes_read, bytes_total = 0;
  esp_err_t i2s_result = i2s_read(I2S_NUM_0, read_ptr, (I2S_DMA_BUFF_LEN * sizeof(uint16_t)), &bytes_read, portMAX_DELAY);
  // Serial.printf("i2s_read -> result=%d, bytes_read=%d\n", i2s_result, bytes_read);

  // Toggle Flag to Allow Task Transmission
  TaskBuff_flag = 0x1;

  // Transfer Stored Data from Ping Buffer to Pong Buffer
  if (DMA_flag) {
    read_ptr = DMA_pong_buff;
    transmit_ptr = DMA_ping_buff;
  } else {
    read_ptr = DMA_ping_buff;
    transmit_ptr = DMA_pong_buff;
  }

  // Toggle DMA Flag
  DMA_flag ^= 0x1;

  // Fully Enable Hardware Timer to Activate Transmission Once System is Stable
  if (!Timer_flag && TaskBuff_flag == 1) {
    Timer_flag = 0x1;
    Serial.println("Enabling Transmit Timer after system is stable...");
    timerAlarmEnable(Transmit_timer);
    Serial.println("Transmit Timer initialized.");
  }
  //Serial.printf("Task flag: %d | Timer flag: %d\n", TaskBuff_flag, Timer_flag);
}
