#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "tim.h"
#include "main.h"
#include <stdint.h>

// (0-180) -> (0.5ms - 2.5ms)
// Convert angle to pulse
#define ANGLE_TO_PULSE(angle) (uint32_t)(5 + (angle) * 20 / 180)

void Servo_Init(void);
void Servo_SetAngle(uint8_t channel, uint16_t angle);

#endif /* INC_SERVO_H_ */