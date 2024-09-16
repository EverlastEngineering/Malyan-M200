//*================================================================================================
//* 文件名				: xmodem.h
//* 功能描述			: xmodem文件传输协议相关定义
//* 作者    			: 罗伟东
//* 版本				: 0.01
//* 建立日期、时间		: 2011/04/24 17:56
//* 最近修改日期、时间	: 
//* 修改原因			: 
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
