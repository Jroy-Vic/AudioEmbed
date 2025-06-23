#ifndef __ESP_COMM_H
#define __ESP_COMM_H


#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

/* MACROS*/
#define ESP_NOW_PEER_MAC {0x8C, 0x4F, 0x00, 0x16, 0x9E, 0x34} // Receiver MAC ID
#define LED_PIN 2

/* Functions */
// Initialize ESP-NOW Communication between MCUs
void setupESPNow(void);


#endif