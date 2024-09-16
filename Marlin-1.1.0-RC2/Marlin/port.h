
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

//#define MSG_M115_REPORT                     "FIRMWARE_NAME:Malyan VERSION 1.1B\n"

#define WIFI_UART   USART1
#define LCD_UART    USART2
#define WIFI_UART_IRQHandler   USART1_IRQHandler
#define LCD_UART_IRQHandler   USART2_IRQHandler

#define DEFAULT_FAN_SPEED       160

#ifdef __cplusplus
extern "C" {
#endif
void reset_invert_bit();
void port_init();
void get_firmware_info();
extern char MSG_M115_REPORT[];
extern uint8_t model;
extern float homing_feedrate[];
extern float DEFAULT_AXIS_STEPS_PER_UNIT[];
extern float DEFAULT_MAX_FEEDRATE[];
extern long DEFAULT_MAX_ACCELERATION[];
extern bool Z_ULN2003;
extern uint8_t EXTRUDERS;
extern uint8_t invert_byte;
extern uint8_t thermistor_type;
extern uint8_t thermistor_bed_type;

/*#define LED_STOP_ON     GPIO_ResetBits(GPIOB,GPIO_Pin_15);
#define LED_STOP_OFF    GPIO_SetBits(GPIOB,GPIO_Pin_15);
#define LED_LINK_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_9);
#define LED_LINK_OFF GPIO_SetBits(GPIOB,GPIO_Pin_9);
#define LED_WORKING_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_8);
#define LED_WORKING_OFF GPIO_SetBits(GPIOB,GPIO_Pin_8);*/

#define LED_STOP_ON     if (model==6) GPIO_ResetBits(GPIOB,GPIO_Pin_15);else if (model==2) GPIO_ResetBits(GPIOB,GPIO_Pin_3);
#define LED_STOP_OFF    if (model==6) GPIO_SetBits(GPIOB,GPIO_Pin_15);else if (model==2) GPIO_SetBits(GPIOB,GPIO_Pin_3);
#define LED_LINK_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_9);
#define LED_LINK_OFF GPIO_SetBits(GPIOB,GPIO_Pin_9);
#define LED_WORKING_ON  if (model==6) GPIO_ResetBits(GPIOB,GPIO_Pin_8);else if (model==2) GPIO_ResetBits(GPIOA,GPIO_Pin_14);
#define LED_WORKING_OFF if (model==6) GPIO_SetBits(GPIOB,GPIO_Pin_8);else if (model==2) GPIO_SetBits(GPIOA,GPIO_Pin_14);

/*#define LED_LINK_ON     GPIO_ResetBits(GPIOB,GPIO_Pin_15);
#define LED_LINK_OFF    GPIO_SetBits(GPIOB,GPIO_Pin_15);
#define LED_WORKING_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_9);
#define LED_WORKING_OFF GPIO_SetBits(GPIOB,GPIO_Pin_9);
#define LED_STOP_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_8);
#define LED_STOP_OFF GPIO_SetBits(GPIOB,GPIO_Pin_8);*/

extern bool X_MIN_ENDSTOP_INVERTING;
extern bool Y_MIN_ENDSTOP_INVERTING;
extern bool Z_MIN_ENDSTOP_INVERTING;
extern bool X_MAX_ENDSTOP_INVERTING;
extern bool Y_MAX_ENDSTOP_INVERTING;
extern bool Z_MAX_ENDSTOP_INVERTING;
extern bool Z_MIN_PROBE_ENDSTOP_INVERTING;

#ifdef __cplusplus
}
#endif

