/*      
  *    ===================================================================      
  *                          composite CDC-MSC
  *    =================================================================== */     

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_msc_wrapper.h"
#include "usbd_cdc_core.h"
#include "usbd_msc_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t  USBD_CDC_MSC_Init         (void *pdev , uint8_t cfgidx);
static uint8_t  USBD_CDC_MSC_DeInit       (void *pdev , uint8_t cfgidx);

/* Control Endpoints*/
static uint8_t  USBD_CDC_MSC_Setup        (void *pdev , USB_SETUP_REQ  *req);  
static uint8_t  USBD_CDC_MSC_EP0_RxReady  (void *pdev);  

/* Class Specific Endpoints*/
static uint8_t  USBD_CDC_MSC_DataIn       (void *pdev , uint8_t epnum);   
static uint8_t  USBD_CDC_MSC_DataOut      (void *pdev , uint8_t epnum); 

static uint8_t  USBD_CDC_MSC_SOF          (void *pdev); 
static uint8_t*  USBD_CDC_MSC_GetConfigDescriptor( uint8_t speed , uint16_t *length); 

USBD_Class_cb_TypeDef  USBD_CDC_MSC_cb = 
{
  USBD_CDC_MSC_Init,
  USBD_CDC_MSC_DeInit,
  USBD_CDC_MSC_Setup,
  NULL, 
  USBD_CDC_MSC_EP0_RxReady, 
  USBD_CDC_MSC_DataIn, 
  USBD_CDC_MSC_DataOut,
  USBD_CDC_MSC_SOF,   
  USBD_CDC_MSC_GetConfigDescriptor, 
};

/* USB MSC_HID device Configuration Descriptor */
const uint8_t USBD_CDC_MSC_CfgDesc[USB_CDC_MSC_CONFIG_DESC_SIZ] =
{  
  0x09, /* bLength: Configuration Descriptor size */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
  USB_CDC_MSC_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  2,         /*bNumInterfaces: 2 interfaces (1 for MSC, 1 for HID)*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0,         /*iConfiguration: Index of string descriptor describing the configuration*/
  0xC0,         /*bmAttributes: bus powered and Support Remote Wake-up */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
  
  /******************** Descriptor of Mass Storage interface ********************/
  0x09,   // bLength: Interface Descriptor size
  0x04,   // bDescriptorType:
  0x00,   // bInterfaceNumber: Number of Interface
  0x00,   // bAlternateSetting: Alternate setting
  0x02,   // bNumEndpoints*/
  0x08,   // bInterfaceClass: MASS STORAGE Class
  0x06,   // bInterfaceSubClass : SCSI transparent*/
  0x50,   // nInterfaceProtocol
  4,      // iInterface:
  
  /********************  Mass Storage Endpoints ********************/
  0x07,   /*Endpoint descriptor length = 7*/
  0x05,   /*Endpoint descriptor type */
  MSC_IN_EP,   /*Endpoint address (IN, address 2) */
  0x02,   /*Bulk endpoint type */
  LOBYTE(MSC_MAX_PACKET),
  HIBYTE(MSC_MAX_PACKET),
  0x00,   /*Polling interval in milliseconds */
  
  0x07,   /*Endpoint descriptor length = 7 */
  0x05,   /*Endpoint descriptor type */
  MSC_OUT_EP,   /*Endpoint address (OUT, address 2) */
  0x02,   /*Bulk endpoint type */
  LOBYTE(MSC_MAX_PACKET),
  HIBYTE(MSC_MAX_PACKET),
  0x00,     /*Polling interval in milliseconds*/
  
  /*************************** CDC interfaces *******************************/
   /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  1,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  3,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x01,   /* bInterfaceProtocol: Common AT commands */
  0,   /* iInterface: */
  
  /*Header Functional Descriptor*/
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,
  
  /*Call Management Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x00,   /* bmCapabilities: D0+D1 */
  1,   /* bDataInterface: 2 */
  
  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */
  
  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0,   /* bMasterInterface: Communication class interface */
  1,   /* bSlaveInterface0: Data Class Interface */
  
  /*Endpoint 2 Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                     /* bEndpointAddress */
  0x03,                           /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SZE),
  0xFF,                           /* bInterval: */
  
  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  0x40,                              /* wMaxPacketSize: */
  0x00,
  0,                              /* bInterval: ignore for Bulk transfer */
  
  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,     /* bDescriptorType: Endpoint */
  CDC_IN_EP,                        /* bEndpointAddress */
  0x02,                             /* bmAttributes: Bulk */
  0x40,                             /* wMaxPacketSize: */
  0x00,
  0,                              /* bInterval */ 
}; /* USBD_HID_MSC_CfgDesc */

/* Private function prototypes -----------------------------------------------*/
/*********************************************
   MSC Device library callbacks
*********************************************/
extern uint8_t  USBD_MSC_Init (void  *pdev, uint8_t cfgidx);
extern uint8_t  USBD_MSC_DeInit (void  *pdev, uint8_t cfgidx);
extern uint8_t  USBD_MSC_Setup (void  *pdev, USB_SETUP_REQ *req);
extern uint8_t  USBD_MSC_DataIn (void  *pdev, uint8_t epnum);
extern uint8_t  USBD_MSC_DataOut (void  *pdev,  uint8_t epnum);
extern uint8_t  *USBD_MSC_GetCfgDesc (uint8_t speed, uint16_t *length);
extern uint8_t  USBD_MSC_CfgDesc[USB_MSC_CONFIG_DESC_SIZ];

/*********************************************
   CDC Device library callbacks
*********************************************/
extern uint8_t  usbd_cdc_Init        (void  *pdev, uint8_t cfgidx);
extern uint8_t  usbd_cdc_DeInit      (void  *pdev, uint8_t cfgidx);
extern uint8_t  usbd_cdc_Setup       (void  *pdev, USB_SETUP_REQ *req);
extern uint8_t  usbd_cdc_EP0_RxReady  (void *pdev);
extern uint8_t  usbd_cdc_DataIn      (void *pdev, uint8_t epnum);
extern uint8_t  usbd_cdc_DataOut     (void *pdev, uint8_t epnum);
extern uint8_t  usbd_cdc_SOF         (void *pdev);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  USBD_CDC_MSC_Init
  *         Initialize the HID & MSC interfaces
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_MSC_Init (void  *pdev, 
                               uint8_t cfgidx)
{
  /* CDC initialization */
  usbd_cdc_Init (pdev,cfgidx);
  
  /* MSC initialization */
  USBD_MSC_Init (pdev,cfgidx);
  
  return USBD_OK;

}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID/CDC interfaces
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_MSC_DeInit (void  *pdev, 
                                 uint8_t cfgidx)
{
  /* CDC De-initialization */
  usbd_cdc_DeInit (pdev,cfgidx);
  
  /* CDC De-initialization */
  USBD_MSC_DeInit (pdev,cfgidx);
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_CDC_MSC_Setup (void  *pdev, 
                                    USB_SETUP_REQ *req)
{
  switch (req->bmRequest & USB_REQ_RECIPIENT_MASK)
  {
  case USB_REQ_RECIPIENT_INTERFACE:
    if (req->wIndex == USBD_MSC_INTERFACE)
    {
      return (USBD_MSC_Setup(pdev, req));
    }
    else
    {
      return (usbd_cdc_Setup(pdev, req));
    }
  case USB_REQ_RECIPIENT_ENDPOINT:
    switch(req->wIndex)
     {

         case CDC_IN_EP:
         case CDC_OUT_EP:
         case CDC_CMD_EP:
             return (usbd_cdc_Setup(pdev, req));

         /*case MSC_EPIN_ADDR:
         case MSC_EPOUT_ADDR:
             return (USBD_MSC_Setup(pdev, req));*/

         default:
            return (USBD_MSC_Setup(pdev, req));
            break;
     }
/*
    if ((req->wIndex == HID_IN_EP) || (req->wIndex == HID_OUT_EP))
    {
      return (USBD_HID_Setup (pdev, req));   
    }
    else
    {
      return (USBD_MSC_Setup(pdev, req));
    }*/
  }   
  return USBD_OK;
}

/**
  * @brief  USBD_CDC_MSC_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t  *USBD_CDC_MSC_GetConfigDescriptor (uint8_t speed, uint16_t *length)
{
  *length = sizeof (USBD_CDC_MSC_CfgDesc);
  return (uint8_t*)USBD_CDC_MSC_CfgDesc;
}

/**
  * @brief  USBD_CDC_MSC_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_CDC_MSC_DataIn (void  *pdev, 
                              uint8_t epnum)
{
  /*DataIN can be for MSC or HID */
  
  if (epnum == (MSC_IN_EP&~0x80) )
  {
    return (USBD_MSC_DataIn(pdev, epnum));
  }
  else
  {
   return (usbd_cdc_DataIn(pdev, epnum));
  }
}

/**
  * @brief  USBD_CDC_MSC_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_CDC_MSC_DataOut(void *pdev , uint8_t epnum)
{
  /*DataOut can be for MSC or HID */
  
  if (epnum == (MSC_OUT_EP&~0x80) )
  {
    return (USBD_MSC_DataOut(pdev, epnum));
  }
  else
  {
    return (usbd_cdc_DataOut(pdev, epnum));
  } 
}

uint8_t  USBD_CDC_MSC_SOF (void *pdev)
{
  /*SOF processing needed for CDC */
  return (usbd_cdc_SOF(pdev));
}

/**
  * @brief  USBD_CDC_MSC_EP0_RxReady
  *         handle RxReady processing
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
uint8_t  USBD_CDC_MSC_EP0_RxReady  (void *pdev)
{
  (usbd_cdc_EP0_RxReady(pdev));
  return USBD_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
