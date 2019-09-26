/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uartecho.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <semaphore.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

/* Example/Board Header files */
#include "Board.h"

#include "knx.h"

#include "helpers.h"

struct knx_info_t
{
    uint16_t sa;
    uint16_t da;
    int reset_retries;
    char link_type;
}BTM = {0x0123,0x1234,3,"TPUART-BTM"},NCN = {0x0111,0x1111,3,"TPUART-NCN5130"};

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{

    const char  echoPrompt[] = "Echoing characters:\r\n";
    UART_Handle uart;
    UART_Params uartParams;

    /* Call driver init functions */
    GPIO_init();
    UART_init();

    /* Configure the LED pin */
    GPIO_setConfig(Board_GPIO_LED0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Turn on user LED */
    GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_ON);

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;

    uart = UART_open(Board_UART0, &uartParams);

    char buffer[FRAME_BUFFER_SIZE];
    int  buffer_length;
    char LSDU[]= "Echoing";
    int LSDU_length=sizeof(LSDU)-1;
    unsigned char tpuart_service[2];
    unsigned char tpuart_service_as_text[65];
    unsigned char *text;
    int j;
    int i;

    if (uart == NULL ) {
        /* UART_open() failed */
        while (1);
    }


    UART_write(uart, echoPrompt, sizeof(echoPrompt));

    /* Loop forever echoing */
    if(KNXFormatStdFrameTX(buffer,FRAME_BUFFER_SIZE,&buffer_length,KNX_DATA_STD_FRAME_REP,KNX_DATA_STD_FRAME_PRIO_NORMAL,
                        BTM.sa,BTM.da,KNX_ADDRESS_TYPE_INDIVIDUAL,KNX_LSDU_BYTE,LSDU,LSDU_length)==1){
       for( i=0; i<buffer_length; i++){
           tpuart_service[0]=0x80+i;
           if(i==0){
               tpuart_service[0]=0x80;   //data start
           }
           if(i==buffer_length-1){
               tpuart_service[0]=0x40+i; // data end
           }
           tpuart_service[1]=buffer[i];
           text = tpuart_service_as_text;
           for(j=0; j<2; j++) {
               text = translateBin2Text(tpuart_service[j],text);
           }
           UART_write(uart, tpuart_service_as_text,text-tpuart_service_as_text);

       }
   }
    else if(KNXFormatStdFrameTX(buffer,FRAME_BUFFER_SIZE,&buffer_length,KNX_DATA_STD_FRAME_REP,KNX_DATA_STD_FRAME_PRIO_NORMAL,
                                BTM.sa,BTM.da,KNX_ADDRESS_TYPE_INDIVIDUAL,KNX_LSDU_BYTE,LSDU,LSDU_length)==1){

           for( i=0; i<buffer_length; i++){
               tpuart_service[0]=0x80+i;
               if(i==0){
                   tpuart_service[0]=0x80;   //data start
               }
               if(i==buffer_length-1){
                   tpuart_service[0]=0x40+i; // data end
               }
               tpuart_service[1]=buffer[i];
               text = tpuart_service_as_text;
               for(j=0; j<2; j++) {
                   text = translateBin2Text(tpuart_service[j],text);
               }
              UART_write(uart, tpuart_service_as_text,text-tpuart_service_as_text);
           }
       }
      while (1);

/*
    char msg[1],input[1] ;
    msg[0]=0x01;

    for(i=0;i<KNX_MAX_RESET_RETRIES;i++){
        printf("Sending to BTM x%02x ...\n", msg[0]);
          UART_write(uart,msg,1);
          UART_read(uart,input,1);
          printf("Received from BTM x%02x ...\n", input[0]);
          if (input[0] == 0x03) {
              break;
          }
    }
*/
}



sem_t uart_tx_rx_sem;
int uart_reset_retries,reset_received,uart_resetting;

void *thread1(void *arg0)
{
    UART_Handle uart;
    UART_Params uartParams;

    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;

    uart = UART_open(Board_UART0, &uartParams);

    // Initialize semaphore
    int res;
    res = sem_init(&uart_tx_rx_sem, 0, 0);
    if (res)
    {
        printf("Semaphore initialization failed!!/n");
        while(1);
    }

    unsigned char uart_tx_buffer[65];

    uart_reset_retries = 0; // Initialize number of reset retries

    uart_tx_buffer[0] = 0x01; // Fill transmission buffer with the reset request service (a single byte, value 0x01)

    UART_write(uart, uart_tx_buffer, 1); // Send reset request to BTM

    reset_received = sem_wait(&uart_tx_rx_sem); // Wait for thread2 signalling no more than 1000 ticks

    if (!reset_received)
    {
        uart_reset_retries++;

        if (uart_reset_retries < 3)
        { /* Loop back, repeat UART_send() */
            UART_write(uart, uart_tx_buffer, 1);
        }
        else
        { /* give up, enter an infinite loop */
            while(1);
        }
    }

    uart_resetting = 1;
    return uart_resetting;
}

void *thread2(void *arg0)
{
     UART_Handle uart;
     UART_Params uartParams;

     UART_Params_init(&uartParams);
     uartParams.writeDataMode = UART_DATA_BINARY;
     uartParams.readDataMode = UART_DATA_BINARY;
     uartParams.readReturnMode = UART_RETURN_FULL;
     uartParams.readEcho = UART_ECHO_OFF;
     uartParams.baudRate = 115200;

     uart = UART_open(Board_UART0, &uartParams);
     unsigned char uart_rx_buffer[65];

     uart_rx_buffer[0] = 0; // Fill reception buffer with known values, just in case

     UART_read(uart, uart_rx_buffer, 1); // Fill reception buffer with known values, just in case

     if (uart_resetting)
     {
         if (uart_rx_buffer[0] == 0x03)
         {
             sem_post(&uart_tx_rx_sem);
         }
         else
         {
              UART_read(uart, uart_rx_buffer, 1);
             /* discard data and loop back to UART_read() */
         }
     }

    // If the transmitting thread is trying to reset the BTM and the data received is 0x03 (reset response) then signal the transmission thread the BTM is ready
}

