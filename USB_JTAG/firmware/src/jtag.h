//=========================================================
// USB_JTAG Project
//=========================================================
// File Name : jtag.h
// Function  : JTAG Control Header
//---------------------------------------------------------
// Rev.01 2015.02.11 Munetomo Maruyama
//---------------------------------------------------------
// Copyright (C) 2015 Munetomo Maruyama
//=========================================================

#ifndef __JTAG_H__
#define	__JTAG_H__

#include <system.h>
#include <system_config.h>

#include <xc.h>

//--------------------------
// GPIO Assignment
//--------------------------
// RC2 (out) ... LED (out)
// RB6 (SCK) ... TCK (out)
// RC6 (out) ... TMS (out)
// RC7 (SDO) ... TDI (out)
// RB4 (SDI) ... TDO (in)
//--------------------------

//--------------
// Define Port
//--------------
#define LED LATCbits.LATC2
#define TCK LATBbits.LATB6
#define TMS LATCbits.LATC6
#define TDI LATCbits.LATC7
#define TDO PORTBbits.RB4

//------------
// Prototype
//------------
void JTAG_Init(void);
void JTAG_Mode_SPI(void);
void JTAG_Mode_PORT(void);
void JTAG_SPI_Wait(void);
void JTAG_SPI_Put(uint8_t byte);
uint8_t JTAG_SPI_Get(void);
uint8_t bitrev(uint8_t byte);

#endif	// __JTAG_H__

//=========================================================
// End of Program
//=========================================================
