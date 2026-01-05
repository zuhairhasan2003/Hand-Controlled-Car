#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lwip/sockets.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>

static bool * is_safe;
SemaphoreHandle_t is_safe_mutex;
uint32_t max_dist;

TaskHandle_t determine_if_safe_task_handle;

// GPIOS
const uint8_t trig = 0;
const uint8_t echo = 1;

void setup_gpio()
{
    gpio_init(trig);
    gpio_init(echo);
    gpio_set_dir(trig, true);
    gpio_set_dir(echo, false);
}

uint32_t read_ultrasonic()
{
    uint32_t avg = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        gpio_put(trig , 1);
        busy_wait_us(10); // sending 10us pulse
        gpio_put(trig , 0);

        absolute_time_t start = 0, end = 0;
        while (gpio_get(echo) == 0)
        {
            start = get_absolute_time();
        }
        while (gpio_get(echo) == 1)
        {
            end = get_absolute_time();
        }

        uint32_t wave_pulse_length = absolute_time_diff_us(start, end);

        avg += (wave_pulse_length / 58); // get dist in cm   
        
        vTaskDelay(pdMS_TO_TICKS(60)); // delay 60 ms to let ultrasonic sensor to reset
    }

    return (avg / 3);
}

void determine_if_safe_task(void *)
{
    while (true)
    {
        uint32_t dist = read_ultrasonic();
        if (xSemaphoreTake(is_safe_mutex, pdMS_TO_TICKS(1))) // acquire mutex to write to is_safe
        {
            if(dist < max_dist)
            {
                *is_safe = false;
            }
            else
            {
                *is_safe = true;
            }
            xSemaphoreGive(is_safe_mutex);
        }
        else
        {
            printf("ERROR : Couldnt acquire 'is_safe_mutex'\n");
        }
        printf("%d cm\n", dist);
    }
}

void html_server_task(void *)
{
    cyw43_arch_init();
}

int main(void) {
    stdio_init_all();

    is_safe = (bool *) pvPortCalloc(1, sizeof(bool));
    is_safe_mutex = xSemaphoreCreateMutex();
    max_dist = 25;

    setup_gpio();

    xTaskCreate(determine_if_safe_task, "Determine if safe", 256, NULL, 3, &determine_if_safe_task_handle);

    vTaskStartScheduler();

    return 0;   
}

// GPIO MAP
/*
    gpio    pin

    0       trig - ultrasonic sensor
    1       echo - ultrasonic sensor
    VBUS    5V   - ultrasonic sensor
    GND (3) GND  - ultrasonic sensor
*/


// PRIORITY LEVELS
/*
    1 = LOW
    2 = MEDIUM
    3 = HIGH
*/