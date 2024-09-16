/**
 * Marlin Firmware
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * About Marlin
 *
 * This firmware is a mashup between Sprinter and grbl.
 *  - https://github.com/kliment/Sprinter
 *  - https://github.com/simen/grbl/tree
 *
 * It has preliminary support for Matthew Roberts advance algorithm
 *  - http://reprap.org/pipermail/reprap-dev/2011-May/003323.html
 */

/* All the implementation is done in *.cpp files to get better compatibility with avr-gcc without the Arduino IDE */
/* Use this file to help the Arduino IDE find which Arduino libraries are needed and to keep documentation on GCode */

#include "Configuration.h"
#include "pins.h"
#include "SEGGER_RTT.h"

#if ENABLED(ULTRA_LCD)
  #if ENABLED(LCD_I2C_TYPE_PCF8575)
    #include <Wire.h>
    #include <LiquidCrystal_I2C.h>
  #elif ENABLED(LCD_I2C_TYPE_MCP23017) || ENABLED(LCD_I2C_TYPE_MCP23008)
    #include <Wire.h>
    #include <LiquidTWI2.h>
  #elif ENABLED(DOGLCD)
    #include "U8glib.h" // library for graphics LCD by Oli Kraus (https://code.google.com/p/u8glib/)
  #else
    #include <LiquidCrystal.h> // library for character LCD
  #endif
#endif

#if HAS_DIGIPOTSS
  #include <SPI.h>
#endif

#if ENABLED(DIGIPOT_I2C)
  #include <Wire.h>
#endif

#if ENABLED(HAVE_TMCDRIVER)
  #include <SPI.h>
  #include <TMC26XStepper.h>
#endif

#if ENABLED(HAVE_L6470DRIVER)
  #include <SPI.h>
  #include <L6470.h>
#endif

extern void setup();
extern void loop();
extern "C" void USB_main();
extern "C" void SetSysTick();
extern void wifi_writeString(char message[]);

__no_init unsigned int MAGIC @ 0x20000400;
//__no_init char HWVER[4] @ 0x20000410;
//__no_init const char MACHINE_TYPE[4] @ 0x08001000;
extern const char MACHINE_TYPE[];

bool unlocked;

extern void update_LCD();
extern void serial_init(int);
extern "C" void GenerateSystemReset(void);
extern "C" void store_char_usb(unsigned char c);
extern "C" void MSD0_SPI_Configuration();

void writeString(char message[]) {
    char* letter = message;
    while (*letter != 0) {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}
        USART_SendData(USART1,*letter);
        letter++;
    }
}

void print_endstop(uint8_t p)
{
    if (p&1) writeString("ENDSTOP X TRIGGERED\n");
    if (p&2) writeString("ENDSTOP Y TRIGGERED\n");
    if (p&4) writeString("ENDSTOP Z TRIGGERED\n");
    if (p&8) writeString("ENDSTOP P TRIGGERED\n");
}

uint8_t get_endstop()
{
    uint8_t pin_stat=0;
    if (READ(X_STOP_PIN) == 0) pin_stat|=1;
    if (READ(Y_STOP_PIN) == 0) pin_stat|=2;
    if (READ(Z_STOP_PIN) == 0) pin_stat|=4;
    
    if (model==6 || model==8)
    {
        SET_INPUT(Z_MIN_PIN);
        if (READ(Z_MIN_PIN) == 0) pin_stat|=8;
    }
    else pin_stat|=8;
    return pin_stat;
}

void main()
{
    extern void Config_RetrieveSettings(bool);
    
    __disable_irq();
    memcpy((void*)0x20000000, (void*)0x08002000, 0x100);
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
    bDeviceState=0;
    com_opened=0;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOC, ENABLE);
    unlocked=false;
    if (MAGIC==0xDEADBEEF)
    {
        unlocked=true;
        USB_main();
    }
    serial_init(512000);
    writeString("serial_init\n");
    
    SetSysTick();
    writeString("SetSysTick\n");
    Config_RetrieveSettings(true);
    writeString("RetrieveSettings\n");
    port_init();
    writeString("port_init\n");
    
    if (model==6)
    {
      LED_WORKING_ON;LED_LINK_ON;LED_STOP_ON;
    }
    writeString("LED_ON\n");

    SET_INPUT(X_STOP_PIN);
    SET_INPUT(Y_STOP_PIN);
    SET_INPUT(Z_STOP_PIN);
    
    uint8_t pin_stat=0;
    pin_stat=get_endstop();
    print_endstop(pin_stat);
    
    pin_stat=get_endstop();
    print_endstop(pin_stat);
    
    if (pin_stat==0xF)
    {
      pin_stat=get_endstop();
      print_endstop(pin_stat);
      
      if (pin_stat==0xF)
      {
          writeString("ENTER UPDATE_LCD\n");
          update_LCD();
      }
    }
    
    setup();
    writeString("setup\n");

#ifdef PRINT_INFO
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
#endif

    extern const char VERSION[];
    wifi_writeString((char *)"{SYS:STARTED}");
    wifi_writeString((char *)"{VER:");
    while(USART_GetFlagStatus(WIFI_UART, USART_FLAG_TXE) == RESET){}
    USART_SendData(WIFI_UART,VERSION[0]+0x80);
    while(USART_GetFlagStatus(WIFI_UART, USART_FLAG_TXE) == RESET){}
    USART_SendData(WIFI_UART,VERSION[2]+0x80);
    wifi_writeString((char *)"}");
    
    store_char_usb('\n');
    
    __enable_irq();
    
    if (model==2)
    {
        //delay(1000);
        LED_WORKING_OFF;LED_LINK_OFF;LED_STOP_OFF;
    }
    else if (model==6)
    {
        LED_WORKING_OFF;LED_LINK_OFF;LED_STOP_OFF;
    }
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    if (MACHINE_TYPE[0]!='M') unlocked=false;
    while (1)
    {
        loop();
    }
}
