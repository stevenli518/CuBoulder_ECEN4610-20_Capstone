/*
 * sci.c
 *
 *  Created on: Oct 23, 2022
 *      Author: LiChe
 */

#include "sci_pwm.h"

#define DEVICE_GPIO_PIN3_SCIRXDA     3U             // GPIO number for SCI RX
#define DEVICE_GPIO_PIN2_SCITXDA     2U             // GPIO number for SCI TX
#define DEVICE_GPIO3_CFG_SCIRXDA     GPIO_3_SCIA_RX  // "pinConfig" for SCI RX
#define DEVICE_GPIO2_CFG_SCITXDA     GPIO_2_SCIA_TX // "pinConfig" for SCI TX

void sci_setup(void);
void sci_gpio(void);
void sciA_init(void);


void sci_setup(void){
    sci_gpio();
    sciA_init();
}

//
// SCI GPIO INIT GPIO28 and GPIO29
//
void sci_gpio(void){
    //
    // GPIO28 is the SCI Rx pin.
    //
//    GPIO_setMasterCore(DEVICE_GPIO_PIN_SCIRXDA, GPIO_CORE_CPU1);
//    GPIO_setPinConfig(DEVICE_GPIO_CFG_SCIRXDA);
//    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SCIRXDA, GPIO_DIR_MODE_IN);
//    GPIO_setPadConfig(DEVICE_GPIO_PIN_SCIRXDA, GPIO_PIN_TYPE_STD);
//    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SCIRXDA, GPIO_QUAL_ASYNC);
    //
    // GPIO3 is the SCI Rx pin.
    //
    GPIO_setMasterCore(DEVICE_GPIO_PIN3_SCIRXDA, GPIO_CORE_CPU1);
    GPIO_setPinConfig(DEVICE_GPIO3_CFG_SCIRXDA);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN3_SCIRXDA, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(DEVICE_GPIO_PIN3_SCIRXDA, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN3_SCIRXDA, GPIO_QUAL_ASYNC);

    //
    // GPIO29 is the SCI Tx pin.
    //
//    GPIO_setMasterCore(DEVICE_GPIO_PIN_SCITXDA, GPIO_CORE_CPU1);
//    GPIO_setPinConfig(DEVICE_GPIO_CFG_SCITXDA);
//    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SCITXDA, GPIO_DIR_MODE_OUT);
//    GPIO_setPadConfig(DEVICE_GPIO_PIN_SCITXDA, GPIO_PIN_TYPE_STD);
//    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SCITXDA, GPIO_QUAL_ASYNC);

    //
    // GPIO2 is the SCI Tx pin.
    //
    GPIO_setMasterCore(DEVICE_GPIO_PIN2_SCITXDA, GPIO_CORE_CPU1);
    GPIO_setPinConfig(DEVICE_GPIO2_CFG_SCITXDA);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN2_SCITXDA, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(DEVICE_GPIO_PIN2_SCITXDA, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN2_SCITXDA, GPIO_QUAL_ASYNC);
}

void sciA_init(void){
    //
    // Initialize SCIA and its FIFO.
    //
    SCI_performSoftwareReset(SCIA_BASE);

    //
    // Configure SCIA for echoback.
    //
    SCI_setConfig(SCIA_BASE, DEVICE_LSPCLK_FREQ, 115200, (SCI_CONFIG_WLEN_8 |
                                                        SCI_CONFIG_STOP_ONE |
                                                        SCI_CONFIG_PAR_NONE));
    SCI_resetChannels(SCIA_BASE);
    SCI_resetRxFIFO(SCIA_BASE);
    SCI_resetTxFIFO(SCIA_BASE);
    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_TXFF | SCI_INT_RXFF);
    SCI_enableFIFO(SCIA_BASE);
    SCI_enableModule(SCIA_BASE);
    SCI_performSoftwareReset(SCIA_BASE);
}


int str_length(char str[]) {
    // initializing count variable (stores the length of the string)
    int count;

    // incrementing the count till the end of the string
    for (count = 0; str[count] != '\0'; ++count);

    // returning the character count of the string
    return count;
}
