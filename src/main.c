#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

static bool * is_safe;
uint16_t safe_dim;

// GPIOS
const uint8_t trig = 0;
const uint8_t echo = 1;
const uint servo = 3;

void setup_gpio()
{
    gpio_init(trig);
    gpio_init(echo);
    gpio_set_dir(trig, true);
    gpio_set_dir(echo, false);

    gpio_set_function(servo, GPIO_FUNC_PWM);
}

void rotate_servo(float deg)
{
    // 976 = min
    // 4883 = max
    uint16_t level = 976 + (deg / 180.0f) * (4883 - 976);
    pwm_set_gpio_level(servo, level);
}

void rotate_servo_task(void *)
{
    uint slice_num = pwm_gpio_to_slice_num(servo);
    pwm_set_clkdiv(slice_num, 64.0f); 
    pwm_set_wrap(slice_num, 39062); 
    pwm_set_enabled(slice_num, true);
    
    while (true)
    {
        vTaskDelay(pdTICKS_TO_MS(500));
        rotate_servo(0);
        vTaskDelay(pdTICKS_TO_MS(500));
        rotate_servo(90);
        vTaskDelay(pdTICKS_TO_MS(500));
        rotate_servo(180);
        vTaskDelay(pdTICKS_TO_MS(500));
        rotate_servo(90);
    }
}

uint32_t read_ultrasonic()
{
    uint32_t avg = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        gpio_put(trig , 1);
        vTaskDelay(pdMS_TO_TICKS(0.01)); // sending 10us pulse
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
        printf("%d cm\n", dist);
    }
}

int main(void) {
    stdio_init_all();

    safe_dim = 5;
    is_safe = (bool *) pvPortCalloc(safe_dim, sizeof(bool));

    setup_gpio();

    xTaskCreate(determine_if_safe_task, "Determine if safe", 256, NULL, 3, NULL);

    // xTaskCreate(rotate_servo_task, "Rotate servo task", 256, NULL, 1, NULL);

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

    3       pwm  - servo
*/


// PRIORITY LEVELS
/*
    1 = LOW
    2 = MEDIUM
    3 = HIGH
*/