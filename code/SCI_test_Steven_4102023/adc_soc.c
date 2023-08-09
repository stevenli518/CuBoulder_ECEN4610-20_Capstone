/*
 * adc_soc.c
 *
 *  Created on: Dec 1, 2022
 *      Author: LiChe
 */
# include "adc_soc.h"


void initADC(void);
void initADCSOC(void);


// ADC Initialization
//
// Function to configure and power up ADC A
//
void initADC(void)
{
    //
    // Setup VREF as internal (ADCA)
    //
    ADC_setVREF(ADCA_BASE, ADC_REFERENCE_EXTERNAL, ADC_REFERENCE_3_3V);
    //
    // Setup VREF as internal (ADCC)
    //
    ADC_setVREF(ADCC_BASE, ADC_REFERENCE_EXTERNAL, ADC_REFERENCE_3_3V);
    ADC_setVREF(ADCB_BASE, ADC_REFERENCE_EXTERNAL, ADC_REFERENCE_3_3V);
    //
    // Set ADCCLK divider to /4
    //
//    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_1_0);
    ADC_setPrescaler(ADCC_BASE, ADC_CLK_DIV_1_0);
    ADC_setPrescaler(ADCB_BASE, ADC_CLK_DIV_1_0);
    //
    // Set pulse positions to late
    //
//    ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_ACQ_WIN);
//    ADC_setInterruptPulseMode(ADCB_BASE, ADC_PULSE_END_OF_ACQ_WIN);
//    ADC_setInterruptPulseMode(ADCC_BASE, ADC_PULSE_END_OF_ACQ_WIN);

    ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV );
    ADC_setInterruptPulseMode(ADCB_BASE, ADC_PULSE_END_OF_CONV );
    ADC_setInterruptPulseMode(ADCC_BASE, ADC_PULSE_END_OF_CONV );
    //
    // Set interrupt offset delay as 20 cycles based on the calculation
    // shown in example header
    //
//    ADC_setInterruptCycleOffset(ADCA_BASE, 30);
//    ADC_setInterruptCycleOffset(ADCB_BASE, 30);
//    ADC_setInterruptCycleOffset(ADCC_BASE, 30);
    //
    // Power up the ADCs and then delay for 1 ms
    //
    ADC_enableConverter(ADCA_BASE);
    ADC_enableConverter(ADCB_BASE);
//    DEVICE_DELAY_US(1000);
    ADC_enableConverter(ADCC_BASE);
    DEVICE_DELAY_US(1000);
}


//
// ADC SOC Initialization
//
// Description: This function will configure the ADC, channel A0 to start
// its conversion on a trigger from EPWM1 (EPMW1SOCA). The ADC will sample this
// channel continuously. After each conversion it will assert ADCINT1, which
// is then used to trigger task 1 of the CLA
//
void initADCSOC(void)
{
    //
    // Configure SOC0 of ADCA
    // - SOC0 will be triggered by EPWM1SOCA
    // - SOC0 will convert pin A3 with a sample window of 10 SYSCLK cycles.
    // - EOC0 will be generated at the end of conversion
    // - SOC0 will sample on each trigger regardless of the interrupt flag
    //
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN3, 10);
    ADC_enableContinuousMode(ADCA_BASE, ADC_INT_NUMBER1);

//    // ADC C2
    ADC_setupSOC(ADCC_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2, 10);
    ADC_enableContinuousMode(ADCC_BASE, ADC_INT_NUMBER2);
//    // ADC B2
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER15, ADC_TRIGGER_EPWM1_SOCA,
                 ADC_CH_ADCIN2, 10);
    ADC_enableContinuousMode(ADCB_BASE, ADC_INT_NUMBER3);
    //
    // Set SOC0 to set the interrupt 1 flag. Enable the interrupt and make
    // sure its flag is cleared.
    //
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
    ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1);
    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);

    ADC_setInterruptSource(ADCC_BASE, ADC_INT_NUMBER2, ADC_SOC_NUMBER1);
    ADC_enableInterrupt(ADCC_BASE, ADC_INT_NUMBER2);
    ADC_clearInterruptStatus(ADCC_BASE, ADC_INT_NUMBER2);

    ADC_setInterruptSource(ADCB_BASE, ADC_INT_NUMBER3, ADC_SOC_NUMBER15);
    ADC_enableInterrupt(ADCB_BASE, ADC_INT_NUMBER3);
    ADC_clearInterruptStatus(ADCB_BASE, ADC_INT_NUMBER3);
}
