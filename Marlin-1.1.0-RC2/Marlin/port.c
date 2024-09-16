#include "port.h"
#include <string.h>
#include "stm32f0xx_gpio.h"

char MSG_M115_REPORT[48];
float homing_feedrate[] = {50*60, 50*60, 4*60, 0};
float DEFAULT_AXIS_STEPS_PER_UNIT[] = {93,93,1097.5,97};
float DEFAULT_MAX_FEEDRATE[] = {150, 150, 1.5, 50};
long DEFAULT_MAX_ACCELERATION[] = {800,800,20,10000};
bool Z_ULN2003=false;
uint8_t model;
uint8_t EXTRUDERS;

/*int16_t X_MAX_POS=150;
int16_t Y_MAX_POS=150;
int16_t Z_MAX_POS=150;*/
#define X_MAX_POS base_max_pos[0]
#define Y_MAX_POS base_max_pos[1]
#define Z_MAX_POS base_max_pos[2]
int16_t base_max_pos[3]={120,130,120};
float max_pos[3] = { 150, 150, 150 };

bool X_MIN_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
bool Y_MIN_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
bool Z_MIN_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
bool X_MAX_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
bool Y_MAX_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
bool Z_MAX_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.
bool Z_MIN_PROBE_ENDSTOP_INVERTING = true; // set to true to invert the logic of the endstop.

#if 1
GPIO_TypeDef* DIO100_PORT;
uint16_t DIO100_PIN;
GPIO_TypeDef* DIO101_PORT;
uint16_t DIO101_PIN;
GPIO_TypeDef* DIO102_PORT;
uint16_t DIO102_PIN;
GPIO_TypeDef* DIO103_PORT;
uint16_t DIO103_PIN;
GPIO_TypeDef* DIO104_PORT;
uint16_t DIO104_PIN;
GPIO_TypeDef* DIO105_PORT;
uint16_t DIO105_PIN;
GPIO_TypeDef* DIO106_PORT;
uint16_t DIO106_PIN;
GPIO_TypeDef* DIO107_PORT;
uint16_t DIO107_PIN;
GPIO_TypeDef* DIO108_PORT;
uint16_t DIO108_PIN;
GPIO_TypeDef* DIO109_PORT;
uint16_t DIO109_PIN;
GPIO_TypeDef* DIO110_PORT;
uint16_t DIO110_PIN;
GPIO_TypeDef* DIO111_PORT;
uint16_t DIO111_PIN;
GPIO_TypeDef* DIO112_PORT;
uint16_t DIO112_PIN;
GPIO_TypeDef* DIO113_PORT;
uint16_t DIO113_PIN;
GPIO_TypeDef* DIO114_PORT;
uint16_t DIO114_PIN;
GPIO_TypeDef* DIO115_PORT;
uint16_t DIO115_PIN;
GPIO_TypeDef* DIO116_PORT;
uint16_t DIO116_PIN;
GPIO_TypeDef* DIO117_PORT;
uint16_t DIO117_PIN;
GPIO_TypeDef* DIO118_PORT;
uint16_t DIO118_PIN;
GPIO_TypeDef* DIO119_PORT;
uint16_t DIO119_PIN;
GPIO_TypeDef* DIO120_PORT;
uint16_t DIO120_PIN;
GPIO_TypeDef* DIO121_PORT;
uint16_t DIO121_PIN;
GPIO_TypeDef* DIO122_PORT;
uint16_t DIO122_PIN;
GPIO_TypeDef* DIO123_PORT;
uint16_t DIO123_PIN;
#endif

#define MAX_MACHINE_TYPE_NUM    8
#define FORCE_MACHINE_TYPE      1

const char machine_list[][MAX_MACHINE_TYPE_NUM+1]=
{//0        1       2       3       4       5       6       7      8
    "M000", "M200", "M100", "M160", "M450", "M180", "M300", "MA10","M203"
};

const char VERSION[]="4.7";
__no_init char HWVER[8] @ 0x20000410;
__no_init const char MACHINE_TYPE[4] @ 0x08001000;
__no_init const char HW_VER[4] @ 0x08001008;

void get_firmware_info()
{
    //char n[1];
    if (FLASH_OB_GetWRP()==0xFFFFFFFF) strcpy(MSG_M115_REPORT, "NAME. Malyan\tVER: ");
    else strcpy(MSG_M115_REPORT, "NAME: Malyan\tVER: ");
    strcat(MSG_M115_REPORT, VERSION);
    strcat(MSG_M115_REPORT, "\tMODEL: ");
    strcat(MSG_M115_REPORT, machine_list[model]);
    //n[0]=model+'0';
    //strncat(MSG_M115_REPORT, n, 1);
    strcat(MSG_M115_REPORT, "\tHW: ");
    strncat(MSG_M115_REPORT, HWVER, 4);
    strcat(MSG_M115_REPORT, "\n");

    /*strncat(MSG_M115_REPORT, HWVER, 8);
    strcat(MSG_M115_REPORT, "\r\n");*/
}

void reset_invert_bit()
{
    //invert_byte = 0xB;
    invert_byte = 0x9;

    /*if (HWVER[0]=='H' && HWVER[1]=='A')
    {
        if (HWVER[2]=='0' && HWVER[3]=='2') invert_byte = 0xA;
    }
    
    if (HWVER[0]=='H' && HWVER[1]=='H')
    {
        if (model<=1 && HWVER[2]=='0' && HWVER[3]=='1') invert_byte = 0xA;
        if (HWVER[2]=='0' && HWVER[3]=='2') invert_byte = 9;
    }

    if (HWVER[0]=='H' && HWVER[1]=='B')
    {
        if (HWVER[2]=='0' && HWVER[3]=='2') invert_byte = 0xA;
    }*/
  
    if (model==2) invert_byte = 0x4A;
    else if (model==5) invert_byte = 0xB;
    else if (model==6) invert_byte = 8;
    
    //3D PRIMA BLUE VERSION
    //invert_byte^=3;
}

void port_init()
{
    extern bool Z_ULN2003;
    
    model = FORCE_MACHINE_TYPE;
    if (model>MAX_MACHINE_TYPE_NUM) model=0;

    /*if (HWVER[4]!='M')
    {
        HWVER[0]=HW_VER[0];
        HWVER[1]=HW_VER[1];
        HWVER[2]=HW_VER[2];
        HWVER[3]=HW_VER[3];

        HWVER[4]=MACHINE_TYPE[0];
        HWVER[5]=MACHINE_TYPE[1];
        HWVER[6]=MACHINE_TYPE[2];
        HWVER[7]=MACHINE_TYPE[3];
    }*/
    //invert_byte = 0x9;
    if (model==0)
    {
        model = 1;//default
        if (HWVER[4]!='M')
        {
            if (MACHINE_TYPE[0]=='M' && MACHINE_TYPE[1]=='1' && MACHINE_TYPE[2]=='0' && MACHINE_TYPE[3]=='0') model=2;
            if (MACHINE_TYPE[0]=='M' && MACHINE_TYPE[1]=='2' && MACHINE_TYPE[2]=='0' && MACHINE_TYPE[3]=='0') model=1;
            if (MACHINE_TYPE[0]=='M' && MACHINE_TYPE[1]=='4' && MACHINE_TYPE[2]=='5' && MACHINE_TYPE[3]=='0') model=4;
            if (MACHINE_TYPE[0]=='M' && MACHINE_TYPE[1]=='1' && MACHINE_TYPE[2]=='6' && MACHINE_TYPE[3]=='0') model=3;
            if (MACHINE_TYPE[0]=='M' && MACHINE_TYPE[1]=='1' && MACHINE_TYPE[2]=='8' && MACHINE_TYPE[3]=='0') model=5;
            if (MACHINE_TYPE[0]=='M' && MACHINE_TYPE[1]=='3' && MACHINE_TYPE[2]=='0' && MACHINE_TYPE[3]=='0') model=6;
            
            if (MACHINE_TYPE[0]!='M')
            {
                //LOCK SD CARD
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
            }
        }
        if (HWVER[4]=='M' && HWVER[5]=='1' && HWVER[6]=='0' && HWVER[7]=='0') model=2;
        if (HWVER[4]=='M' && HWVER[5]=='2' && HWVER[6]=='0' && HWVER[7]=='0') model=1;
        if (HWVER[4]=='M' && HWVER[5]=='4' && HWVER[6]=='5' && HWVER[7]=='0') model=4;
        if (HWVER[4]=='M' && HWVER[5]=='1' && HWVER[6]=='6' && HWVER[7]=='0') model=3;
        if (HWVER[4]=='M' && HWVER[5]=='1' && HWVER[6]=='8' && HWVER[7]=='0') model=5;
    }

    EXTRUDERS=1;
    Z_ULN2003=false;

    if (model==1 || model==0 || model==8)
    {
        /*if (HWVER[0]=='H' && HWVER[1]=='A' && HWVER[2]=='0' && HWVER[3]=='2')
        {
            DEFAULT_AXIS_STEPS_PER_UNIT[3]=125.0;
        }*/
        if (HWVER[3]>='4')
        {
            DEFAULT_AXIS_STEPS_PER_UNIT[0] = 46.5;
            DEFAULT_AXIS_STEPS_PER_UNIT[1] = 46.5;
            DEFAULT_AXIS_STEPS_PER_UNIT[2] = 548.75;
            DEFAULT_AXIS_STEPS_PER_UNIT[3] = 48.5;
        }
        //GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
        //GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
        base_max_pos[0]=120;
        base_max_pos[1]=120;
        base_max_pos[2]=120;
        
        //Z_MIN_ENDSTOP_INVERTING=false;

        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_ResetBits(GPIOB,GPIO_Pin_3);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_ResetBits(GPIOB,GPIO_Pin_3);

        X_MAX_POS=120;Y_MAX_POS=120;Z_MAX_POS=120;
        DIO100_PORT=GPIOB;
        DIO100_PIN=GPIO_Pin_14;

        DIO101_PORT=GPIOB;
        DIO101_PIN=GPIO_Pin_15;

        DIO102_PORT=GPIOB;
        DIO102_PIN=GPIO_Pin_4;

        DIO103_PORT=GPIOB;
        DIO103_PIN=GPIO_Pin_12;

        DIO104_PORT=GPIOB;
        DIO104_PIN=GPIO_Pin_13;

        DIO105_PORT=GPIOA;
        DIO105_PIN=GPIO_Pin_15;

        DIO106_PORT=GPIOB;
        DIO106_PIN=GPIO_Pin_10;

        DIO107_PORT=GPIOB;
        DIO107_PIN=GPIO_Pin_2;

        DIO108_PORT=GPIOB;
        DIO108_PIN=GPIO_Pin_5;

        DIO109_PORT=GPIOB;
        DIO109_PIN=GPIO_Pin_0;

        DIO110_PORT=GPIOC;
        DIO110_PIN=GPIO_Pin_13;

        DIO111_PORT=GPIOB;
        DIO111_PIN=GPIO_Pin_8;

        DIO112_PORT=GPIOB;
        DIO112_PIN=GPIO_Pin_6;

        DIO113_PORT=GPIOB;
        DIO113_PIN=GPIO_Pin_7;

        DIO114_PORT=GPIOA;
        DIO114_PIN=GPIO_Pin_8;

        DIO115_PORT=GPIOA;
        DIO115_PIN=GPIO_Pin_8;

        DIO116_PORT=GPIOB;
        DIO116_PIN=GPIO_Pin_11;

        DIO117_PORT=GPIOB;
        DIO117_PIN=GPIO_Pin_1;

        DIO118_PORT=GPIOA;
        DIO118_PIN=GPIO_Pin_0;

        DIO119_PORT=GPIOA;
        DIO119_PIN=GPIO_Pin_1;
    }
    else if (model==2)
    {
        Z_ULN2003=true;
        DEFAULT_MAX_FEEDRATE[2]=1.0;
        DEFAULT_AXIS_STEPS_PER_UNIT[2]=137.1875;
        X_MAX_POS=100;Y_MAX_POS=100;Z_MAX_POS=100;
        DEFAULT_MAX_ACCELERATION[0]=1500;
        DEFAULT_MAX_ACCELERATION[1]=1500;

        GPIO_InitTypeDef GPIO_InitStructure;
        TIM_OCInitTypeDef  TIM_OCInitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;
        EXTI_InitTypeDef EXTI_InitStructure;

        //GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        
        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        GPIO_ResetBits(GPIOB, GPIO_Pin_0);
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        GPIO_ResetBits(GPIOC, GPIO_Pin_14);

        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        GPIO_ResetBits(GPIOA,GPIO_Pin_13);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        GPIO_SetBits(GPIOA,GPIO_Pin_14);

        /*GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_ResetBits(GPIOA,GPIO_Pin_2);
        GPIO_ResetBits(GPIOA,GPIO_Pin_3);*/

        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

/*
        NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource15);
        EXTI_InitStructure.EXTI_Line = EXTI_Line15;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);*/

        DIO100_PORT=GPIOB;
        DIO100_PIN=GPIO_Pin_14;

        DIO101_PORT=GPIOB;
        DIO101_PIN=GPIO_Pin_15;

        DIO102_PORT=GPIOB;
        DIO102_PIN=GPIO_Pin_4;

        DIO103_PORT=GPIOB;
        DIO103_PIN=GPIO_Pin_10;

        DIO104_PORT=GPIOB;
        DIO104_PIN=GPIO_Pin_2;

        DIO105_PORT=GPIOA;
        DIO105_PIN=GPIO_Pin_15;

        DIO106_PORT=GPIOB;
        DIO106_PIN=GPIO_Pin_10;

        DIO107_PORT=GPIOB;
        DIO107_PIN=GPIO_Pin_2;

        DIO108_PORT=GPIOB;
        DIO108_PIN=GPIO_Pin_5;

        DIO109_PORT=GPIOB;
        DIO109_PIN=GPIO_Pin_12;

        DIO110_PORT=GPIOB;
        DIO110_PIN=GPIO_Pin_13;

        DIO111_PORT=GPIOB;
        DIO111_PIN=GPIO_Pin_8;

        DIO112_PORT=GPIOB;
        DIO112_PIN=GPIO_Pin_6;

        DIO113_PORT=GPIOB;
        DIO113_PIN=GPIO_Pin_7;

        DIO114_PORT=GPIOA;
        DIO114_PIN=GPIO_Pin_8;

        DIO115_PORT=GPIOA;
        DIO115_PIN=GPIO_Pin_8;

        DIO116_PORT=GPIOB;
        DIO116_PIN=GPIO_Pin_11;

        DIO117_PORT=GPIOB;
        DIO117_PIN=GPIO_Pin_11;

        DIO118_PORT=GPIOA;
        DIO118_PIN=GPIO_Pin_0;

        DIO119_PORT=GPIOA;
        DIO119_PIN=GPIO_Pin_1;
    }
    else if (model==3)
    {
        X_MAX_POS=200;Y_MAX_POS=200;Z_MAX_POS=180;

        DIO103_PORT=GPIOB;
        DIO103_PIN=GPIO_Pin_15;

        DIO104_PORT=GPIOB;
        DIO104_PIN=GPIO_Pin_14;

        DIO102_PORT=GPIOB;
        DIO102_PIN=GPIO_Pin_4;

        DIO100_PORT=GPIOB;
        DIO100_PIN=GPIO_Pin_13;

        DIO101_PORT=GPIOB;
        DIO101_PIN=GPIO_Pin_12;

        DIO105_PORT=GPIOB;
        DIO105_PIN=GPIO_Pin_0;

        DIO106_PORT=GPIOB;
        DIO106_PIN=GPIO_Pin_10;

        DIO107_PORT=GPIOB;
        DIO107_PIN=GPIO_Pin_2;

        DIO108_PORT=GPIOB;
        DIO108_PIN=GPIO_Pin_5;

        DIO109_PORT=GPIOC;
        DIO109_PIN=GPIO_Pin_14;

        DIO110_PORT=GPIOC;
        DIO110_PIN=GPIO_Pin_13;

        DIO111_PORT=GPIOB;
        DIO111_PIN=GPIO_Pin_8;

        DIO112_PORT=GPIOB;
        DIO112_PIN=GPIO_Pin_7;

        DIO113_PORT=GPIOB;
        DIO113_PIN=GPIO_Pin_6;

        DIO114_PORT=GPIOA;
        DIO114_PIN=GPIO_Pin_8;

        DIO115_PORT=GPIOA;
        DIO115_PIN=GPIO_Pin_8;

        DIO116_PORT=GPIOB;
        DIO116_PIN=GPIO_Pin_11;

        DIO117_PORT=GPIOB;
        DIO117_PIN=GPIO_Pin_1;

        DIO118_PORT=GPIOA;
        DIO118_PIN=GPIO_Pin_0;

        DIO119_PORT=GPIOA;
        DIO119_PIN=GPIO_Pin_1;
    }
    else if (model==4)
    {
        extern bool unlocked;

        //GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);
        EXTRUDERS=2;
        DEFAULT_AXIS_STEPS_PER_UNIT[2]=640;
        DEFAULT_MAX_FEEDRATE[2]=15;
        DEFAULT_MAX_ACCELERATION[2]=200;

        X_MAX_POS=300;Y_MAX_POS=300;Z_MAX_POS=450;

        //X_MIN_ENDSTOP_INVERTING=false;
        //Y_MIN_ENDSTOP_INVERTING=false;
        //Z_MIN_ENDSTOP_INVERTING=false;

        DIO100_PORT=GPIOB;
        DIO100_PIN=GPIO_Pin_15;

        DIO101_PORT=GPIOB;
        DIO101_PIN=GPIO_Pin_14;

        DIO102_PORT=GPIOB;
        DIO102_PIN=GPIO_Pin_4;

        DIO103_PORT=GPIOB;
        DIO103_PIN=GPIO_Pin_13;

        DIO104_PORT=GPIOB;
        DIO104_PIN=GPIO_Pin_12;

        DIO105_PORT=GPIOA;
        DIO105_PIN=GPIO_Pin_15;

        DIO106_PORT=GPIOB;
        DIO106_PIN=GPIO_Pin_10;

        DIO107_PORT=GPIOB;
        DIO107_PIN=GPIO_Pin_2;

        DIO108_PORT=GPIOB;
        DIO108_PIN=GPIO_Pin_5;

        DIO109_PORT=GPIOC;
        DIO109_PIN=GPIO_Pin_14;

        DIO110_PORT=GPIOC;
        DIO110_PIN=GPIO_Pin_13;

        DIO111_PORT=GPIOB;
        DIO111_PIN=GPIO_Pin_9;

        DIO112_PORT=GPIOB;
        DIO112_PIN=GPIO_Pin_7;

        DIO113_PORT=GPIOB;
        DIO113_PIN=GPIO_Pin_6;

        DIO114_PORT=GPIOA;
        DIO114_PIN=GPIO_Pin_8;

        DIO115_PORT=GPIOA;
        DIO115_PIN=GPIO_Pin_8;

        DIO116_PORT=GPIOB;
        DIO116_PIN=GPIO_Pin_11;

        DIO117_PORT=GPIOB;
        DIO117_PIN=GPIO_Pin_1;

        DIO118_PORT=GPIOA;
        DIO118_PIN=GPIO_Pin_0;

        DIO119_PORT=GPIOA;
        DIO119_PIN=GPIO_Pin_1;

        DIO120_PORT=GPIOB;
        DIO120_PIN=GPIO_Pin_8;

        DIO121_PORT=GPIOC;
        DIO121_PIN=GPIO_Pin_15;

        DIO122_PORT=GPIOA;
        DIO122_PIN=GPIO_Pin_14;

        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_SetBits(GPIOB,GPIO_Pin_0);
    }
    else if (model==5)
    {
        extern bool unlocked;

        //GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable,ENABLE);
        EXTRUDERS=2;
        DEFAULT_AXIS_STEPS_PER_UNIT[2]=1600;
        DEFAULT_MAX_FEEDRATE[2]=8;
        DEFAULT_MAX_ACCELERATION[2]=200;

        X_MAX_POS=220;Y_MAX_POS=170;Z_MAX_POS=165;

        X_MIN_ENDSTOP_INVERTING=false;
        Y_MIN_ENDSTOP_INVERTING=false;
        Z_MIN_ENDSTOP_INVERTING=false;

        DIO100_PORT=GPIOB;
        DIO100_PIN=GPIO_Pin_15;

        DIO101_PORT=GPIOB;
        DIO101_PIN=GPIO_Pin_14;

        DIO102_PORT=GPIOB;
        DIO102_PIN=GPIO_Pin_4;

        DIO103_PORT=GPIOB;
        DIO103_PIN=GPIO_Pin_13;

        DIO104_PORT=GPIOB;
        DIO104_PIN=GPIO_Pin_12;

        DIO105_PORT=GPIOA;
        DIO105_PIN=GPIO_Pin_15;

        DIO106_PORT=GPIOB;
        DIO106_PIN=GPIO_Pin_10;

        DIO107_PORT=GPIOB;
        DIO107_PIN=GPIO_Pin_2;

        DIO108_PORT=GPIOB;
        DIO108_PIN=GPIO_Pin_5;

        DIO121_PORT=GPIOC;
        DIO121_PIN=GPIO_Pin_14;

        DIO122_PORT=GPIOC;
        DIO122_PIN=GPIO_Pin_13;

        DIO111_PORT=GPIOB;
        DIO111_PIN=GPIO_Pin_9;

        DIO112_PORT=GPIOB;
        DIO112_PIN=GPIO_Pin_7;

        DIO113_PORT=GPIOB;
        DIO113_PIN=GPIO_Pin_6;

        DIO114_PORT=GPIOA;
        DIO114_PIN=GPIO_Pin_8;

        DIO115_PORT=GPIOA;
        DIO115_PIN=GPIO_Pin_8;

        DIO116_PORT=GPIOB;
        DIO116_PIN=GPIO_Pin_11;

        DIO117_PORT=GPIOB;
        DIO117_PIN=GPIO_Pin_1;

        DIO118_PORT=GPIOA;
        DIO118_PIN=GPIO_Pin_0;

        DIO119_PORT=GPIOA;
        DIO119_PIN=GPIO_Pin_1;

        DIO120_PORT=GPIOB;
        DIO120_PIN=GPIO_Pin_8;

        DIO109_PORT=GPIOC;
        DIO109_PIN=GPIO_Pin_15;

        DIO110_PORT=GPIOA;
        DIO110_PIN=GPIO_Pin_14;

        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_SetBits(GPIOB,GPIO_Pin_0);

        
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_SetBits(GPIOB,GPIO_Pin_3);
    }
    else if (model==6)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOC, ENABLE);
        
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        
        //////////////////////////////////////
        //  Disable BTN LED                 //
        //////////////////////////////////////
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        //////////////////////////////////////
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        
        DIO100_PORT=GPIOB;
        DIO100_PIN=GPIO_Pin_14;

        DIO101_PORT=GPIOB;
        DIO101_PIN=GPIO_Pin_13;

        DIO102_PORT=GPIOC;
        DIO102_PIN=GPIO_Pin_13;

        DIO103_PORT=GPIOB;
        DIO103_PIN=GPIO_Pin_12;

        DIO104_PORT=GPIOB;
        DIO104_PIN=GPIO_Pin_11;

        DIO105_PORT=GPIOC;
        DIO105_PIN=GPIO_Pin_14;

        DIO106_PORT=GPIOB;
        DIO106_PIN=GPIO_Pin_2;

        DIO107_PORT=GPIOB;
        DIO107_PIN=GPIO_Pin_1;

        DIO108_PORT=GPIOC;
        DIO108_PIN=GPIO_Pin_15;

        DIO109_PORT=GPIOA;
        DIO109_PIN=GPIO_Pin_7;

        DIO110_PORT=GPIOA;
        DIO110_PIN=GPIO_Pin_6;

        DIO111_PORT=GPIOA;
        DIO111_PIN=GPIO_Pin_8;

        DIO112_PORT=GPIOA;
        DIO112_PIN=GPIO_Pin_1;
        
        DIO120_PORT=GPIOA;
        DIO120_PIN=GPIO_Pin_1;

        DIO113_PORT=GPIOA;
        DIO113_PIN=GPIO_Pin_5;

        DIO114_PORT=GPIOB;
        DIO114_PIN=GPIO_Pin_10;

        DIO115_PORT=GPIOB;
        DIO115_PIN=GPIO_Pin_10;

        /*DIO116_PORT=GPIOB;
        DIO116_PIN=GPIO_Pin_10;*/
        DIO116_PORT=GPIOA;
        DIO116_PIN=GPIO_Pin_2;

        DIO117_PORT=GPIOB;
        DIO117_PIN=GPIO_Pin_0;

        DIO118_PORT=GPIOB;
        DIO118_PIN=GPIO_Pin_0;

        DIO119_PORT=GPIOA;
        DIO119_PIN=GPIO_Pin_4;
                
        DIO123_PORT=GPIOB;
        DIO123_PIN=GPIO_Pin_7;
        
        //DIO108_PORT=DIO123_PORT;
        //DIO108_PIN=DIO123_PIN;
        
        invert_byte=0;
        /*X_MIN_ENDSTOP_INVERTING=true;
        Y_MIN_ENDSTOP_INVERTING=true;
        Z_MIN_ENDSTOP_INVERTING=true;
        X_MAX_ENDSTOP_INVERTING=false;
        Y_MAX_ENDSTOP_INVERTING=false;
        Z_MAX_ENDSTOP_INVERTING=false;*/
        
        /*homing_feedrate[0]=80*60;
        homing_feedrate[1]=80*60;
        homing_feedrate[2]=80*60;
        
        DEFAULT_AXIS_STEPS_PER_UNIT[0]=56.5;
        DEFAULT_AXIS_STEPS_PER_UNIT[1]=56.5;
        DEFAULT_AXIS_STEPS_PER_UNIT[2]=56.5;
        DEFAULT_AXIS_STEPS_PER_UNIT[3]=48.5;
        
        DEFAULT_MAX_FEEDRATE[2]=DEFAULT_MAX_FEEDRATE[0];
        DEFAULT_MAX_ACCELERATION[2]=DEFAULT_MAX_ACCELERATION[0];*/
    }
    else if (model==7)
    {
        base_max_pos[0]=300;
        base_max_pos[1]=300;
        base_max_pos[2]=450;
        
        invert_byte = 0x48;

        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_ResetBits(GPIOB,GPIO_Pin_3);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_ResetBits(GPIOB,GPIO_Pin_3);

        X_MAX_POS=300;Y_MAX_POS=300;Z_MAX_POS=450;
        DEFAULT_AXIS_STEPS_PER_UNIT[2]=1600;
        DEFAULT_MAX_FEEDRATE[2]=6;
        homing_feedrate[2]=5*60;
        
        DIO100_PORT=GPIOB;
        DIO100_PIN=GPIO_Pin_14;

        DIO101_PORT=GPIOB;
        DIO101_PIN=GPIO_Pin_15;

        DIO102_PORT=GPIOB;
        DIO102_PIN=GPIO_Pin_4;

        DIO103_PORT=GPIOB;
        DIO103_PIN=GPIO_Pin_12;

        DIO104_PORT=GPIOB;
        DIO104_PIN=GPIO_Pin_13;

        DIO105_PORT=GPIOA;
        DIO105_PIN=GPIO_Pin_15;

        DIO106_PORT=GPIOB;
        DIO106_PIN=GPIO_Pin_10;

        DIO107_PORT=GPIOB;
        DIO107_PIN=GPIO_Pin_2;

        DIO108_PORT=GPIOB;
        DIO108_PIN=GPIO_Pin_5;

        DIO109_PORT=GPIOB;
        DIO109_PIN=GPIO_Pin_0;

        DIO110_PORT=GPIOC;
        DIO110_PIN=GPIO_Pin_13;

        DIO111_PORT=GPIOB;
        DIO111_PIN=GPIO_Pin_8;

        DIO112_PORT=GPIOB;
        DIO112_PIN=GPIO_Pin_6;

        DIO113_PORT=GPIOB;
        DIO113_PIN=GPIO_Pin_7;

        DIO114_PORT=GPIOA;
        DIO114_PIN=GPIO_Pin_8;

        DIO115_PORT=GPIOA;
        DIO115_PIN=GPIO_Pin_8;

        DIO116_PORT=GPIOB;
        DIO116_PIN=GPIO_Pin_11;

        DIO117_PORT=GPIOB;
        DIO117_PIN=GPIO_Pin_1;

        DIO118_PORT=GPIOA;
        DIO118_PIN=GPIO_Pin_0;

        DIO119_PORT=GPIOA;
        DIO119_PIN=GPIO_Pin_1;
    }

    get_firmware_info();
    return;
}

uint8_t GPIO_ReadInputDataBit_filtered(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  //return GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
  uint8_t bitstatus_old = 0;
  uint8_t bitstatus = 0xFF;

  while (bitstatus!=bitstatus_old)
  {
      bitstatus_old=bitstatus;
      if ((GPIOx->IDR & GPIO_Pin) != (uint32_t)Bit_RESET)
      {
        bitstatus = (uint8_t)Bit_SET;
      }
      else
      {
        bitstatus = (uint8_t)Bit_RESET;
      }
  }
  
  return bitstatus;
}
