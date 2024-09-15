//*================================================================================================
//* 文件名				: xmodem.c
//* 功能描述			: 定时器相关函数
//* 作者    			: 罗伟东
//* 版本				: 0.01
//* 建立日期、时间		: 2011/04/24 13:30
//* 最近修改日期、时间	: 
//* 修改原因			: 
//*================================================================================================
/*------------------------------------------ 头文件 ---------------------------------------------*/
//#include 	"crc16.h"
//#include 	"timer.h"
#include 	"xmodem.h"
//#include	"decodeHex.h"
//#include	"ProgramFlash.h"
//#include	"interface.h"
//#include 	<stdio.h>
#include <stdbool.h>

#define printf(a) SEGGER_RTT_WriteString(0,a)
/*-----------------------------------------------------------------------------------------------*/
#define	BUFFER_SIZE					256U	/*暂存HEX文件数据缓冲区大小							*/

/*----------------------------------------- 全局变量 --------------------------------------------*/
uint32_t	g_RequestTimes=0;	//发送文件请求次数
uint8_t	g_CheckType='C';	//接收文件的校验方式
uint8_t	g_HexDataTemp[1530];//接收HEX文件缓冲区
extern uint8_t	g_ProgramFlashDataTemp[BUFFER_SIZE];
/*LAST_DATA_INFO g_LastData=
{
NO_DATA,
0,
{0}
};*/
//HEX_DATA_TEMP g_HexDataTemp
/*-----------------------------------------------------------------------------------------------*/

/*-------------------------------------- 自定义数据类型 -----------------------------------------*/

/*-----------------------------------------------------------------------------------------------*/

//*================================================================================================
//*											函	数	区
//*================================================================================================

/**************************************************************************************************
** 函数名称 : port_outbyte
** 功能描述 : 向串口发送一个字符数据
** 入口参数 : <trychar>[IN] 发送的数据
** 出口参数 : 无
** 返 回 值 : 无
** 其他说明 : 无
***************************************************************************************************/ 
void port_outbyte(uint8_t trychar)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET){}
    USART_SendData(USART1,trychar);
}

/**************************************************************************************************
** 函数名称 : port_inbyte
** 功能描述 : 向串口接收一个字符数据
** 入口参数 : <Time>[in] 读取数据最大延时时间
** 出口参数 : 无
** 返 回 值 : 从串口接收到的数据或者为0
** 其他说明 : 无
***************************************************************************************************/  
char port_inbyte(uint32_t Time)
{
    extern volatile unsigned long systick_count;
    extern volatile int16_t wifi_r_buff_ptr,wifi_w_buff_ptr;
    extern char Uart_RX_WIFI[];

    unsigned long timeout=systick_count+Time;
    
    #define WIFI_USART_RX_BUF_SIZE 256
    //wifi_r_buff_ptr = wifi_w_buff_ptr;
    while(systick_count<timeout){
        if (wifi_r_buff_ptr!=wifi_w_buff_ptr)
        {
            char ch = Uart_RX_WIFI[wifi_r_buff_ptr];
            wifi_r_buff_ptr++;
            if (wifi_r_buff_ptr>=WIFI_USART_RX_BUF_SIZE) wifi_r_buff_ptr=0;
            return ch;
        }
    }
    return 0;
}

/**************************************************************************************************
** 函数名称 : check
** 功能描述 : 校验，CRC为真则为CRC校验，否则为校验和
** 入口参数 : <crc>[in] 选择是CRC校验还是SUM校验
**			  <buf>[in] 校验的原始数据
**			  <sz>[in]  校验的数据长度
** 出口参数 : 无
** 返 回 值 : 校验无误返回TURE,反之返回FALSE
** 其他说明 : 无
***************************************************************************************************/   
bool check(bool crc, const unsigned char *buf, short sz)
{
    /*if (crc){
        unsigned short crc = crc16_ccitt(buf, sz);
        unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
        if (crc == tcrc)
            return true;
    }
    else*/{
        short i;
        unsigned char cks = 0;
        for (i = sz; i != 0; i--){
            cks += *(buf++);
        }
        if (cks == *buf)
            return true;
    }
    return false;
}

/**************************************************************************************************
** 函数名称 : xmodemReceive
** 功能描述 : xmodem协议接收接收文件
** 入口参数 : <checkType>[in] 接收文件的校验方式，'C':crc校验，NAK:累加和校验
** 出口参数 : 无
** 返 回 值 : 接收文件操作时的相关错误代码
** 其他说明 : 无
***************************************************************************************************/
uint8_t xmodemReceive(uint8_t checkType)
{
    uint8_t xbuff[133]; /* 128 for XModem  + 3 head chars + 2 crc + nul */
    uint8_t crc = 1;//启用CRC校验
    uint8_t packetno = 1;
    uint8_t bufsz;
    short i=0;
    short j=0;
    uint8_t	*p_LastData;
    uint8_t	*p_DataType;
    uint8_t chTemp=0;//接收字符缓存
    //	volatile uint32_t	RowNum2=0;	
    uint8_t	RowNum=0;	//接收到的行数
    p_LastData=g_HexDataTemp;
    p_DataType=g_HexDataTemp;
    if(checkType==CHECK_CRC){//CRC校验
        crc = 1;
        g_CheckType='C';
    }else if(checkType==CHECK_SUM){
        crc = 0;
        g_CheckType=NAK;
    }
    while(1){
        bufsz=128;
        printf("\n\r->");
        //向上位机请求发送文件
        for(i=100;i!=0;i--){
            chTemp = port_inbyte(500);
            if(chTemp>0){
                break;
            }else{
                port_outbyte(g_CheckType);
            }
        }
        //文件传输超时
        if((chTemp==0)&&(i==0)){
            printf("\n\r->Transmit Time Out!!");
            printf("\n\r->");
            /*此处可以返回错误信息*/
            return TIME_OUT;
        }
        //用户取消文件传输
        else if((chTemp=='B')||(chTemp=='b')){
            //printf("\n\r->%c",chTemp);
            printf("\n\r->User Canceled!!");
            printf("\n\r->");
            /*此处可以返回错误信息*/
            return USER_CANCELED;
        }
        else{ //开始传输文件
            while(chTemp==SOH){//接收到有效数据帧头
                xbuff[0]=chTemp;
                for(i=0;i<sizeof(xbuff);i++){//接收一帧数据
                    xbuff[i+1]=port_inbyte(1);
                }
                
                if((xbuff[1]==(uint8_t)~xbuff[2])&&(packetno==xbuff[1])//包序号无误
                   &&(check(crc, &xbuff[3], bufsz))){//CRC校验无误
                       packetno++;
                       for(j=0;(j<128)&&(RowNum<16);){
                           if(xbuff[3+j]==':'){
                               p_DataType=p_LastData;
                           }
                           else if(xbuff[3+j]==0x0A){//接收到换行符
                               if(*(p_DataType+8)=='0'){
                                   RowNum++; //(((8+k)>=128)?((k+8)-128):(8+k))
                               }
                           }
                           *(p_LastData++)=xbuff[3+j];
                           j++;
                           
                       }
                       if(RowNum==16){//可以进行数据烧写了
                           RowNum=0;
                           *p_LastData=0x0A;
                           //ProgramHexToFlash(g_HexDataTemp,USE_DFT_ADDR,PROM_ADDR);
                           p_LastData=g_HexDataTemp;
                           while(j<128){
                               if(xbuff[3+j]==':'){
                                   p_DataType=p_LastData;
                               }
                               if(xbuff[3+j]==0x0A){//接收到换行符
                                   if(*(p_DataType+8)=='0'){
                                       RowNum++;
                                   }		
                               }
                               *(p_LastData++)=xbuff[3+j];//将不能构成一行的数据暂存
                               j++;
                           }
                       }
                       port_outbyte(ACK);
                   }
                else{//要求重发
                    port_outbyte(NAK);
                }
                do{
                    chTemp = port_inbyte(10);//读取下一帧数据的帧头
                }while(chTemp==0);
            }
            port_outbyte(chTemp);
            if(chTemp==EOT){//文件发送结束标志       
                port_outbyte(ACK);
                /*烧写最后一帧数据*/
                if(RowNum<16){
                    *p_LastData=0x0A;
                    //ProgramHexToFlash(g_HexDataTemp,USE_DFT_ADDR,PROM_ADDR);
                }
                printf("\n\r->Load Hex File To Flash Of CRC OK ^_^");
                printf("\n\r->");
            }
            
            chTemp=port_inbyte(50);
            if(chTemp==EOT){//文件发送结束标志
                port_outbyte(ACK);
                printf("\n\r->Load Hex File To Flash Of CRC OK ^_^");
                printf("\n\r->");
            }
            /*此处可以返回错误信息*/
            return SUCCESSFULL;
        }
    }
}

