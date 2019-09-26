
/*
 *  ======== spislave.c ========
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/display/Display.h>

/* Example/Board Header files */
#include "Board.h"

#define THREADSTACKSIZE (1024)

unsigned char SLAVE_MSG[] = {0x80};

#define SPI_MSG_LENGTH  (sizeof(SLAVE_MSG))


static Display_Handle display;

#define SPI_BUFFER_SIZE     1024

unsigned char slaveRxBuffer[SPI_BUFFER_SIZE];
unsigned char slaveTxBuffer[SPI_BUFFER_SIZE];

#define DEBUG_MSG_SIZE 100
char debug_msg[DEBUG_MSG_SIZE];

/* Semaphore to block slave until transfer is complete */
sem_t slaveSem;

/*
 *  ======== transferCompleteFxn ========
 *  Callback function for SPI_transfer().
 */
void transferCompleteFxn(SPI_Handle handle, SPI_Transaction *transaction)
{
    sem_post(&slaveSem);
}

/*
 * ======== slaveThread ========
 *  Slave SPI sends a message to master while simultaneously receiving a
 *  message from the master.
 */
void *slaveThread(void *arg0)
{
    SPI_Handle      slaveSpi;
    SPI_Params      spiParams;
    SPI_Transaction transaction;
    uint32_t        i, j;
    bool            transferOK;
    int32_t         status;
    uint32_t        transfersOK_count = 0;
    uint32_t        recvsOK_count = 0;

    GPIO_setConfig(Board_SPI_SLAVE_READY, GPIO_CFG_OUTPUT | GPIO_CFG_OUT_HIGH);
        GPIO_write(Board_SPI_SLAVE_READY, 1);

    status = sem_init(&slaveSem, 0, 0);
    if (status != 0) {
        Display_printf(display, 0, 0, "Error creating slaveSem\n");

        while(1);
    }


    /*
     * Open SPI as slave in callback mode; callback mode is used to allow us to
     * configure the transfer & then set Board_SPI_SLAVE_READY high.
     */
    SPI_Params_init(&spiParams);

    spiParams.frameFormat = SPI_POL0_PHA0;
    spiParams.mode = SPI_SLAVE;
    spiParams.transferCallbackFxn = transferCompleteFxn;
    spiParams.transferMode = SPI_MODE_CALLBACK;
    slaveSpi = SPI_open(Board_SPI_SLAVE, &spiParams);

    if (slaveSpi == NULL) {
        Display_printf(display, 0, 0, "Error initializing slave SPI\n");
        while (1);
    }
    else {
        Display_printf(display, 0, 0, "Slave SPI initialized\n");
    }

    /* Copy message to transmit buffer */
    memset(slaveTxBuffer, 0x55, SPI_BUFFER_SIZE);
    memcpy(slaveTxBuffer, SLAVE_MSG, SPI_MSG_LENGTH);
    memset(slaveRxBuffer, 0xAA, SPI_BUFFER_SIZE);

    // *******************************
    // Transmission block of N bytes
    // *******************************

    for (i = 0; i < SPI_MSG_LENGTH; i++) {
        /* Initialize slave SPI transaction structure */
        transaction.count = 1;
        transaction.txBuf = (void *) &slaveTxBuffer[i];
        transaction.rxBuf = (void *) &slaveRxBuffer[i];

        /* Toggle on user LED, indicating a SPI transfer is in progress */
        GPIO_toggle(Board_GPIO_LED1);

        /*
         * Setup SPI transfer; Board_SPI_SLAVE_READY will be set to notify
         * master the slave is ready.
         */
        transferOK = SPI_transfer(slaveSpi, &transaction);
        if (transferOK) {
            GPIO_write(Board_SPI_SLAVE_READY, 0);

            /* Wait until transfer has completed */
            sem_wait(&slaveSem);

            /*
             * Drive Board_SPI_SLAVE_READY high to indicate slave is not ready
             * for another transfer yet.
             */
            GPIO_write(Board_SPI_SLAVE_READY, 1);
            transfersOK_count++;

        }
        else {
            //Display_printf(display, 0, 0, "Unsuccessful slave SPI transfer");
        }
    }


    // *******************************
    // Reception block of 1 byte
    // *******************************

    /* Initialize slave SPI transaction structure */
    transaction.count = 1;
    transaction.txBuf = (void *) &slaveTxBuffer[i];
    transaction.rxBuf = (void *) &slaveRxBuffer[i];

    transferOK = SPI_transfer(slaveSpi, &transaction);
    if (transferOK) {
        /* Wait until transfer has completed */
        sem_wait(&slaveSem);
        recvsOK_count++;
    }
    else {
        //Display_printf(display, 0, 0, "Unsuccessful slave SPI transfer (reception block)");
    }

    SPI_close(slaveSpi);

    Display_printf(display, 0, 0, "SPI transfer block (%d bytes, %d ok)", SPI_MSG_LENGTH, transfersOK_count);
    for (i=0; i < SPI_MSG_LENGTH; i++) {
        snprintf(debug_msg, DEBUG_MSG_SIZE, "Slave transmitted: 0x%02X, received: 0x%02X", slaveTxBuffer[i], slaveRxBuffer[i]);
        Display_printf(display, 0, 0, debug_msg);
    }
    Display_printf(display, 0, 0, "SPI receive block (%d bytes, %d ok)", 1, recvsOK_count);
    for (j=0; j < 1; j++, i++) {
        snprintf(debug_msg, DEBUG_MSG_SIZE, "Slave transmitted: 0x%02X, received: 0x%02X", slaveTxBuffer[i], slaveRxBuffer[i]);
        Display_printf(display, 0, 0, debug_msg);
    }

    Display_printf(display, 0, 0, "\nDone");

    return (NULL);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    pthread_t           thread0;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;

    /* Call driver init functions. */
    Display_init();
    GPIO_init();
    SPI_init();

    /* Configure the LED pins */
    GPIO_setConfig(Board_GPIO_LED0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(Board_GPIO_LED1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Open the display for output */
    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        /* Failed to open display driver */
        while (1);
    }

    /* Turn on user LED */
    GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_ON);

    Display_printf(display, 0, 0, "Starting the SPI slave example");
    Display_printf(display, 0, 0, "This example requires external wires to be "
        "connected to the header pins. Please see the Board.html for details.\n");

    /* Create application thread */
    pthread_attr_init(&attrs);

    detachState = PTHREAD_CREATE_DETACHED;
    /* Set priority and stack size attributes */
    retc = pthread_attr_setdetachstate(&attrs, detachState);
    if (retc != 0) {
        /* pthread_attr_setdetachstate() failed */
        while (1);
    }

    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0) {
        /* pthread_attr_setstacksize() failed */
        while (1);
    }

    /* Create slave thread */
    priParam.sched_priority = 1;
    pthread_attr_setschedparam(&attrs, &priParam);

    retc = pthread_create(&thread0, &attrs, slaveThread, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1);
    }

    return (NULL);
}
