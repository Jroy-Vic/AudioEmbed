#ifndef __TRANSMITTER_H
#define __TRANSMITTER_H


#include <Arduino.h>
#include "I2S.h"
#include "ESP_COMM.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

/* MACROS */
#define DEBUG_PIN 23
#define TRANSMITTER_PINS {26, 33}
#define BYTES_READ (I2S_DMA_BUFF_LEN * sizeof(uint16_t))
#define ADC_TIMER_INTERVAL_US 454

/* Functions */
void transmitter_setup(void);
void transmitter_loop(void);

// Transmit Data Packet to Peer MCU Synchronously (Occurs every 454us / 20 samples = 44.1kHz)
void IRAM_ATTR sendPacket(void);

// ESP-Now Transmission Offloader
void packetSendTask(void *param);

#endif