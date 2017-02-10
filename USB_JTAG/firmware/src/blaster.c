//=========================================================
// USB_JTAG Project
//=========================================================
// File Name : blaster.c
// Function  : USB Blaster Control Routine
//---------------------------------------------------------
// Rev.01 2015.03.01 Munetomo Maruyama
//---------------------------------------------------------
// Copyright (C) 2015 Munetomo Maruyama
//=========================================================

#include <system.h>

#include <usb/usb.h>
#include <usb/usb_device_generic.h>

#include "blaster.h"
#include "jtag.h"

//--------------------
// Define Constant
//--------------------
#define SD001_SIZ (USB_SD_Ptr[1][0])
#define SD002_SIZ (USB_SD_Ptr[2][0])
#define SD003_SIZ (USB_SD_Ptr[3][0])
#define SD001_POS (14 + 6)
#define SD002_POS (SD001_POS + SD001_SIZ)
#define SD003_POS (SD002_POS + SD002_SIZ)
#define SD004_POS (SD003_POS + SD003_SIZ)
#define TXFIFO_SIZE (256)  //dont' change this value, ring buffer 

//-------------
// Globals
//-------------
extern const USB_DEVICE_DESCRIPTOR device_dsc; // Device Descriptor
extern const uint8_t configDescriptor1[];      // Configuration 1 Descriptor
extern const uint8_t *const USB_SD_Ptr[];      // Array of string descriptors
extern volatile CTRL_TRF_SETUP SetupPkt;       // EP0 Buffer Space
extern volatile uint8_t CtrlTrfData[];         // EP0 Buffer Space
//

#ifdef _PIC18F14K50_H_
USB_VOLATILE uint8_t USB_InPacket[USBGEN_EP_SIZE]     @0x240; // In Packet
USB_VOLATILE uint8_t USB_OutPacket[2][USBGEN_EP_SIZE] @0x280; // Out Packet
#else

#ifdef _PIC16F1459_H_
// check datasheet DUAL PORT RAM ADDRESSING
USB_VOLATILE uint8_t USB_OutPacket[2][USBGEN_EP_SIZE] @0x20A0;  //0x120; // Out Packet
USB_VOLATILE uint8_t USB_InPacket[USBGEN_EP_SIZE]     @0x2140;  //0x220; // In Packet
#endif

#endif



//
USB_HANDLE USB_OutHandle0;
USB_HANDLE USB_OutHandle1;
USB_HANDLE USB_InHandle0;
//
uint16_t eeprom_checksum; // EEPROM Checksum
//
uint8_t  TXFIFO[TXFIFO_SIZE]; // TXFIFO, 256 ring buffer
uint8_t  TXFIFO_rp, TXFIFO_wp;
uint16_t TXFIFO_ocpy;
//
enum Rx_State_Enum {RX_INIT, RX_PPB_0, RX_PPB_1};
enum JTAG_State_Enum {JTAG_BIT_BANG, JTAG_BYTE_SHIFT};
//
uint8_t Rx_State;
uint8_t RxSize;
uint8_t ValidPPB;
uint8_t RxPtr;
uint8_t JTAG_State;
uint8_t Read;
uint8_t ByteSize;
uint8_t nCS;

#ifdef DEBUG
//------------------
// USART Port
//------------------
#define UART_TRISTx   TRISBbits.TRISB7
#define UART_TRISRx   TRISBbits.TRISB5
#define UART_Tx       PORTBbits.RB7
#define UART_Rx       PORTBbits.RB5

//------------------
// USART Initialize
//------------------
void USART_Init(void)
{
    unsigned char c;

#ifdef _PIC18F14K50_H_
    ANSELHbits.ANS11 = 0;   // Make RB5 pin digital
#endif
    
    UART_TRISRx=1;          // RX
    UART_TRISTx=0;          // TX
    TXSTA = 0x24;           // TX enable BRGH=1
    RCSTA = 0x90;           // Single Character RX
    //SPBRG = 0x71;
    //SPBRGH = 0x02;          // 0x0271 for 48MHz -> 19200 baud
    //timijk 2016.12.11
    SPBRG = 0x0C;
    SPBRGH = 0x00;          // 0x000C for 48MHz -> 921600 baud 
    BAUDCON = 0x08;         // BRG16 = 1
    c = RCREG;              // read
}
//-----------------
// USART Tx Ready
//-----------------
uint8_t USART_TxRdy(void)
{
    return TXSTAbits.TRMT;
}
//-----------------
// USART Rx Ready
//-----------------
uint8_t USART_RxRdy(void)
{
    return PIR1bits.RCIF;
}
//-----------------
// USART Put Char
//-----------------
void USART_Put_Char(uint8_t ch)
{
    TXREG = ch;
}
//-----------------
// USART Get Char
//-----------------
uint8_t USART_Get_Char(void)
{
    return RCREG;
}
//--------------------
// USART Put String
//--------------------
void USART_Put_String(const char *str)
{
    uint8_t ch;
    while ((ch = *str++) != '\0')
    {
        while(!USART_TxRdy());
        USART_Put_Char(ch);
    }
}
//--------------------
// USART Put Hex
//--------------------
void USART_Put_Hex(uint8_t data)
{
    uint8_t h = (data >> 4);
    uint8_t l = (data & 0x0f);
    h = (h < 0x0a)? ('0' + h) : ('A' + h - 0x0A);
    l = (l < 0x0a)? ('0' + l) : ('A' + l - 0x0A);
    USART_Put_String("0x");
    while(!USART_TxRdy());
    USART_Put_Char(h);
    while(!USART_TxRdy());
    USART_Put_Char(l);
}
#endif

//---------------------------
// Blaster Initialization
//---------------------------
void Blaster_Init(void)
{
    // Initialize EEPROM Checksum
    EEPROM_Checksum();

    // Initialize JTAG Port and SPI
    JTAG_Init();

    #ifdef DEBUG
    // Initialize USART
    USART_Init();
    USART_Put_String("USB_JTAG\n\r");
    #endif
    
    // Start 10ms Timer
    TIMER_Start_10ms();
}

//-------------------------------
// USB Endpoint Initialization
//-------------------------------
void USB_Ept_Init(void)
{
    // Initialize the variable holding the handle for the last transmission
    USB_OutHandle0 = 0;
    USB_OutHandle1 = 0;
    USB_InHandle0 = 0;

    // Initialize TXFIFO
    TXFIFO_Init();

    // Initialize State
    Rx_State = RX_INIT;
    JTAG_State = JTAG_BIT_BANG;
    JTAG_Mode_PORT();
    ByteSize = 0;
    nCS = 1;

    // Initialize End Point
    USBEnableEndpoint(1,USB_IN_ENABLED |USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    USBEnableEndpoint(2,USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
}

//----------------------------------------------------
// FT245 Emulation, a response for EVENT_EP0_REQUEST
//----------------------------------------------------
void USB_FT245_Emulation(void)
{
    // Vendor Request?
    if (SetupPkt.RequestType == 2)
    {
        // Responce by sending zero-length packet
        // thanks to USB-Blaster7, http://sa89a.net
        if (SetupPkt.DataDir == 0)
        {
            USBEP0SendRAMPtr((uint8_t*)CtrlTrfData, 0, USB_EP0_INCLUDE_ZERO);
        }
        // IN Request
        else if (SetupPkt.bRequest == 0x90)
        {
            uint8_t addr = (SetupPkt.wIndex << 1) & 0x7e;
            CtrlTrfData[0] = EEPROM_Read(addr + 0);
            CtrlTrfData[1] = EEPROM_Read(addr + 1);
        }
        // Dummy Data
        else
        {
            CtrlTrfData[0] = 0x36;
            CtrlTrfData[1] = 0x83;
        }
        USBEP0SendRAMPtr((uint8_t*)CtrlTrfData, 2, USB_EP0_INCLUDE_ZERO);
    }
}

//----------------
// EEPROM Read
//----------------
uint8_t EEPROM_Read(uint8_t addr)
{
    uint8_t data = 0;
    //
    switch(addr)
    {
        case   0 : {data = 0; break;}
        case   1 : {data = 0; break;}
        case   2 : {data = ((uint8_t*)(&device_dsc.idVendor))[0]; break;} // vid
        case   3 : {data = ((uint8_t*)(&device_dsc.idVendor))[1]; break;} // vid
        case   4 : {data = ((uint8_t*)(&device_dsc.idProduct))[0]; break;} // pid
        case   5 : {data = ((uint8_t*)(&device_dsc.idProduct))[1]; break;} // pid
        case   6 : {data = ((uint8_t*)(&device_dsc.bcdDevice))[0]; break;} // ver
        case   7 : {data = ((uint8_t*)(&device_dsc.bcdDevice))[1]; break;} // ver
        case   8 : {data = configDescriptor1[7]; break;} // attr
        case   9 : {data = configDescriptor1[8]; break;} // power
        case  10 : {data = 0x1c; break;}
        case  11 : {data = 0; break;}
        case  12 : {data = ((uint8_t*)(&device_dsc.bcdUSB))[0]; break;} // usbver
        case  13 : {data = ((uint8_t*)(&device_dsc.bcdUSB))[1]; break;} // usbver
        case  14 : {data = 0x80 + SD001_POS; break;} // 0x80 + &sd001
        case  15 : {data =        SD001_SIZ; break;} // sizeof(sd001)
        case  16 : {data = 0x80 + SD002_POS; break;} // 0x80 + &sd002
        case  17 : {data =        SD002_SIZ; break;} // sizeof(sd002)
        case  18 : {data = 0x80 + SD003_POS; break;} // 0x80 + &sd003
        case  19 : {data =        SD003_SIZ; break;} // sizeof(sd003)
        case 126 : {data = (uint8_t)(eeprom_checksum & 0x00ff); break;} // lsb
        case 127 : {data = (uint8_t)(eeprom_checksum >> 8);     break;} // msb
        default  : {data = 0; break;} // Do Nothing
    }
    //
    if ((SD001_POS <= addr) && (addr < SD002_POS))
    {
        data = USB_SD_Ptr[1][addr - SD001_POS];
    }
    else if ((SD002_POS <= addr) && (addr < SD003_POS))
    {
        data = USB_SD_Ptr[2][addr - SD002_POS];
    }
    else if ((SD003_POS <= addr) && (addr < SD004_POS))
    {
        data = USB_SD_Ptr[3][addr - SD003_POS];
    }
    //
    return data;
}

//--------------------
// EEPROM Checksum
//--------------------
void EEPROM_Checksum(void)
{
    uint8_t i;
    uint16_t eeprom_data;
    eeprom_checksum = 0xaaaa;
    for (i = 0; i < 126; i = i + 2)
    {
        eeprom_data = (uint16_t)(EEPROM_Read(i+1)); // msb
        eeprom_data = eeprom_data << 8;
        eeprom_data = eeprom_data | (uint16_t)(EEPROM_Read(i)); // lsb
        eeprom_checksum = eeprom_checksum ^ eeprom_data;
        //
        //timijk 2016.12.10 
        // eeprom_checksum = (eeprom_checksum << 1) | (eeprom_checksum >> 15);
        if(eeprom_checksum&0x8000) eeprom_checksum= (eeprom_checksum<<1)|1;
        else eeprom_checksum= (eeprom_checksum<<1);
    }    
}

//-------------------
// TXFIFO Initialize
//-------------------
void TXFIFO_Init(void)
{
    TXFIFO_rp = 0;
    TXFIFO_wp = 0;
    TXFIFO_ocpy = 0;
}

//-----------------
// TXFIFO Write
//-----------------
void TXFIFO_Write(uint8_t data)
{
    TXFIFO[TXFIFO_wp++] = data;
    TXFIFO_ocpy++;
}

//-----------------
// TXFIFO Read
//-----------------
void TXFIFO_Read(uint8_t *ptr, uint8_t size)
{
    TXFIFO_ocpy = TXFIFO_ocpy - (uint16_t)size;
    while(size > 0)
    {
        *ptr++ = TXFIFO[TXFIFO_rp++];
        size--;
    }
}

//--------------------
// TXFIFO Fill Count
//--------------------
uint16_t TXFIFO_Fill(void)
{
    return TXFIFO_ocpy;
}

//--------------------
// TXFIFO Room Count
//--------------------
uint16_t TXFIFO_Room(void)
{
    return (TXFIFO_SIZE - TXFIFO_ocpy);
}

#ifdef _PIC18F14K50_H_
//-------------------------
// Timer0 Start 10ms Count
//-------------------------
void TIMER_Start_10ms(void)
{
    // Disable Interrupt (always)
    INTCONbits.TMR0IE=0;
    // Stop Timer
    T0CON=0x00;
    // Clear Flag
    INTCONbits.TMR0IF = 0;
    // 10ms --> 100Hz --> (12MHz/2)/60000
    TMR0H = (65536 - 60000) >> 8;
    TMR0L = (65536 - 60000) &0x00FF;
    // Start Timer, 16bit, Prescaler=1:2
    T0CON=0x80;
}

//----------------------
// Timer0 Reach 10ms?
//----------------------
uint8_t TIMER_Reach_10ms(void)
{
    uint8_t flag;
    flag = INTCONbits.TMR0IF;
    INTCONbits.TMR0IF = 0;
    return flag;
}

#endif

#ifdef _PIC16F1459_H_
//-------------------------
// Timer2 Start 10ms Count
//-------------------------
void TIMER_Start_10ms(void)
{
    //Setup Keepalive Timer 100Hz
    //  Setup TMR2  //12MHz -> 100Hz(10ms) period
    //  Prescalor: 64, Postscalor: 15
    TMR2IE = 0;
    T2CON = 0b01110011;
    TMR2 = 0;
    PR2 = 124;
    TMR2IF = 0;

    //Turn On TMR2
    T2CONbits.TMR2ON = 1;
}

//----------------------
// Timer2 Reach 10ms?
//----------------------
uint8_t TIMER_Reach_10ms(void)
{
    uint8_t flag;
    flag = TMR2IF;
    TMR2IF = 0;
    return flag;
}

#endif

//---------------------------
// Main Task for Blaster
//---------------------------
void Main_Task_Blaster(void)
{    
    //=====================
    // RX Handling
    //=====================
    switch (Rx_State)
    {
        //---------------------------------
        // Initialize
        //---------------------------------
        case RX_INIT :
        {
            // Arm the application OUT endpoint,
            // so it can receive a packet from the host
            USB_OutHandle0 = USBGenRead(2, (uint8_t*)&USB_OutPacket[0], 64);
            RxSize = 0;
            Rx_State = RX_PPB_0;
            break;
        }
        //----------------------------------------
        // Receive Packet into Ping-Pong Buffer 0
        //----------------------------------------
        case RX_PPB_0 :
        {
            if (RxSize == 0)
            {
                if (!USBHandleBusy(USB_OutHandle0))
                {
                    RxSize = USBHandleGetLength(USB_OutHandle0);
                    ValidPPB = 0;
                    RxPtr = 0;
                    Rx_State = RX_PPB_1;
                    // Re-Arm the application OUT endpoint,
                    // so it can receive a packet from the host
                    USB_OutHandle1 = USBGenRead(2, (uint8_t*)&USB_OutPacket[1], 64);
                    //
                    #ifdef DEBUG
                    USART_Put_String("RX_PPB_0 done. RxSize=");
                    USART_Put_Hex(RxSize);
                    USART_Put_String("\n\r");
                    #endif
                }
            }
            break;
        }
        //----------------------------------------
        // Receive Packet into Ping-Pong Buffer 1
        //----------------------------------------
        case RX_PPB_1 :
        {
            if (RxSize == 0)
            {
                if (!USBHandleBusy(USB_OutHandle1))
                {
                    RxSize = USBHandleGetLength(USB_OutHandle1);
                    ValidPPB = 1;
                    RxPtr = 0;
                    Rx_State = RX_PPB_0;
                    // Re-Arm the application OUT endpoint,
                    // so it can receive a packet from the host
                    USB_OutHandle0 = USBGenRead(2, (uint8_t*)&USB_OutPacket[0], 64);
                    //
                    #ifdef DEBUG
                    USART_Put_String("RX_PPB_1 done. RxSize=");
                    USART_Put_Hex(RxSize);
                    USART_Put_String("\n\r");
                    #endif
                }
            }
            break;
        }
        //----------------------
        // Never Reach Here
        //----------------------
        default :
        {
            Rx_State = RX_INIT;
            break;
        }
    } // switch (Rx_State)

  //if (RxSize > 0) LED = 1; // OK

    //=====================
    // JTAG Handling
    //=====================
    switch (JTAG_State)
    {
        //---------------------------------
        // Bit Bang Mode
        //---------------------------------
        case JTAG_BIT_BANG :
        {
            if ((RxSize > 0) && (TXFIFO_Room() >=  RxSize))
            {
                JTAG_Mode_PORT();
                //
                while(RxSize > 0)
                {
                    uint8_t opcode;
                    opcode = USB_OutPacket[ValidPPB][RxPtr++];
                    RxSize--;
                    //
                    Read = opcode & 0x40;
                    //
                    if (opcode & 0x80) // Go To Byte Shift Mode?
                    {
                        TCK = 0;
                        ByteSize = opcode & 0x3f;
                        JTAG_State = JTAG_BYTE_SHIFT;
                        //
                        #ifdef DEBUG
                        USART_Put_String("GOTO_BYTE_SHIFT. opcode=");
                        USART_Put_Hex(opcode);
                        USART_Put_String(" ByteSize=");
                        USART_Put_Hex(ByteSize);
                        USART_Put_String("\n\r");
                        #endif
                        //
                        break; // Quit from Bit Bang Mode
                    }
                    else // Stay Bit Bang Mode
                    {
                        //TMS and TDI go before TCK
                        TMS = (opcode & 0x02)? 1 : 0;
                        TDI = (opcode & 0x10)? 1 : 0;
                        TCK = (opcode & 0x01)? 1 : 0;
                        LED = (opcode & 0x20)? 1 : 0;
                        nCS = (opcode & 0x08)? 1 : 0;
                        //
                        if (Read)
                        {
                            TXFIFO_Write(TDO | 0x02); // AS is always assumed High.
                        }
                        //
                        #ifdef DEBUG
                        USART_Put_String("JTAG_BIT_BANG. RxSize=");
                        USART_Put_Hex(RxSize);
                        USART_Put_String(" opcode=");
                        USART_Put_Hex(opcode);
                        USART_Put_String(" Read=");
                        USART_Put_Hex(Read);
                        USART_Put_String("\n\r");
                        #endif
                    }
                } // while
            } // if
            break;
        }
        //---------------------------------
        // Byte Shift Mode
        //---------------------------------
        case JTAG_BYTE_SHIFT :
        {
            if ((RxSize > 0) && (TXFIFO_Room() >=  RxSize))
            {
                JTAG_Mode_SPI();
                //
                while((RxSize > 0) && (ByteSize > 0))
                {
                    uint8_t tx;
                    //uint8_t rx;
                    tx = USB_OutPacket[ValidPPB][RxPtr++];
                    JTAG_SPI_Put(tx);
                    JTAG_SPI_Wait();
                    //rx = JTAG_SPI_Get();
                    if (Read)
                    {
                        if (nCS)
                          //TXFIFO_Write(rx);
                            TXFIFO_Write(JTAG_SPI_Get());
                        else
                            TXFIFO_Write(0xff); // suppose AS = High
                    }
                    RxSize--;
                    ByteSize--;
                } // while
                //
                #ifdef DEBUG
                USART_Put_String("JTAG_BYTE_SHIFT. RxSize=");
                USART_Put_Hex(RxSize);
                USART_Put_String(" ByteSize=");
                USART_Put_Hex(ByteSize);
                USART_Put_String(" Read=");
                USART_Put_Hex(Read);
                USART_Put_String("\n\r");
                #endif
            }
            //
            if (ByteSize == 0)
            {
                JTAG_State = JTAG_BIT_BANG;
            }
            break;
        }
        //----------------------
        // Never Reach Here
        //----------------------
        default :
        {
            JTAG_State = JTAG_BIT_BANG;
            break;
        }
    } // switch (JTAG_State)


    //=====================
    // TX Handling
    //=====================
    if (!USBHandleBusy(USB_InHandle0))
    {
        uint8_t txfifo_size;

        txfifo_size = TXFIFO_Fill();
        //
        #ifdef DEBUG
        if (txfifo_size > 0)
        {
            USART_Put_String("Tx Handling. txfifo_size=");
            USART_Put_Hex(txfifo_size);
            USART_Put_String("\n\r");
        }
        #endif

        //-----------------------
        // Send TxFIFO Contents
        //-----------------------
        if (txfifo_size > 0)
        {
            uint8_t txsize;
            txsize = (txfifo_size < 62)? txfifo_size : 62;

            // Initialize Packet
            USB_InPacket[0] = 0x31;
            USB_InPacket[1] = 0x60;

            // Get TXFIFO to USB_InPacket
            TXFIFO_Read((uint8_t*)(USB_InPacket + 2), txsize);
            // Send InPacket
            USB_InHandle0 = USBGenWrite(1, (uint8_t*)&USB_InPacket, txsize + 2);
            // Re-Start 10ms Timer
            TIMER_Start_10ms();
        }
        //-----------------------------------------------------
        // TXFIFO has no data, but need to send dummy 2bytes
        //-----------------------------------------------------
        else if (TIMER_Reach_10ms())
        {
            // Initialize Packet
            USB_InPacket[0] = 0x31;
            USB_InPacket[1] = 0x60;

            // Send InPacket (dummy 2 bytes)
            USB_InHandle0 = USBGenWrite(1, (uint8_t*)&USB_InPacket, 2);
            // Re-Start 10ms Timer
            TIMER_Start_10ms();
        }
    }
}

//=========================================================
// End of Program
//=========================================================
