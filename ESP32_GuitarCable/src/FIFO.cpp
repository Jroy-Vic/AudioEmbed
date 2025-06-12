#include <Arduino.h>
#include "FIFO.h"
#include "I2S.h"


/* External Variables */
uint8_t Task_buff[PREBUFF_LEN] = {0};
volatile uint16_t head = 0;
volatile uint16_t tail = 0;


/* Functions */
// Check if Buffer is Full
uint8_t isFull() {
    return ((head + 1) % PREBUFF_LEN) == tail;
}


// Check if Buffer is Empty
uint8_t isEmpty() {
    return head == tail;
}


// Push Sample into FIFO Queue
uint8_t push_Sample(uint8_t sample) {
    if (isFull()) {
        return 0;
    }

    Task_buff[head] = sample;
    head = (head + 1) % PREBUFF_LEN;
    return 1;
}


// Pop Sample out of FIFO Queue
uint8_t pop_Sample(uint8_t *sample) {
    if (isEmpty()) {
        return 0;
    }

    *sample = Task_buff[tail];
    tail = (tail + 1) % PREBUFF_LEN;
    return 1;
}


// Check how Many Samples are Pre-Buffered
uint16_t sampleCount() {
    return (PREBUFF_LEN + head - tail) % PREBUFF_LEN;
}