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
// DMA and ADC Handlers
uint16_t *read_ptr = DMA_ping_buff, *transmit_ptr = DMA_pong_buff;
uint16_t RTOS_buff[I2S_DMA_BUFF_LEN];
static uint8_t DMA_flag = 0x1;                    // High on Reset
// RTOS Handlers
static uint8_t ADC_timer_flag = 0x0;              // Low on Reset
QueueHandle_t packet_send_queue;
hw_timer_t *Transmit_timer = NULL;
// ESP-Now Handlers
static uint8_t Transmission_ready_flag = 0x1;     // High on Reset
static uint8_t Packet_pending_flag = 0x0;         // Low on Reset
// Circular FIFO Handlers
static uint16_t Transmission_Packet_buff[MAX_PENDING_PACKETS][I2S_DMA_BUFF_LEN];
static volatile uint8_t Transmission_FIFO_head = 0;
static volatile uint8_t Transmission_FIFO_tail = 0;
static uint8_t Transmission_SentPacket_Count = 0;
static uint16_t DMA_Packet_buff[MAX_DMA_PACKETS][I2S_DMA_BUFF_LEN];
static uint16_t *DMA_packet_ptr;
static volatile uint8_t DMA_FIFO_head = 0;
static volatile uint8_t DMA_FIFO_tail = 0;
static uint8_t DMA_SentPacket_Count = 0;


/* External Variables */


/* Functions */
// Transmit Data Packet to Peer MCU Synchronously (Occurs every 454us / 20 samples = 44.1kHz)
void IRAM_ATTR sendPacket() {
  // Toggle ADC Timer Flag
  ADC_timer_flag = 0x1;
}


// ESP-Now Transmission Offloader (ADC Timer ISR cannot handle ESP-Now Tasks)
void packetSendTask(void *param) {
  // Handle Task
  uint8_t Transmit_flag;
  static uint32_t packet_cnt = 0;
  static uint32_t last_Tx_time = 0;

  while(1) {
    if (xQueueReceive(packet_send_queue, &Transmit_flag, portMAX_DELAY)) {
      // Copy DMA Buffer to Respective Task Buffer to Avoid Illegal Manipulation
      memcpy(RTOS_buff, DMA_Packet_buff[DMA_FIFO_tail], BYTES_READ);
      Serial.printf("DMA FIFO (RTOS): head=%d, tail=%d\n", DMA_FIFO_head, DMA_FIFO_tail);
      Serial.printf("Transmit from: %p\n", DMA_Packet_buff[DMA_FIFO_tail]);

      // for (int i = 0; i < I2S_DMA_BUFF_LEN; ++i) {
      //   Serial.printf("Packet #%d: RTOS_BUFF[%d] = 0x%04X\n", packet_cnt, i, RTOS_buff[i]);
      // }

      // Pop Old Packet Data from DMA FIFO Buffer
      DMA_FIFO_tail = (DMA_FIFO_tail + 1) % MAX_DMA_PACKETS;

      // Push Packet Data to FIFO Buffer in Case of Communication Error
      static uint16_t *packet_ptr;
      packet_ptr = Transmission_Packet_buff[Transmission_FIFO_head];
      memcpy(packet_ptr, RTOS_buff, BYTES_READ);
      Transmission_FIFO_head = (Transmission_FIFO_head + 1) % MAX_PENDING_PACKETS;

      // If FIFO is Full, Block New Packets from Queueing
      if (Transmission_FIFO_head == Transmission_FIFO_tail) {
        Serial.println("Warning: FIFO overrun, dropping oldest packet");
        continue;
      }
    }

    // Begin Transmitting Packets / Transmit Oldest Packet if Ready
    if (Transmission_ready_flag && ((Transmission_FIFO_head != Transmission_FIFO_tail) || Transmission_SentPacket_Count++ == 0)) {
      // Toggle Transmission Ready Flag for Next Transmission until Previous Send was Successful
      Transmission_ready_flag = 0x0;

      // Transmit Data from Task Buffer
      esp_err_t result = esp_now_send(ESP_Now_Peer_MAC, (uint8_t*) Transmission_Packet_buff[Transmission_FIFO_tail], BYTES_READ);
      // vTaskDelay(pdMS_TO_TICKS(2));
      // for (int i = 0; i < I2S_DMA_BUFF_LEN; ++i) {
      //   Serial.printf("Packet #%d: ADC[%d] = 0x%04X\n", packet_cnt, i, Transmission_Packet_buff[Transmission_FIFO_tail][i]);
      // }
      // packet_cnt++;

      if (result != ESP_OK) {
        Serial.printf("esp_now_send() failed: 0x%X (%d)\n", result, result);
        if (result == ESP_ERR_ESPNOW_NOT_INIT)
          Serial.println("ESP-NOW not initialized");
        if (result == ESP_ERR_ESPNOW_ARG)
          Serial.println("Invalid argument");
        if (result == ESP_ERR_ESPNOW_INTERNAL)
          Serial.println("Internal error");
        if (result == ESP_ERR_ESPNOW_NO_MEM)
          Serial.println("Out of memory");
        if (result == ESP_ERR_ESPNOW_NOT_FOUND)
          Serial.println("Peer not found");
      }

      // Pop Old Packet Data from FIFO Buffer
      Transmission_FIFO_tail = (Transmission_FIFO_tail + 1) % MAX_PENDING_PACKETS;

      // Record Total Transmit Time
      last_Tx_time = millis();
    }

    if ((millis() - last_Tx_time) > 500) {
      Serial.println("TX stalled! Forcing packet dequeue.");
      Transmission_FIFO_tail = (Transmission_FIFO_tail + 1) % MAX_PENDING_PACKETS;
      Transmission_ready_flag = 1;
      last_Tx_time = millis(); // Reset timer so it doesn't repeatedly trigger
    }
  }
}


// ESP-Now Transmission Callback Status
void Transmission_Status(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Toggle LED to show Connectivity Status (LED off if disconnected at any time)
  static int fail_cnt = 0;
  if (status == ESP_NOW_SEND_FAIL) {
    if (++fail_cnt > 3) {
        Serial.println("Too many ESP-NOW failures. Skipping packet.");
        Transmission_FIFO_tail = (Transmission_FIFO_tail + 1) % MAX_PENDING_PACKETS;
        fail_cnt = 0;
    } else {
        // Retry same packet next time
        Transmission_ready_flag = 1;
    }
  } else {
    // If Successful, Prepare for Next Transmission
    Transmission_ready_flag = 0x1;

    // Reset Fail Counter
    fail_cnt = 0;
  }

  
}


// Initialize Serial, FreeRTOS, I2S Communication, ESP-Now Communication, and Hardware Timer
void setup(void) {
  // LED Used for Debugging
  pinMode(DEBUG_PIN, OUTPUT);
  pinMode(DEBUG_PIN2, OUTPUT);
  digitalWrite(DEBUG_PIN, LOW);
  digitalWrite(DEBUG_PIN2, LOW);

  Serial.begin(115200);
  Serial.println("Transmitter initialized.");

  packet_send_queue = xQueueCreate(512, sizeof(uint8_t));
  xTaskCreatePinnedToCore(packetSendTask, "ESP_Now Sender", 4096, NULL, 1, NULL, 1);
  Serial.println("FreeRTOS initialized.");

  setupI2S(Transmitter_pins);

  setupESPNow();
  esp_now_register_send_cb(Transmission_Status);

  Transmit_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(Transmit_timer, &sendPacket, true);
  timerAlarmWrite(Transmit_timer, ADC_TIMER_INTERVAL_US, true);
  Serial.println("Transmitter Initialization Complete... ready to read Data.");

  // Fully Enable Hardware Timer to Activate Transmission Once System is Stable
  delay(1);
  static uint8_t Timer_flag = 0x0;
  Serial.println("Enabling Transmit Timer after system is stable...");
  timerAlarmEnable(Transmit_timer);
  Serial.println("Transmit Timer initialized... reading Data.");
}


// Read and Transmit Guitar Signal Data
void loop(void) {
  // Handle ADC Timer ISR
  if (ADC_timer_flag) {
    // Toggle Flag
    ADC_timer_flag = 0x0;

    // Wait until DMA Buffer is Ready for Transfer
    if (DMA_flag) {
      // Toggle Flag
      DMA_flag = 0x0;

      // Sample Guitar Data and Store to DMA Buffer in Packets
      static size_t bytes_read;
      esp_err_t i2s_result = i2s_read(I2S_NUM_0, read_ptr, (I2S_DMA_BUFF_LEN * sizeof(uint16_t)), &bytes_read, portMAX_DELAY);
      if (i2s_result != ESP_OK || bytes_read != BYTES_READ) {
        Serial.printf("DMA Read Failed! err=%d, bytes_read=%d\n", i2s_result, bytes_read);
      }

      // Push Packet Data to FIFO Buffer; Drop Oldest Sample when Full
      if ((DMA_FIFO_head + 1) % MAX_DMA_PACKETS == DMA_FIFO_tail) {
        DMA_FIFO_tail = (DMA_FIFO_tail + 1) % MAX_DMA_PACKETS;
        Serial.println("Warning: DMA FIFO overrun, dropping oldest packet");
      } 
      // DMA_packet_ptr = DMA_Packet_buff[DMA_FIFO_head];
      memcpy(DMA_Packet_buff[DMA_FIFO_head], transmit_ptr, BYTES_READ);
      Serial.printf("DMA FIFO (Loop): head=%d, tail=%d\n", DMA_FIFO_head, DMA_FIFO_tail);
      Serial.printf("Read from: %p\n", DMA_Packet_buff[DMA_FIFO_head]);

      DMA_FIFO_head = (DMA_FIFO_head + 1) % MAX_DMA_PACKETS;
      
      
      // Transfer Stored Data from Ping Buffer to Pong Buffer
      static uint8_t ping_pong_flag = 0x0;
      if (ping_pong_flag) {
        read_ptr = DMA_ping_buff;
        transmit_ptr = DMA_pong_buff;
      } else {
        read_ptr = DMA_pong_buff;
        transmit_ptr = DMA_ping_buff;
      }

      // Toggle Flags to Allow Task Transmission and Toggle Ping-Pong after Successful DMA Read
      if (i2s_result == ESP_OK && bytes_read == BYTES_READ) {
        DMA_flag = 0x1;
        ping_pong_flag ^= 0x1;
      }

      // Send Task to Queue if Space is Available
      if (uxQueueSpacesAvailable(packet_send_queue) > 0) {
        static uint8_t Transmit_flag = 0x0;
        xQueueSendFromISR(packet_send_queue, &Transmit_flag, NULL);
      }
    } else {
      Serial.println("ADC ISR Triggered, but DMA Flag was not set.");
    }
  }
}
