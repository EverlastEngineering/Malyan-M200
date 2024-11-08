#include "Configuration.h"
//#include "platform.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_usart.h"
#include "stdio.h"

// Must be volatile or gcc will optimize away some uses of it.
volatile unsigned long systick_count;
volatile unsigned timeout=0;

char btn_status=0;
unsigned long btn_timer;

//__no_init const char MACHINE_TYPE[4] @ 0x08001000;
extern const char MACHINE_TYPE[];

extern "C" void SysTick_Handler(void)
{
	systick_count++;

        if (systick_count&~16)
        {
            //if (model!=2) return;
            //extern volatile signed long count_position[];
            //SERIAL_ECHOLN(count_position[2]);
            if (model==6)
            {
#if ENABLED(DELTA)
                if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)==RESET) {LED_STOP_ON;}
                else {LED_STOP_OFF;}
#endif
                if (!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15))
                {//release
                   if (btn_status &1)
                   {
                       btn_status &= ~1;
                       if ((systick_count-btn_timer)>100) btn_status|=2;
                       //if ((systick_count-btn_timer)>3000) btn_status|=4;
                   }
                }
                else
                {//press
                    if (!(btn_status &1))
                    {
                        btn_status |= 1;
                        btn_timer=systick_count;
                    }
                    else
                    {
                        if ((systick_count-btn_timer)>4000) btn_status|=4;
                        if ((systick_count-btn_timer)>8000) btn_status|=8;
                    }
                }
                return;
            }
            
            else if (model==2)
            {
                if (!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_15))
                {//release
                   if (btn_status &1)
                   {
                       btn_status &= ~1;
                       if ((systick_count-btn_timer)>100) btn_status|=2;
                       //if ((systick_count-btn_timer)>3000) btn_status|=4;
                   }
                }
                else
                {//press
                    if (!(btn_status &1))
                    {
                        btn_status |= 1;
                        btn_timer=systick_count;
                    }
                    else
                    {
                        if ((systick_count-btn_timer)>4000) btn_status|=4;
                        if ((systick_count-btn_timer)>8000) btn_status|=8;
                    }
                }
            }
        }
}

unsigned long millis()
{
    return systick_count;
}

void delay(unsigned long ms)
{
    unsigned long start = millis();

    while (millis() - start < ms)
    {
        IWDG_ReloadCounter();
    }
}

void delay_ms(uint32_t ms)
{
    delay(ms);
}

#define MCK 48000000UL
unsigned long micros()
{
	// Glitch free clock
	long v0 = SysTick->VAL;
	long c0 = systick_count;
	long v1 = SysTick->VAL;
	long c1 = systick_count;

	if (v1 < v0)
		// Downcounting, no systick rollover
		return c0*8000-v1/(MCK/8000000UL);
	else
		// systick rollover, use last count value
		return c1*8000-v1/(MCK/8000000UL);
}

/* Delay for the given number of microseconds.  Assumes a 72 MHz clock.
 * Disables interrupts, which will disrupt the millis() function if used
 * too frequently. */
void delayMicroseconds(unsigned int us)
{
	uint32_t startUs = micros();
	uint32_t endUs = startUs + 8*us;

//	DisableInterrups();

	if (endUs>startUs)
		while (micros() < endUs )
			;
	else
	{
		// Handle micros() overflow
		while (micros() >= startUs)
			;
		while (micros() < endUs)
			;
	}

//	EnableInterrups();
}

extern "C" void delay_C(unsigned long ms)
{
    delay(ms);
}
extern "C" void delayMicroseconds_C(unsigned int us)
{
    delayMicroseconds(us);
}

#include "stm32f0xx_adc.h"
#define ADC1_DR_Address    ((uint32_t)0x40012440)

volatile int16_t* adc_destination; //< Address to write the sampled data to
volatile bool* adc_finished; //< Flag to set once the data is sampled

uint16_t AD_Value[5];
uint16_t AD_Value_avr[5];

void analog_init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    extern void delay_ms(uint32_t ms);

    //RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    ADC_ClockModeConfig(ADC1,RCC_ADCCLK_PCLK_Div4);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1|RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    if (model!=6)
    {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }

    if (model==4 || model==5 || model==6)
    {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);          // Enable the DMA Interrupt

    ADC_DeInit(ADC1);
    ADC_StructInit(&ADC_InitStructure);
    /* ADC1 configuration ------------------------------------------------------*/
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* ADC1 regular channel14 configuration */
    
    //ADC_ChannelConfig(ADC1, ADC_Channel_1, ADC_SampleTime_239_5Cycles);
    ADC_TempSensorCmd(ENABLE);
    if (model==6) ADC_ChannelConfig(ADC1, ADC_Channel_0|ADC_Channel_4|ADC_Channel_TempSensor, ADC_SampleTime_239_5Cycles);
    else ADC_ChannelConfig(ADC1, ADC_Channel_0|ADC_Channel_1|ADC_Channel_TempSensor, ADC_SampleTime_239_5Cycles);
    
    /* Enable ADC1 reset calibaration register */
    ADC_GetCalibrationFactor(ADC1);

/*    ADC_TempSensorVrefintCmd(ENABLE);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 3, ADC_SampleTime_239Cycles5);
    if (model==4 || model==5) ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 4, ADC_SampleTime_239Cycles5);
*/
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&AD_Value;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;

    DMA_InitStructure.DMA_BufferSize = 3;
    //if (model==4 || model==5) DMA_InitStructure.DMA_BufferSize = 4;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    DMA_Cmd(DMA1_Channel1, ENABLE);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    
    ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);
    
    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);

    /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  
  /* Wait the ADCEN falg */
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN)); 
  
  /* ADC1 regular Software Start Conv */ 
  ADC_StartOfConversion(ADC1);
  
  /* Test DMA1 TC flag */
    while((DMA_GetFlagStatus(DMA1_FLAG_TC1)) == RESET ); 
    
    /* Clear DMA TC flag */
    DMA_ClearFlag(DMA1_FLAG_TC1);
}

int16_t abs16(int16_t i)
{
    if (i>=0) return i;
    return -i;
}

uint16_t raw_temp_value[4] = { 0 };
uint16_t raw_temp_bed_value = 0;
uint16_t raw_temp_cpu_value = 0;
unsigned char temp_count;//yongzong

static char adc_loop;

extern "C" void DMA1_Channel1_IRQHandler(void)
{
    uint8_t i;

  if(DMA_GetITStatus(DMA1_IT_TC1) != RESET)
  {
    DMA_ClearITPendingBit(DMA1_IT_TC1);

    /*for (i=0;i<3;i++)
    {
        if (abs16(AD_Value[i]-AD_Value_avr[i]) < 256) AD_Value_avr[i]=(AD_Value_avr[i]*7+AD_Value[i])>>3;
        else AD_Value_avr[i]=AD_Value[i];
    }*/

    AD_Value_avr[0]+=AD_Value[0];
    AD_Value_avr[1]+=AD_Value[1];
    AD_Value_avr[2]+=AD_Value[2];

    if (model==4 || model==5)
    {
        AD_Value_avr[3]+=AD_Value[3];

        if (adc_loop>=3)
        {
            adc_loop=0;
            raw_temp_value[1] = AD_Value_avr[0];
            raw_temp_value[0] = AD_Value_avr[1];
            raw_temp_cpu_value = AD_Value_avr[2]/4;
            raw_temp_bed_value = AD_Value_avr[3];
#if FAKE_TEMPERATURE
            raw_temp_value[0]=2000;
            raw_temp_value[1]=2000;
            raw_temp_bed_value=12000;
#endif
            AD_Value_avr[0]=0;
            AD_Value_avr[1]=0;
            AD_Value_avr[2]=0;
            AD_Value_avr[3]=0;
            temp_count++;
        } else adc_loop++;
        return;
    }
    else
    {
        if (adc_loop>=3)
        {
          adc_loop=0;
          raw_temp_value[0] = AD_Value_avr[0];
          raw_temp_bed_value = AD_Value_avr[1];
          raw_temp_cpu_value = AD_Value_avr[2]/4;
    #if FAKE_TEMPERATURE
          raw_temp_value[0]=2000;
          raw_temp_bed_value=12000;
    #endif
          AD_Value_avr[0]=0;
          AD_Value_avr[1]=0;
          AD_Value_avr[2]=0;
          temp_count++;
        } else adc_loop++;
        //printf("ADC1=%d, ADC2=%d, ADC3=%d\r\n",AD_Value_avr[0],AD_Value_avr[1],AD_Value_avr[2]);
        //*adc_finished = true;
      }
  }
}

//Page 127 0x0801 FC00 - 0x0801 FFFF 1 Kbyte
#define EEPROM_ADDRESS  0x0801FC00
__no_init uint16_t eeprom[512]@EEPROM_ADDRESS;

void eeprom_write_byte(uint16_t pos, uint8_t value)
{
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    FLASH_ProgramHalfWord(EEPROM_ADDRESS+(pos*2),value|0xFF00);
}

void eeprom_erase()
{
    RCC_HSICmd(ENABLE);

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    FLASH_ErasePage(EEPROM_ADDRESS);
}

uint8_t eeprom_read_byte(uint16_t pos)
{
    return eeprom[pos];
}

void eeprom_lock()
{
    FLASH_Lock();
}

//TIM1 = 72Mhz -> 2Mhz
void stepper_timer_init()
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_UP_TRG_COM_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = 1000;
    TIM_TimeBaseStructure.TIM_Prescaler = 24;   //36    //2Mhz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

/*void stepper_timer_set(uint16_t t)	//inline
{
    TIM_SetAutoreload(TIM1,t);
}*/

void stepper_timer_enable()	//inline
{
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    //TIM_Cmd(TIM4, ENABLE);
}

void stepper_timer_disable()	//inline
{
    TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);
    //TIM_Cmd(TIM4, DISABLE);
}

extern void stepper_timer();
extern "C" void TIM1_BRK_UP_TRG_COM_IRQHandler()
{
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    stepper_timer();
}

//TIM3 = 36Mhz->10Khz
void adv_timer_init()
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the TIM2 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);

    /* TIM2 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 100;//64; //10Khz
    TIM_TimeBaseStructure.TIM_Prescaler = 48;//72;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /* TIM IT enable */
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    /* TIM2 enable counter */
    TIM_Cmd(TIM3, ENABLE);
}

extern void adv_timer_exec();
extern "C" void TIM3_IRQHandler()
{
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	adv_timer_exec();
}

//TIM2 = 36Mhz
void extruder_timer_init()
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the TIM2 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);

    /* TIM2 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 200;
    TIM_TimeBaseStructure.TIM_Prescaler = 24;   //2Mhz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /* TIM IT enable */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    /* TIM2 enable counter */
    TIM_Cmd(TIM2, ENABLE);

    //////////////////////////
    if (model!=2) return;
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    /*GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);*/
    //GPIO_PinRemapConfig(GPIO_Remap_USART2,ENABLE);
/*
    GPIO_PinRemapConfig( GPIO_FullRemap_TIM2, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

    TIM_OC3Init(TIM2, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_SetCompare2(TIM2,200);
    TIM_SetCompare4(TIM2,0);
    TIM_CtrlPWMOutputs(TIM2,ENABLE);*/
}

extern void os_timer_exec();
extern void extruder_timer_exec();
extern "C" void TIM2_IRQHandler()
{
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    //os_timer_exec();
    extruder_timer_exec();
}

extern "C" void SetSysTick()
{
    RCC_APB2PeriphClockCmd(0xFFFFFFFF, ENABLE);
    //GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);

    SysTick_Config(48000);
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}
/*
extern "C" void SetSysClockTo72()
{
    RCC_HSEConfig(RCC_HSE_ON);
    RCC_WaitForHSEStartUp();
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    FLASH_SetLatency(FLASH_Latency_2);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
    RCC_PLLCmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while(RCC_GetSYSCLKSource()!=0x08);

    RCC_APB2PeriphClockCmd(0xFFFFFFFF, ENABLE);

    SysTick_Config(72000);
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}
*/
/*
extern "C" void SetSysClockTo64()
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_OFF);
    RCC_HSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    FLASH_SetLatency(FLASH_Latency_2);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);
    RCC_PLLCmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while(RCC_GetSYSCLKSource()!=0x08);

    RCC_APB2PeriphClockCmd(0xFFFFFFFF, ENABLE);
    //GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);

    SysTick_Config(64000);
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}
*/
//uint32_t tmp;
/* Usart rx dma receive buffer, this address must be 256 aligned  */

void gpio_init()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);*/

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //PB8 = FAN-PIN
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    if (Z_ULN2003)
    {
        //Z-AXIS
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        GPIO_ResetBits(GPIOB, GPIO_Pin_0);
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        GPIO_ResetBits(GPIOC, GPIO_Pin_14);
    }
}

/* Usart rx dma receive buffer, this address must be 256 aligned  */
#define USART_RX_BUF_SIZE 256
//#pragma data_alignment=256
uint8_t Uart_Rx[USART_RX_BUF_SIZE];
uint16_t buff_ptr=USART_RX_BUF_SIZE;

void serial_init(int baud)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    gpio_init();

    //if (USARTx == USART1)
    {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->RDR);//0x40013824
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Uart_Rx;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = USART_RX_BUF_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5,&DMA_InitStructure);

    SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_USART1Rx,ENABLE);
    DMA_Cmd(DMA1_Channel5,ENABLE);

    buff_ptr=USART_RX_BUF_SIZE;

        NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
        USART_ClearITPendingBit(USART1, USART_IT_TC | USART_IT_RXNE);
        USART_ClearITPendingBit(USART1, USART_IT_PE | USART_IT_FE | USART_IT_NE);

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1); 
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);
        
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//GPIO_PuPd_UP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        USART_InitStructure.USART_BaudRate = baud;//512000;//115200;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
    USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    }
    //else
/*
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
        USART_ClearITPendingBit(USART2, USART_IT_TC | USART_IT_RXNE);
        USART_ClearITPendingBit(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE);

        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        USART_InitStructure.USART_BaudRate = 115200;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

        USART_Init(USART2, &USART_InitStructure);
        USART_Cmd(USART2, ENABLE);
    }
*/
}

extern "C" void store_char(unsigned char c);

extern "C" void DMA1_Channel4_5_6_7_IRQHandler(void)
{
  if(DMA_GetITStatus(DMA1_IT_TC5) != RESET)
  {
    DMA_ClearITPendingBit(DMA1_IT_TC5);
  }

  if (DMA_GetITStatus(DMA1_IT_TE5) != RESET)
  {
    DMA_ClearITPendingBit(DMA1_IT_TE5);
  }
}

#define WIFI_USART_RX_BUF_SIZE 256
char Uart_RX_WIFI[WIFI_USART_RX_BUF_SIZE];
volatile int16_t wifi_w_buff_ptr=0;
extern unsigned char download_state;

void WIFI_enable(unsigned char e)
{
    if (e!=0)
    {
        USART_ITConfig(WIFI_UART, USART_IT_RXNE, ENABLE);
    }
    else
    {
        USART_ITConfig(WIFI_UART, USART_IT_RXNE, DISABLE);
    }
}

extern "C" void WIFI_UART_IRQHandler()
{
  uint8_t c;
  extern volatile uint8_t wifi_download;

  /*if(USART_GetITStatus(WIFI_UART, USART_IT_RXNE) != RESET)
  {
    USART_ClearITPendingBit(WIFI_UART, USART_IT_RXNE);

    c=WIFI_UART->DR;
    if (wifi_download)
    {
        Uart_RX_WIFI[wifi_w_buff_ptr++]=c;
        if (wifi_w_buff_ptr>=WIFI_USART_RX_BUF_SIZE) wifi_w_buff_ptr=0;
        return;
    }

    /if (c<0x80)
    {
      store_char(c);
      //extern bool sdprint_cancel;
      //if (c==0x1B) sdprint_cancel=true;
    }
    else
    {
      Uart_RX_WIFI[wifi_w_buff_ptr++]=(c-0x80);
      if (wifi_w_buff_ptr>=WIFI_USART_RX_BUF_SIZE) wifi_w_buff_ptr=0;
    }
  }
  */

  ////////////////////////////////////////////////////
  if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
  {
      uint8_t c;
      uint16_t temp;
    // Read one byte from the receive data register
    //serial_rx_exec(USART_ReceiveData(USART1));

    //if (temp==USART_RX_BUF_SIZE) temp=0;

    //printf("+%c: DMA=%d/CNT=%d\n",Uart_Rx[USART_RX_BUF_SIZE-buff_ptr],temp,buff_ptr);
    //serial_rx_exec(Uart_Rx[USART_RX_BUF_SIZE-buff_ptr]);
    c=Uart_Rx[USART_RX_BUF_SIZE-buff_ptr];
    if (wifi_download)
    {
        Uart_RX_WIFI[wifi_w_buff_ptr++]=c;
        if (wifi_w_buff_ptr>=WIFI_USART_RX_BUF_SIZE) wifi_w_buff_ptr=0;
    }
    else
    {
        if (c<0x80)
        {
          store_char(c);
          /*extern bool sdprint_cancel;
          if (c==0x1B) sdprint_cancel=true;*/
        }
        else
        {
          Uart_RX_WIFI[wifi_w_buff_ptr++]=(c-0x80);
          if (wifi_w_buff_ptr>=WIFI_USART_RX_BUF_SIZE) wifi_w_buff_ptr=0;
        }
    }

    if (buff_ptr<=1) buff_ptr=USART_RX_BUF_SIZE;
    else buff_ptr--;

    temp=DMA_GetCurrDataCounter(DMA1_Channel5);
    if (buff_ptr==temp)
    {
        //temp = USART1->SR;
        //temp = USART1->RDR;
        USART_ClearITPendingBit(USART1, USART_IT_IDLE);
    }

  }

  else if(USART_GetFlagStatus(WIFI_UART,USART_FLAG_ORE)==SET)
  {
      USART_ClearFlag(WIFI_UART,USART_FLAG_ORE);
      USART_ReceiveData(WIFI_UART);
  }

  /*if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
  {
    // Write one byte to the transmit data register
    serial_tx_exec();
  }*/
  ////////////////////////////////////////////////////

}

//#include "platform_config.h"
//#define USB_DISCONNECT                      GPIOB
//#define USB_DISCONNECT_PIN                  GPIO_Pin_9
extern "C" void PowerOn();
extern "C" void PowerOff();
void USB_enable(uint8_t en)
{
    if (en==0)
    {
        PowerOff();
        //GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
        bDeviceState = 0;
    }
    else PowerOn();//GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
}

void uart_flush()
{
    char c;
    extern volatile int16_t wifi_r_buff_ptr;

    USART_ClearITPendingBit(WIFI_UART, USART_IT_RXNE);
    c=WIFI_UART->RDR;

    MYSERIAL.flush();
    wifi_w_buff_ptr=0;
    wifi_r_buff_ptr=0;
}
