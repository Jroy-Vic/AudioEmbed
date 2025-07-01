#include <Arduino.h>
#include "transmitter.h"
#include "I2S.h"
#include "ESP_COMM.h"
#include <Ticker.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

extern "C" {
  #include "esp_heap_caps.h"
}


/* Global Variables */
const uint8_t ESP_Now_Peer_MAC[6] = {0x8C, 0x4F, 0x00, 0x16, 0x9E, 0x34};
const uint8_t Transmitter_pins[] = TRANSMITTER_PINS;
// DMA and ADC Handlers
static uint16_t *read_ptr, *transmit_ptr;
static uint16_t RTOS_buff[I2S_DMA_BUFF_LEN] = {0};
volatile uint8_t DMA_flag = 0x1;                    // High on Reset
// RTOS Handlers
QueueHandle_t RTOS_queue = nullptr;          
hw_timer_t *Transmit_timer = NULL;
TaskHandle_t i2sReadTaskHandle = NULL;
// ESP-Now Handlers
volatile uint8_t Transmission_ready_flag = 0x1;     // High on Reset
volatile uint8_t Packet_pending_flag = 0x0;         // Low on Reset
// Circular FIFO Handlers
uint16_t **Transmission_Packet_buff = nullptr;
uint16_t **DMA_Packet_buff = nullptr;
volatile uint8_t Transmission_FIFO_head = 0;
volatile uint8_t Transmission_FIFO_tail = 0;
volatile uint8_t Transmission_SentPacket_Count = 0;
volatile uint8_t DMA_FIFO_head = 0;
volatile uint8_t DMA_FIFO_tail = 0;


static uint32_t tx_count = 0;
static volatile uint32_t isr_count = 0;


/* External Variables */


/* Functions */
// Transmit Data Packet to Peer MCU Synchronously (Occurs every 454us / 20 samples = 44.1kHz)
void IRAM_ATTR sendPacket() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(i2sReadTaskHandle, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }

  // Toggle Debug Pin to Show ADC Timer Trigger
  digitalWrite(DEBUG_PIN, !digitalRead(DEBUG_PIN));
}


// DMA-I2S Read Offloader
void i2sReaderTask(void *param) {
  Serial.println("[DEBUG] i2sReaderTask is alive");

  static uint8_t ping_pong_flag = 0x0;
  static size_t bytes_read;

  while(1) {
    // Wait for ADC ISR to Trigger
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Wait until DMA Buffer is Ready for Transfer
    if (DMA_flag) {
      // Toggle Flag
      DMA_flag = 0x0;

      // Toggle Debug Pin2 to Show DMA Read Trigger
      digitalWrite(DEBUG_PIN2, !digitalRead(DEBUG_PIN2));

      // Sample Guitar Data and Store to DMA Buffer in Packets
      esp_err_t i2s_result = i2s_read(I2S_NUM_0, read_ptr, (I2S_DMA_BUFF_LEN * sizeof(uint16_t)), &bytes_read, portMAX_DELAY);
      if (i2s_result != ESP_OK || bytes_read != BYTES_READ) {
        Serial.printf("[WARNING] DMA Read Failed! err=%d, bytes_read=%d\n", i2s_result, bytes_read);
        return;
      }

      // Push Packet Data to FIFO Buffer; Drop Oldest Sample when Full
      if ((DMA_FIFO_head + 1) % MAX_DMA_PACKETS == DMA_FIFO_tail) {
        DMA_FIFO_tail = (DMA_FIFO_tail + 1) % MAX_DMA_PACKETS;
        Serial.println("[WARNING] DMA FIFO overrun, dropping oldest packet");
      } 
      uint16_t *DMA_packet_ptr = DMA_Packet_buff[DMA_FIFO_head];
      if (DMA_packet_ptr == nullptr) {
        Serial.printf("[FATAL] DMA_packet_ptr (HEAD) is NULL! Index = %d\n", DMA_FIFO_head);
        while (1);
      }
      // Serial.printf("[DEBUG] Copying from DMA_pong_buff to DMA_FIFO_head=%d\n", DMA_FIFO_head);
      memcpy(DMA_packet_ptr, transmit_ptr, BYTES_READ);
      // Serial.printf("DMA FIFO (Loop): head=%d, tail=%d\n", DMA_FIFO_head, DMA_FIFO_tail);
      // Serial.printf("Read from: %p\n", DMA_Packet_buff[DMA_FIFO_head]);

      DMA_FIFO_head = (DMA_FIFO_head + 1) % MAX_DMA_PACKETS;
      
      
      // Transfer Stored Data from Ping Buffer to Pong Buffer
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
      if (uxQueueSpacesAvailable(RTOS_queue) > 0) {
        static uint8_t Transmit_flag = 0x0;
        if (xQueueSend(RTOS_queue, &Transmit_flag, 0) != pdTRUE) {
          Serial.println("[WARNING] Failed to enqueue transmit flag");
        }
      }
    } else {
      Serial.println("[WARNING] ADC ISR Triggered, but DMA Flag was not set.");
    }
  }
}


// ESP-Now Transmission Offloader
void packetSendTask(void *param) {
  Serial.println("[DEBUG] packetSendTask is alive");
  // Handle Task
  uint8_t Transmit_flag;
  static uint32_t packet_cnt = 0;
  static uint32_t last_Tx_time = 0;

  while(1) {
    if (xQueueReceive(RTOS_queue, &Transmit_flag, portMAX_DELAY)) {
      // Copy DMA Buffer to Respective Task Buffer to Avoid Illegal Manipulation
      uint16_t *DMA_packet_ptr = DMA_Packet_buff[DMA_FIFO_tail];
      if (DMA_packet_ptr == nullptr) {
        Serial.printf("[FATAL] DMA_packet_ptr (TAIL) is NULL! Index = %d\n", DMA_FIFO_tail);
        while (1);
      }
      // Serial.printf("[DEBUG] Copying from DMA_FIFO_tail=%d to RTOS_buff\n", DMA_FIFO_tail);
      memcpy(RTOS_buff, DMA_packet_ptr, BYTES_READ);
      // Serial.printf("DMA FIFO (RTOS): head=%d, tail=%d\n", DMA_FIFO_head, DMA_FIFO_tail);
      // Serial.printf("Transmit from: %p\n", DMA_Packet_buff[DMA_FIFO_tail]);

      // for (int i = 0; i < I2S_DMA_BUFF_LEN; ++i) {
      //   Serial.printf("Packet #%d: RTOS_BUFF[%d] = 0x%04X\n", packet_cnt, i, RTOS_buff[i]);
      // }

      // Pop Old Packet Data from DMA FIFO Buffer
      DMA_FIFO_tail = (DMA_FIFO_tail + 1) % MAX_DMA_PACKETS;

      // Push Packet Data to FIFO Buffer in Case of Communication Error
      if (Transmission_FIFO_head >= MAX_PENDING_PACKETS) {
        Serial.printf("[FATAL] FIFO_HEAD out of bounds: %d (max %d)\n", Transmission_FIFO_head, MAX_PENDING_PACKETS);
        while (1);
      }
      uint16_t *packet_ptr = Transmission_Packet_buff[Transmission_FIFO_head];
      if (packet_ptr == nullptr) {
        Serial.printf("[FATAL] packet_ptr is NULL! Index = %d\n", Transmission_FIFO_head);
        while (1);
      }
      // Serial.printf("[DEBUG] Copying from RTOS_buff to Transmission_FIFO_head=%d\n", Transmission_FIFO_head);
      memcpy(packet_ptr, RTOS_buff, BYTES_READ);
      Transmission_FIFO_head = (Transmission_FIFO_head + 1) % MAX_PENDING_PACKETS;

      // If FIFO is Full, Block New Packets from Queueing
      if (Transmission_FIFO_head == Transmission_FIFO_tail) {
        // Serial.println("Warning: FIFO overrun, dropping oldest packet");
        continue;
      }
    }

    // Begin Transmitting Packets / Transmit Oldest Packet if Ready
    if (Transmission_ready_flag && ((Transmission_FIFO_head != Transmission_FIFO_tail) || Transmission_SentPacket_Count++ == 0)) {
      // Toggle Transmission Ready Flag for Next Transmission until Previous Send was Successful
      Transmission_ready_flag = 0x0;

      if (Transmission_FIFO_tail >= MAX_PENDING_PACKETS) {
        Serial.printf("[FATAL] FIFO_TAIL out of bounds: %d (max %d)\n", Transmission_FIFO_tail, MAX_PENDING_PACKETS);
        while (1);
      }
      
      uint16_t *packet_ptr = Transmission_Packet_buff[Transmission_FIFO_tail];
      if (packet_ptr == nullptr) {
        Serial.printf("[FATAL] packet_ptr is NULL! Index = %d\n", Transmission_FIFO_tail);
        while (1);
      }

      // Transmit Data from Task Buffer
      esp_err_t result = esp_now_send(ESP_Now_Peer_MAC, (uint8_t*) Transmission_Packet_buff[Transmission_FIFO_tail], BYTES_READ);
      
      // Toggle LED to Show ADC Timer Trigger
      static uint32_t DMA_SentPacket_Count;
      if (++DMA_SentPacket_Count == 100) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        DMA_SentPacket_Count = 0;
      }
      // vTaskDelay(pdMS_TO_TICKS(2));
      // for (int i = 0; i < I2S_DMA_BUFF_LEN; ++i) {
      //   Serial.printf("Packet #%d: ADC[%d] = 0x%04X\n", packet_cnt, i, Transmission_Packet_buff[Transmission_FIFO_tail][i]);
      // }
      // packet_cnt++;

      if (result != ESP_OK) {
        Serial.printf("[WARNING] esp_now_send() failed: 0x%X (%d)\n", result, result);
        if (result == ESP_ERR_ESPNOW_NOT_INIT)
          Serial.println("[WARNING] ESP-NOW not initialized");
        if (result == ESP_ERR_ESPNOW_ARG)
          Serial.println("[WARNING] Invalid argument");
        if (result == ESP_ERR_ESPNOW_INTERNAL)
          Serial.println("[WARNING] Internal error");
        if (result == ESP_ERR_ESPNOW_NO_MEM)
          Serial.println("[WARNING] Out of memory");
        if (result == ESP_ERR_ESPNOW_NOT_FOUND)
          Serial.println("[WARNING] Peer not found");
        return;
      }

      // Pop Old Packet Data from FIFO Buffer
      Transmission_FIFO_tail = (Transmission_FIFO_tail + 1) % MAX_PENDING_PACKETS;
    }
  }
}


// ESP-Now Transmission Callback Status
void Transmission_Status(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // If Successful, Prepare for Next Transmission
  if (status == ESP_NOW_SEND_SUCCESS) {
    Transmission_ready_flag = 0x1;
    tx_count++;
  }
}


// Initialize Serial, DRAM Allocation, I2S Communication, ESP-Now Communication, FreeRTOS, and Hardware Timer
void setup(void) {
  // 3 Second Delay for Debugging
  delay(3000);

  // LED Used for Debugging
  pinMode(DEBUG_PIN, OUTPUT);
  pinMode(DEBUG_PIN2, OUTPUT);
  digitalWrite(DEBUG_PIN, LOW);
  digitalWrite(DEBUG_PIN2, LOW);

  // Setup LED Alert to Confirm Connection
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Setup Serial Communication
  Serial.begin(115200);
  Serial.println("Transmitter initialized.");
  Serial.printf("    => MAX_PENDING_PACKETS = %d\n", MAX_PENDING_PACKETS);
  Serial.printf("    => MAX_DMA_PACKETS = %d\n", MAX_DMA_PACKETS);
  Serial.printf("    => I2S_DMA_BUFF_LEN = %d\n", I2S_DMA_BUFF_LEN);
  Serial.printf("    => BYTES_READ = %d\n", BYTES_READ);

  // Setup DRAM Allocation for DMA Buffers
  DMA_ping_buff = (uint16_t*) heap_caps_calloc(I2S_DMA_BUFF_LEN, sizeof(uint16_t), MALLOC_CAP_DMA);
  // Serial.printf("DMA_ping_buff = %p\n", DMA_ping_buff);
  DMA_pong_buff = (uint16_t*) heap_caps_calloc(I2S_DMA_BUFF_LEN, sizeof(uint16_t), MALLOC_CAP_DMA);
  // Serial.printf("DMA_pong_buff = %p\n", DMA_pong_buff);
  if (DMA_ping_buff == nullptr || DMA_pong_buff == nullptr) {
      Serial.println("[FATAL] Failed to allocate memory for DMA buffers!");
      while(1);
  }
  // Setup DRAM Allocation for Packet Buffers
  Transmission_Packet_buff = (uint16_t **) heap_caps_calloc(MAX_PENDING_PACKETS, sizeof(uint16_t*), MALLOC_CAP_DMA);
  for (uint32_t i = 0; i < MAX_PENDING_PACKETS; ++i) {
    Transmission_Packet_buff[i] = (uint16_t*) heap_caps_calloc(I2S_DMA_BUFF_LEN, sizeof(uint16_t), MALLOC_CAP_DMA);
    // Serial.printf("Transmission_Packet_buff[%d] = %p\n", i, Transmission_Packet_buff[i]);
    if (Transmission_Packet_buff[i] == nullptr) {
        Serial.printf("[FATAL] Transmission_Packet_buff[%d] is NULL!\n", i);
        while (1);
    }
  }
  DMA_Packet_buff = (uint16_t **) heap_caps_calloc(MAX_DMA_PACKETS, sizeof(uint16_t*), MALLOC_CAP_DMA);
  for (uint32_t i = 0; i < MAX_DMA_PACKETS; ++i) {
    DMA_Packet_buff[i] = (uint16_t*) heap_caps_calloc(I2S_DMA_BUFF_LEN, sizeof(uint16_t), MALLOC_CAP_DMA);
    // Serial.printf("DMA_Packet_buff[%d] = %p\n", i, DMA_Packet_buff[i]);
    if (DMA_Packet_buff[i] == nullptr) {
        Serial.printf("[FATAL] DMA_Packet_buff[%d] is NULL!\n", i);
        while (1);
    }
  }
  if (Transmission_Packet_buff == nullptr || DMA_Packet_buff == nullptr) {
    Serial.println("[FATAL] Failed to allocate memory for packet buffers!");
    Serial.println("[FATAL] DRAM Allocation (0) Initialization Failed.");
    while(1);
  } else {
    for (uint32_t i = 0; i < MAX_PENDING_PACKETS; ++i) {
      if (Transmission_Packet_buff[i] == nullptr || DMA_Packet_buff[i] == nullptr) {
        Serial.println("[FATAL] Failed to allocate memory for packet buffers!");
        Serial.println("[FATAL] DRAM Allocation (1) Initialization Failed.");
        while(1);
      }
    }

    for (uint32_t i = 0; i < MAX_DMA_PACKETS; ++i) {
      if (DMA_Packet_buff[i] == nullptr) {
        Serial.println("[FATAL] Failed to allocate memory for DMA packet buffers!");
        Serial.println("[FATAL] DRAM Allocation (2) Initialization Failed.");
        while(1);
      }
    }
  }
  Serial.println("DRAM Allocation for Buffers Initialized.");

  // Setup I2S Communication
   if (!setupI2S(Transmitter_pins)) {
    Serial.println("[FATAL] I2S setup failed!");
    while (1);
  }
  read_ptr = DMA_ping_buff;
  transmit_ptr = DMA_pong_buff;
  Serial.println("I2S Communication Initialized.");

  // Setup ESP-Now Communication
  setupESPNow();
  esp_now_register_send_cb(Transmission_Status);
  Serial.println("ESP-NOW Communication Initialized.");

  // Setup FreeRTOS
  RTOS_queue = xQueueCreate(512, sizeof(uint8_t));
  if (RTOS_queue == nullptr) {
    Serial.println("[FATAL] Failed to create RTOS_queue!");
    while (1);
  }
  xTaskCreatePinnedToCore(i2sReaderTask, "I2S Reader", 4096, NULL, 1, &i2sReadTaskHandle, 1);
  if (i2sReadTaskHandle == nullptr) {
    Serial.println("[FATAL] i2sReaderTask creation failed!");
    while (true);
  }
  xTaskCreatePinnedToCore(packetSendTask, "ESP_Now Sender", 4096, NULL, 3, NULL, 0);
  Serial.println("FreeRTOS initialized.");

  // Setup DMA Timer to Read ADC Data
  Transmit_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(Transmit_timer, &sendPacket, true);
  timerAlarmWrite(Transmit_timer, ADC_TIMER_INTERVAL_US, true);
  Serial.println("Transmitter Initialization Complete... ready to read Data.");

  // 3 Second Delay for Debugging
  delay(3000);
  digitalWrite(LED_PIN, LOW);

  // Fully Enable Hardware Timer to Activate Transmission Once System is Stable
  vTaskDelay(pdMS_TO_TICKS(500));
  static uint8_t Timer_flag = 0x0;
  Serial.println("Enabling Transmit Timer after system is stable...");
  Serial.println("Transmit Timer initialized... reading Data.");
  timerAlarmEnable(Transmit_timer);
}


// Read and Transmit Guitar Signal Data
void loop(void) {

  // static unsigned long last_tx_time = 0;
  // if (millis() - last_tx_time >= 1000) {
  //   Serial.printf("[TX] Packets sent this second: %lu\n", tx_count);
  //   tx_count = 0;
  //   last_tx_time = millis();
  // }
  // static uint32_t last_isr_count = 0;
  // static unsigned long last_time = 0;
  // if (millis() - last_time >= 1000) {
  //   Serial.printf("[TX] ISR fire count: %lu\n", isr_count - last_isr_count);
  //   last_isr_count = isr_count;
  //   last_time = millis();
  // }
}
