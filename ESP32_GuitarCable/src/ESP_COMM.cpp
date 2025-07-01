#include "ESP_COMM.h"
#include <esp_now.h>
#include <WiFi.h>

extern "C" {
    #include "esp_wifi.h"
}


/* Functions */
// Initialize ESP-NOW Communication between MCUs
void setupESPNow() {
    // Initialize ESP NOW Protocol for Transmitter
    WiFi.mode(WIFI_STA);
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_LR);   // Enable Long-Range Mode
    esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);  // Use Channel 6, No Secondary
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed!");
        return;
    }
      
    // Connect Master MCU to Peer
    esp_now_peer_info_t peerInfo = {};
    uint8_t ESP_Now_Peer_MAC[6] = {0x8C, 0x4F, 0x00, 0x16, 0x9E, 0x34};
    memcpy(peerInfo.peer_addr, ESP_Now_Peer_MAC, 6);
    peerInfo.channel = 6;
    peerInfo.encrypt = false;
    esp_err_t espn_result = esp_now_add_peer(&peerInfo);

    // Check if Connection was Successful
    if (espn_result == ESP_OK) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("Peer added successfully.");
    } else if (espn_result == ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Peer already exists.");
    } else {
        Serial.printf("Failed to add peer. Error: %d\n", espn_result);
    }
}