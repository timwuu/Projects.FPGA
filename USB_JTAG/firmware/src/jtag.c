//=========================================================
// USB_JTAG Project
//=========================================================
// File Name : jtag.c
// Function  : JTAG Control Routine
//---------------------------------------------------------
// Rev.01 2015.02.11 Munetomo Maruyama
//---------------------------------------------------------
// Copyright (C) 2015 Munetomo Maruyama
//=========================================================


#include "jtag.h"

//------------------
// Initialize JTAG
//------------------
void JTAG_Init(void)
{
    //
    // Analog Multiplexed Port
    //
    // Slew Rate Control
    //
    
#ifdef _PIC18F14K50_H_
    ANSEL  = 0x00; // disable analog on RC3, RC2, RC1, RC0, RA4
    ANSELH = 0x00; // disable analog on RB5, RB4, RC7, RC6
    SLRCON = 0x00; // standard slew rate on PORTA,B,C
#endif
    //
#ifdef _PIC16F1459_H_    
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;
#endif
    
    //
    // Default Port Output Level
    //
    LED = 0;
    TCK = 0;
    TMS = 1;
    TDI = 0;
    //
    // PORT Direction
    //
    TRISB = 0x30; // PB7-4: ooii----
    TRISC = 0x3b; // PC7-0: ooiiioii
    //
    // Configure SPI
    //     CKP=0, CKE=1 : TCK default low
    //     SMP=0        : sampling at TCK rise
    SSPSTAT = 0x40;
    SSPCON1 = 0x00; // Master, 12MHz
    //SSPCON1 = 0x01; // Master, 3MHz
}

//--------------------------
// Select JTAG Mode as SPI
//--------------------------
#ifdef _PIC16F1459_H_
void JTAG_Mode_SPI(void)
{
    TRISBbits.TRISB6 = 1; // prevent glitch on TCK
    SSPCON1bits.SSPEN = 1;
    TRISBbits.TRISB6 = 0;
}
#endif

#ifdef _PIC18F14K50_H_
void JTAG_Mode_SPI(void)
{
    TRISBbits.RB6 = 1; // prevent glitch on TCK
    SSPCON1bits.SSPEN = 1;
    TRISBbits.RB6 = 0;
}
#endif

//--------------------------
// Select JTAG Mode as Port
//--------------------------
void JTAG_Mode_PORT(void)
{
    SSPCON1bits.SSPEN = 0;
}

//----------------------------
// Wait for SPI Transfer Done
//----------------------------
void JTAG_SPI_Wait(void)
{
    while(SSPSTATbits.BF == 0);
}

//----------------------
// Put a byte to SPI
//----------------------
void JTAG_SPI_Put(uint8_t byte)
{
    SSPBUF = bitrev(byte);
}

//----------------------
// Get a byte from SPI
//----------------------
uint8_t JTAG_SPI_Get(void)
{
    return bitrev(SSPBUF);
}

//---------------
// Bit Reverse
//---------------
const uint8_t REV[256] =
{
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
    0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
    0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
    0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
    0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
    0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
    0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
    0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
    0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
    0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
    0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
    0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
    };
//
uint8_t bitrev(uint8_t byte)
{
    return REV[byte];
}

//=========================================================
// End of Program
//=========================================================