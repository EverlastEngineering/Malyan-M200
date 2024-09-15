/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CDC_MSC_WRAPPER_H_
#define __USB_CDC_MSC_WRAPPER_H_

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/* Exported defines ----------------------------------------------------------*/
#define CDC_INTERFACE 0x0
#define MSC_INTERFACE 1

/* Exported types ------------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
//#define USB_HID_MSC_CONFIG_DESC_SIZ  (CUSTOMHID_SIZ_CONFIG_DESC -9 + USB_MSC_CONFIG_DESC_SIZ)
#define USB_CDC_MSC_CONFIG_DESC_SIZ  (81)

/* Exported variables --------------------------------------------------------*/
extern USBD_Class_cb_TypeDef  USBD_CDC_MSC_cb;

/* Exported functions ------------------------------------------------------- */ 


#endif  /* __USB_CDC_MSC_WRAPPER_H_ */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
