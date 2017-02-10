//=========================================================
// USB_JTAG Project
//=========================================================
// File Name : blaster.h
// Function  : USB Blaster Control Header
//---------------------------------------------------------
// Rev.01 2015.03.01 Munetomo Maruyama
//---------------------------------------------------------
// Copyright (C) 2015 Munetomo Maruyama
//=========================================================

#ifndef __BLASTER_H__
#define	__BLASTER_H__

//#define DEBUG

#include <system.h>
#include <system_config.h>

#include <xc.h>

//------------
// Prototype
//------------
#ifdef DEBUG
void USART_Init(void);
uint8_t USART_TxRdy(void);
uint8_t USART_RxRdy(void);
void USART_Put_Char(uint8_t ch);
uint8_t USART_Get_Char(void);
void USART_Put_String(const char *str);
void USART_Put_Hex(uint8_t data);
#endif
//
void Blaster_Init(void);
void USB_Ept_Init(void);
//
uint8_t EEPROM_Read(uint8_t addr);
void EEPROM_Checksum(void);
//
void USB_FT245_Emulation(void);
//
void TXFIFO_Init(void);
void TXFIFO_Write(uint8_t data);
void TXFIFO_Read(uint8_t *ptr, uint8_t size);
uint16_t TXFIFO_Fill(void);
uint16_t TXFIFO_Room(void);
//
void TIMER_Start_10ms(void);
uint8_t TIMER_Reach_10ms(void);
//
void Main_Task_Blaster(void);

#endif	// __BLASTER_H__

//=========================================================
// End of Program
//=========================================================


