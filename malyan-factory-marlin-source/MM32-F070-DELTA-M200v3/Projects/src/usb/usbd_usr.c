/**
  ******************************************************************************
  * @file    usbd_usr.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-January-2014
  * @brief   This file contains user callback structure for USB events Management
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "port.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern USBD_Class_cb_TypeDef  USBD_CDC_MSC_cb;
USB_CORE_HANDLE  USB_Device_dev ;
USBD_Usr_cb_TypeDef USR_cb =
{
  USBD_USR_Init,
  USBD_USR_DeviceReset,
  USBD_USR_DeviceConfigured,
  USBD_USR_DeviceSuspended,
  USBD_USR_DeviceResumed,  
};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void PowerOn(){DCD_DevConnect(&USB_Device_dev);}
void PowerOff(){DCD_DevDisconnect(&USB_Device_dev);}
extern uint8_t bDeviceState;
#define CONFIGURED      5
extern uint8_t com_opened;

void USB_main()
{
  com_opened=0;
  if (composite)
  {
      USBD_Init(&USB_Device_dev,
            (USBD_DEVICE*)&USR_desc, 
            &USBD_CDC_MSC_cb,
            &USR_cb);
  }
  else USBD_Init(&USB_Device_dev,
            (USBD_DEVICE*)&USR_desc, 
            &USBD_CDC_cb, 
            &USR_cb);
return;
}
/**
  * @brief  Device lib initialization
  * @param  None
  * @retval None
  */
void USBD_USR_Init(void)
{   
  bDeviceState=0;
  com_opened=0;
  LED_LINK_OFF;
}

/**
  * @brief  Reset Event
  * @param  speed : device speed
  * @retval None
  */
void USBD_USR_DeviceReset(uint8_t speed )
{
  bDeviceState=0;
  com_opened=0;
  LED_LINK_OFF;
}


/**
  * @brief  Configuration Event
  * @param  None
  * @retval Status
*/
void USBD_USR_DeviceConfigured (void)
{
  extern uint32_t APP_Rx_ptr_in,APP_Rx_ptr_out;

  bDeviceState=CONFIGURED;
  com_opened=0;
  LED_LINK_ON;
  APP_Rx_ptr_in = 0;
  APP_Rx_ptr_out = 0;
}

/**
  * @brief  Device suspend Event
  * @param  None
  * @retval None
  */
void USBD_USR_DeviceSuspended(void)
{
  bDeviceState=0;
  com_opened=0;
  LED_LINK_OFF;
}


/**
  * @brief  Device resume Event
  * @param  None
  * @retval None
  */
void USBD_USR_DeviceResumed(void)
{
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
