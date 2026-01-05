// This file will only contain code that I have decided not to use anymore (just incase i need it in future)





// ---------------------------- Servo motor

// reason to remove: is I dont have servo parts and I dont want to hold my project until i receieve them
// void rotate_servo(float deg)
// {
//     // 976 = min
//     // 4883 = max
//     uint16_t level = 976 + (deg / 180.0f) * (4883 - 976);
//     pwm_set_gpio_level(servo, level);
// }

// void rotate_servo_task(void *)
// {
//     uint slice_num = pwm_gpio_to_slice_num(servo);
//     pwm_set_clkdiv(slice_num, 64.0f); 
//     pwm_set_wrap(slice_num, 39062); 
//     pwm_set_enabled(slice_num, true);
    
//     while (true)
//     {
//         vTaskDelay(pdTICKS_TO_MS(500));
//         rotate_servo(0);
//         vTaskDelay(pdTICKS_TO_MS(500));
//         rotate_servo(90);
//         vTaskDelay(pdTICKS_TO_MS(500));
//         rotate_servo(180);
//         vTaskDelay(pdTICKS_TO_MS(500));
//         rotate_servo(90);
//     }
// }