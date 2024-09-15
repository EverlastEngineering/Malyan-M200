/**
  ******************************************************************************
  * @file    usbd_conf.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-January-2014
  * @brief   USB Device configuration file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define USBD_CFG_MAX_NUM                1
#define USBD_ITF_MAX_NUM                1
#define USB_MAX_STR_DESC_SIZ       64 
#define USBD_SELF_POWERED               

#define USBD_MSC_INTERFACE           2

#define MSC_IN_EP                    0x82
#define MSC_OUT_EP                   0x02
#define MSC_MAX_PACKET                64

#define MSC_MEDIA_PACKET              512

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#define APP_FOPS                        VCP_fops

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define CDC_DATA_MAX_PACKET_SIZE       64   /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SZE             8    /* Control Endpoint Packet size */

#define CDC_IN_FRAME_INTERVAL          5    /* Number of frames between IN transfers */
#define APP_RX_DATA_SIZE               256 /* Total size of IN buffer: 
                                                APP_RX_DATA_SIZE*8/MAX_BAUDARATE*1000 should be > CDC_IN_FRAME_INTERVAL */

#define CDC_IN_EP                       0x81  /* EP1 for data IN */
#define CDC_OUT_EP                      0x01  /* EP1 for data OUT */
#define CDC_CMD_EP                      0x83  /* EP2 for CDC commands */

/* EP2 Tx buffer base address */
#define BULK_IN_TX_ADDRESS  (0xC0) 

/* EP2 Rx buffer base address */
#define BULK_OUT_RX_ADDRESS (0x110)

/* EP3 Tx buffer base address */
#define INT_IN_TX_ADDRESS   (0x100)

#endif /* __USBD_CONF__H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

