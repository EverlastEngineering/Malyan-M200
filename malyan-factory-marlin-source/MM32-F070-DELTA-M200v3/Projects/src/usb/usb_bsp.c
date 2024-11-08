/**
  ******************************************************************************
  * @file    usb_bsp.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    31-January-2014
  * @brief   This file Provides Device Core configuration Functions
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
#include "usb_bsp.h"
#include "usbd_cdc_vcp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#if defined USB_CLOCK_SOURCE_CRS
 static void CRS_Config(void);
#endif /* USB_CLOCK_SOURCE_CRS */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize BSP configurations
  * @param  None
  * @retval None
  */
uint8_t xtal_failed=0;
void HSE_failed()
{
    RCC_PREDIV1Config(RCC_PREDIV1_Div1);
    RCC_PLLConfig(RCC_PLLSource_HSI_Div2,RCC_PLLMul_12);
    
    RCC_PLLCmd(ENABLE);
  
    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {}
    
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    
    /* Wait till PLL is used as system clock source */
    uint32_t timeout=0;
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL)
    {
        if (timeout++>100000) break;
    }
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLKConfig(RCC_HCLK_Div1);
    
    /* Configure USBCLK from PLL clock */
    RCC_USBCLKConfig(RCC_USBCLK_PLLCLK); 
    xtal_failed=1;
}

void USB_BSP_Init(USB_CORE_HANDLE *pdev)
{
#ifdef USB_DEVICE_LOW_PWR_MGMT_SUPPORT 
  EXTI_InitTypeDef EXTI_InitStructure;
#endif /*USB_DEVICE_LOW_PWR_MGMT_SUPPORT */  
  xtal_failed=0;
  /* Enable USB clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
  
#if defined USB_CLOCK_SOURCE_CRS
  
  /*For using CRS, you need to do the following:
  - Enable HSI48 (managed by the SystemInit() function at the application startup)
  - Select HSI48 as USB clock
  - Enable CRS clock
  - Set AUTOTRIMEN
  - Set CEN
  */
  
  /* Select HSI48 as USB clock */
  RCC_USBCLKConfig(RCC_USBCLK_HSI48);
  
  /* Configure the Clock Recovery System */
  CRS_Config();  
#else 
  /* Configure PLL to be used as USB clock:
     - Enable HSE external clock (for this example the system is clocked by HSI48
       managed by the SystemInit() function at the application startup)
     - Enable PLL
     - Select PLL as USB clock */
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);
  
  /* Wait till HSE is ready */
  /*uint32_t timeout=0;
  while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
  {
      if (timeout++>100000)
      {
          //RCC_HSEConfig(RCC_HSE_ON);
          RCC_PREDIV1Config(RCC_PREDIV1_Div1);
          RCC_PLLConfig(RCC_PLLSource_HSI_Div2,RCC_PLLMul_12);
          goto pass;
      }
  }*/
  if (RCC_WaitForHSEStartUp()==ERROR)
  {
      RCC_PREDIV1Config(RCC_PREDIV1_Div1);
      RCC_PLLConfig(RCC_PLLSource_HSI_Div2,RCC_PLLMul_12);
      IWDG_ReloadCounter();
      xtal_failed=1;
      goto pass;
  }
  
  /*Config the PREDIV for RCC_CFGR2*/
  RCC_PREDIV1Config(RCC_PREDIV1_Div1);
  /*HSE/PREDIV selected as PLL input clock*/
  RCC_PLLConfig(RCC_PLLSource_PREDIV1,RCC_PLLMul_6);
pass:
  /* Enable PLL */
  RCC_PLLCmd(ENABLE);
  
  /* Wait till PLL is ready */
  uint32_t timeout=0;
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
  {
      IWDG_ReloadCounter();
      if (timeout++>100000)
      {
          HSE_failed();
          return;
      }
  }
  
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  
  /* Wait till PLL is used as system clock source */
  timeout=0;
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL)
  {
      IWDG_ReloadCounter();
      if (timeout++>100000)
      {
          HSE_failed();
          return;
      }
  }
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  RCC_PCLKConfig(RCC_HCLK_Div1);
  
  /* Configure USBCLK from PLL clock */
  RCC_USBCLKConfig(RCC_USBCLK_PLLCLK); 
#endif /*USB_CLOCK_SOURCE_CRS */ 

#ifdef USB_DEVICE_LOW_PWR_MGMT_SUPPORT  
  
  EXTI_InitTypeDef EXTI_InitStructure;
  
  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  
  /* EXTI line 18 is connected to the USB Wakeup from suspend event   */
  EXTI_ClearITPendingBit(EXTI_Line18);
  EXTI_InitStructure.EXTI_Line = EXTI_Line18; 
  /*Must Configure the EXTI Line 18 to be sensitive to rising edge*/
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
#endif /*USB_DEVICE_LOW_PWR_MGMT_SUPPORT */
  
}

/**
  * @brief  Enable USB Global interrupt
  * @param  None
  * @retval None
  */
void USB_BSP_EnableInterrupt(USB_CORE_HANDLE *pdev)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Enable the USB interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USB_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

#if defined USB_CLOCK_SOURCE_CRS
/**
  * @brief  Configure CRS peripheral to automatically trim the HSI 
  *         oscillator according to USB SOF
  * @param  None
  * @retval None
  */
static void CRS_Config(void)
{
  /*Enable CRS Clock*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CRS, ENABLE);
  
  /* Select USB SOF as synchronization source */
  CRS_SynchronizationSourceConfig(CRS_SYNCSource_USB);
  
  /*Enables the automatic hardware adjustment of TRIM bits: AUTOTRIMEN:*/
  CRS_AutomaticCalibrationCmd(ENABLE);
  
  /*Enables the oscillator clock for frequency error counter CEN*/
  CRS_FrequencyErrorCounterCmd(ENABLE);
}

#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
