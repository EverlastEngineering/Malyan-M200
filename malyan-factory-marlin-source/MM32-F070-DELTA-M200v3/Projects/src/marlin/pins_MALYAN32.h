/**
 * Malyan32 board pin assignments
 */
#include "configuration.h"

#if 1
#define PA0 0
#define PA1 1
#define PA2 2
#define PA3 3
#define PA4 4
#define PA5 5
#define PA6 6
#define PA7 7
#define PA8 8
#define PA9 9
#define PA10 10
#define PA11 11
#define PA12 12
#define PA13 13
#define PA14 14
#define PA15 15
#define PB0 16
#define PB1 17
#define PB2 18
#define PB3 19
#define PB4 20
#define PB5 21
#define PB6 22
#define PB7 23
#define PB8 24
#define PB9 25
#define PB10 26
#define PB11 27
#define PB12 28
#define PB13 29
#define PB14 30
#define PB15 31
#define PC0 32
#define PC1 33
#define PC2 34
#define PC3 35
#define PC4 36
#define PC5 37
#define PC6 38
#define PC7 39
#define PC8 40
#define PC9 41
#define PC10 42
#define PC11 43
#define PC12 44
#define PC13 45
#define PC14 46
#define PC15 47
#define PD0 48
#define PD1 49

#define GPIOA0 0
#define GPIOA1 1
#define GPIOA2 2
#define GPIOA3 3
#define GPIOA4 4
#define GPIOA5 5
#define GPIOA6 6
#define GPIOA7 7
#define GPIOA8 8
#define GPIOA9 9
#define GPIOA10 10
#define GPIOA11 11
#define GPIOA12 12
#define GPIOA13 13
#define GPIOA14 14
#define GPIOA15 15
#define GPIOB0 16
#define GPIOB1 17
#define GPIOB2 18
#define GPIOB3 19
#define GPIOB4 20
#define GPIOB5 21
#define GPIOB6 22
#define GPIOB7 23
#define GPIOB8 24
#define GPIOB9 25
#define GPIOB10 26
#define GPIOB11 27
#define GPIOB12 28
#define GPIOB13 29
#define GPIOB14 30
#define GPIOB15 31
#define GPIOC0 32
#define GPIOC1 33
#define GPIOC2 34
#define GPIOC3 35
#define GPIOC4 36
#define GPIOC5 37
#define GPIOC6 38
#define GPIOC7 39
#define GPIOC8 40
#define GPIOC9 41
#define GPIOC10 42
#define GPIOC11 43
#define GPIOC12 44
#define GPIOC13 45
#define GPIOC14 46
#define GPIOC15 47
#define GPIOD0 48
#define GPIOD1 49

#include "stm32f0xx_gpio.h"
extern "C" uint8_t GPIO_ReadInputDataBit_filtered(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

extern GPIO_TypeDef* DIO100_PORT;
extern uint16_t DIO100_PIN;
extern GPIO_TypeDef* DIO101_PORT;
extern uint16_t DIO101_PIN;
extern GPIO_TypeDef* DIO102_PORT;
extern uint16_t DIO102_PIN;
extern GPIO_TypeDef* DIO103_PORT;
extern uint16_t DIO103_PIN;
extern GPIO_TypeDef* DIO104_PORT;
extern uint16_t DIO104_PIN;
extern GPIO_TypeDef* DIO105_PORT;
extern uint16_t DIO105_PIN;
extern GPIO_TypeDef* DIO106_PORT;
extern uint16_t DIO106_PIN;
extern GPIO_TypeDef* DIO107_PORT;
extern uint16_t DIO107_PIN;
extern GPIO_TypeDef* DIO108_PORT;
extern uint16_t DIO108_PIN;
extern GPIO_TypeDef* DIO109_PORT;
extern uint16_t DIO109_PIN;
extern GPIO_TypeDef* DIO110_PORT;
extern uint16_t DIO110_PIN;
extern GPIO_TypeDef* DIO111_PORT;
extern uint16_t DIO111_PIN;
extern GPIO_TypeDef* DIO112_PORT;
extern uint16_t DIO112_PIN;
extern GPIO_TypeDef* DIO113_PORT;
extern uint16_t DIO113_PIN;
extern GPIO_TypeDef* DIO114_PORT;
extern uint16_t DIO114_PIN;
extern GPIO_TypeDef* DIO115_PORT;
extern uint16_t DIO115_PIN;
extern GPIO_TypeDef* DIO116_PORT;
extern uint16_t DIO116_PIN;
extern GPIO_TypeDef* DIO117_PORT;
extern uint16_t DIO117_PIN;
extern GPIO_TypeDef* DIO118_PORT;
extern uint16_t DIO118_PIN;
extern GPIO_TypeDef* DIO119_PORT;
extern uint16_t DIO119_PIN;
extern GPIO_TypeDef* DIO120_PORT;
extern uint16_t DIO120_PIN;
extern GPIO_TypeDef* DIO121_PORT;
extern uint16_t DIO121_PIN;
extern GPIO_TypeDef* DIO122_PORT;
extern uint16_t DIO122_PIN;
extern GPIO_TypeDef* DIO123_PORT;
extern uint16_t DIO123_PIN;
#endif

#define X_STEP_PIN         100
#define X_DIR_PIN          101
#define X_STOP_PIN         102

#define Y_STEP_PIN         103
#define Y_DIR_PIN          104
#define Y_STOP_PIN         105

#define Z_STEP_PIN         106
#define Z_DIR_PIN          107
#define Z_STOP_PIN         108

#if ENABLED(DELTA)
#define Z_MIN_PIN          123
#else
//#define Z_MIN_PIN          -1
#define Z_MIN_PIN          108
#endif

#define E0_STEP_PIN        109
#define E0_DIR_PIN         110

#define E1_STEP_PIN        121
#define E1_DIR_PIN         122

#define LED_PIN            -1
#define FAN_PIN            111

#define LED_PIN            -1
#define LCD_PIN_BL         -1

#if ENABLED(Z_PROBE_SLED)
  #define SLED_PIN         -1
#endif

#define PS_ON_PIN          -1
#define KILL_PIN           -1

#define HEATER_0_PIN       112 // (extruder)
#define HEATER_1_PIN       120
#define HEATER_2_PIN       -1

#define HEATER_BED_PIN   113  // (bed)
#define X_ENABLE_PIN     114
#define Y_ENABLE_PIN     115
#define Z_ENABLE_PIN     116

#define E0_ENABLE_PIN    117
#define E1_ENABLE_PIN    117

#define TEMP_0_PIN         118
#define TEMP_1_PIN         118
#define TEMP_2_PIN         -1
#define TEMP_BED_PIN       119
#define SDPOWER            -1
#define SDSS               -1

#if ENABLED(ULTRA_LCD) && ENABLED(NEWPANEL)

  // No buzzer installed
  #define BEEPER_PIN -1

  // LCD Pins
  #if ENABLED(DOGLCD)

    #if ENABLED(U8GLIB_ST7920) //SPI GLCD 12864 ST7920 ( like [www.digole.com] ) For Melzi V2.0

      #if MB(MELZI) // Melzi board
        #define LCD_PINS_RS     30 //CS chip select /SS chip slave select
        #define LCD_PINS_ENABLE 29 //SID (MOSI)
        #define LCD_PINS_D4     17 //SCK (CLK) clock
        #define BEEPER_PIN      27 // Pin 27 is taken by LED_PIN, but Melzi LED does nothing with Marlin so this can be used for BEEPER_PIN. You can use this pin with M42 instead of BEEPER_PIN.
      #else         // Sanguinololu 1.3
        #define LCD_PINS_RS      4
        #define LCD_PINS_ENABLE 17
        #define LCD_PINS_D4     30
        #define LCD_PINS_D5     29
        #define LCD_PINS_D6     28
        #define LCD_PINS_D7     27
      #endif

    #else // DOGM SPI LCD Support

      #define DOGLCD_A0         30
      #define DOGLCD_CS         29
      #define LCD_CONTRAST       1

    #endif

    // Uncomment screen orientation
    #define LCD_SCREEN_ROT_0
    //#define LCD_SCREEN_ROT_90
    //#define LCD_SCREEN_ROT_180
    //#define LCD_SCREEN_ROT_270

  #else // !DOGLCD - Standard Hitachi LCD controller

    #define LCD_PINS_RS          4
    #define LCD_PINS_ENABLE     17
    #define LCD_PINS_D4         30
    #define LCD_PINS_D5         29
    #define LCD_PINS_D6         28
    #define LCD_PINS_D7         27

  #endif // !DOGLCD

  //The encoder and click button
  #define BTN_EN1               0
  #define BTN_EN2               0
  #if ENABLED(LCD_I2C_PANELOLU2)
    #if MB(MELZI)
      #define BTN_ENC           0
      #define LCD_SDSS          0 // Panelolu2 SD card reader rather than the Melzi
    #else
      #define BTN_ENC           0
    #endif
  #else
    #define BTN_ENC             0
    #define LCD_SDSS            0 // Smart Controller SD card reader rather than the Melzi
  #endif //Panelolu2

  #define SD_DETECT_PIN         -1

#elif ENABLED(MAKRPANEL)

  #define BEEPER_PIN            0

  // Pins for DOGM SPI LCD Support
  #define DOGLCD_A0             0
  #define DOGLCD_CS             0
  #define LCD_PIN_BL            0 // backlight LED on PA3
  // GLCD features
  #define LCD_CONTRAST           0
  // Uncomment screen orientation
  #define LCD_SCREEN_ROT_0
  //#define LCD_SCREEN_ROT_90
  //#define LCD_SCREEN_ROT_180
  //#define LCD_SCREEN_ROT_270
  //The encoder and click button
  #define BTN_EN1               0
  #define BTN_EN2               0
  #define BTN_ENC               0

  #define SD_DETECT_PIN         -1

#endif // MAKRPANEL

// #if FAN_PIN == 12 || FAN_PIN ==13
//#define FAN_SOFT_PWM
// #endif
