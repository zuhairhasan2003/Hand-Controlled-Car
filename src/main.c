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

const uint8_t right_motor_forward = 14; 
const uint8_t right_motor_backward = 15; 

const uint8_t left_motor_forward = 16; 
const uint8_t left_motor_backward = 17; 

const uint8_t is_safe_led = 18; 

uint16_t movement_time_ms = 5;

QueueHandle_t turn_right_queue;
QueueHandle_t turn_left_queue;
QueueHandle_t move_back_queue;

typedef enum {
    reverse,
    left,
    right,
    forward
} Motor_Cmd;

QueueHandle_t motor_cmd_queue;

void setup_gpio()
{
    gpio_init(trig);
    gpio_init(echo);
    gpio_set_dir(trig, true);
    gpio_set_dir(echo, false);

    gpio_init(right_motor_forward);
    gpio_set_dir(right_motor_forward, true);

    gpio_init(right_motor_backward);
    gpio_set_dir(right_motor_backward, true);

    gpio_init(left_motor_forward);
    gpio_set_dir(left_motor_forward, true);

    gpio_init(left_motor_backward);
    gpio_set_dir(left_motor_backward, true);

    gpio_init(is_safe_led);
    gpio_set_dir(is_safe_led, true);
    gpio_put(is_safe_led, 1);  // starting safe
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
            gpio_put(is_safe_led, *is_safe);
        }
        else
        {
            printf("ERROR : Couldnt acquire 'is_safe_mutex'\n");
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

void motor_movement_task(void *)
{
    while(true)
    {
        gpio_put(left_motor_forward, 0);
        gpio_put(left_motor_backward, 0);
        gpio_put(right_motor_forward, 0);
        gpio_put(right_motor_backward, 0);

        Motor_Cmd cmd;
        xQueueReceive(motor_cmd_queue, &cmd, portMAX_DELAY);

        switch (cmd)
        {
        case left:
            printf("LEFT\n");
            gpio_put(left_motor_backward, 1);
            gpio_put(right_motor_forward, 1);
            
            vTaskDelay(pdMS_TO_TICKS(movement_time_ms));
            
            gpio_put(left_motor_backward, 0);
            gpio_put(right_motor_forward, 0);
            break;
            
        case right:
            printf("RIGHT\n");
            gpio_put(right_motor_backward, 1);
            gpio_put(left_motor_forward, 1);
            
            vTaskDelay(pdMS_TO_TICKS(movement_time_ms));
            
            gpio_put(right_motor_backward, 0);
            gpio_put(left_motor_forward, 0);
            break;
            
        case reverse:
            printf("REVERSE\n");
            gpio_put(right_motor_backward, 1);
            gpio_put(left_motor_backward, 1);
            
            vTaskDelay(pdMS_TO_TICKS(movement_time_ms));

            gpio_put(right_motor_backward, 0);
            gpio_put(left_motor_backward, 0);
            break;

        case forward:
            if(xSemaphoreTake(is_safe_mutex, pdMS_TO_TICKS(2)))
            {
                if(*is_safe)
                {
                    printf("FORWARD\n");
                    gpio_put(right_motor_forward, 1);
                    gpio_put(left_motor_forward, 1);
                    
                    vTaskDelay(pdMS_TO_TICKS(movement_time_ms));
        
                    gpio_put(right_motor_forward, 0);
                    gpio_put(left_motor_forward, 0);
                }
                else
                {
                    printf("FORWARD - NOT SAFE\n");
                }

                xSemaphoreGive(is_safe_mutex);
            }
            break;
        
        default:
            break;
        }
    }
}

void html_server_task(void *)
{
    cyw43_arch_init();
    cyw43_arch_enable_ap_mode("pico_wifi", "zuhair_knows", CYW43_AUTH_WPA2_AES_PSK);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);     
    addr.sin_addr.s_addr = INADDR_ANY; 
    
    int s = lwip_socket(AF_INET, SOCK_STREAM, 0);
    lwip_bind(s, (struct sockaddr *)&addr, sizeof(addr));
    lwip_listen(s, 5);
    
    const char *html_page = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n"
    "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<style>body{margin:0;display:flex;justify-content:center;align-items:center;height:100vh;background:#222}"
    "#c{display:grid;grid-template:1fr 1fr 1fr/1fr 1fr 1fr;gap:10px}"
    "button{width:80px;height:80px;font-size:40px;border:none;border-radius:10px;background:#444;color:#fff;cursor:pointer}"
    "button:active{background:#666}"
    "#u{grid-column:2}#l{grid-column:1;grid-row:2}#r{grid-column:3;grid-row:2}#d{grid-column:2;grid-row:3}"
    "</style></head><body><div id='c'>"
    "<button id='u' onclick='s(\"up\")'>^</button>"
    "<button id='l' onclick='s(\"left\")'><</button>"
    "<button id='r' onclick='s(\"right\")'>></button>"
    "<button id='d' onclick='s(\"down\")'>v</button>"
    "</div><script>function s(d){fetch('/?'+d)}</script></body></html>";

    
    while (true)
    {
        socklen_t addr_size = sizeof(addr);
        char buffer[512];

        int client_sock = lwip_accept(s, (struct sockaddr *)&addr, &addr_size);
        int len = lwip_recv(client_sock, buffer, sizeof(buffer), 0);

        if(len > 0)
        {
            if(strstr(buffer, "/?up") != NULL)
            {
                Motor_Cmd val = forward;
                xQueueSend(motor_cmd_queue, &val, portMAX_DELAY);
            }
            else if(strstr(buffer, "/?down") != NULL)
            {
                Motor_Cmd val = reverse;
                xQueueSend(motor_cmd_queue, &val, portMAX_DELAY);
            }
            else if(strstr(buffer, "/?right") != NULL)
            {
                Motor_Cmd val = right;
                xQueueSend(motor_cmd_queue, &val, portMAX_DELAY);
            }
            else if(strstr(buffer, "/?left") != NULL)
            {
                Motor_Cmd val = left;
                xQueueSend(motor_cmd_queue, &val, portMAX_DELAY);
            }
        }
        lwip_send(client_sock, html_page, strlen(html_page), 0);
        lwip_close(client_sock);
    }
}

int main(void) {
    stdio_init_all();

    is_safe = (bool *) pvPortCalloc(1, sizeof(bool));
    is_safe_mutex = xSemaphoreCreateMutex();
    max_dist = 25;

    motor_cmd_queue = xQueueCreate(10, sizeof(Motor_Cmd));

    setup_gpio();

    xTaskCreate(determine_if_safe_task, "Determine if safe", 256, NULL, 3, &determine_if_safe_task_handle);
    xTaskCreate(html_server_task, "Html server task", 2048, NULL, 1, NULL);
    xTaskCreate(motor_movement_task, "Motor movement task", 256, NULL, 2, NULL);

    vTaskStartScheduler();

    return 0;   
}

// GPIO MAP
/*
    gpio    pin

    0       trig - ultrasonic sensor
    1       echo - ultrasonic sensor
    VBUS    5V   - ultrasonic sensor

    14      right motor forward
    15      right motor backward

    16      left motor forward
    17      left motor backward

    18      is safe led - onn if safe, off if unsafe to move forward
*/


// PRIORITY LEVELS
/*
    1 = LOW
    2 = MEDIUM
    3 = HIGH
*/


// 1 learning : 1 peripheral per tasks