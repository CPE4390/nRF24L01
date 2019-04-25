/* 
 * File:   rfMain.c
 * Author: bmcgarvey
 *
 * Created on November 21, 2014, 10:03 AM
 */

#include <xc.h>

#pragma config FOSC=HSPLL
#pragma config WDTEN=OFF
#pragma config XINST=OFF

#include "LCD.h"
#include "nRF24L01.h"
#include <string.h>

char rxData[32];
char strings[][20] = {"Hello", "Goodbye", "Yes", "No"};
char rxString[20];
volatile char n = 0;
volatile char dataReady = 0;
char dataLen = 6;
volatile char transmitting = 0;
volatile int rxCount = 0;

void InitSystem(void);
void ConfigureInterrupts(void);

void main(void) {
    InitSystem();
    LCDInit();
    lprintf(0, "nRF24L01 Demo");
    rfInit();
    rfSetChannel(82);
    rfSetAutoRetryDelay(1);
    rfMode(rfRX);
    rfEnableRxInterrupt(rfEnable);
    rfEnableTxInterrupt(rfEnable);
    rfEnableTxMaxRTInterrupt(rfEnable);
    rfSetFeatures(rfEN_DPL | rfEN_ACK_PAY);
    rfEnableDynamicPayloadLength(0, rfEnable);
    rfPower(rfON);
    ConfigureInterrupts();
    rfCE = 1;
    while (1) {
        if (dataReady) {
            dataReady = 0;
            lprintf(0, "RX Count=%d", rxCount);
            lprintf(1, rxString);
        }
    }

}

void InitSystem(void) {
    PLLEN = 1;
    ANCON0 = ANCON1 = 0xff;
    LATDbits.LATD0 = 0;
    TRISBbits.TRISB0 = 1;
    TRISDbits.TRISD0 = 0;
    TRISBbits.TRISB3 = 1;
}

void ConfigureInterrupts(void) {
    INT3IF = 0;
    INTEDG3 = 0;
    INT3IE = 1;
    INT0IF = 0;
    INTEDG0 = 0;
    INT0IE = 1;
    GIE = 1;
    PEIE = 1;
}

void __interrupt(high_priority) HighISR(void) {
    char status;
    char len;
    if (INT0IF) {
        __delay_ms(10);
        if (PORTBbits.RB0 == 0) {
            if (!transmitting) {
                rfCE = 0;
                transmitting = 1;
                rfMode(rfTX);
                rfWriteTxPayload(strings[n], strlen(strings[n]) + 1);
                __delay_us(10);
                rfCE = 1;
                __delay_us(15);
                rfCE = 0;
                ++n;
                if (n >= 4) {
                    n = 0;
                }
            }
        }
        INT0IF = 0;
    }
    if (INT3IF) {
        LATDbits.LATD0 ^= 1;
        status = rfReadStatus();
        if (status & rfRX_INT) {
            rfClearRxInterrupt();
            do {
                len = rfReadRxPayloadWidth();
                rfReadRxPayload(rxString, len);
                dataReady = 1;
                ++rxCount;
            } while (!rfRxFIFOEmpty());
        }
        if (status & rfTX_INT) {
            transmitting = 0;
            rfMode(rfRX);
            rfClearTxInterrupt();
            strcpy(rxString, "Tx Success");
            dataReady = 1;
            rfCE = 1;
        }
        if (status & rfTX_MAX_RT_INT) {
            transmitting = 0;
            rfFlushTx();
            rfClearTxMaxRTInterrupt();
            rfMode(rfRX);
            strcpy(rxString, "Tx Error");
            dataReady = 1;
            rfCE = 1;
        }
        INT3IF = 0;
    }
}