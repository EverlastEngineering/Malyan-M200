//*================================================================================================
//* �ļ���				: xmodem.h
//* ��������			: xmodem�ļ�����Э����ض���
//* ����    			: ��ΰ��
//* �汾				: 0.01
//* �������ڡ�ʱ��		: 2011/04/24 17:56
//* ����޸����ڡ�ʱ��	: 
//* �޸�ԭ��			: 
//*================================================================================================
#ifndef __XMODEM_H
#define __XMODEM_H

#include "stm32f0xx_conf.h"

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define CHECK_CRC		0x01
#define CHECK_SUM		0x02

#define SUCCESSFULL     0X00
#define TIME_OUT        0X01
#define USER_CANCELED   0X02
#define PARAMETER_ERROR 0X03

#define DLY_1S 1000
#define MAXRETRANS 25

#define	NO_DATA		0
#define	HAVE_DATA	1

extern uint8_t xmodemReceive(uint8_t checkType);
extern char port_inbyte(uint32_t Time);


#endif
