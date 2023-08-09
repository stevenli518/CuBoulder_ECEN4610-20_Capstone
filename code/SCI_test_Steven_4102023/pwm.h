/*
 * pwm.h
 *
 *  Created on: Oct 9, 2022
 *      Author: LiChe
 */

#ifndef PWM_H_
#define PWM_H_

#include "driverlib.h"
#include "device.h"


#define GPIO_PIN_EPWM4_A 6
#define GPIO_PIN_EPWM4_B 7
#define GPIO_PIN_EPWM5_A 8
#define GPIO_PIN_EPWM5_B 9
#define GPIO_PIN_EPWM6_A 10
#define GPIO_PIN_EPWM6_B 11

#define myEPWM4_BASE EPWM4_BASE
#define myEPWM5_BASE EPWM5_BASE
#define myEPWM6_BASE EPWM6_BASE

void pwm_setup(void);
void pwm_pin_init(void);
void pwm_init(void);
void pwm_run(void);
void configurePhase(uint32_t base, uint32_t masterBase, uint16_t phaseVal);


#endif /* PWM_H_ */
