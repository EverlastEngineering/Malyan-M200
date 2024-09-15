/**
  ******************************************************************************
  * @file    SPI_MSD0_Driver.c
  * $Author: wdluo $
  * $Revision: 17 $
  * $Date:: 2012-07-06 11:16:48 +0800 #$
  * @brief   Ö÷º¯Êý.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, ViewTool</center>
  *<center><a href="http:\\www.viewtool.com">http://www.viewtool.com</a></center>
  *<center>All Rights Reserved</center></h3>
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "SPI_MSD0_Driver.h"
#include <stdio.h>
//#define PRINT_INFO  1

/* Private define ------------------------------------------------------------*/
extern uint8_t model;
void MSD0_card_enable()
{
    if (model==4 || model==5)
        GPIO_ResetBits(GPIOB,GPIO_Pin_0);
    else if (model==6)
        GPIO_ResetBits(GPIOB,GPIO_Pin_6);
    else 
        GPIO_ResetBits(GPIOA,GPIO_Pin_4);
}
void MSD0_card_disable()
{
    if (model==4 || model==5)
        GPIO_SetBits(GPIOB,GPIO_Pin_0);
    else if (model==6)
        GPIO_SetBits(GPIOB,GPIO_Pin_6);
    else
        GPIO_SetBits(GPIOA,GPIO_Pin_4);
}
/* Private variables ---------------------------------------------------------*/
MSD_CARDINFO SD0_CardInfo;

static const uint16_t crc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

uint16_t sd_crc16(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0;
    while (len--)
        crc = (crc << 8) ^
            (crc16_table[((crc >> 8) ^ (*data++)) & 0xFF]);
    return crc;
}

static char crc7;
uint8_t update_crc7(uint8_t data)
{
  uint8_t j,d;
  d=data;
  for (j = 0; j < 8; j++)
  {
  crc7 <<= 1;
  if ( (d & 0x80) ^ (crc7 & 0x80) )
      crc7 ^= 0x09;
  d <<= 1;
  }
  return data;
}
/*******************************************************************************
* Function Name  : MSD0_spi_read_write
* Description    : None
* Input          : - data:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
uint8_t MSD0_spi_read_write(uint8_t data)
{
  int timeout;

  //update_crc7(data);
  //while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  /*while(!(SPI1->SR & SPI_I2S_FLAG_TXE));

  //SPI_I2S_SendData(SPI1, data);
  SPI1->DR = data;

  //while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  timeout=0;
  while(!(SPI1->SR & SPI_I2S_FLAG_RXNE));

  //return SPI_I2S_ReceiveData(SPI1);
  return SPI1->DR;*/
  timeout=0;
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET)
  {
    if (timeout++>100) break;
  }
  SPI_SendData8(SPI1, data);
  timeout=0;
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) != SET)
  {
    if (timeout++>100) break;
  }
  return SPI_ReceiveData8(SPI1);
}

/*******************************************************************************
* Function Name  : MSD0_SPI_Configuration
* Description    : SD Card SPI Configuration
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void MSD0_SPI_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB, ENABLE);

  if (model==6)
  {
    GPIO_PinAFConfig( GPIOB , GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5, GPIO_AF_0 );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
  }
  else
  {
    GPIO_PinAFConfig(GPIOA,GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7,GPIO_AF_0);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    if (model!=4 && model!=5)
    {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }

    /*
    GPIO_PinAFConfig(GPIOB,GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5,GPIO_AF_0);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    */
  }
  MSD0_card_disable();

  MSD0_SPIHighSpeed(0);
  
  SPI_Cmd(SPI1, ENABLE);
}

/*******************************************************************************
* Function Name  : MSD0_SPIHighSpeed
* Description    : SD Card Speed Set
* Input          : - b_high: 1 = 18MHz, 0 = 281.25Hz
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
char slow_card=0;
void MSD0_SPIHighSpeed(uint8_t b_high)
{
  SPI_InitTypeDef SPI_InitStructure;

  //SPI_I2S_DeInit(SPI1);  
  //SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);
  //while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET);
  SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);

  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  
  if (model==4 || model==5)
  {
      if(b_high == 0) SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
      else SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
      SPI_Init(SPI1, &SPI_InitStructure);
      return;
  }
  /* Speed select */
  if (slow_card==1)
  {
      SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  }
  else if(b_high == 0)
  {
	 SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
  }
  else
  {
	 SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  }

  SPI_Init(SPI1, &SPI_InitStructure);
}

/*******************************************************************************
* Function Name  : MSD0_Init
* Description    : SD Card initializtion
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD0_Init(void)
{
	uint8_t r1;
	uint8_t buff[6] = {0};
	uint16_t retry;

	/* Check , if no card insert */
    if( MSD0_card_insert() )
	{
#ifdef PRINT_INFO
		printf("There is no card detected! \r\n");
#endif
	  /* FATFS error flag */
      return -1;
	}

	/* Power on and delay some times */
	for(retry=0; retry<0x100; retry++)
	{
		MSD0_card_power_on();
	}

	/* Satrt send 74 clocks at least */
	for(retry=0; retry<10; retry++)
	{
		MSD0_spi_read_write(DUMMY_BYTE);
	}

	/* Start send CMD0 till return 0x01 means in IDLE state */
	for(retry=0; retry<0x7F; retry++)
	{
		r1 = MSD0_send_command(CMD0, 0, 0x95);
		if(r1 == 0x01)
		{
			retry = 0;
			break;
		}
	}
	/* Timeout return */
	if(retry == 0x7F)
	{
#ifdef PRINT_INFO
		printf("Reset card into IDLE state failed!\r\n");
#endif
		return 1;
	}
    
        for(retry=0; retry<0x7F; retry++)
	{
                r1 = MSD0_send_command(CMD1, 0, 0xFF);
		if(r1 == 0)
		{
			retry = 0;
			break;
		}
	}

	/* Get the card type, version */
	r1 = MSD0_send_command_hold(CMD8, 0x1AA, 0x87);
	/* r1=0x05 -> V1.0 */
	if(r1 == 0x05)
	{
	  SD0_CardInfo.CardType = CARDTYPE_SDV1;

	  /* End of CMD8, chip disable and dummy byte */
	  MSD0_card_disable();
	  MSD0_spi_read_write(DUMMY_BYTE);

	  /* SD1.0/MMC start initialize */
	  /* Send CMD55+ACMD41, No-response is a MMC card, otherwise is a SD1.0 card */
	  for(retry=0; retry<0xFF; retry++)
	  {
	     r1 = MSD0_send_command(CMD55, 0, 0);			/* should be return 0x01 */
		 if(r1 != 0x01)
		 {
#ifdef PRINT_INFO
			printf("Send CMD55 should return 0x01, response=0x%02x\r\n", r1);
#endif
			return r1;
		 }

		 r1 = MSD0_send_command(ACMD41, 0, 0);			/* should be return 0x00 */
		 if(r1 == 0x00)
		 {
			retry = 0;
			break;
		 }
	  }

	  /* MMC card initialize start */
	  if(retry == 0xFF)
	  {
		 for(retry=0; retry<0xFF; retry++)
	     {
			 r1 = MSD0_send_command(CMD1, 0, 0);		/* should be return 0x00 */
			 if(r1 == 0x00)
			 {
				retry = 0;
				break;
			 }
		 }

		 /* Timeout return */
		 if(retry == 0xFF)
		 {
#ifdef PRINT_INFO
			printf("Send CMD1 should return 0x00, response=0x%02x\r\n", r1);
#endif
			return 2;
		 }

		SD0_CardInfo.CardType = CARDTYPE_MMC;
#ifdef PRINT_INFO
		printf("Card Type                     : MMC\r\n");
#endif
	  }
		/* SD1.0 card detected, print information */
#ifdef PRINT_INFO
	  else
	  {
		 printf("Card Type                     : SD V1\r\n");
	  }
#endif

	  /* Set spi speed high */
	  MSD0_SPIHighSpeed(1);
#if 0
	  /* CRC disable */
	  r1 = MSD0_send_command(CMD59, 1, 0x01);
	  if(r1 != 0x00)
	  {
#ifdef PRINT_INFO
		  printf("Send CMD59 should return 0x00, response=0x%02x\r\n", r1);
#endif
		  return r1;		/* response error, return r1 */
	  }
#endif

	  /* Set the block size */
	  r1 = MSD0_send_command(CMD16, MSD_BLOCKSIZE, 0xFF);
      //extern int MSD0_send_command_crc(uint8_t cmd, uint32_t arg);
      //r1 = MSD0_send_command_crc(CMD16, MSD_BLOCKSIZE);
	  if(r1 != 0x00)
	  {
#ifdef PRINT_INFO
		  printf("Send CMD16 should return 0x00, response=0x%02x\r\n", r1);
#endif
		  return r1;		/* response error, return r1 */
	  }
   }
   /* r1=0x01 -> V2.x, read OCR register, check version */
   else if(r1 == 0x01)
   {
	 /* 4Bytes returned after CMD8 sent	*/
	 buff[0] = MSD0_spi_read_write(DUMMY_BYTE);				/* should be 0x00 */
	 buff[1] = MSD0_spi_read_write(DUMMY_BYTE);				/* should be 0x00 */
	 buff[2] = MSD0_spi_read_write(DUMMY_BYTE);				/* should be 0x01 */
	 buff[3] = MSD0_spi_read_write(DUMMY_BYTE);				/* should be 0xAA */

	 /* End of CMD8, chip disable and dummy byte */
	 MSD0_card_disable();
	 MSD0_spi_read_write(DUMMY_BYTE);

	 /* Check voltage range be 2.7-3.6V	*/
	 if(buff[2]==0x01 && buff[3]==0xAA)
	 {
		for(retry=0; retry<0xFF; retry++)
		{
			r1 = MSD0_send_command(CMD55, 0, 0xFF);			/* should be return 0x01 */
			if(r1!=0x01)
			{
#ifdef PRINT_INFO
				printf("Send CMD55 should return 0x01, response=0x%02x\r\n", r1);
#endif
                                
                                //uint16_t retry=0;
                                for (retry=0;retry<0x7F;retry++)
                                {
                                  MSD0_card_disable();
                                  MSD0_spi_read_write(DUMMY_BYTE);
                                  MSD0_card_enable();
                                  r1 = MSD0_send_command_hold(CMD1, 0, 0xFF);
                                  if (r1==0) break;
                                }
                                
                                if (retry==0x7F) return 1;
                                MSD0_card_disable();
                                MSD0_spi_read_write(DUMMY_BYTE);
                                
                                return 0;
                                
				//return r1;
			}

			r1 = MSD0_send_command(ACMD41, 0x40000000, 0xFF);	/* should be return 0x00 */
			if(r1 == 0x00)
			{
				retry = 0;
				break;
			}
		}

		/* Timeout return */
		if(retry == 0xFF)
		{
#ifdef PRINT_INFO
			printf("Send ACMD41 should return 0x00, response=0x%02x\r\n", r1);
#endif
			return 3;
		}

		/* Read OCR by CMD58 */
	    r1 = MSD0_send_command_hold(CMD58, 0, 0);
	    if(r1!=0x00)
	    {
#ifdef PRINT_INFO
			printf("Send CMD58 should return 0x00, response=0x%02x\r\n", r1);
#endif
            return r1;		/* response error, return r1 */
	    }

	    buff[0] = MSD0_spi_read_write(DUMMY_BYTE);
		buff[1] = MSD0_spi_read_write(DUMMY_BYTE);
		buff[2] = MSD0_spi_read_write(DUMMY_BYTE);
		buff[3] = MSD0_spi_read_write(DUMMY_BYTE);

		/* End of CMD58, chip disable and dummy byte */
		MSD0_card_disable();
		MSD0_spi_read_write(DUMMY_BYTE);

	    /* OCR -> CCS(bit30)  1: SDV2HC	 0: SDV2 */
	    if(buff[0] & 0x40)
	    {
           SD0_CardInfo.CardType = CARDTYPE_SDV2HC;
#ifdef PRINT_INFO
		   printf("Card Type                     : SD V2HC\r\n");
#endif
	    }
	    else
	    {
           SD0_CardInfo.CardType = CARDTYPE_SDV2;
#ifdef PRINT_INFO
		   printf("Card Type                     : SD V2\r\n");
#endif
	    }

		/* Set spi speed high */
		//MSD0_SPIHighSpeed(1);
		}
   }
   return 0;
}

/*******************************************************************************
* Function Name  : MSD0_GetCardInfo
* Description    : Get SD Card Information
* Input          : None
* Output         : None
* Return         : 0£ºNO_ERR; TRUE: Error
* Attention		 : None
*******************************************************************************/
int MSD0_GetCardInfo(PMSD_CARDINFO SD0_CardInfo)
{
  uint8_t r1;
  uint8_t CSD_Tab[16];
  uint8_t CID_Tab[16];

  /* Send CMD9, Read CSD */
  r1 = MSD0_send_command(CMD9, 0, 0xFF);
  if(r1 != 0x00)
  {
    return r1;
  }

  if(MSD0_read_buffer(CSD_Tab, 16, RELEASE))
  {
	return 1;
  }

  /* Send CMD10, Read CID */
  r1 = MSD0_send_command(CMD10, 0, 0xFF);
  if(r1 != 0x00)
  {
    return r1;
  }

  if(MSD0_read_buffer(CID_Tab, 16, RELEASE))
  {
	return 2;
  }

  /* Byte 0 */
  SD0_CardInfo->CSD.CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
  SD0_CardInfo->CSD.SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
  SD0_CardInfo->CSD.Reserved1 = CSD_Tab[0] & 0x03;
  /* Byte 1 */
  SD0_CardInfo->CSD.TAAC = CSD_Tab[1] ;
  /* Byte 2 */
  SD0_CardInfo->CSD.NSAC = CSD_Tab[2];
  /* Byte 3 */
  SD0_CardInfo->CSD.MaxBusClkFrec = CSD_Tab[3];
  /* Byte 4 */
  SD0_CardInfo->CSD.CardComdClasses = CSD_Tab[4] << 4;
  /* Byte 5 */
  SD0_CardInfo->CSD.CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
  SD0_CardInfo->CSD.RdBlockLen = CSD_Tab[5] & 0x0F;
  /* Byte 6 */
  SD0_CardInfo->CSD.PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
  SD0_CardInfo->CSD.WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
  SD0_CardInfo->CSD.RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
  SD0_CardInfo->CSD.DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
  SD0_CardInfo->CSD.Reserved2 = 0; /* Reserved */
  SD0_CardInfo->CSD.DeviceSize = (CSD_Tab[6] & 0x03) << 10;
  /* Byte 7 */
  SD0_CardInfo->CSD.DeviceSize |= (CSD_Tab[7]) << 2;
  /* Byte 8 */
  SD0_CardInfo->CSD.DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
  SD0_CardInfo->CSD.MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
  SD0_CardInfo->CSD.MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);
  /* Byte 9 */
  SD0_CardInfo->CSD.MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
  SD0_CardInfo->CSD.MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
  SD0_CardInfo->CSD.DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
  /* Byte 10 */
  SD0_CardInfo->CSD.DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
  SD0_CardInfo->CSD.EraseGrSize = (CSD_Tab[10] & 0x7C) >> 2;
  SD0_CardInfo->CSD.EraseGrMul = (CSD_Tab[10] & 0x03) << 3;
  /* Byte 11 */
  SD0_CardInfo->CSD.EraseGrMul |= (CSD_Tab[11] & 0xE0) >> 5;
  SD0_CardInfo->CSD.WrProtectGrSize = (CSD_Tab[11] & 0x1F);
  /* Byte 12 */
  SD0_CardInfo->CSD.WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
  SD0_CardInfo->CSD.ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
  SD0_CardInfo->CSD.WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
  SD0_CardInfo->CSD.MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;
  /* Byte 13 */
  SD0_CardInfo->CSD.MaxWrBlockLen |= (CSD_Tab[13] & 0xc0) >> 6;
  SD0_CardInfo->CSD.WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
  SD0_CardInfo->CSD.Reserved3 = 0;
  SD0_CardInfo->CSD.ContentProtectAppli = (CSD_Tab[13] & 0x01);
  /* Byte 14 */
  SD0_CardInfo->CSD.FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
  SD0_CardInfo->CSD.CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
  SD0_CardInfo->CSD.PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
  SD0_CardInfo->CSD.TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
  SD0_CardInfo->CSD.FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
  SD0_CardInfo->CSD.ECC = (CSD_Tab[14] & 0x03);
  /* Byte 15 */
  SD0_CardInfo->CSD.CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
  SD0_CardInfo->CSD.Reserved4 = 1;

  if(SD0_CardInfo->CardType == CARDTYPE_SDV2HC)
  {
	 /* Byte 7 */
	 SD0_CardInfo->CSD.DeviceSize = (u16)(CSD_Tab[8]) *256;
	 /* Byte 8 */
	 SD0_CardInfo->CSD.DeviceSize += CSD_Tab[9] ;
  }

  SD0_CardInfo->Capacity = SD0_CardInfo->CSD.DeviceSize * MSD_BLOCKSIZE * 1024;
  SD0_CardInfo->BlockSize = MSD_BLOCKSIZE;

  /* Byte 0 */
  SD0_CardInfo->CID.ManufacturerID = CID_Tab[0];
  /* Byte 1 */
  SD0_CardInfo->CID.OEM_AppliID = CID_Tab[1] << 8;
  /* Byte 2 */
  SD0_CardInfo->CID.OEM_AppliID |= CID_Tab[2];
  /* Byte 3 */
  SD0_CardInfo->CID.ProdName1 = CID_Tab[3] << 24;
  /* Byte 4 */
  SD0_CardInfo->CID.ProdName1 |= CID_Tab[4] << 16;
  /* Byte 5 */
  SD0_CardInfo->CID.ProdName1 |= CID_Tab[5] << 8;
  /* Byte 6 */
  SD0_CardInfo->CID.ProdName1 |= CID_Tab[6];
  /* Byte 7 */
  SD0_CardInfo->CID.ProdName2 = CID_Tab[7];
  /* Byte 8 */
  SD0_CardInfo->CID.ProdRev = CID_Tab[8];
  /* Byte 9 */
  SD0_CardInfo->CID.ProdSN = CID_Tab[9] << 24;
  /* Byte 10 */
  SD0_CardInfo->CID.ProdSN |= CID_Tab[10] << 16;
  /* Byte 11 */
  SD0_CardInfo->CID.ProdSN |= CID_Tab[11] << 8;
  /* Byte 12 */
  SD0_CardInfo->CID.ProdSN |= CID_Tab[12];
  /* Byte 13 */
  SD0_CardInfo->CID.Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
  /* Byte 14 */
  SD0_CardInfo->CID.ManufactDate = (CID_Tab[13] & 0x0F) << 8;
  /* Byte 15 */
  SD0_CardInfo->CID.ManufactDate |= CID_Tab[14];
  /* Byte 16 */
  SD0_CardInfo->CID.CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
  SD0_CardInfo->CID.Reserved2 = 1;

  return 0;
}

/*******************************************************************************
* Function Name  : MSD0_read_buffer
* Description    : None
* Input          : - *buff:
*				   - len:
*				   - release:
* Output         : None
* Return         : 0£ºNO_ERR; TRUE: Error
* Attention		 : None
*******************************************************************************/
int MSD0_read_buffer(uint8_t *buff, uint16_t len, uint8_t release)
{
  uint8_t r1;
  register uint16_t retry;

restart:
  /* Card enable, Prepare to read	*/
  MSD0_card_enable();

  /* Wait start-token 0xFE */
  for(retry=0; retry<2000; retry++)
  {
	 r1 = MSD0_spi_read_write(DUMMY_BYTE);
	 if(r1 == 0xFE)
	 {
		 retry = 0;
		 break;
	 }
  }

  /* Timeout return	*/
  if(retry == 2000)
  {
	 MSD0_card_disable();
	 return 1;
  }

  /* Start reading */
  for(retry=0; retry<len; retry++)
  {
     *(buff+retry) = MSD0_spi_read_write(DUMMY_BYTE);
  }

  uint16_t crc16 = sd_crc16(buff,len);
  //printf("CRC == %X\n",crc16);

  /* 2bytes dummy CRC */
  uint8_t crc_h,crc_l;
  crc_h=MSD0_spi_read_write(DUMMY_BYTE);
  crc_l=MSD0_spi_read_write(DUMMY_BYTE);
  //printf("CRC: %X/%X\n",crc_h,crc_l);

  /* chip disable and dummy byte */
  if(release)
  {
	 MSD0_card_disable();
	 MSD0_spi_read_write(DUMMY_BYTE);
  }

  if (((crc16>>8)!=crc_h) || ((crc16&0xFF) != crc_l))
  {
      //printf("CRC ERROR!\n");
      goto restart;
  }

  return 0;
}

/*******************************************************************************
* Function Name  : MSD0_ReadSingleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD0_ReadSingleBlock(uint32_t sector, uint8_t *buffer)
{
  uint8_t r1;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(SD0_CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }

  /* Send CMD17 : Read single block command */
  r1 = MSD0_send_command(CMD17, sector, (SD0_CardInfo.CardType == CARDTYPE_SDV2HC) ? 0xFF:0);

  if(r1 != 0x00)
  {
	 return 1;
  }

  /* Start read and return the result */
  r1 = MSD0_read_buffer(buffer, MSD_BLOCKSIZE, RELEASE);

  /* Send stop data transmit command - CMD12 */
  MSD0_send_command(CMD12, 0, (SD0_CardInfo.CardType == CARDTYPE_SDV2HC) ? 0xFF:0);

  return r1;
}

/*******************************************************************************
* Function Name  : MSD0_ReadMultiBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
*                  - NbrOfSector:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD0_ReadMultiBlock(uint32_t sector, uint8_t *buffer, uint32_t NbrOfSector)
{
  uint8_t r1;
  register uint32_t i;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(SD0_CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }

  /* Send CMD18 : Read multi block command */
  r1 = MSD0_send_command(CMD18, sector, (SD0_CardInfo.CardType == CARDTYPE_SDV2HC) ? 0xFF:0);
  if(r1 != 0x00)
  {
     return 1;
  }

  /* Start read	*/
  for(i=0; i<NbrOfSector; i++)
  {
     if(MSD0_read_buffer(buffer+i*MSD_BLOCKSIZE, MSD_BLOCKSIZE, HOLD))
     {
		 /* Send stop data transmit command - CMD12	*/
		 MSD0_send_command(CMD12, 0, (SD0_CardInfo.CardType == CARDTYPE_SDV2HC) ? 0xFF:0);
		 /* chip disable and dummy byte */
		 MSD0_card_disable();
		 return 2;
     }
  }

  /* Send stop data transmit command - CMD12 */
  MSD0_send_command(CMD12, 0, (SD0_CardInfo.CardType == CARDTYPE_SDV2HC) ? 0xFF:0);

  /* chip disable and dummy byte */
  MSD0_card_disable();
  MSD0_spi_read_write(DUMMY_BYTE);

  return 0;
}

/*******************************************************************************
* Function Name  : MSD0_WriteSingleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD0_WriteSingleBlock(uint32_t sector, uc8 *buffer)
{
  uint8_t r1;
  register uint16_t i;
  uint32_t retry;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(SD0_CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }

  /* Send CMD24 : Write single block command */
  r1 = MSD0_send_command(CMD24, sector, 0);

  if(r1 != 0x00)
  {
	 return 1;
  }

  /* Card enable, Prepare to write */
  MSD0_card_enable();
  MSD0_spi_read_write(DUMMY_BYTE);
  MSD0_spi_read_write(DUMMY_BYTE);
  MSD0_spi_read_write(DUMMY_BYTE);
  /* Start data write token: 0xFE */
  MSD0_spi_read_write(0xFE);

  /* Start single block write the data buffer */
  for(i=0; i<MSD_BLOCKSIZE; i++)
  {
    MSD0_spi_read_write(*buffer++);
  }

  /* 2Bytes dummy CRC */
  MSD0_spi_read_write(DUMMY_BYTE);
  MSD0_spi_read_write(DUMMY_BYTE);

  /* MSD card accept the data */
  r1 = MSD0_spi_read_write(DUMMY_BYTE);
  if((r1&0x1F) != 0x05)
  {
    MSD0_card_disable();
    return 2;
  }

  /* Wait all the data programm finished */
  retry = 0;
  while(MSD0_spi_read_write(DUMMY_BYTE) == 0x00)
  {
	 /* Timeout return */
	 if(retry++ == 0x40000)
	 {
	    MSD0_card_disable();
	    return 3;
	 }
  }

  /* chip disable and dummy byte */
  MSD0_card_disable();
  MSD0_spi_read_write(DUMMY_BYTE);

  return 0;
}

/*******************************************************************************
* Function Name  : MSD0_WriteMultiBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
*                  - NbrOfSector:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD0_WriteMultiBlock(uint32_t sector, uc8 *buffer, uint32_t NbrOfSector)
{
  uint8_t r1;
  register uint16_t i;
  register uint32_t n;
  uint32_t retry;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(SD0_CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	  sector = sector<<9;
  }

  /* Send command ACMD23 berfore multi write if is not a MMC card */
  if(SD0_CardInfo.CardType != CARDTYPE_MMC)
  {
	  MSD0_send_command(ACMD23, NbrOfSector, 0x00);
  }

  /* Send CMD25 : Write nulti block command	*/
  r1 = MSD0_send_command(CMD25, sector, 0);

  if(r1 != 0x00)
  {
	  return 1;
  }

  /* Card enable, Prepare to write */
  MSD0_card_enable();
  MSD0_spi_read_write(DUMMY_BYTE);
  //MSD0_spi_read_write(DUMMY_BYTE);
  //MSD0_spi_read_write(DUMMY_BYTE);

  for(n=0; n<NbrOfSector; n++)
  {
	 /* Start multi block write token: 0xFC */
	 MSD0_spi_read_write(0xFC);

	 for(i=0; i<MSD_BLOCKSIZE; i++)
	 {
		MSD0_spi_read_write(*buffer++);
	 }

	 /* 2Bytes dummy CRC */
	 MSD0_spi_read_write(DUMMY_BYTE);
	 MSD0_spi_read_write(DUMMY_BYTE);

	 /* MSD card accept the data */
	 r1 = MSD0_spi_read_write(DUMMY_BYTE);
	 if((r1&0x1F) != 0x05)
	 {
	    MSD0_card_disable();
	    return 2;
	 }

	 /* Wait all the data programm finished	*/
	 retry = 0;
	 while(MSD0_spi_read_write(DUMMY_BYTE) != 0xFF)
	 {
		/* Timeout return */
		if(retry++ == 0x40000)
		{
		   MSD0_card_disable();
		   return 3;
		}
	 }
  }

  /* Send end of transmit token: 0xFD */
  r1 = MSD0_spi_read_write(0xFD);
  if(r1 == 0x00)
  {
	 return 4;
  }

  /* Wait all the data programm finished */
  retry = 0;
  while(MSD0_spi_read_write(DUMMY_BYTE) != 0xFF)
  {
	 /* Timeout return */
	 if(retry++ == 0x40000)
	 {
	     MSD0_card_disable();
	     return 5;
	 }
  }

  /* chip disable and dummy byte */
  MSD0_card_disable();
  MSD0_spi_read_write(DUMMY_BYTE);

  return 0;
}


/*******************************************************************************
* Function Name  : MSD0_send_command
* Description    : None
* Input          : - cmd:
*				   - arg:
*                  - crc:
* Output         : None
* Return         : R1 value, response from card
* Attention		 : None
*******************************************************************************/
int MSD0_send_command(uint8_t cmd, uint32_t arg, uint8_t crc)
{
  uint8_t r1;
  uint8_t retry;

  /* Dummy byte and chip enable */
  MSD0_spi_read_write(DUMMY_BYTE);
  MSD0_card_enable();

  /* Command, argument and crc */
  //crc7=0;
  MSD0_spi_read_write(cmd | 0x40);
  MSD0_spi_read_write(arg >> 24);
  MSD0_spi_read_write(arg >> 16);
  MSD0_spi_read_write(arg >> 8);
  MSD0_spi_read_write(arg);
  MSD0_spi_read_write(crc);

  /* Wait response, quit till timeout */
  for(retry=0; retry<50; retry++)
  {
	 r1 = MSD0_spi_read_write(DUMMY_BYTE);
	 if(r1 != 0xFF)
	 {
		 break;
	 }
  }

  /* Chip disable and dummy byte */
  MSD0_card_disable();
  MSD0_spi_read_write(DUMMY_BYTE);

  return r1;
}

/*******************************************************************************
* Function Name  : MSD0_send_command_hold
* Description    : None
* Input          : - cmd:
*				   - arg:
*                  - crc:
* Output         : None
* Return         : R1 value, response from card
* Attention		 : None
*******************************************************************************/
int MSD0_send_command_hold(uint8_t cmd, uint32_t arg, uint8_t crc)
{
  uint8_t r1;
  uint8_t retry;

  /* Dummy byte and chip enable */
  MSD0_spi_read_write(DUMMY_BYTE);
  MSD0_card_enable();

  /* Command, argument and crc */
  MSD0_spi_read_write(cmd | 0x40);
  MSD0_spi_read_write(arg >> 24);
  MSD0_spi_read_write(arg >> 16);
  MSD0_spi_read_write(arg >> 8);
  MSD0_spi_read_write(arg);
  MSD0_spi_read_write(crc);

  /* Wait response, quit till timeout */
  for(retry=0; retry<200; retry++)
  {
	 r1 = MSD0_spi_read_write(DUMMY_BYTE);
	 if(r1 != 0xFF)
	 {
		 break;
	 }
  }

  return r1;
}

