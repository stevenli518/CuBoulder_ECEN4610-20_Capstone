//###########################################################################
//
// FILE:   epwm_ex11_configure_signal.c
//
// TITLE:  Configure desired EPWM frequency & duty
//
//! \addtogroup driver_example_list
//! <h1> EPWM Configure Signal </h1>
//!
//! This example configures ePWM1, ePWM2, ePWM3 to produce signal of desired
//! frequency and duty. It also configures phase between the configured
//! modules.
//!
//! Signal of 10kHz with duty of 0.5 is configured on ePWMxA & ePWMxB
//! with ePWMxB inverted. Also, phase of 120 degree is configured between
//! ePWM1 to ePWM3 signals.
//!
//! During the test, monitor ePWM1, ePWM2, and/or ePWM3 outputs
//! on an oscilloscope.
//!
//! - ePWM1A is on GPIO0
//! - ePWM1B is on GPIO1
//! - ePWM2A is on GPIO2
//! - ePWM2B is on GPIO3
//! - ePWM3A is on GPIO4
//! - ePWM3B is on GPIO5
//!
//
//###########################################################################
//
//
// $Copyright:
// Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//###########################################################################

//
// Included Files
//
#include <sci_pwm.h>
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "pwm.h"
#include "sci_pwm.h"
#include "cla_custom.h"
#include "adc_soc.h"
#include "custom_dac.h"
#include "custom_gpio.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cla_ex5_adc_just_in_time_shared.h"
// Linker Defined variables


//
// Globals
//
volatile float V_rms_Core = 0.0f;
volatile float I_rms_Core = 0.0f;
volatile float P_average_Core = 0.0f;

#pragma DATA_SECTION(P_average_CLA,"Cla1ToCpuMsgRAM");
volatile float P_average_CLA;

#pragma DATA_SECTION(V_sample,"CpuToCla1MsgRAM");
volatile float V_sample[size_of_samples];

#pragma DATA_SECTION(I_sample,"CpuToCla1MsgRAM");
volatile float I_sample[size_of_samples];

//#pragma DATA_SECTION(dac_sample,"CpuToCla1MsgRAM");
//volatile uint16_t dac_sample[size_of_samples] = {0, 1024, 2048, 3072, 4095, 3072, 2048, 1024};

#pragma DATA_SECTION(I_rms_CLA,"Cla1ToCpuMsgRAM");
volatile float I_rms_CLA;

#pragma DATA_SECTION(I_mean_square,"CpuToCla1MsgRAM");
volatile float I_mean_square;

//#pragma DATA_SECTION(I_sum,"Cla1ToCpuMsgRAM");
//volatile float I_sum;

//#pragma DATA_SECTION(Idac,"Cla1ToCpuMsgRAM");
//volatile uint16_t Idac;

#pragma DATA_SECTION(V_rms_CLA,"Cla1ToCpuMsgRAM");
volatile float V_rms_CLA;

#pragma DATA_SECTION(V_mean_square,"CpuToCla1MsgRAM");
volatile float V_mean_square;

//#pragma DATA_SECTION(V_sum,"Cla1ToCpuMsgRAM");
//volatile float V_sum;

//#pragma DATA_SECTION(Vdac,"Cla1ToCpuMsgRAM");
//volatile uint16_t Vdac;

//// CPU -> CLA
//#pragma DATA_SECTION(sum_frozen,"CpuToCla1MsgRAM");
//volatile float sum_frozen;

// Global Variable
enum menu{Start = 49, Phase = 50, Reset = 51, Turnoff = 52, Help = 53, Send = 54, Exit = 55};
static char float_array[20];
float v_data[size_of_samples];
float i_data[size_of_samples];
//static char minus_float_array[20];
const float scal_ratio = 3.3f/4096.0f;

// Function Definition
float scaling (float val_in);
float calculate_rms(float* array_in, uint16_t size);
float calculate_pavg(float* voltage, float* current, uint16_t size);
void ftoa(float n, char* res, int afterpoint);
int intToStr(int x, char str[], int d);
void reversef(char* str, int len);
void printMenu();

//
// Main
//
void main(void)
{
    unsigned char *receivedDegree;
    uint16_t receivedChar;
    char *msg;
    uint16_t rxStatus = 0U;
    int i = 0;
    int j = 0;
    float localVRmsCla = 0.0f;
    float localIRmsCla = 0.0f;
    float localPavgCla = 0.0f;
//    float localImpeCla = 0.0f;

    float localVRmsCore = 0.0f;
    float localIRmsCore = 0.0f;
    float localPavgCore = 0.0f;
//    float localImpeCore = 0.0f;

//    double scal_ratio = 3.3/4096;
//    minus_float_array[0] = '0';
    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Disable pin locks and enable internal pull-ups.
    //
    Device_initGPIO();

    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    //
    // Disable sync(Freeze clock to PWM as well)
    //
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    //
    // Init modules
    //
    //Board_init();
    GPIO_init();
    pwm_setup();
    sci_setup();
    initADC();
    initADCSOC();
    DAC_init();
    initCLA();
    DmaSetup();
    DmaStart();

    //
    // Enable global Interrupts and higher priority real-time debug events:
    //
    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM

    //
    // Send starting message.
    //
    msg = "\r\n\n\nHello World!\0";
    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 17);
    printMenu();

#if 0
    // Start
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
    EPWM_enableInterrupt(EPWM1_BASE);
    EPWM_enableInterrupt(EPWM5_BASE);
#endif // 0
    for(;;)
    {

        // Menu choice
        msg = "\r\nPlease Make your choice:\n\0";
        SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 28);
        receivedChar = SCI_readCharBlockingFIFO(SCIA_BASE);
        rxStatus = SCI_getRxStatus(SCIA_BASE);
        if((rxStatus & SCI_RXSTATUS_ERROR) != 0)
        {
            //
            //If Execution stops here there is some error
            //Analyze SCI_getRxStatus() API return value
            //
            ESTOP0;
        }
            switch(receivedChar){
            case Start:
                msg = "\r\nPWM is turned on\n\0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 23);
                SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
                EPWM_enableInterrupt(EPWM1_BASE);
                EPWM_enableInterrupt(EPWM5_BASE);
                break;
            case Phase:
                // House keeping
                msg = "\r\nFormat: 45-045, 180-180 \0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 25);

                // Receive input
                msg = "\r\nEnter a degree(0-180): \0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 27);
                SCI_readCharArray(SCIA_BASE, (uint16_t*)receivedDegree, 3);
                rxStatus = SCI_getRxStatus(SCIA_BASE);
                if((rxStatus & SCI_RXSTATUS_ERROR) != 0)
                {
                    //
                    //If Execution stops here there is some error
                    //Analyze SCI_getRxStatus() API return value
                    //
                    ESTOP0;
                }
                // Calculate the phase
                char degree[3];
                int phase;
                for( i = 0; i < 3; i++){
                    degree[i] = (uint16_t)receivedDegree[i];
                }
                phase = atoi(degree);

                // Tell user what they sent
                msg = "\r\nPWM Phase: \0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 14);
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)receivedDegree, 3);

                // Configure phase shift for EPWM6
                if (phase >= 0 && phase  <= 180){
                    configurePhase(myEPWM6_BASE, myEPWM5_BASE, (uint16_t)phase);
                    EPWM_enablePhaseShiftLoad(myEPWM6_BASE);
                    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
                }
                break;
            case Reset:
                //
                // Reset phase shift for EPWM6
                //
                msg = "\r\nReset PWM6 to 90 phase shift\n\0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 35);
                configurePhase(myEPWM6_BASE, myEPWM5_BASE, 90);
                EPWM_enablePhaseShiftLoad(myEPWM6_BASE);
                break;
            case Turnoff:
                //
                //Turn off PWM
                //
                msg = "\r\nPWM is turned off\n\0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 24);
                SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
                break;
            case Help:
                printMenu();

                break;

            case Send:
                for(j = 0; j < size_of_samples; j++){
                    v_data[j] = V_sample[j];
                    i_data[j] = I_sample[j];
                }
                localVRmsCla = V_rms_CLA;
                localIRmsCla = I_rms_CLA;
                localPavgCla = P_average_CLA;
//                localPavgCla = localVRmsCla * localIRmsCla;
//                localImpeCla = localVRmsCla / localIRmsCla;
                localVRmsCore = V_rms_Core;
                localIRmsCore = I_rms_Core;
                localPavgCore = P_average_Core;
//                localPavgCore = localVRmsCore * localIRmsCore;
//                localImpeCore = localVRmsCore / localIRmsCore;
                msg = "\r\n        Core        CLA\0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));

                msg = "\r\nPavg(W): \0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));
                if(localPavgCore < 0){
                    msg = "-";
                    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 1);
                    ftoa((-1*localPavgCore), float_array, 3);
                    SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
                }
                else{
                    ftoa(localPavgCore, float_array, 3);
                    SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
                }
                msg = "        \0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));

                if(localPavgCla < 0){
                    msg = "-";
                    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 1);
                    ftoa((-1*localPavgCla), float_array, 3);
                    SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
                }
                else{
                    ftoa(localPavgCla, float_array, 3);
                    SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
                }


                msg = "\r\nVrms(V): \0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));
                ftoa(localVRmsCore, float_array, 2);
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
                msg = "        \0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));
                ftoa(localVRmsCla, float_array, 2);
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));


                msg = "\r\nIrms(A): \0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));
                ftoa(localIRmsCore, float_array, 2);
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
                msg = "        \0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));
                ftoa(localIRmsCla, float_array, 2);
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
                msg = "\n";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 1);

//                msg = "\r\nImpedance(ohm): \0";
//                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));
//                ftoa(localImpeCore, float_array, 2);
//                SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
//                msg = "        \0";
//                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));
//                ftoa(localImpeCla, float_array, 2);
//                SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
//                msg = "\n";
//                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 1);

                msg = "\r\nV_Sample(V): \0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));
                for( j = 0; j < size_of_samples; j++){
#if 1
                    if(v_data[j] < 0){
                        msg = "-";
                        SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 1);
                        ftoa(scaling(v_data[j]), float_array, 3);
                        SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
                    }
                    else{
                        ftoa(scaling(v_data[j]), float_array, 3);
                        SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
                    }
#else
                    intToStr(myADC0DataBuffer[j], float_array, 1);
                    SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
#endif
                    msg = ",";
                    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 1);
                }
//                msg = "\n";
//                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 1);

                msg = "\r\nI_Sample(A): \0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));
                for(j = 0; j < size_of_samples; j++){
#if 1
                    if(i_data[j] < 0){
                        msg = "-";
                        SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 1);
                        ftoa(scaling(i_data[j]), float_array,3);
                        SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
                    }
                    else{
                        ftoa(scaling(i_data[j]), float_array,3);
                        SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
                    }

#else
                    intToStr(myADC1DataBuffer[j], float_array, 1);
                    SCI_writeCharArray(SCIA_BASE, (uint16_t*)float_array, strlen(float_array));
#endif

                    msg = ",";
                    SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 1);
                }
                msg = "\n";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, 1);
                break;
            default:

                msg = "\r\nInput Incorrect\0";
                SCI_writeCharArray(SCIA_BASE, (uint16_t*)msg, strlen(msg));
                break;

            }

    }

}


static const char s_menu[] = "\r\nMenu:\r\n1.Start PWM\r\n2.PWM Phase Control\r\n3.Reset\r\n4.Turn off\r\n5.Help\r\n6.Send";
void printMenu()
{
    SCI_writeCharArray(SCIA_BASE, (uint16_t*)s_menu, sizeof(s_menu));

}

float scaling (float val_in){
    if(val_in < 0){
        return -1*(val_in) * scal_ratio;
    }
    else{
        return val_in * scal_ratio;
    }
//    return (val_in - 2048) * scal_ratio;
//    return (val_in) * scal_ratio;
}
// Calculate the RMS
float calculate_rms(float* array_in, uint16_t size){
    float sum = 0;
    int i;
    float scaling = 0;

    for(i = 0; i < size; i++){
        scaling = (array_in[i]- 2048) * scal_ratio;
//        scaling = (array_in[i]) * scal_ratio;
        sum += (scaling  * scaling);
    }
    sum *= 0.125;
    return sqrtf(sum);
}
float calculate_pavg(float* voltage, float* current, uint16_t size){
    float p_sum = 0;
    float scal = scal_ratio;
    int i;
    float v_scaling, i_scaling;
    float temp_p = 0;
//    float factor= 1/size_of_samples;
    for( i = 0; i < size; i++){
        v_scaling = (voltage[i]- 2048);
        i_scaling = (current[i]- 2048);
        v_scaling *=   scal;
        i_scaling *=   scal;
        temp_p = v_scaling * i_scaling * 100;
        p_sum += temp_p;
    }
    p_sum = p_sum*0.125;
//    p_sum *= ( scal*  scal);
    return p_sum;
}

// Float to array of char
void ftoa(float n, char* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);
//    if(n < 0){
//        res = '-'+res;
//        i++;
//    }
    // check for display option after point
    if (afterpoint != 0) {
        res[i] = '.'; // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter
        // is needed to handle cases like 233.007
        int power = 1;

        int j=0;

        for(; j<afterpoint; j++)

        {

            power *= 10;

        }

        fpart *= power;
//        fpart = fpart * pow(10, afterpoint);


        intToStr((int)fpart, res + i + 1, afterpoint);

    }
}

// Int to Str
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reversef(str, i);
    // NULL terminate string
    str[i] = '\0';
    return i;
}

// Reverse
void reversef(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

//#pragma CODE_SECTION(cla1Isr1, ".TI.ramfunc")
//__attribute__((interrupt))  void cla1Isr1 (void){
//    DINT;
//    CLA_clearTaskFlags(CLA1_BASE, CLA_TASK_1);
//
//    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
//    if(i > 7){
//        sum_frozen = sum;
//        CLA_forceTasks(CLA1_BASE, CLA_TASKFLAG_2);
//    }
//    EINT;
//}
