/*----------------------------------------------------------------------------
 * Name:    can.h
 * Purpose: CAN interface for LPC11C1x
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2010 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#ifndef _CAN_H_
#define _CAN_H_

#include <stdint.h>                   /* Include standard types               */

#define STANDARD_FORMAT  0
#define EXTENDED_FORMAT  1

#define DATA_FRAME       0
#define REMOTE_FRAME     1

typedef struct  {
  uint32_t  id;                       /* 29 bit identifier                    */
  uint8_t   data[8];                  /* Data field                           */
  uint8_t   len;                      /* Length of data field in bytes        */
  uint8_t   format;                   /* 0 - STANDARD, 1- EXTENDED IDENTIFIER */
  uint8_t   type;                     /* 0 - DATA FRAME, 1 - REMOTE FRAME     */
} CAN_msg;

/* Functions defined in module CAN.c */
void CAN_setup         (uint32_t baudrate);
void CAN_init          (void);
void CAN_start         (void);
void CAN_stop          (void);
void CAN_waitReady     (void);
int  CAN_isboff        (void);
void CAN_wrMsg         (CAN_msg *msg);
void CAN_stMsg         (CAN_msg *msg);
void CAN_rdMsg         (uint32_t can_msgObj, CAN_msg *msg);
void CAN_wrFilter      (uint32_t id, uint8_t filter_type);
void CAN_noFilter      (uint8_t format);

void CAN_testmode      (void);

extern CAN_msg       CAN_TxMsg;       /* CAN messge for sending               */
extern CAN_msg       CAN_RxMsg;       /* CAN message for receiving            */                                
extern CAN_msg       CAN_RtMsg;       /* CAN message for RTR                  */                                
extern unsigned int  CAN_TxRdy;       /* CAN HW ready to transmit a message   */
extern unsigned int  CAN_RxRdy;       /* CAN HW received a message            */

#if 0
#define CAN_LOOPBACK(msg)  
#else
extern void obd_loopback(CAN_msg *p);
#define CAN_LOOPBACK(msg)  {obd_loopback(msg);CAN_RxRdy=0;}
#endif

#endif // _CAN_H_


