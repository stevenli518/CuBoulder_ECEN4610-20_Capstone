/*
 * custom.c
 *
 *  Created on: Jan 31, 2023
 *      Author: LiChe
 */


#include"custom_gpio.h"



void GPIO_init(){
    // GPIO28 -> myGPIOOutput0 Pinmux
    GPIO_setPinConfig(GPIO_17_GPIO17);

    //myGPIOOutput0 initialization
    GPIO_setDirectionMode(myGPIOOutput, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(myGPIOOutput, GPIO_PIN_TYPE_STD);
//    GPIO_setMasterCore(myGPIOOutput_26, GPIO_CORE_CPU1);
    GPIO_setMasterCore(myGPIOOutput, GPIO_CORE_CPU1_CLA1);
    GPIO_setQualificationMode(myGPIOOutput, GPIO_QUAL_SYNC);


#if 0
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_STD);
    GPIO_setMasterCore(32, GPIO_CORE_CPU1_CLA1);
    GPIO_setQualificationMode(32, GPIO_QUAL_SYNC);
#endif
}

