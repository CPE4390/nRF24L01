/* 
 * File:   nRF24L01.h
 * Author: bmcgarvey
 *
 * Created on November 21, 2014, 10:16 AM
 */

#ifndef NRF24L01_H
#define	NRF24L01_H

/*
 Pin assignments
    RB3 = IRQ
    RD3 = CE
    RD4 = SDO connect to MOSI
    RD5 = SDI connect to MISO
    RD6 = SCL
    RD7 = CSN
 */

//Hardware dependent defines

#define _XTAL_FREQ  32000000L

#define rfCSN     PORTDbits.RD7
#define rfCE      PORTDbits.RD3
#define rfTRIS()  TRISD &= 0b01110111  //make above pins outputs

#define MSSPx           2   //1 or 2 to select MSSP1 or MSSP2 for SPI

//Constants
//
//Used by rfMode()
#define rfTX        0
#define rfRX        1
//Used by rfPower
#define rfON        1
#define rfOFF       0
//Features - combine with | for rfSetFeatures()
#define rfEN_DPL    4
#define rfEN_ACK_PAY    2
#define rfEN_DYN_ACK    1
//Used by rfSetOutputPower()
#define rf0DB           3
#define rf6DB           2
#define rf12DB          1
#define rf18DB          0
//used by rfSetDataRate();
#define rf1MBPS         0
#define rf2MBPS         1
#define rf250KBPS       2
//Used by rfEnableXXX()
#define rfEnable        1
#define rfDisable       0
//Interrupt flags in STATUS
#define rfRX_INT        0b01000000
#define rfTX_INT        0b00100000
#define rfTX_MAX_RT_INT 0b00010000

//Registers
#define rfCONFIG        0x00
#define rfEN_AA         0x01
#define rfEN_RXADDR     0x02
#define rfSETUP_AW      0x03
#define rfSETUP_RETR    0x04
#define rfRF_CH         0x05
#define rfRF_SETUP      0x06
#define rfSTATUS        0x07
#define rfOBSERV_TX     0x08
#define rfRPD           0x09
#define rfRX_ADDR_P0    0x0a
#define rfRX_ADDR_P1    0x0b
#define rfRX_ADDR_P2    0x0c
#define rfRX_ADDR_P3    0x0d
#define rfRX_ADDR_P4    0x0e
#define rfRX_ADDR_P5    0x0f
#define rfTX_ADDR       0x10
#define rfRX_PW_P0      0x11
#define rfRX_PW_P1      0x12
#define rfRX_PW_P2      0x13
#define rfRX_PW_P3      0x14
#define rfRX_PW_P4      0x15
#define rfRX_PW_P5      0x16
#define rfFIFO_STATUS   0x17
#define rfDYNPD         0x1c
#define rfFEATURE       0x1d


void rfInit(void);
void rfMode(char mode);
void rfPower(char enable);
char rfReadRxPayloadWidth(void);
char rfReadRxPayload(char *data, char bytes);
char rfWriteTxPayload(char *data, char bytes);
char rfReadRegister(char r, char *data, char count);
char rfWriteRegister(char r, char *data, char count);
void rfSetRxLen(unsigned char pipe, char len);
void rfEnableRxInterrupt(char enable);
void rfEnableTxInterrupt(char enable);
void rfEnableTxMaxRTInterrupt(char enable);
char rfReadStatus(void);
void rfClearAllInterrupts(void);
void rfClearRxInterrupt(void);
void rfClearTxInterrupt(void);
void rfClearTxMaxRTInterrupt(void);
void rfSetAutoRetryDelay(char value);
void rfSetAutoRetryCount(char value);
void rfSetChannel(char chan);
void rfSetDataRate(char rate);
void rfSetOutputPower(char power);
void rfEnableAutoAck(unsigned char pipe, char enable);
void rfEnablePipe(unsigned char pipe, char enable);
void rfSetRxAddress(unsigned char pipe, char *address);
void rfSetTxAddress(char *address);
void rfEnableDynamicPayloadLength(unsigned char pipe, char enable);
void rfSetFeatures(char features);
char rfWriteTxPayloadNoAck(char *data, char bytes);
char rfWriteAckPayload(unsigned char pipe, char *data, char bytes);
char rfReuseTxPayload(void);
char rfFlushTx(void);
char rfFlushRx(void);
char rfGetRxPipe(void);
char rfTxFIFOFull(void);
char rfTxFIFOEmpty(void);
char rfRxFIFOFull(void);
char rfRxFIFOEmpty(void);
void rfSetAddressLength(char len);

#endif	/* NRF24L01_H */

