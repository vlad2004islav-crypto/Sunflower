#include "servo.h"
#include "main.h"
#include <stdint.h>

static TIM_HandleTypeDef *htim2_ptr;

// Initialization of servos
void Servo_Init(void) {
    extern TIM_HandleTypeDef htim2;
    htim2_ptr = &htim2;

    HAL_TIM_PWM_Start(htim2_ptr, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(htim2_ptr, TIM_CHANNEL_3);

    Servo_SetAngle(2, 90); // Circle
    Servo_SetAngle(3, 90); // Up/Down
    HAL_Delay(500);
}

// Setting the servos to a specific angle
void Servo_SetAngle(uint8_t channel, uint16_t angle) {
    uint32_t pulse = ANGLE_TO_PULSE(angle);

    // Check on overflow
    if (angle > 180) angle = 180;
    if (angle < 0) angle = 0;

    // Compare Register
    if (channel == 2) {
        __HAL_TIM_SET_COMPARE(htim2_ptr, TIM_CHANNEL_2, pulse);
    } else if (channel == 3) {
        __HAL_TIM_SET_COMPARE(htim2_ptr, TIM_CHANNEL_3, pulse);
    }
}