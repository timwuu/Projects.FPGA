//=========================================================
// USB_JTAG Project
//=========================================================
// File Name : ssytem.h
// Function  : System Routine
//---------------------------------------------------------
// Rev.01 2015.02.11 Munetomo Maruyama
//---------------------------------------------------------
// Copyright (C) 2015 Munetomo Maruyama
//=========================================================

#include <system.h>
#include <system_config.h>
#include <usb/usb.h>

//--------------------------------
// Configuration Bits
//--------------------------------

#ifdef _PIC16F1459_H_
// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config CPUDIV = NOCLKDIV// CPU System Clock Selection Bit (NO CPU system divide)
#pragma config USBLSCLK = 48MHz // USB Low SPeed Clock Selection bit (System clock expects 48 MHz, FS/LS USB CLKENs divide-by is set to 8.)
#pragma config PLLMULT = 3x     // PLL Multipler Selection Bit (3x Output Frequency Selected)
#pragma config PLLEN = ENABLED  // PLL Enable Bit (3x or 4x PLL Enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

#endif

#ifdef _PIC18F14K50_H_

#pragma config CPUDIV = NOCLKDIV
#pragma config USBDIV = OFF
#pragma config FOSC   = HS
#pragma config PLLEN  = ON
#pragma config FCMEN  = OFF
#pragma config IESO   = OFF
#pragma config PWRTEN = OFF
#pragma config BOREN  = OFF
#pragma config BORV   = 30
#pragma config WDTEN  = OFF
#pragma config WDTPS  = 32768
#pragma config MCLRE  = OFF
#pragma config HFOFST = OFF
#pragma config STVREN = ON
#pragma config LVP    = ON  // enable low voltage program
#pragma config XINST  = OFF
#pragma config BBSIZ  = OFF
#pragma config CP0    = OFF
#pragma config CP1    = OFF
#pragma config CPB    = OFF
#pragma config WRT0   = OFF
#pragma config WRT1   = OFF
#pragma config WRTB   = OFF
#pragma config WRTC   = OFF
#pragma config EBTR0  = OFF
#pragma config EBTR1  = OFF
#pragma config EBTRB  = OFF

#endif


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
void SYSTEM_Initialize( SYSTEM_STATE state )
{
    switch(state)
    {
        case SYSTEM_STATE_USB_START:
            #if defined(USE_INTERNAL_OSC)
                //Make sure to turn on active clock tuning for USB full speed 
                //operation from the INTOSC
                #if defined(_PIC14E)
                OSCCON = 0xFC;  //HFINTOSC @ 16MHz, 3X PLL, PLL enabled
                ACTCON = 0x90;  //Active clock tuning enabled for USB
                #endif
            #endif

            Blaster_Init();
    
            //LED_Enable(LED_USB_DEVICE_STATE);
            //LED_Enable(LED_DEVICE_VENDOR_BASIC_DEMO);
            //BUTTON_Enable(BUTTON_DEVICE_VENDOR_BASIC_DEMO);
            break;
            
        case SYSTEM_STATE_USB_SUSPEND: 
            break;
            
        case SYSTEM_STATE_USB_RESUME:
            break;
    }
}

		
//------------------------
// USB Interrupt Service
//------------------------
void interrupt SYS_InterruptHigh(void)
{
    #if defined(USB_INTERRUPT)
        USBDeviceTasks();
    #endif
}


//=========================================================
// End of Program
//=========================================================
