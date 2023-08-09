/*
 * pwm.c
 *
 *  Created on: Oct 9, 2022
 *      Author: LiChe
 */

#include "pwm.h"

void pwm_setup(void);
void pwm_pin_init(void);
void pwm_init(void);
void pwm_run(void);
void configurePhase(uint32_t base, uint32_t masterBase, uint16_t phaseVal);

// If defined, about 1-2 seconds between ADC triggers
//#define SLOW_ADC

//
// Global
//
#define EPWM1_FREQ          3400000UL

#ifdef SLOW_ADC
//#define EPWM1_PERIOD        (uint16_t)(65535)
#define EPWM1_PERIOD        (uint16_t)(4095)
#else
#define EPWM1_PERIOD        (uint16_t)(DEVICE_SYSCLK_FREQ / (EPWM1_FREQ))
#endif // SLOW_ADC

//
// ePWM4 parameter set
// 4B set to 100% duty cycle
//
EPWM_SignalParams pwmSignal_epwm4 =
            {431250 , 1.0f, 1.0f, false, DEVICE_SYSCLK_FREQ,
            EPWM_COUNTER_MODE_UP_DOWN, EPWM_CLOCK_DIVIDER_1,
            EPWM_HSCLOCK_DIVIDER_1};

//
// ePWM5 parameter set
//
//EPWM_SignalParams pwmSignal_epwm5 =
//            {431250 , 0.5f, 0.5f, true, DEVICE_SYSCLK_FREQ,
//            EPWM_COUNTER_MODE_UP_DOWN, EPWM_CLOCK_DIVIDER_1,
//            EPWM_HSCLOCK_DIVIDER_1};

//
// ePWM6 parameter set
//
//EPWM_SignalParams pwmSignal_epwm6 =
//            {431250 , 0.5f, 0.5f, true, DEVICE_SYSCLK_FREQ,
//            EPWM_COUNTER_MODE_UP_DOWN, EPWM_CLOCK_DIVIDER_1,
//            EPWM_HSCLOCK_DIVIDER_1};


//
//ePWM setup
//
void pwm_setup(){

    EALLOW;
    pwm_pin_init();
    pwm_init();
    pwm_run();
    EDIS;
}

//
//pwm gpio init for ePWM4, ePWM5, ePWM6
//
void pwm_pin_init(){

    //
    // GPIO0 is set to EPWM1A
    //
//GPIO_setMasterCore(0, GPIO_CORE_CPU1);
//    GPIO_setPadConfig(0,GPIO_PIN_TYPE_STD);
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setPinConfig(GPIO_1_EPWM1_B);

#if 1
    //
    // EPWM14 -> myEPWM4 Pinmux
    //
    GPIO_setPinConfig(GPIO_6_EPWM4_A);
    GPIO_setPinConfig(GPIO_7_EPWM4_B);
    //
    // EPWM5 -> myEPWM5 Pinmux
    //
    GPIO_setPinConfig(GPIO_8_EPWM5_A);
    GPIO_setPinConfig(GPIO_9_EPWM5_B);
    //
    // EPWM6 -> myEPWM6 Pinmux
    //
    GPIO_setPinConfig(GPIO_10_EPWM6_A);
    GPIO_setPinConfig(GPIO_11_EPWM6_B);
#else
    // Use GPIOs 8 & 9 for instrumentation
    // Set up GPIO8 for CLA use
    GPIO_setPinConfig(GPIO_8_GPIO8);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_STD);
    GPIO_setMasterCore(8, GPIO_CORE_CPU1_CLA1);
    GPIO_setQualificationMode(8, GPIO_QUAL_SYNC);

    // Set up GPIO9 for core use
    GPIO_setPinConfig(GPIO_9_GPIO9);
    GPIO_setDirectionMode(9, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(9, GPIO_PIN_TYPE_STD);
    GPIO_setMasterCore(9, GPIO_CORE_CPU1);
    GPIO_setQualificationMode(9, GPIO_QUAL_SYNC);
#endif // 0
}

//
//ePWM init
//
void pwm_init(){
    //
    // Set up EPWM1 to
    // - run on a base clock of SYSCLK
    // - have a period of EPWM1_PERIOD
    // - run in count up mode
    // - initialize the COMPA register to realize 0.5 duty
    //
#ifdef SLOW_ADC
    EPWM_setClockPrescaler(EPWM1_BASE, EPWM_CLOCK_DIVIDER_128, EPWM_HSCLOCK_DIVIDER_14);
    EPWM_setTimeBasePeriod(EPWM1_BASE, EPWM1_PERIOD - 1U);
#else
    EPWM_setClockPrescaler(EPWM1_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
    EPWM_setTimeBasePeriod(EPWM1_BASE, EPWM1_PERIOD - 1U); //3.4MHz
#endif // SLOW_ADC

    EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 0.5f * EPWM1_PERIOD);
    EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP);
    EPWM_setTimeBaseCounter(EPWM1_BASE, 0U);
    //
    // Configuring action-qualifiers for EPWM1
    //
    EPWM_setActionQualifierAction(EPWM1_BASE, EPWM_AQ_OUTPUT_A,
                         EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(EPWM1_BASE, EPWM_AQ_OUTPUT_A,
                             EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

    //
    // Enabling Shadow mode
    //
    EPWM_setCounterCompareShadowLoadMode(EPWM1_BASE, EPWM_COUNTER_COMPARE_A,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO );

    EPWM_setDeadBandDelayPolarity(EPWM1_BASE, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);
    EPWM_setDeadBandDelayPolarity(EPWM1_BASE, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);
    EPWM_setDeadBandDelayMode(EPWM1_BASE, EPWM_DB_RED, true);
    EPWM_setRisingEdgeDelayCount(EPWM1_BASE, 10);
    EPWM_setDeadBandDelayMode(EPWM1_BASE, EPWM_DB_FED, true);
    EPWM_setFallingEdgeDelayCount(EPWM1_BASE, 10);

    //
    // Enable SOC-A and set it to assert when the counter hits
    // zero. It asserts on every event
    //
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);

    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1U);

    //
    // Enable SOC-B and set it to assert when the counter hits
    // zero. It asserts on every event
    //
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_B);

    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_B, EPWM_SOC_TBCTR_U_CMPA);
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_B, 1U);
    //
    // Enable ePWM1 interrupt for CLA task 1
    //
//    EPWM_setInterruptSource(EPWM1_BASE, EPWM_INT_TBCTR_U_CMPA);
//    EPWM_disableInterrupt(EPWM1_BASE);
//    EPWM_setInterruptEventCount(EPWM1_BASE, 1U);

    //
    // EPWM 1 should run freely in emulation mode
    //
    EPWM_setEmulationMode(EPWM1_BASE, EPWM_EMULATION_FREE_RUN);

    //
    // EPWM4 Setup
    //
//      EPWM_setClockPrescaler(myEPWM4_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
//      EPWM_setTimeBasePeriod(myEPWM4_BASE, 0);
//      EPWM_setTimeBaseCounter(myEPWM4_BASE, 0);
//      EPWM_setTimeBaseCounterMode(myEPWM4_BASE, EPWM_COUNTER_MODE_STOP_FREEZE);
//      EPWM_disablePhaseShiftLoad(myEPWM4_BASE);
//      EPWM_setPhaseShift(myEPWM4_BASE, 0);
//      EPWM_setCounterCompareValue(myEPWM4_BASE, EPWM_COUNTER_COMPARE_A, 0);
//      EPWM_setCounterCompareShadowLoadMode(myEPWM4_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);
//      EPWM_setCounterCompareValue(myEPWM4_BASE, EPWM_COUNTER_COMPARE_B, 0);
//      EPWM_setCounterCompareShadowLoadMode(myEPWM4_BASE, EPWM_COUNTER_COMPARE_B, EPWM_COMP_LOAD_ON_CNTR_ZERO);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);
//      EPWM_setActionQualifierAction(myEPWM4_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);




      //
      // EPWM5 Setup
      //

      EPWM_setClockPrescaler(EPWM5_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
      EPWM_setTimeBasePeriod(EPWM5_BASE, EPWM1_PERIOD*8 - 1U); // 430KHz

      EPWM_setCounterCompareValue(EPWM5_BASE, EPWM_COUNTER_COMPARE_A, (0.5f * EPWM1_PERIOD * 8));
      EPWM_setTimeBaseCounterMode(EPWM5_BASE, EPWM_COUNTER_MODE_UP);
      EPWM_setTimeBaseCounter(EPWM5_BASE, 0U);
      //
      // Configuring action-qualifiers for EPWM5
      //
      EPWM_setActionQualifierAction(EPWM5_BASE, EPWM_AQ_OUTPUT_A,
                           EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
      EPWM_setActionQualifierAction(EPWM5_BASE, EPWM_AQ_OUTPUT_A,
                               EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

      //
      // Enabling Shadow mode
      //
      EPWM_setCounterCompareShadowLoadMode(EPWM5_BASE, EPWM_COUNTER_COMPARE_A,
                                           EPWM_COMP_LOAD_ON_CNTR_ZERO );

      EPWM_setDeadBandDelayPolarity(myEPWM5_BASE, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);
      EPWM_setDeadBandDelayPolarity(myEPWM5_BASE, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);
      EPWM_setDeadBandDelayMode(myEPWM5_BASE, EPWM_DB_RED, true);
      EPWM_setRisingEdgeDelayCount(myEPWM5_BASE, 10);
      EPWM_setDeadBandDelayMode(myEPWM5_BASE, EPWM_DB_FED, true);
      EPWM_setFallingEdgeDelayCount(myEPWM5_BASE, 10);

      //
      // Enable ePWM5 interrupt
      //

      EPWM_setInterruptSource(EPWM5_BASE, EPWM_INT_TBCTR_PERIOD);
//      EPWM_disableInterrupt(EPWM5_BASE);
      EPWM_enableInterrupt(EPWM5_BASE);
      EPWM_clearEventTriggerInterruptFlag(EPWM5_BASE);
//      EPWM_setInterruptEventCount(EPWM5_BASE, 1U);

      //
      // EPWM6 Setup
      //

      EPWM_setClockPrescaler(EPWM6_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
      EPWM_setTimeBasePeriod(EPWM6_BASE, EPWM1_PERIOD*8 - 1U);

      EPWM_setCounterCompareValue(EPWM6_BASE, EPWM_COUNTER_COMPARE_A, (0.5f * EPWM1_PERIOD*8));
      EPWM_setTimeBaseCounterMode(EPWM6_BASE, EPWM_COUNTER_MODE_UP);
      EPWM_setTimeBaseCounter(EPWM6_BASE, 0U);
      //
      // Configuring action-qualifiers for EPWM6
      //
      EPWM_setActionQualifierAction(EPWM6_BASE, EPWM_AQ_OUTPUT_A,
                           EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
      EPWM_setActionQualifierAction(EPWM6_BASE, EPWM_AQ_OUTPUT_A,
                               EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

      //
      // Enabling Shadow mode
      //
      EPWM_setCounterCompareShadowLoadMode(EPWM6_BASE, EPWM_COUNTER_COMPARE_A,
                                           EPWM_COMP_LOAD_ON_CNTR_ZERO );

      EPWM_setDeadBandDelayPolarity(myEPWM6_BASE, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);
      EPWM_setDeadBandDelayPolarity(myEPWM6_BASE, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);
      EPWM_setDeadBandDelayMode(myEPWM6_BASE, EPWM_DB_RED, true);
      EPWM_setRisingEdgeDelayCount(myEPWM6_BASE, 10);
      EPWM_setDeadBandDelayMode(myEPWM6_BASE, EPWM_DB_FED, true);
      EPWM_setFallingEdgeDelayCount(myEPWM6_BASE, 10);
}

//
//ePWM run
//
void pwm_run(){
    //
    // Disable sync(Freeze clock to PWM as well)
    //
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    //
    // Configuring ePWM module for desired frequency and duty
    //


    EPWM_configureSignal(myEPWM4_BASE, &pwmSignal_epwm4); // Stay High
//    EPWM_configureSignal(myEPWM5_BASE, &pwmSignal_epwm5); // Master
//    EPWM_configureSignal(myEPWM6_BASE, &pwmSignal_epwm6); // Phase shift

    //
    // Configure phase between PWM4, PWM5 & PWM6.
    //
    EPWM_disablePhaseShiftLoad(myEPWM4_BASE);
    EPWM_setPhaseShift(myEPWM4_BASE, 0U);
    EPWM_disablePhaseShiftLoad(myEPWM5_BASE);
    EPWM_setPhaseShift(myEPWM5_BASE, 0U);


    //
    // ePWM5 SYNCO is generated on CTR=0
    //

    EPWM_setSyncOutPulseMode(EPWM5_BASE, EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);


    //
    // Configure phase shift for EPWM6
    //
    configurePhase(myEPWM6_BASE, myEPWM5_BASE, 180);
    EPWM_enablePhaseShiftLoad(myEPWM6_BASE);
//    configurePhase(EPWM1_BASE, myEPWM5_BASE, 305);
//    EPWM_enablePhaseShiftLoad(EPWM1_BASE);

    //
    // Enable sync and clock to PWM
    //
//    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    //
    // Enable interrupts required for this example
    //
//    Interrupt_enable(INT_EPWM1);
//    Interrupt_enable(INT_EPWM5);

}

//
// configurePhase - Configure ePWMx Phase
//
void configurePhase(uint32_t base, uint32_t masterBase, uint16_t phaseVal)
{
    uint32_t readPrdVal, phaseRegVal;

    //
    // Read Period value to calculate value for Phase Register
    //
    readPrdVal = EPWM_getTimeBasePeriod(masterBase);

    //
    // Caluclate phase register values based on Time Base counter mode
    //
    if((HWREGH(base + EPWM_O_TBCTL) & 0x3U) == EPWM_COUNTER_MODE_UP_DOWN)
    {
        phaseRegVal = (2U * readPrdVal * phaseVal) / 360U;
    }
    else if((HWREGH(base + EPWM_O_TBCTL) & 0x3U) < EPWM_COUNTER_MODE_UP_DOWN)
    {
        phaseRegVal = (readPrdVal * phaseVal) / 360U;
    }

    EPWM_selectPeriodLoadEvent(base, EPWM_SHADOW_LOAD_MODE_SYNC);
    EPWM_setPhaseShift(base, phaseRegVal);
    EPWM_setTimeBaseCounter(base, phaseRegVal);
}

//
// End of file
//
