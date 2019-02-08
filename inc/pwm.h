#ifndef PWM_H
#define PWM_H

void write_pwm(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t value);

#endif
