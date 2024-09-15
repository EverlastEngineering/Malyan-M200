#include "temperature.h"
#include "Marlin.h"
#include "cardreader.h"
#include "port.h"

#if ENABLED(MESH_BED_LEVELING)
  #include "mesh_bed_leveling.h"
#endif

#include <ctype.h>
#include <stdlib.h>

unsigned char wifi_buff_obj[8];
unsigned char wifi_buff_value[32];
unsigned char wifi_buff_ptr,wifi_buff_state;

char wifi_ssid[32];
char wifi_password[32];
uint8_t wifi_ip[4];
uint8_t wifi_mask[4];
uint8_t wifi_gw[4];
char wifi_mac[20];
char wifi_status;
bool wifi_download=false;

extern "C" void DisableUSB();
extern "C" void store_char(unsigned char c);

inline void wifi_put(char c)
{
   while(USART_GetFlagStatus(WIFI_UART, USART_FLAG_TXE) == RESET){}
   USART_SendData(WIFI_UART,c+0x80);
}

void wifi_writeString(char message[]) {
    char* letter = message;
    while (*letter != 0) {
        wifi_put(*letter);
        letter++;
    }
}

void wifi_writeString_safe(char message[]) {
    uint8_t len;
    char* letter = message;
    len=0;
    while (*letter != 0) {
        char c=*letter;
        if (c<0x80) wifi_put(c);
        else wifi_put('?');
        letter++;
        if (len++>24) return;
    }
}

void wifi_writeInt(uint16_t value, uint8_t digits) {
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
        wifi_put((uvalue % currentDigit) / nextDigit + '0');
        currentDigit = nextDigit;
    }
}

void wifi_writeInt32(uint32_t value, uint8_t digits) {
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
        wifi_put((uvalue % currentDigit) / nextDigit + '0');
        currentDigit = nextDigit;
    }
}

volatile unsigned char download_state;
#define WIFI_USART_RX_BUF_SIZE 256
extern char Uart_RX_WIFI[WIFI_USART_RX_BUF_SIZE];
extern volatile int16_t wifi_w_buff_ptr;
volatile int16_t wifi_r_buff_ptr=0;

/*unsigned char wifi_buff_cmd[20];
unsigned char wifi_buff_ptr;*/
static uint8_t last_endstop_stat=0;

extern void WIFI_enable(unsigned char e);
extern const char VERSION[];

void wifi_init() {

    extern void serial_init(int);
    extern void delayMicroseconds(unsigned int us);

    //serial_init(WIFI_UART,512000);
    WIFI_enable(1);

    wifi_buff_ptr=0;
    wifi_buff_state=0;

    /*wifi_writeString((char *)"{SYS:STARTED}");
    wifi_writeString((char *)"{VER:");
    wifi_put(VERSION[0]);
    wifi_put(VERSION[2]);
    wifi_writeString((char *)"}");*/
    download_state=0;
    wifi_ssid[0]=0;
    wifi_password[0]=0;
    wifi_ip[0]=0;wifi_ip[1]=0;wifi_ip[2]=0;wifi_ip[3]=0;
    wifi_mask[0]=0;wifi_mask[1]=0;wifi_mask[2]=0;wifi_mask[3]=0;
    wifi_gw[0]=0;wifi_gw[1]=0;wifi_gw[2]=0;wifi_gw[3]=0;
    wifi_mac[0]=0;
    wifi_status=0;
    wifi_download=false;

    strcpy(wifi_ssid,"DEFAULTSSID");
    strcpy(wifi_password,"DEFAULTPASS");
}

/*uint8_t countFiles(bool update)
{
	uint16_t nr;
	nr = card.getnrfilenames();
	if (nr>200) nr=200;
	return nr;
}*/

void StartBuild()
{
    wifi_writeString((char *)"{SYS:BUILD}");
    if (model==1) GPIO_SetBits(GPIOB,GPIO_Pin_3);
}

void EndBuild()
{
    //void blink(uint8_t t);
    //wifi_writeString((char *)"{SYS:ENDOFBUILD}");
    wifi_writeString((char *)"{TQ:100}");
    //blink(3);
    if (model==1) GPIO_ResetBits(GPIOB,GPIO_Pin_3);
}

/*
void wifi_PrintingStatus()
{
    int16_t t;
    int32_t t32;

    wifi_writeString((char *)"{T0:");
    t=degHotend(0);
    if (t>999) t=999;
    wifi_writeInt(t,3);
    wifi_put('/');
    t=degTargetHotend(0);
    wifi_writeInt(t,3);
    wifi_put('}');

    wifi_writeString((char *)"{TP:");
    t=degBed();
    if (t>999) t=999;
    wifi_writeInt(t,3);
    wifi_put('/');
    t=degTargetBed();
    wifi_writeInt(t,3);
    wifi_put('}');

    wifi_writeString((char *)"{TQ:");
    t=card.percentDone();
    wifi_writeInt(t,3);

	if (card.sdprinting) wifi_put('P');
	else wifi_put('C');
    wifi_put('}');
}*/

//extern "C" void USB_Cable_Config (FunctionalState NewState);
typedef enum _RESULT
{
  USB_SUCCESS = 0,    /* Process successfully */
  USB_ERROR,
  USB_UNSUPPORT,
  USB_NOT_READY       /* The process has not been finished, endpoint will be
                         NAK to further request */
} RESULT;
extern "C" RESULT PowerOn(void);
extern "C" RESULT PowerOff(void);

void cancel_print()
{
    extern bool cancel_heatup;
    extern bool relative_mode;
    bool usb_state;

    //DisableUSB();
    //cli();
    IWDG_ReloadCounter();

    usb_state=(bDeviceState == CONFIGURED);
    if (usb_state)
    {
        //if (model!=4) GPIO_ResetBits(GPIOB,GPIO_Pin_3);
        //else GPIO_ResetBits(GPIOA, GPIO_Pin_13);
        PowerOff();
    }
    wifi_writeString((char *)"{SYS:CANCELING}");

    quickStop();

    extern bool sdprint_cancel;
    sdprint_cancel=true;
    
    extern unsigned int cleaning_buffer_counter;
    while (cleaning_buffer_counter>0) IWDG_ReloadCounter();
    enable_z();
    relative_mode=true;
    if (axis_known_position[2]!=0)
    {
        extern float current_position[NUM_AXIS],destination[NUM_AXIS];
        extern float feedrate;

        if (model==6)
        {
            /*if (destination[0]<100) destination[0] = current_position[0] + 10;
            if (destination[1]<100) destination[1] = current_position[1] + 10;
            if (destination[2]<100) destination[2] = current_position[2] + 10;
            feedrate = 4000;
            
            prepare_move();
            while (block_buffer_tail!=block_buffer_head) idle();*/
            
            //extern void home();
            //home();
            //quickStop();
            //return;
        }
        else if (model==2)
        {
            //if (destination[1]<80) destination[1]=80;
            if (destination[2]<80) destination[2] = current_position[2] + 10;
            feedrate = 3000;
            
            prepare_move();
            while (block_buffer_tail!=block_buffer_head) idle();
        }
        else
        {
            if (destination[1]<100) destination[1]=100;
            if (destination[2]<100) destination[2] = current_position[2] + 10;
            feedrate = 4000;
            
            prepare_move();
            while (block_buffer_tail!=block_buffer_head) idle();
        }
    }
    
    quickStop();

    wifi_writeString((char *)"{SYS:STARTED}");

    /*
    relative_mode=false;
    disable_all_steppers();
    card.closefile(false);*/
    
    /*delay(1000);
    packet_sent=1;
    if (model!=4) GPIO_SetBits(GPIOB,GPIO_Pin_3);
    else GPIO_SetBits(GPIOA, GPIO_Pin_13);*/
    if (usb_state)
    {
        delay(1000);
        PowerOn();
        //packet_sent=1;
    }
    relative_mode=false;
}
uint32_t bDeviceState_last=2;
void PrintingStatus()
{
    extern bool buffered_printing();

    int16_t t;
    int32_t t32;

    wifi_writeString((char *)"{T0:");
    t=degHotend(0);
    if (t>999) t=999;
    wifi_writeInt(t,3);
    wifi_put('/');
    t=degTargetHotend(0);
    wifi_writeInt(t,3);
    wifi_put('}');

    wifi_writeString((char *)"{T1:");
    t=degHotend(1);
    if (t>999) t=999;
    wifi_writeInt(t,3);
    wifi_put('/');
    t=degTargetHotend(1);
    wifi_writeInt(t,3);
    wifi_put('}');

    wifi_writeString((char *)"{TP:");
    t=degBed();
    if (t>999) t=999;
    wifi_writeInt(t,3);
    wifi_put('/');
    t=degTargetBed();
    wifi_writeInt(t,3);
    wifi_put('}');

    wifi_writeString((char *)"{TQ:");

    if (sdprinting)
    {
        t=card.percentDone();
        if (t>99) t=99;
    }
    else {
        extern bool homing;
        t=0;
        if (axis_known_position[X_AXIS] == false &&
            axis_known_position[Y_AXIS] == false &&
            axis_known_position[Z_AXIS] == false &&
            degTargetHotend(0)==0 && degTargetBed()==0 &&
            homing==false)
          t=100;
    }
    wifi_writeInt(t,3);

    if (sdprinting || buffered_printing()) wifi_put('P');
    else wifi_put('C');
    wifi_put('}');

    wifi_writeString((char *)"{TT:");
    if(print_job_start_ms != 0) t32 = millis()/60000 - print_job_start_ms/60000;
	else t32=0;
    wifi_writeInt32(t32,6);
    wifi_put('}');

    /*
    wifi_writeString((char *)"{TR:");
    //t32=command::estimatedTimeLeftInSeconds();
	t32=0;
    wifi_writeInt32(t32,6);
    wifi_put('}');

    wifi_writeString((char *)"{TF:");
    //t32=command::filamentUsed();
	t32=0;
    wifi_writeInt32(t32,6);
    wifi_put('}');*/
}

void wifi_process()
{
    switch (wifi_buff_obj[0]) {
        case 'M':
            uint16_t t;
            float f;
            t=atoi((const char*)wifi_buff_value);
            if (t>=999)
            {
                f=(-zprobe_zoffset*100);
                t=f;
                wifi_writeString((char *)"{M:");
                wifi_writeInt(t,3);
                wifi_put('}');
            }
            else
            {
                f=t;
                f=-f*0.01;
#if ENABLED(MESH_BED_LEVELING)
                if (Z_PROBE_OFFSET_RANGE_MIN <= f && f <= Z_PROBE_OFFSET_RANGE_MAX) {
                  zprobe_zoffset = f;
                  float diff = zprobe_zoffset - mbl.z_values[0][0];
                  mbl.z_values[0][0] += diff;
                  mbl.z_values[0][1] += diff;
                  mbl.z_values[1][0] += diff;
                  mbl.z_values[1][1] += diff;
                  extern void Config_StoreSettings();
                  Config_StoreSettings();
                }
#endif
            }
            break;
        case 'C':
            if (wifi_buff_value[0]=='P')
            {
                uint16_t t;

                t=atoi((const char*)wifi_buff_value+1);

                if (t<0 || t>150) return;
                setTargetBed(t);
            }
            else if (wifi_buff_value[0]=='T')
            {
                int16_t t;

                t=atoi((const char*)wifi_buff_value+2);
                if (t<0 || t>280) return;

                if (wifi_buff_value[1] == '0')
                {
                    setTargetHotend(t,0);
                }
                else
                {
                    setTargetHotend(t,1);
                }
            }
            else if (wifi_buff_value[0]=='S')
            {
                int16_t t;
                uint8_t i;

                t=atoi((const char*)wifi_buff_value+1);

                if (t<1) t=1;
                else if (t>50) t=50;

                feedrate_multiplier=t*10;
            }
            break;

        case 'B':
            PrintingStatus();
            break;

        case 'W':
            wifi_writeString((char *)"{WS:");
            wifi_writeString(wifi_ssid);
            wifi_writeString((char *)"}{WP:");
            wifi_writeString(wifi_password);
            wifi_put('}');
            break;

        case 'e':
            wifi_writeString((char *)"{e:e}");
            break;

        case 'R':
            if (wifi_buff_obj[1]=='I')
            {
                char i,b;
                TIM_SetCompare2(TIM2,200);
                b=0;
                wifi_ip[0]=0;wifi_ip[1]=0;wifi_ip[2]=0;wifi_ip[3]=0;
                for (i=0;i<16;i++)
                {
                    if (wifi_buff_value[i]==0) break;
                    else if (wifi_buff_value[i]=='.')
                    {
                        b++;
                        if (b>3) break;
                    }
                    else wifi_ip[b]=wifi_ip[b]*10 + (wifi_buff_value[i]-'0');
                }
            }
            else if (wifi_buff_obj[1]=='C')
            {
                if (wifi_buff_value[0]=='C') wifi_status=1;
                else wifi_status=0;
            }
            else if (wifi_buff_obj[1]=='M')
            {
                //printf("%s\n",wifi_buff_value);
                strcpy(wifi_mac, (char const *)wifi_buff_value);
            }
            break;

//LCD
        case 'V':
            extern bool unlocked;
            wifi_writeString((char *)"{VER:");
            //wifi_writeString((char *)VERSION);
            if (unlocked==false)
            {
                wifi_put('0');
                wifi_put('0');
            }
            else
            {
                wifi_put(VERSION[0]);
                wifi_put(VERSION[2]);
            }
            wifi_writeString((char *)"}");
            return;

        case 'S':
            if (wifi_buff_value[0]=='S')
            {
                last_endstop_stat=0;
            }
            if (wifi_buff_value[0]=='3')
            {
                enqueuecommands_P(PSTR("M563 S3"));
            }
            else if (wifi_buff_value[0]=='4')
            {
                store_char('\r');
                enqueuecommands_P(PSTR("M563 S4"));
            }
            else if (wifi_buff_value[0]=='C')
            {
                MYSERIAL.flush();
                store_char('\r');
            }
            else if (wifi_buff_value[0]=='U')
            {
                if (bDeviceState == CONFIGURED) wifi_writeString((char *)"{R:UC}");
                else wifi_writeString((char *)"{R:UD}");
                bDeviceState_last=bDeviceState;
            }
            else if (wifi_buff_value[0]=='E') wifi_writeString((char *)"{SYS:echo}");
            else if (wifi_buff_value[0]=='L')
            {
                uint8_t i;
                uint8_t itemCount;

                delay(200);
                itemCount = card.LcdListFile();
                if (itemCount==0) itemCount = card.LcdListFile();
                else delay(200);
                wifi_writeString((char *)"{SYS:OK}");
            }
            else if (wifi_buff_value[0]=='I')
            {
                int16_t t;

				wifi_writeString((char *)"{T0:");
				t=degHotend(0);
				if (t>999) t=999;
				wifi_writeInt(t,3);
				wifi_put('/');
				t=degTargetHotend(0);
				wifi_writeInt(t,3);
				wifi_put('}');

				wifi_writeString((char *)"{T1:");
				t=degHotend(1);
				if (t>999) t=999;
				wifi_writeInt(t,3);
				wifi_put('/');
				t=degTargetHotend(1);
				wifi_writeInt(t,3);
				wifi_put('}');

				wifi_writeString((char *)"{TP:");
				t=degBed();
				if (t>999) t=999;
				wifi_writeInt(t,3);
				wifi_put('/');
				t=degTargetBed();
				wifi_writeInt(t,3);
				wifi_put('}');
            }
            else if (wifi_buff_value[0]=='F')
            {
                /*if (buff_value[1]=='X')
                {
                    eeprom::setEepromInt64(eeprom_offsets::FILAMENT_TRIP, eeprom::getEepromInt64(eeprom_offsets::FILAMENT_LIFETIME, 0));
                    eeprom::setEepromInt64(eeprom_offsets::FILAMENT_TRIP + sizeof(int64_t), eeprom::getEepromInt64(eeprom_offsets::FILAMENT_LIFETIME + sizeof(int64_t), 0));
                }*/

                wifi_writeString((char *)"{TU:");

                uint16_t total_hours;
                uint8_t total_minutes;
                //eeprom::getBuildTime(&total_hours, &total_minutes);
				total_hours=0;
				total_minutes=0;
                wifi_writeInt(total_hours,5);
                wifi_put('.');
                wifi_writeInt(total_minutes,2);
                wifi_put('/');

                uint8_t build_hours;
                uint8_t build_minutes;
                //host::getPrintTime(build_hours, build_minutes);
				build_hours=0;
				build_minutes=0;
                wifi_writeInt(build_hours,3);
                wifi_put('.');
                wifi_writeInt(build_minutes,2);
                wifi_put('/');

                uint32_t filamentUsedA,filamentUsedB,filamentUsed;
                char str[11];
				filamentUsed=0;
                wifi_writeString((char *)"0");
                wifi_put('/');
				filamentUsed=0;
                wifi_writeString((char *)"0");
                wifi_put('}');
            }
            else if (wifi_buff_value[0]=='R' &&
                wifi_buff_value[1]=='E' &&
                wifi_buff_value[2]=='S' &&
                wifi_buff_value[3]=='E' &&
                wifi_buff_value[4]=='T')
            {
                //Motherboard::getBoard().reset(true);
            }
            break;

        case 'P':
            uint8_t i;
            extern uint8_t m600_resume;
            if (wifi_buff_value[0]=='H')
            {
                
            	enqueuecommands_P(PSTR("G28"));
                enqueuecommands_P(PSTR("M18"));
                enqueuecommands_P(PSTR("M400"));
            }
            else if (wifi_buff_value[0]=='I')
            {
            	//enqueuecommands_P(PSTR("G28"));
                enqueuecommands_P(PSTR("G29 S1"));
                enqueuecommands_P(PSTR("M500"));
                enqueuecommands_P(PSTR("M400"));
            }
            else if (wifi_buff_value[0]=='C')
            {
                //host::startOnboardBuild(utility::TOOLHEAD_CALIBRATE);
            }
            else if (wifi_buff_value[0]=='M')
            {
                enqueuecommands_P(PSTR("M565"));
            }
            else if (wifi_buff_value[0]=='X')
            {
                m600_resume=1;
                cancel_print();
                return;
                {
                  SCB->AIRCR = 0x05FA0004;
                  for(;;);
                }
                wifi_writeString((char *)"{SYS:STARTED}");
                //sei();
            }
            else if (wifi_buff_value[0]=='P')
            {
                wifi_writeString((char *)"{SYS:PAUSE}");
                //card.pauseSDPrint();
                enqueuecommands_P(PSTR("M600"));
            }
            else if (wifi_buff_value[0]=='R')
            {
                wifi_writeString((char *)"{SYS:RESUME}");
                m600_resume=1;
                wifi_writeString((char *)"{SYS:RESUMED}");
            }
            else if (wifi_buff_value[0]=='Z')
            {
                /*i=(buff_value[1]-'0')*100 + (buff_value[2]-'0')*10 + (buff_value[3]-'0');
                float pauseAtZPos = i;
                command::pauseAtZPos(stepperAxisMMToSteps(pauseAtZPos, Z_AXIS));*/
            }
            else
            {
                i=(wifi_buff_value[0]-'0')*100 + (wifi_buff_value[1]-'0')*10 + (wifi_buff_value[2]-'0');
                card.getfilename(i);
                if (card.filenameIsDir)
                {
                    wifi_writeString((char *)"{SYS:DIR}");
                    card.chdir(card.fileinfo.fname);
                }
                else
                {
                    char cmd[30];
                    char* c;

                    wifi_writeString((char *)"{PRINTFILE:");
                    /*if (card.longFilename[0]!=0) wifi_writeString(card.longFilename);
                    else */wifi_writeString(card.fileinfo.fname);
                    wifi_put('}');

                    if (0==strcmp(card.fileinfo.fname,"firmware.bin"))
                    {
                        wifi_writeString((char *)"{E:UPDATING.}");
                        enqueuecommands_P(PSTR("M560"));
                        return;
                    }
                    StartBuild();
                    MYSERIAL.flush();
                    store_char('\r');

                    int16_t path_len = strlen(abspath);
                    int16_t fn_len = strlen(card.fileinfo.fname);

                    if (path_len+fn_len<_MAX_LFN && abspath[0]!=0)
                    {
                        for (int16_t i=fn_len;i>=0;i--)
                            card.fileinfo.fname[i+path_len+1]=card.fileinfo.fname[i];

                        for (i=0;i<path_len;i++)
                            card.fileinfo.fname[i]=abspath[i];

                        card.fileinfo.fname[path_len]='/';
                    }

                    card.openFile(card.fileinfo.fname, true);
                    card.startFileprint();
                    print_job_start_ms = millis();
                    /*
                    enqueuecommands_P(PSTR("M21"));
                    sprintf(cmd, PSTR("M23 %s"), card.fileinfo.fname);
                    for(c = &cmd[4]; *c; c++)
                    *c = tolower(*c);
                    enqueuecommand(cmd);
                    enqueuecommands_P(PSTR("M24"));*/
                }
            }
            break;

        case 'J':
            switch (wifi_buff_value[0])
            {
                case 'E':
                    extern bool relative_mode;
                    extern unsigned int cleaning_buffer_counter;
                    //enqueuecommands_P(PSTR("M18"));
                    quickStop();
                    while (cleaning_buffer_counter>0) IWDG_ReloadCounter();
                    relative_mode=false;
                    disable_all_steppers();

                    break;

                case 'X':
                case 'Y':
                case 'Z':
                    extern void update_endstops_ext();
                    update_endstops_ext();

                    if (wifi_buff_value[0]=='X' && wifi_buff_value[1]=='-')
                    {
                        if (READ(X_STOP_PIN) != X_MIN_ENDSTOP_INVERTING) return;
                    }
                    else if (wifi_buff_value[0]=='Y' && wifi_buff_value[1]=='-')
                    {
                        if (READ(Y_STOP_PIN) != Y_MIN_ENDSTOP_INVERTING) return;
                    }
                    else if (wifi_buff_value[0]=='Z' && wifi_buff_value[1]=='-')
                    {
                        if (READ(Z_MIN_PIN) != Z_MIN_ENDSTOP_INVERTING) return;
                    }

                    enable_all_steppers();
                    //enqueuecommands_P(PSTR("G91"));
                    //enqueuecommands_P(PSTR("M120"));
                    extern bool relative_mode;
                    relative_mode=true;

                    extern float current_position[NUM_AXIS],destination[NUM_AXIS];
                    extern float feedrate;

                    for (int i = 0; i < NUM_AXIS; i++) {
                      if (wifi_buff_value[0]=='X'+i)
                      {
                        if (wifi_buff_value[1]=='+') destination[i] = current_position[i] + 120;
                        else destination[i] = current_position[i] -120;
                      }
                      else
                        destination[i] = current_position[i];
                    }
                    feedrate = 1000;
                    prepare_move();
                  break;
                case 'A':
                case 'B':
                    char cmd[16];
                    extern bool relative_mode;
                    relative_mode=true;

                    if (wifi_buff_value[0]=='A') enqueuecommand("T0");
                    else enqueuecommand("T1");

                    strcpy(cmd,"G1 F1000 ");
                    cmd[9]=wifi_buff_value[0];   //X
                    cmd[10]=wifi_buff_value[1];   //+
                    if (cmd[9]=='A')
                    {
                        cmd[9]='E';
                    }
                    else if (cmd[9]=='B')
                    {
                        cmd[9]='E';
                    }
                    cmd[11]=wifi_buff_value[2];   //1
                    cmd[12]=wifi_buff_value[3];  //0
                    cmd[13]=wifi_buff_value[4];  //0
                    cmd[14]=0;
                    enqueuecommand(cmd);

                    break;
            }
            break;
//

        case 'U':
            int total_wr,total_rcv;
            int length;
            FIL fp;
            FRESULT result;

            //Disable heaters
            disable_all_heaters();

            result=f_open(&fp,"cache.gc",FA_WRITE | FA_CREATE_ALWAYS);
            if (result!=FR_OK)
            {
                card.initsd();
                //card.ls();
                result=f_open(&fp,"cache.gc",FA_WRITE | FA_CREATE_ALWAYS);
                //printf("Open: %x\n",result);
            }

            if (result==FR_OK)
            {
                char i;
                char c;

                volatile unsigned int wifi_timeout=0;
                #define WIFI_TIMEOUT    100000
                #define USART_RX_BUF_SIZE 256
                extern uint8_t Uart_Rx[];
                extern uint16_t buff_ptr;

                char rx_state=0;
                char chksum=0;
                //char rx_buff[256];
                //extern uint8_t Uart_RX_LCD[];
                uint16_t rx_len=0;
                uint32_t wr_len=0;

                //printf("Download started\n");
                wifi_download=true;
                wifi_writeString((char *)"{U:OK}");
                wifi_r_buff_ptr=0;
                wifi_w_buff_ptr=0;

                //file_len=0;
                uint8_t f_ptr=0;
                while (1)
                {
                    IWDG_ReloadCounter();
                    /*if (wifi_timeout++>WIFI_TIMEOUT)
                    {
                      rx_state=0;
                      chksum=0;
                      rx_len=0;
                      wifi_put('T');
                      wifi_timeout=0;
                    }*/

                    while (wifi_r_buff_ptr!=wifi_w_buff_ptr)
                    {
                        c=Uart_RX_WIFI[wifi_r_buff_ptr++];
                        chksum^=c;
                        if (wifi_r_buff_ptr>=WIFI_USART_RX_BUF_SIZE) wifi_r_buff_ptr=0;

                        if (rx_state==1)
                        {
                            rx_state=0;
                            if (chksum==0)
                            {
                              IWDG_ReloadCounter();
                              wifi_put('A');
                              //f_write(&fp,Uart_RX_LCD,rx_len,&wr_len);
                              //file_len+=rx_len;

                              if ((f_ptr+rx_len)>=WIFI_USART_RX_BUF_SIZE)
                              {
                                f_write(&fp,Uart_RX_WIFI+f_ptr,WIFI_USART_RX_BUF_SIZE-f_ptr,&wr_len);
                                f_write(&fp,Uart_RX_WIFI,f_ptr+rx_len-WIFI_USART_RX_BUF_SIZE,&wr_len);
                              }
                              else
                              {
                                f_write(&fp,Uart_RX_WIFI+f_ptr,rx_len,&wr_len);
                              }

                            }
                            else wifi_put('E');
                            rx_len=0;
                            chksum=0;
                            f_ptr=wifi_r_buff_ptr;
                        }
                        else if (c==0xA5) rx_state=1;
                        else if (c==0xAA && rx_len<=2)
                        {
                            extern bool start_after_download;
                            wifi_r_buff_ptr=0;
                            wifi_w_buff_ptr=0;
                            f_close(&fp);
                            wifi_download=false;
                            wifi_writeString((char *)"{SYS:STARTED}");
                            if (start_after_download)
                            {
                                StartBuild();
                                MYSERIAL.flush();
                                store_char('\r');
                                enqueuecommands_P(PSTR("M23 cache.gc"));
                                enqueuecommands_P(PSTR("M24"));
                                start_after_download=false;
                            }
                            return;
                        }
                        else
                        {
                          rx_len++;
                          //Uart_RX_LCD[rx_len++]=c;
                        }
                        wifi_timeout=0;
                    }
                }

            }
            else
            {
                wifi_writeString((char *)"{U:ERR}");
            }

            break;
        default:
            break;
    }
}

uint8_t started=0;
void wifi_update() {
    uint8_t c;
    extern uint8_t bDeviceState;
    extern char btn_status;
    extern uint8_t m600_resume;

    if (model==2 || model==6)
    {
        if (btn_status & 2)
        {
            //if (bDeviceState != CONFIGURED)
            {
                if (!sdprinting)
                {
                    if (started==1)
                    {
                        sdprinting = true;
                        m600_resume=1;

                        //TIM_SetCompare4(TIM2,200);
                        //GPIO_ResetBits(GPIOA,GPIO_Pin_14);
                        LED_WORKING_ON;
                    }
                    else
                    {
                        bool ret = card.findautoprint();
                        if (ret==false) ret = card.findautoprint();
                        if (ret)
                        {
                            ret=card.openFile(card.fileinfo.fname, true);
                            if (ret==false)
                            {
                                ret = card.findautoprint();
                                if (ret) ret=card.openFile(card.fileinfo.fname, true);
                            }
                        }

                        if (ret)
                        {
                            /*if (card.openFile(card.fileinfo.fname, true)==false)
                            {
                                ret = card.findautoprint();
                                if (ret==false) ret = card.findautoprint();
                            }*/

                            store_char('\r');
                            card.startFileprint();
                            started=1;

                            //TIM_SetCompare4(TIM2,200);
                            //GPIO_ResetBits(GPIOA,GPIO_Pin_14);
                            
                            LED_WORKING_ON;
                            delay(500);
                            LED_WORKING_OFF;
                            delay(500);

                            LED_WORKING_ON;
                            delay(500);
                            LED_WORKING_OFF;
                            delay(500);
                            LED_WORKING_ON;
                        }
                        else
                        {
                            //blink(5);
                        }
                    }
                }
                else
                {
                    sdprinting = false;
                    enqueuecommands_P(PSTR("M600"));
                    //TIM_SetCompare4(TIM2,0);
                    //GPIO_SetBits(GPIOA,GPIO_Pin_14);
                    LED_WORKING_OFF;
                }
            }
            delay(100);
            btn_status=0;

            /*if (sdprinting) TIM_SetCompare4(TIM2,200);
            else TIM_SetCompare4(TIM2,0);*/
        }

        if (btn_status & 4)
        {
            if (sdprinting)
            {
                m600_resume=1;
                //TIM_SetCompare4(TIM2,0);
                //GPIO_SetBits(GPIOA,GPIO_Pin_14);
                LED_WORKING_OFF;
                cancel_print();
                started=0;
                delay(1000);
            }
            else
            {
                wifi_writeString((char *)"{WC:0}");
                /*TIM_SetCompare4(TIM2,200);
                GPIO_ResetBits(GPIOA,GPIO_Pin_14);
                delay(500);
                TIM_SetCompare4(TIM2,0);
                GPIO_SetBits(GPIOA,GPIO_Pin_14);
                delay(500);

                TIM_SetCompare4(TIM2,200);
                GPIO_ResetBits(GPIOA,GPIO_Pin_14);
                delay(500);
                TIM_SetCompare4(TIM2,0);
                GPIO_SetBits(GPIOA,GPIO_Pin_14);
                delay(500);

                TIM_SetCompare4(TIM2,200);
                GPIO_ResetBits(GPIOA,GPIO_Pin_14);
                delay(500);
                TIM_SetCompare4(TIM2,0);
                GPIO_SetBits(GPIOA,GPIO_Pin_14);*/
                LED_LINK_ON;
                delay(500);
                LED_LINK_OFF;
                delay(500);

                LED_LINK_ON;
                delay(500);
                LED_LINK_OFF;
                delay(500);

                LED_LINK_ON;
                delay(500);
                LED_LINK_OFF;
                //delay(500);
            }
            btn_status=0;
        }
    }
    /*if (btn_status & 2) TIM_SetCompare4(TIM2,0);
    else TIM_SetCompare4(TIM2,200);*/
/*
    if (bDeviceState_last!=bDeviceState)
    {
      if (bDeviceState == CONFIGURED)
      {
          wifi_writeString((char *)"{R:UC}");
      }
      else
      {
          wifi_writeString((char *)"{R:UD}");
          com_opened=0;
      }
      bDeviceState_last=bDeviceState;
    }
*/
    if (bDeviceState_last!=com_opened)
    {
        if (com_opened == 1)
        {
            wifi_writeString((char *)"{R:UC}");
        }
        else
        {
            wifi_writeString((char *)"{R:UD}");
        }
        bDeviceState_last=com_opened;
    }
    
    uint8_t endstop_stat=0;
    if (READ(X_STOP_PIN) == 0) endstop_stat|=1;
    if (READ(Y_STOP_PIN) == 0) endstop_stat|=2;
    if (READ(Z_MIN_PIN) == 0) endstop_stat|=4;
    
    if (endstop_stat != last_endstop_stat)
    {
        wifi_writeString((char *)"{TS:");
        if (endstop_stat&1) wifi_put('X');
        else wifi_put('x');
        if (endstop_stat&2) wifi_put('Y');
        else wifi_put('y');
        if (Z_MIN_ENDSTOP_INVERTING)
        {
            if (endstop_stat&4) wifi_put('Z');
            else wifi_put('z');
        }
        else
        {
            if (endstop_stat&4) wifi_put('z');
            else wifi_put('Z');
        }
        last_endstop_stat=endstop_stat;
        wifi_put('}');
    }
    
    while (wifi_r_buff_ptr!=wifi_w_buff_ptr)
    {
        c=Uart_RX_WIFI[wifi_r_buff_ptr++];
        if (wifi_r_buff_ptr>=WIFI_USART_RX_BUF_SIZE) wifi_r_buff_ptr=0;
        //putchar(c);
        switch (c)
		{
			case '{': if (wifi_buff_state==0)
                      {
                          wifi_buff_state=1;
                          wifi_buff_ptr=0;
                      }
                      else wifi_buff_state=0;
                      break;

			case ':': if (wifi_buff_state==1)
                      {
                          wifi_buff_obj[wifi_buff_ptr++]=0;
                          wifi_buff_state=2;
                          wifi_buff_ptr=0;
                      }
                      else wifi_buff_state=0;
                      break;

			case ',': if (wifi_buff_state==2)
                      {
                          wifi_buff_value[wifi_buff_ptr++]=0;
                          wifi_process();
                          wifi_buff_state=1;
                          wifi_buff_ptr=0;
                          wifi_buff_obj[0]=0;
                      }
                      else if (wifi_buff_state==1)
                      {
                          //wifi_put('?');
                          wifi_buff_state=0;
                          wifi_buff_ptr=0;
                      }
                      else wifi_buff_state=0;
                      break;

            case '}': if (wifi_buff_state==2)
                      {
                          wifi_buff_value[wifi_buff_ptr++]=0;
                          wifi_process();
                          wifi_buff_state=0;
                          wifi_buff_ptr=0;
                          wifi_buff_obj[0]=0;
                      }
                      else if (wifi_buff_state==1)
                      {
                          //wifi_put('?');
                          wifi_buff_state=0;
                          wifi_buff_ptr=0;
                      }
                      else wifi_buff_state=0;
                      break;

            case '\r':
            case '\n':wifi_buff_state=0;
                      wifi_buff_ptr=0;
                      break;


            default:  if (c>=0x21 && c<=0x7E)
                      {
                          if (wifi_buff_state==1)
                          {
                              if (wifi_buff_ptr>6)
                              {
                                  wifi_buff_state=0;
                                  wifi_buff_ptr=0;
                              }
                              else wifi_buff_obj[wifi_buff_ptr++]=c;
                          }
                          else if (wifi_buff_state==2)
                          {
                              if (wifi_buff_ptr>30)
                              {
                                  wifi_buff_state=0;
                                  wifi_buff_ptr=0;
                              }
                              else wifi_buff_value[wifi_buff_ptr++]=c;
                          }
                      }
                      else
                      {
                          //wifi_put('?');
                          wifi_buff_state=0;
                          wifi_buff_ptr=0;
                      }
		}
    }

}
