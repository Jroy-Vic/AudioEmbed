#include "I2S.h"
#include "driver/i2s.h"


/* External Variables */
// Initialize DMA Buffers to Zero (Ping = Read, Pong = Send)
uint16_t *DMA_ping_buff = nullptr;
uint16_t *DMA_pong_buff = nullptr;


/* Functions */
/* Initialize I2S Driver for ADC Reading/Receiving of Guitar Signal Data */
uint8_t setupI2S(const uint8_t Transmitter_pins[]) {
    // Check if DMA Buffers are already allocated
    if (DMA_ping_buff == nullptr || DMA_pong_buff == nullptr) {
        Serial.println("Failed to allocate memory for DMA buffers!");
        Serial.println("IS2 Communication Initialization Failed.");
        return 0;
    }

    // Configure I2S Settings 
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 10,
        .dma_buf_len = I2S_DMA_BUFF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    // Instantiate I2S Settings (Zero Queue Size)
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_5);   // GPIO33

    // Configure ADC Settings
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);     // 3.3V Max
    i2s_set_clk(I2S_NUM_0, I2S_SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    i2s_adc_enable(I2S_NUM_0);

    // Allow Clock to Stabilize
    delay(50);

    return 1;
}