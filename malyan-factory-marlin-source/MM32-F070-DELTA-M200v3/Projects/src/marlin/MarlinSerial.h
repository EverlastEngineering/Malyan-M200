/*
  HardwareSerial.h - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 28 September 2010 by Mark Sproul
*/

#ifndef MarlinSerial_h
#define MarlinSerial_h
#include "Marlin.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_usart.h"

extern "C" uint32_t CDC_Send_DATA (uint8_t *ptrBuffer, uint8_t Send_length);
extern "C" uint32_t CDC_Send_DATA_str (uint8_t *ptrBuffer);
extern "C" uint8_t bDeviceState;

#define CONFIGURED      5

#ifndef SERIAL_PORT
  #define SERIAL_PORT 0
#endif

// The presence of the UBRRH register is used to detect a UART.
#define UART_PRESENT(port) (port == 0)
/*
#define UART_PRESENT(port) ((port == 0 && (defined(UBRRH) || defined(UBRR0H))) || \
                            (port == 1 && defined(UBRR1H)) || (port == 2 && defined(UBRR2H)) || \
                            (port == 3 && defined(UBRR3H)))
*/

// These are macros to build serial port register names for the selected SERIAL_PORT (C preprocessor
// requires two levels of indirection to expand macro values properly)
#define SERIAL_REGNAME(registerbase,number,suffix) SERIAL_REGNAME_INTERNAL(registerbase,number,suffix)
#if SERIAL_PORT == 0 && (!defined(UBRR0H) || !defined(UDR0)) // use un-numbered registers if necessary
  #define SERIAL_REGNAME_INTERNAL(registerbase,number,suffix) registerbase##suffix
#else
  #define SERIAL_REGNAME_INTERNAL(registerbase,number,suffix) registerbase##number##suffix
#endif

// Registers used by MarlinSerial class (these are expanded
// depending on selected serial port
#define M_UCSRxA SERIAL_REGNAME(UCSR,SERIAL_PORT,A) // defines M_UCSRxA to be UCSRnA where n is the serial port number
#define M_UCSRxB SERIAL_REGNAME(UCSR,SERIAL_PORT,B)
#define M_RXENx SERIAL_REGNAME(RXEN,SERIAL_PORT,)
#define M_TXENx SERIAL_REGNAME(TXEN,SERIAL_PORT,)
#define M_RXCIEx SERIAL_REGNAME(RXCIE,SERIAL_PORT,)
#define M_UDREx SERIAL_REGNAME(UDRE,SERIAL_PORT,)
#define M_UDRx SERIAL_REGNAME(UDR,SERIAL_PORT,)
#define M_UBRRxH SERIAL_REGNAME(UBRR,SERIAL_PORT,H)
#define M_UBRRxL SERIAL_REGNAME(UBRR,SERIAL_PORT,L)
#define M_RXCx SERIAL_REGNAME(RXC,SERIAL_PORT,)
#define M_USARTx_RX_vect SERIAL_REGNAME(USART,SERIAL_PORT,_RX_vect)
#define M_U2Xx SERIAL_REGNAME(U2X,SERIAL_PORT,)


#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
#define BYT 0


#ifndef USBCON
// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which rx_buffer_head is the index of the
// location to which to write the next incoming character and rx_buffer_tail
// is the index of the location from which to read.
#define RX_BUFFER_SIZE 256


struct ring_buffer {
  unsigned char buffer[RX_BUFFER_SIZE];
  int head;
  int tail;
};

#if UART_PRESENT(SERIAL_PORT)
  extern ring_buffer rx_buffer;
#endif

extern uint8_t com_opened;

class MarlinSerial { //: public Stream

  public:
    MarlinSerial();
    void begin(long);
    void end();
    int peek(void);
    int read(void);
    void flush(void);

    FORCE_INLINE int available(void) {
      return (unsigned int)(RX_BUFFER_SIZE + rx_buffer.head - rx_buffer.tail) % RX_BUFFER_SIZE;
    }
#define CONFIGURED      5
    FORCE_INLINE void write(uint8_t c) {//yongzong
      uint32_t timeout=0;
      /*while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
      USART_SendData(USART1, c);*/
      if (com_opened==0)
      /*extern char USB_Connected;
      if (USB_Connected==0 && bDeviceState != CONFIGURED)*/
      {
          while(USART_GetFlagStatus(WIFI_UART, USART_FLAG_TXE) == RESET);
          USART_SendData(WIFI_UART,c);
          return;
      }
      //while (packet_sent == 0) if (timeout++>10000) return;
      CDC_Send_DATA((unsigned char*)&c,1);
    }
    
    FORCE_INLINE void write(const char* s)
    {
      uint32_t timeout=0;
      if (com_opened==0)
      {
          while (*s!=0)
          {
              while(USART_GetFlagStatus(WIFI_UART, USART_FLAG_TXE) == RESET);
              USART_SendData(WIFI_UART,*s);
              s++;
          }
          return;
      }
      //while (packet_sent == 0) if (timeout++>10000) return;
      CDC_Send_DATA_str((uint8_t*)s);
    }
    FORCE_INLINE void write(uint8_t* s) {//yongzong
      uint32_t timeout=0;
      if (com_opened==0)
      {
          while (*s!=0)
          {
              while(USART_GetFlagStatus(WIFI_UART, USART_FLAG_TXE) == RESET);
              USART_SendData(WIFI_UART,*s);
              s++;
          }
          return;
      }
      //while (packet_sent == 0) if (timeout++>10000) return;
      CDC_Send_DATA_str(s);
    }
    
    FORCE_INLINE void write(uint8_t* s, uint8_t len) {//yongzong
      uint32_t timeout=0;
      if (com_opened==0)
      {
          uint8_t i;
          for (i=0;i<len;i++)
          {
              while(USART_GetFlagStatus(WIFI_UART, USART_FLAG_TXE) == RESET);
              USART_SendData(WIFI_UART,*s);
              s++;
          }
          return;
      }
      //while (packet_sent == 0) if (timeout++>10000) return;
      
      CDC_Send_DATA(s,len);
    }
/*
    FORCE_INLINE void checkRx(void) {
      if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
      //if (TEST(M_UCSRxA, M_RXCx)) {
        unsigned char c;//  =  M_UDRx;
        c = USART_ReceiveData(USART1);
        int i = (unsigned int)(rx_buffer.head + 1) % RX_BUFFER_SIZE;

        // if we should be storing the received character into the location
        // just before the tail (meaning that the head would advance to the
        // current location of the tail), we're about to overflow the buffer
        // and so we don't write the character or advance the head.
        if (i != rx_buffer.tail) {
          rx_buffer.buffer[rx_buffer.head] = c;
          rx_buffer.head = i;
        }
      }
    }
*/
  private:
    void printNumber(unsigned long, uint8_t);
    void printFloat(double, uint8_t);

  public:
    //FORCE_INLINE void write(const char* str) { write(str);}//CDC_Send_DATA_str((uint8_t*)str); }//{ while (*str) write(*str++); }
    //FORCE_INLINE void write(const uint8_t* buffer, size_t size) { CDC_Send_DATA((uint8_t*)buffer,size); }//{ while (size--) write(*buffer++); }
    //FORCE_INLINE void print(const String& s) { for (int i = 0; i < (int)s.length(); i++) write(s[i]); }
    FORCE_INLINE void print(const char* str) {write(str);}//{ CDC_Send_DATA_str((uint8_t*)str); }

    void print(char, int = BYT);
    void print(unsigned char, int = BYT);
    void print(int, int = DEC);
    void print(unsigned int, int = DEC);
    void print(long, int = DEC);
    void print(unsigned long, int = DEC);
    void print(double, int = 2);

    //void println(const String& s);
    void println(const char[]);
    void println(char, int = BYT);
    void println(unsigned char, int = BYT);
    void println(int, int = DEC);
    void println(unsigned int, int = DEC);
    void println(long, int = DEC);
    void println(unsigned long, int = DEC);
    void println(double, int = 2);
    void println(void);
};

extern MarlinSerial customizedSerial;
#endif // !USBCON

// Use the UART for Bluetooth in AT90USB configurations
#if defined(USBCON) && ENABLED(BLUETOOTH)
  extern HardwareSerial bluetoothSerial;
#endif

#endif
