//=========================================================
// USB_JTAG Project
//=========================================================
// File Name : system.h
// Function  : System Header
//---------------------------------------------------------
// Rev.01 2015.02.11 Munetomo Maruyama
//---------------------------------------------------------
// Copyright (C) 2015 Munetomo Maruyama
//=========================================================

#ifndef __SYSTEM_H__
#define	__SYSTEM_H__

#include <xc.h>
#include <stdbool.h>
#include "blaster.h"

//Internal oscillator option setting.  Uncomment if using HFINTOSC+active clock 
//tuning, instead of a crystal.  
#define USE_INTERNAL_OSC        //Make sure 1uF-8uF extra capacitance is added on VDD net
                                //to smooth VDD ripple from MAX3232 chip, before using this
                                //with the original Low Pin Count USB Development Kit board.
                                //If using the latest version of the board, this is not
                                //required and is already present.

#define MAIN_RETURN void

#define __PIC16

//------------
// Prototype
//------------

/*** System States **************************************************/
typedef enum
{
    SYSTEM_STATE_USB_START,
    SYSTEM_STATE_USB_SUSPEND,
    SYSTEM_STATE_USB_RESUME
} SYSTEM_STATE;

/*********************************************************************
* Function: void SYSTEM_Initialize( SYSTEM_STATE state )
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  SYSTEM_STATE - the state to initialize the system into
*
* Output: None
*
********************************************************************/
void SYSTEM_Initialize( SYSTEM_STATE state );


#endif	// __SYSTEM_H__

//=========================================================
// End of Program
//=========================================================
