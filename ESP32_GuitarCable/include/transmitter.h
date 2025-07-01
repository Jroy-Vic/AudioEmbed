#ifndef __TRANSMITTER_H
#define __TRANSMITTER_H


#include <Arduino.h>
#include "I2S.h"
#include "ESP_COMM.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

extern "C" {
    #include "esp_heap_caps.h"
  }


/* MACROS */
#define DEBUG_PIN 23
#define DEBUG_PIN2 22
#define LED_PIN 2
#define TRANSMITTER_PINS {33}   // ADC Input
// ADC Timer Handlers
#define ADC_TIMER_INTERVAL_US 23

/* Functions */
void transmitter_setup(void);
void transmitter_loop(void);

// Transmit Data Packet to Peer MCU Synchronously (Occurs every 454us / 20 samples = 44.1kHz)
void IRAM_ATTR sendPacket(void);

// DMA-I2S Read Offloader
void i2sReaderTask(void *param);

// ESP-Now Transmission Offloader
void packetSendTask(void *param);

// ESP-Now Transmission Callback Status
void Transmission_Status(const uint8_t *mac_addr, esp_now_send_status_t status);


#endif