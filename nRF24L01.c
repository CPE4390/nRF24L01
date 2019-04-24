#include <xc.h>
#include "nRF24L01.h"

//These functions are processor specific
//The might need to be modified if using another uController or MSSP1

#if MSSPx == 1
#define SSPxCON1bits    SSPCON1bits
#define SSPxSTATbits    SSPSTATbits
#define SSPxBUF         SSPBUF
#define SPI_TRIS()      TRISCbits.TRISC3=0;TRISCbits.TRISC4=1;TRISCbits.TRISC5=0  
                        //SDO=RC5, SDI=RC4, SCL=RC3
#define SSPxIF          SSPIF
#elif MSSPx == 2
#define SSPxCON1bits    SSP2CON1bits
#define SSPxSTATbits    SSP2STATbits
#define SSPxBUF         SSP2BUF
#define SPI_TRIS()      TRISDbits.TRISD4=0;TRISDbits.TRISD5=1;TRISDbits.TRISD6=0
                        //SDO=RD4, SDI=RD5, SCL=RD6
#define SSPxIF          SSP2IF
#else
#error Invalid MSSPx selection
#endif


inline char rfSPIrw(char b) {
    SSPxIF = 0;
    SSPxBUF = b;
    while (SSPxIF == 0);
    return SSPxBUF;
}

void rfInitPins(void) {
    rfCE = 0;
    rfCSN = 1;
    rfTRIS();
}

void rfInitSPI(void) {
    SPI_TRIS();
    SSPxSTATbits.CKE = 1;
    SSPxCON1bits.CKP = 0;
    SSPxSTATbits.SMP = 1;
    SSPxCON1bits.SSPM = 0b0001;
    SSPxCON1bits.SSPEN = 1;
}

//The following functions are not processor dependent

void rfInit(void) {
    char config = 0b01111000; //Mask all interrupts, CRC on
    rfInitPins();
    rfInitSPI();
    rfWriteRegister(rfCONFIG, &config, 1);
    rfSetAutoRetryDelay(1); //500us    
}

char rfWriteRegister(char r, char *data, char count) {
    char status;
    rfCSN = 0;
    status = rfSPIrw(0b00100000 | (r & 0b00011111));
    while (count) {
        rfSPIrw(*data);
        ++data;
        --count;
    }
    rfCSN = 1;
    return status;
}

char rfReadRegister(char r, char *data, char count) {
    char status;
    rfCSN = 0;
    status = rfSPIrw(r & 0b00011111);
    while (count) {
        *data = rfSPIrw(0);
        ++data;
        --count;
    }
    rfCSN = 1;
    return status;
}

char rfWriteTxPayload(char *data, char bytes) {
    char status;
    rfCSN = 0;
    status = rfSPIrw(0b10100000);
    while (bytes) {
        rfSPIrw(*data);
        ++data;
        --bytes;
    }
    rfCSN = 1;
    return status;
}

char rfReadRxPayload(char *data, char bytes) {
    char status;
    rfCSN = 0;
    status = rfSPIrw(0b01100001);
    while (bytes) {
        *data = rfSPIrw(0);
        ++data;
        --bytes;
    }
    rfCSN = 1;
    return status;
}

char rfReadRxPayloadWidth(void) {
    char width;
    rfCSN = 0;
    rfSPIrw(0b01100000);
    width = rfSPIrw(0);
    rfCSN = 1;
    return width;
}

void rfPower(char b) {
    char config;
    rfReadRegister(rfCONFIG, &config, 1);
    if (b) {
        config |= 0b00000010;
    } else {
        config &= 0b11111101;
    }
    rfWriteRegister(rfCONFIG, &config, 1);
    __delay_ms(2);
}

void rfMode(char m) {
    char config = 0;
    rfReadRegister(rfCONFIG, &config, 1);
    if (m == rfRX) {
        config |= 0b00000001;
    } else {
        config &= 0b11111110;
    }
    rfWriteRegister(rfCONFIG, &config, 1);
}

void rfSetRxLen(unsigned char pipe, char len) {
    if (pipe > 5) {
        return;
    }
    char reg = rfRX_PW_P0 + pipe;
    rfWriteRegister(reg, &len, 1);
}

void rfEnableRxInterrupt(char enable) {
    char config;
    rfReadRegister(rfCONFIG, &config, 1);
    if (enable) {
        config &= 0b10111111;
    } else {
        config |= 0b01000000;
    }
    rfWriteRegister(rfCONFIG, &config, 1);
}

void rfEnableTxInterrupt(char enable) {
    char config;
    rfReadRegister(rfCONFIG, &config, 1);
    if (enable) {
        config &= 0b11011111;
    } else {
        config |= 0b00100000;
    }
    rfWriteRegister(rfCONFIG, &config, 1);
}

void rfEnableTxMaxRTInterrupt(char enable) {
    char config;
    rfReadRegister(rfCONFIG, &config, 1);
    if (enable) {
        config &= 0b11101111;
    } else {
        config |= 0b00010000;
    }
    rfWriteRegister(rfCONFIG, &config, 1);
}

char rfReadStatus(void) {
    char status;
    rfCSN = 0;
    status = rfSPIrw(0xff); //NOP
    rfCSN = 1;
    return status;
}

void rfClearAllInterrupts(void) {
    char status = 0b01110000;
    rfWriteRegister(rfSTATUS, &status, 1);
}

void rfClearRxInterrupt(void) {
    char status = 0b01000000;
    rfWriteRegister(rfSTATUS, &status, 1);
}

void rfClearTxInterrupt(void) {
    char status = 0b00100000;
    rfWriteRegister(rfSTATUS, &status, 1);
}

void rfClearTxMaxRTInterrupt(void) {
    char status = 0b00010000;
    rfWriteRegister(rfSTATUS, &status, 1);
}

void rfSetAutoRetryDelay(char value) {
    char b;
    rfReadRegister(rfSETUP_RETR, &b, 1);
    b &= 0b00001111;
    value = value << 4;
    b = value | b;
    rfWriteRegister(rfSETUP_RETR, &b, 1);
}

void rfSetAutoRetryCount(char value) {
    char b;
    rfReadRegister(rfSETUP_RETR, &b, 1);
    b &= 0b11110000;
    value = value & 0b00001111;
    b = value | b;
    rfWriteRegister(rfSETUP_RETR, &b, 1);
}

void rfSetChannel(char chan) {
    chan &= 0b01111111;
    rfWriteRegister(rfRF_CH, &chan, 1);
}

void rfSetDataRate(char rate) {
    char b;
    rfReadRegister(rfRF_SETUP, &b, 1);
    b &= 0b11010111;
    if (rate == rf250KBPS) {
        b |= 0b00100000;
    } else {
        b |= rate << 3;
    }
    rfWriteRegister(rfRF_SETUP, &b, 1);
}

void rfSetOutputPower(char power) {
    char b;
    rfReadRegister(rfRF_SETUP, &b, 1);
    b &= 0b11111001;
    b |= (power << 1);
    rfWriteRegister(rfRF_SETUP, &b, 1);
}

void rfEnableAutoAck(unsigned char pipe, char enable) {
    char r;
    char mask = 1;
    if (pipe > 5) {
        return;
    }
    mask <<= pipe;
    rfReadRegister(rfEN_AA, &r, 1);
    if (enable) {
        r |= mask;
    } else {
        r &= ~mask;
    }
    r &= 0b00111111;
    rfWriteRegister(rfEN_AA, &r, 1);
}

void rfEnablePipe(unsigned char pipe, char enable) {
    char r;
    char mask = 1;
    if (pipe > 5) {
        return;
    }
    mask <<= pipe;
    rfReadRegister(rfEN_RXADDR, &r, 1);
    if (enable) {
        r |= mask;
    } else {
        r &= ~mask;
    }
    r &= 0b00111111;
    rfWriteRegister(rfEN_RXADDR, &r, 1);
}

void rfSetRxAddress(unsigned char pipe, char *address) {
    char len;
    if (pipe > 5) {
        return;
    }
    if (pipe > 1) {
        len = 1;
    } else {
        rfReadRegister(rfSETUP_AW, &len, 1);
        len += 2;
    }
    rfWriteRegister(rfRX_ADDR_P0 + pipe, address, len);
}

void rfSetTxAddress(char *address) {
    char len;
    rfReadRegister(rfSETUP_AW, &len, 1);
    len += 2;
    rfWriteRegister(rfTX_ADDR, address, len);
}

void rfEnableDynamicPayloadLength(unsigned char pipe, char enable) {
    char dynPD;
    char mask = 1;
    if (pipe > 5) {
        return;
    }
    mask <<= pipe;
    rfReadRegister(rfDYNPD, &dynPD, 1);
    if (enable) {
        dynPD |= mask;
    } else {
        dynPD &= ~mask;
    }
    rfWriteRegister(rfDYNPD, &dynPD, 1);
}

void rfSetFeatures(char features) {
    features &= 0b00000111;
    rfWriteRegister(rfFEATURE, &features, 1);
}

char rfWriteTxPayloadNoAck(char *data, char bytes) {
    char status;
    rfCSN = 0;
    status = rfSPIrw(0b10110000);
    while (bytes) {
        rfSPIrw(*data);
        ++data;
        --bytes;
    }
    rfCSN = 1;
    return status;
}

char rfWriteAckPayload(unsigned char pipe, char *data, char bytes) {
    char status;
    char command = 0b10101000;
    command |= (pipe & 0b00000111);
    rfCSN = 0;
    status = rfSPIrw(command);
    while (bytes) {
        rfSPIrw(*data);
        ++data;
        --bytes;
    }
    rfCSN = 1;
    return status;
}

char rfReuseTxPayload(void) {
    char status;
    rfCSN = 0;
    status = rfSPIrw(0b11100011);
    rfCSN = 1;
    return status;
}

char rfFlushTx(void) {
    char status;
    rfCSN = 0;
    status = rfSPIrw(0b11100001);
    rfCSN = 1;
    return status;
}

char rfFlushRx(void) {
    char status;
    rfCSN = 0;
    status = rfSPIrw(0b11100010);
    rfCSN = 1;
    return status;
}

char rfGetRxPipe(void) {
    char pipe;
    pipe = rfReadStatus();
    pipe &= 0b00001110;
    pipe >>= 1;
    return pipe;
}

char rfTxFIFOFull(void) {
    char fifoStatus;
    rfReadRegister(rfFIFO_STATUS, &fifoStatus, 1);
    return fifoStatus & 0b00100000;
}

char rfTxFIFOEmpty(void) {
    char fifoStatus;
    rfReadRegister(rfFIFO_STATUS, &fifoStatus, 1);
    return fifoStatus & 0b00010000;
}

char rfRxFIFOFull(void) {
    char fifoStatus;
    rfReadRegister(rfFIFO_STATUS, &fifoStatus, 1);
    return fifoStatus & 0b00000010;
}

char rfRxFIFOEmpty(void) {
    char fifoStatus;
    rfReadRegister(rfFIFO_STATUS, &fifoStatus, 1);
    return fifoStatus & 0b00000001;
}

void rfSetAddressLength(char len) {
    if (len < 3 || len > 5) {
        return;
    } else {
        len -= 2;
        rfWriteRegister(rfSETUP_AW, &len, 1);
    }
}
