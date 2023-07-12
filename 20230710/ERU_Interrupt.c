/**********************************************************************************************************************
 * \file ERU_Interrupt.c
 * \copyright Copyright (C) Infineon Technologies AG 2019
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG or its licensees. If and as long as no such terms of use
 * are agreed, use of this file is subject to following:
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and
 * accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute,
 * and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the
 * Software is furnished to do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all
 * derivative works of the Software, unless such copies or derivative works are solely in the form of
 * machine-executable object code generated by a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *********************************************************************************************************************/

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "ERU_Interrupt.h"
#include "Bsp.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define ISR_PRIORITY_SCUERU_INT0   40                       /* Define the SCU ERU interrupt priority                */
#define REQ_IN                     &IfxScu_REQ8_P33_7_IN    /* External request pin                                 */
#define TRIGGER_PIN                &MODULE_P00,7            /* Pin that is toggled by pressing the board's button   */
#define LED                        &MODULE_P00,5            /* LED which gets toggled in Interrupt Service Routine  */
#define ENCODER_A                  &MODULE_P00,2
#define ENCODER_B                  &MODULE_P00,3

#define WAIT_TIME                  50

int time=0;
int time_old=0;
float count = 0;
int encoderPos=0;
int distance=0;
float rpm=0;
int encoder_mode = 0;
long test_time=0;


/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
ERUconfig g_ERUconfig;                                      /* SCU_ERU global data                                  */

/*********************************************************************************************************************/
/*----------------------------------------------Function Implementations---------------------------------------------*/
/*********************************************************************************************************************/
/* Macro to define Interrupt Service Routine.
 * This macro makes the following definitions:
 * 1) Define linker section as .intvec_tc<vector number>_<interrupt priority>.
 * 2) Define compiler specific attribute for the interrupt functions.
 * 3) Define the Interrupt Service Routine as ISR function.
 *
 * IFX_INTERRUPT(isr, vectabNum, priority)
 *  - isr: Name of the ISR function.
 *  - vectabNum: Vector table number.
 *  - priority: Interrupt priority. Refer Usage of Interrupt Macro for more details.
 */

/* Interrupt Service Routine */
IFX_INTERRUPT(SCUERU_Int0_Handler, 0, ISR_PRIORITY_SCUERU_INT0);

void SCUERU_Int0_Handler(void)
{
    if(IfxPort_getPinState(ENCODER_B) == 1)
    {
        distance++;
    }
    //waitTime(IfxStm_getTicksFromMicroseconds(BSP_DEFAULT_TIMER, 5));
    if(IfxPort_getPinState(ENCODER_B) == 1 && IfxPort_getPinState(ENCODER_A) == 1)        encoder_mode = 0;
    else if(IfxPort_getPinState(ENCODER_B) == 1 && IfxPort_getPinState(ENCODER_A) == 0)   encoder_mode = 1;

    if(encoder_mode == 1)
    {
        if((IfxPort_getPinState(ENCODER_B) == 0))   encoderPos++;
    }
    else if(encoder_mode == 0)
    {
        if((IfxPort_getPinState(ENCODER_B) == 0))   encoderPos--;
    }

    test_time=GPT12_T2_Int0_Handler();
    if(distance>=1 && distance <210){
            time_old = GPT12_T2_Int0_Handler()/1000;
        }
        else if(distance==210){
            time = GPT12_T2_Int0_Handler()/1000;
            rpm = 2*3.1415/(time-time_old)*60;
            distance=0;
            clear_overflow();
        }

}
/* This functions initializes the output pin for the LED and the pin which toggles the state for generating
 * the falling and rising edges which are used to trigger the interrupt.
 * Additionally this function is configuring the ERU module including the service request configuration */
void initPeripheralsAndERU(void)
{
    /* Initialize pins which are used to trigger and visualize the interrupt and set the default states */
    IfxPort_setPinMode(TRIGGER_PIN, IfxPort_Mode_inputPullUp);              /* Initialize TRIGGER_PIN port pin  */
    IfxPort_setPinMode(LED, IfxPort_Mode_outputPushPullGeneral);            /* Initialize LED port pin          */
    IfxPort_setPinMode(ENCODER_A, IfxPort_Mode_inputPullUp);
    IfxPort_setPinMode(ENCODER_B, IfxPort_Mode_inputPullUp);

    IfxPort_setPinState(LED, IfxPort_State_high);                           /* Turn off LED (LED is low-active) */

    /* Trigger pin */
    g_ERUconfig.reqPin = REQ_IN; /* Select external request pin */

    /* Initialize this pin with pull-down enabled
     * This function will also configure the input multiplexers of the ERU (Register EXISx)
     */
    IfxScuEru_initReqPin(g_ERUconfig.reqPin, IfxPort_InputMode_pullDown);

    /* Determine input channel depending on input pin */
    g_ERUconfig.inputChannel = (IfxScuEru_InputChannel) g_ERUconfig.reqPin->channelId;

    /* Input channel configuration */
    IfxScuEru_enableRisingEdgeDetection(g_ERUconfig.inputChannel);          /* Interrupt triggers on
                                                                               rising edge (Register RENx) and  */
    IfxScuEru_enableFallingEdgeDetection(g_ERUconfig.inputChannel);         /* on falling edge (Register FENx)  */

    /* Signal destination */
    g_ERUconfig.outputChannel = IfxScuEru_OutputChannel_0;                  /* OGU channel 0                    */
    /* Event from input ETL0 triggers output OGU0 (signal TRx0) */
    g_ERUconfig.triggerSelect = IfxScuEru_InputNodePointer_0;

    /* Connecting Matrix, Event Trigger Logic ETL block */
    /* Enable generation of trigger event (Register EIENx) */
    IfxScuEru_enableTriggerPulse(g_ERUconfig.inputChannel);
    /* Determination of output channel for trigger event (Register INPx) */
    IfxScuEru_connectTrigger(g_ERUconfig.inputChannel, g_ERUconfig.triggerSelect);

    /* Configure Output channels, OutputGating Unit OGU (Register IGPy) */
    IfxScuEru_setInterruptGatingPattern(g_ERUconfig.outputChannel, IfxScuEru_InterruptGatingPattern_alwaysActive);

    /* Service request configuration */
    /* Get source pointer depending on outputChannel (SRC_SCUERU0 for outputChannel0) */
    g_ERUconfig.src = &MODULE_SRC.SCU.SCU.ERU[(int) g_ERUconfig.outputChannel % 4];
    IfxSrc_init(g_ERUconfig.src, IfxSrc_Tos_cpu0, ISR_PRIORITY_SCUERU_INT0);
    IfxSrc_enable(g_ERUconfig.src);
}
