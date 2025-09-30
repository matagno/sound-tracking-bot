#include "i2s_task.hpp"

#include "driver/i2s.h"
#include "BiquadFilter.hpp"
#include <vector>
#include <cstdio>

#define SAMPLE_RATE 44100
#define MAX_BUFFER 100

void i2s_task(void* arg) {
    for(;;) {
        int32_t sample;//[2];  // [0] = gauche, [1] = droite
        size_t bytes_read;
        i2s_read(I2S_NUM_0, &sample, sizeof(sample), &bytes_read, portMAX_DELAY);

        if (bytes_read == sizeof(sample)) {
            //float left  = float(samples[0] >> 8) / 8388608.0f;
            //float right = float(samples[1] >> 8) / 8388608.0f;
            float val = float(sample >> 8) / 8388608.0f;
            //float filtered_left = bpFilterL.process(left);
            //float filtered_right = bpFilterR.process(right);
            float filtered = bpFilterR.process(val);

            if(xSemaphoreTake(bufferMutex, portMAX_DELAY) == pdTRUE) {

                static int counter = 0;
                if (++counter >= SAMPLE_RATE/1) {  
                    buffer.push_back(filtered);
                    counter = 0;
                }

                // Securité overflow
                if (buffer.size() > MAX_BUFFER) {
                    buffer.clear();
                    printf("Buffer overflow, clearing buffer\n");
                }
                xSemaphoreGive(bufferMutex);
            }
        }
    }
}

