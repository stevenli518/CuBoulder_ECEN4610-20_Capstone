/*
 * myDma.c
 *
 *  Created on: Mar 2, 2023
 *      Author: shislj1
 */
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "dma.h"
#include "cla_ex5_adc_just_in_time_shared.h"
#include "myDma.h"
#include "math.h"

__interrupt void dma1ISR(void);

// Number of cycles to collect between interrupts
#define num_cycles 4
#define DAC_WAVEFORM_SIZE (size_of_samples)

#pragma DATA_SECTION(VSampleDataBuffer, "ramgs0");
#pragma DATA_SECTION(ISampleDataBuffer, "ramgs0");
#pragma DATA_SECTION(dac_sample, "ramgs0");
volatile uint16_t VSampleDataBuffer[num_cycles * size_of_samples];
volatile uint16_t ISampleDataBuffer[num_cycles * size_of_samples];
//uint16_t dac_sample[DAC_WAVEFORM_SIZE] = {0, 600, 2048, 3290, 4095, 3496, 2048, 600};
uint16_t dac_sample[DAC_WAVEFORM_SIZE] = {807, 1171, 2048, 2925, 3289, 2925, 2048, 1171};

void DmaSetup()
{
    Interrupt_register(INT_DMA_CH1, &dma1ISR);

    //
    // Perform a hard reset on DMA
    //
    DMA_initController();

    //
    // Allow DMA to run free on emulation suspend
    //
    DMA_setEmulationMode(DMA_EMULATION_FREE_RUN);

    //
    // DMA channel 1 set up for ADCA, result 0 (voltage sample).  Trigger A1
    //
    DMA_configAddresses(DMA_CH1_BASE, (uint16_t *)&VSampleDataBuffer,
                        (uint16_t *)ADCARESULT_BASE + ADC_O_RESULT0);

    // Set up transfer to wrap once the destination buffer is filled
    DMA_configTransfer(DMA_CH1_BASE, size_of_samples * num_cycles, 0, 1);
    DMA_configWrap(DMA_CH1_BASE, 0xFFFF, 0, size_of_samples * num_cycles, 0 );
    DMA_configMode(DMA_CH1_BASE, DMA_TRIGGER_ADCA1,
                   (DMA_CFG_ONESHOT_DISABLE | DMA_CFG_CONTINUOUS_ENABLE |
                           DMA_CFG_SIZE_16BIT));

    DMA_enableTrigger(DMA_CH1_BASE);
    DMA_disableOverrunInterrupt(DMA_CH1_BASE);
    DMA_setInterruptMode(DMA_CH1_BASE, DMA_INT_AT_END);
    DMA_enableInterrupt(DMA_CH1_BASE);

    //
    // DMA channel 2 set up for ADCB, result 15 (current sample).  Trigger B3
    //
    DMA_configAddresses(DMA_CH2_BASE, (uint16_t *)&ISampleDataBuffer,
                        (uint16_t *)ADCBRESULT_BASE + ADC_O_RESULT15);

    // Set up transfer to wrap once the destination buffer is filled
    DMA_configTransfer(DMA_CH2_BASE, size_of_samples * num_cycles, 0, 1);
    DMA_configWrap(DMA_CH2_BASE, 0xFFFF, 0, size_of_samples * num_cycles, 0 );
    DMA_configMode(DMA_CH2_BASE, DMA_TRIGGER_ADCB3,
                   (DMA_CFG_ONESHOT_DISABLE | DMA_CFG_CONTINUOUS_ENABLE |
                           DMA_CFG_SIZE_16BIT));

    DMA_enableTrigger(DMA_CH2_BASE);
    DMA_disableOverrunInterrupt(DMA_CH2_BASE);
    DMA_setInterruptMode(DMA_CH2_BASE, DMA_INT_AT_END);
    DMA_enableInterrupt(DMA_CH2_BASE);

    //
    // DMA channel 3 set up for DACA
    //
    DMA_configAddresses(DMA_CH3_BASE, (uint16_t *)DACA_BASE + DAC_O_VALS, &dac_sample);

    // Set up transfer to wrap once the destination buffer is filled
    DMA_configTransfer(DMA_CH3_BASE, DAC_WAVEFORM_SIZE, 1, 0);
    DMA_configWrap(DMA_CH3_BASE, DAC_WAVEFORM_SIZE, 0, 0xFFFF, 0);
    DMA_configMode(DMA_CH3_BASE, DMA_TRIGGER_ADCB3,
                   (DMA_CFG_ONESHOT_DISABLE | DMA_CFG_CONTINUOUS_ENABLE |
                           DMA_CFG_SIZE_16BIT));

    DMA_enableTrigger(DMA_CH3_BASE);
    DMA_disableOverrunInterrupt(DMA_CH3_BASE);
    DMA_setInterruptMode(DMA_CH3_BASE, DMA_INT_AT_END);
//    DMA_enableInterrupt(DMA_CH3_BASE);

}

void DmaStart()
{
    Interrupt_enable(INT_DMA_CH1);
    DMA_startChannel(DMA_CH1_BASE);
    DMA_startChannel(DMA_CH2_BASE);
    DMA_startChannel(DMA_CH3_BASE);

    // Start timer
    EPWM_clearEventTriggerInterruptFlag(EPWM1_BASE);
    EPWM_enableInterrupt(EPWM1_BASE);
}

const float meanSquareScale = ((3.3f * 3.3f) / (4095.0f * 4095.0f * 8.0f));

__interrupt void
dma1ISR(void)
{
    GPIO_writePin(9, 1);
    float V_sum = 0.0f;
    float I_sum = 0.0f;
    float P_sum = 0.0f;

#if 0
    int i=0;
    // Store most recent cycle
    for(i=0; i<size_of_samples; i++)
    {
        V_sample[i] = (float)VSampleDataBuffer[(num_cycles - 1) * size_of_samples + i] - 2048.0f;
        I_sample[i] = (float)ISampleDataBuffer[(num_cycles - 1) * size_of_samples + i] - 2048.0f;

        V_sum += V_sample[i] * V_sample[i];
        I_sum += I_sample[i] * I_sample[i];
        P_sum += I_sample[i] * V_sample[i];
    }
#else

#if (num_cycles != 4) || (size_of_samples != 8)
// If number of cycles or samples per cycle changes, the manual unrolling will break.
#error "Change manual unrolling"
#endif
    V_sample[0] = (float)VSampleDataBuffer[24] - 2048.0f;
    I_sample[0] = ((float)ISampleDataBuffer[24] - 2048.0f) * 2;
    V_sample[0] *= -100;
//    I_sample[0] *= 2;
    V_sum += V_sample[0] * V_sample[0];
    I_sum += I_sample[0] * I_sample[0];
    P_sum += I_sample[0] * V_sample[0];

    V_sample[1] = (float)VSampleDataBuffer[25] - 2048.0f;
    I_sample[1] = ((float)ISampleDataBuffer[25] - 2048.0f) * 2;
    V_sample[1] *= -100;
//    I_sample[1] *= 2;
    V_sum += V_sample[1] * V_sample[1];
    I_sum += I_sample[1] * I_sample[1];
    P_sum += I_sample[1] * V_sample[1];

    V_sample[2] = (float)VSampleDataBuffer[26] - 2048.0f;
    I_sample[2] = ((float)ISampleDataBuffer[26] - 2048.0f) * 2;
    V_sample[2] *= -100;
//    I_sample[2] *= 2;
    V_sum += V_sample[2] * V_sample[2];
    I_sum += I_sample[2] * I_sample[2];
    P_sum += I_sample[2] * V_sample[2];

    V_sample[3] = (float)VSampleDataBuffer[27] - 2048.0f;
    I_sample[3] = (float)ISampleDataBuffer[27] - 2048.0f;
    V_sample[3] *= -100;
//    I_sample[3] *= 2;
    V_sum += V_sample[3] * V_sample[3];
    I_sum += I_sample[3] * I_sample[3];
    P_sum += I_sample[3] * V_sample[3];

    V_sample[4] = (float)VSampleDataBuffer[28] - 2048.0f;
    I_sample[4] = ((float)ISampleDataBuffer[28] - 2048.0f) * 2;
    V_sample[4] *= -100;
//    I_sample[4] *= 2;
    V_sum += V_sample[4] * V_sample[4];
    I_sum += I_sample[4] * I_sample[4];
    P_sum += I_sample[4] * V_sample[4];

    V_sample[5] = (float)VSampleDataBuffer[29] - 2048.0f;
    I_sample[5] = ((float)ISampleDataBuffer[29] - 2048.0f) * 2;
    V_sample[5] *= -100;
//    I_sample[5] *= 2;
    V_sum += V_sample[5] * V_sample[5];
    I_sum += I_sample[5] * I_sample[5];
    P_sum += I_sample[5] * V_sample[5];

    V_sample[6] = (float)VSampleDataBuffer[30] - 2048.0f;
    I_sample[6] = ((float)ISampleDataBuffer[30] - 2048.0f) * 2;
    V_sample[6] *= -100;
//    I_sample[6] *= 2;
    V_sum += V_sample[6] * V_sample[6];
    I_sum += I_sample[6] * I_sample[6];
    P_sum += I_sample[6] * V_sample[6];

    V_sample[7] = (float)VSampleDataBuffer[31] - 2048.0f;
    I_sample[7] = ((float)ISampleDataBuffer[31] - 2048.0f) * 2;
    V_sample[7] *= -100;
//    I_sample[7] *= 2;
    V_sum += V_sample[7] * V_sample[7];
    I_sum += I_sample[7] * I_sample[7];
    P_sum += I_sample[7] * V_sample[7];
#endif

    // Perform mean-square calculation
    V_rms_Core = sqrtf(V_sum * meanSquareScale);
    I_rms_Core = sqrtf(I_sum * meanSquareScale);
    P_average_Core = P_sum * meanSquareScale;

    CLA_forceTasks(CLA1_BASE, CLA_TASKFLAG_3);

    GPIO_writePin(9, 0);

    //
    // Acknowledge this interrupt
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP7);
}

