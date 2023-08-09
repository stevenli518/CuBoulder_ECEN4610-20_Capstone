/*
 * sci.h
 *
 *  Created on: Oct 23, 2022
 *      Author: LiChe
 */

#ifndef SCI_PWM_H_
#define SCI_PWM_H_

#include "driverlib.h"
#include "device.h"
#include "sci.h"

void sci_setup(void);
void sci_gpio(void);
void sciA_init(void);
int str_length(char str[]);

#endif /* SCI_PWM_H_ */
