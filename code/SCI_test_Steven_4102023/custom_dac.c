/*
 * custom_dac.c
 *
 *  Created on: Jan 29, 2023
 *      Author: LiChe
 */

#include "custom_dac.h"


#define myDAC0_BASE DACB_BASE
void DAC_init(){
    // myDAC0 initialization
    // Set DAC reference voltage.
    DAC_setReferenceVoltage(DACA_BASE, DAC_REF_ADC_VREFHI);
    // Set DAC load mode.myDAC0_BASE
    DAC_setLoadMode(DACA_BASE, DAC_LOAD_SYSCLK);
    // Enable the DAC output
    DAC_enableOutput(DACA_BASE);
    // Set the DAC shadow output
    DAC_setShadowValue(DACA_BASE, 2048U);

    // Delay for buffered DAC to power up.
    DEVICE_DELAY_US(500);

    DAC_setReferenceVoltage(DACB_BASE, DAC_REF_ADC_VREFHI);
    // Set DAC load mode.myDAC0_BASE
    DAC_setLoadMode(DACB_BASE, DAC_LOAD_SYSCLK);
    // Enable the DAC output
    DAC_enableOutput(DACB_BASE);
    // Set the DAC shadow output
    DAC_setShadowValue(DACB_BASE, 0U);

    // Delay for buffered DAC to power up.
    DEVICE_DELAY_US(500);

}
