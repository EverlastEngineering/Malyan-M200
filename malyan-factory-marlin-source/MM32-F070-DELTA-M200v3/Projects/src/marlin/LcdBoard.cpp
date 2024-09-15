#include "LcdBoard.h"
#include "temperature.h"
#include "Marlin.h"
#include "cardreader.h"

#include <ctype.h>
#include <stdlib.h>

unsigned char buff_obj[8];
unsigned char buff_value[12];
unsigned char buff_ptr,buff_state;

int16_t jog_speed=250;

inline void put(char c) 
{
   while(USART_GetFlagStatus(LCD_UART, USART_FLAG_TXE) == RESET){}
   USART_SendData(LCD_UART,c);
}

void writeString(char message[]) {
    char* letter = message;
    while (*letter != 0) {
        put(*letter);
        letter++;
    }
}

void writeInt(uint16_t value, uint8_t digits) {
    uint32_t currentDigit, nextDigit, uvalue;
    
    switch (digits) {
        case 1:  currentDigit = 10;	     break;
        case 2:  currentDigit = 100;     break;
        case 3:  currentDigit = 1000;    break;
        case 4:  currentDigit = 10000;   break;
        case 5:  currentDigit = 100000;  break;
        case 6:  currentDigit = 1000000;  break;
        default: return;
    }
    
    uvalue = (uint32_t)value;
    for (uint8_t i = 0; i < digits; i++) {
        nextDigit = currentDigit / 10;
        put((uvalue % currentDigit) / nextDigit + '0');
        currentDigit = nextDigit;
    }
}

void writeInt32(uint32_t value, uint8_t digits) {
    uint32_t currentDigit, nextDigit, uvalue;
    
    switch (digits) {
        case 1:  currentDigit = 10;      break;
        case 2:  currentDigit = 100;     break;
        case 3:  currentDigit = 1000;    break;
        case 4:  currentDigit = 10000;   break;
        case 5:  currentDigit = 100000;  break;
        case 6:  currentDigit = 1000000;  break;
        case 7:  currentDigit = 10000000;  break;
        default: return;
    }
    
    uvalue = (uint32_t)value;
    for (uint8_t i = 0; i < digits; i++) {
        nextDigit = currentDigit / 10;
        put((uvalue % currentDigit) / nextDigit + '0');
        currentDigit = nextDigit;
    }
}

void malyan_lcd_init() {
	//INIT_SERIAL(3);
    //building = false;

    //writeString((char *)"{U:RG1R180180120P0L1S0D0O1E1H0C0X1Y1Z1A2B2N3M0}");
}



/*
 {FN:LIST},{}
 */
//void malyan_lcd_update() {
#define LCD_USART_RX_BUF_SIZE 32
extern uint8_t Uart_RX_LCD[LCD_USART_RX_BUF_SIZE];
extern volatile int8_t lcd_w_buff_ptr;
volatile int8_t lcd_r_buff_ptr=0;

void malyan_lcd_update() {
    uint8_t c;
    //while (UCSR3A & (1<<RXC3))
    while (lcd_r_buff_ptr!=lcd_w_buff_ptr)
	{
        c=Uart_RX_LCD[lcd_r_buff_ptr++];
        if (lcd_r_buff_ptr>=LCD_USART_RX_BUF_SIZE) lcd_r_buff_ptr=0;
        //put(c);
		switch (c)
		{
			case '{': if (buff_state==0)
                      {
                          buff_state=1;
                          buff_ptr=0;
                      }
                      break;
                
			case ':': if (buff_state==1)
                      {
                          buff_obj[buff_ptr++]=0;
                          buff_state=2;
                          buff_ptr=0;
                      }
                      break;
                
			case ',': if (buff_state==2)
                      {
                          buff_value[buff_ptr++]=0;
                          process();
                          buff_state=1;
                          buff_ptr=0;
                      }
                      else if (buff_state==1)
                      {
                          put('?');
                          buff_state=0;
                          buff_ptr=0;
                      }
                      break;
			
            case '}': if (buff_state==2)
                      {
                          buff_value[buff_ptr++]=0;
                          process();
                          buff_state=0;
                          buff_ptr=0;
                      }
                      else if (buff_state==1)
                      {
                          put('?');
                          buff_state=0;
                          buff_ptr=0;
                      }
                      break;

            default:  if (c>=0x21 && c<=0x7E)
                      {
                          if (buff_state==1) buff_obj[buff_ptr++]=c;
                          else if (buff_state==2) buff_value[buff_ptr++]=c;
                      }
                      else
                      {
                          put('?');
                          buff_state=0;
                          buff_ptr=0;
                      }
		}
	}
}

bool lcd_onboard_build = false;

