#include "Marlin.h"
#include "cardreader.h"
#include "stepper.h"
#include "temperature.h"
#include "language.h"

#if ENABLED(SDSUPPORT)

char abspath[64];
extern "C" void MSD0_SPI_Configuration();
extern "C" void MSD0_Init();
bool sdprinting;

CardReader::CardReader() {

    sdprinting = false;
    cardOK = false;
    saving = false;
    logging = false;

    for (uint16_t i=0;i<64;i++)
        abspath[i]=0;

    for (uint16_t i=0;i<=_MAX_LFN;i++)
        fileinfo.fname[i]=0;

    //initsd();
    //LcdListFile();
}

void CardReader::ls() {
  ls(false);
}

void CardReader::ls(bool withLongFileNames)  {
  uint16_t i;
  uint8_t retry_cnt=0;
  FRESULT ret;

/*
#if _USE_LFN
	fileinfo.lfname = lfn;
	fileinfo.lfsize = sizeof lfn;
#endif*/

  //initsd();
retry:
  if (retry_cnt++>2) return;
  if (abspath[0]==0) ret=f_opendir(&dp,"\\");
  else ret=f_opendir(&dp,abspath);
  if (ret!=FR_OK)
  {
      abspath[0]=0;
      ret=f_opendir(&dp,"\\");
      if (ret!=FR_OK)
      {
        //SERIAL_PROTOCOLLN("Open dir error.");
        return;
      }
  }

  
  if (abspath[0]!=0 && abspath[1]!=0) SERIAL_PROTOCOLLN("..");
  else SERIAL_PROTOCOLLN(".");

  for (i=0;i<100;i++)
  {
      ret=f_readdir(&dp,&fileinfo);
      if (fileinfo.fname[0]==0 || ret!=FR_OK)
      {
        if (i==0) goto retry;
        return;
      }
      if (fileinfo.fattrib & AM_SYS) continue;
      else if (fileinfo.fattrib & AM_HID) continue;
      /*if (strstr(fileinfo.fname, ".G")!=NULL)
        SERIAL_PROTOCOLLN(fileinfo.fname);
      else if (strstr(fileinfo.fname, ".g")!=NULL)*/
        SERIAL_PROTOCOL(fileinfo.altname);
        if (fileinfo.fattrib & AM_DIR) SERIAL_PROTOCOL("/");
        if (withLongFileNames) {
          SERIAL_PROTOCOL(" ");
          SERIAL_PROTOCOL(fileinfo.fname);
          if (fileinfo.fattrib & AM_DIR) SERIAL_PROTOCOL("/");
        }
        SERIAL_EOL;
        delay(50);
  }
  
}
/*
void CardReader::swd_ls()  {
  uint16_t i;
  FRESULT ret;

#if _USE_LFN
	fileinfo.lfname = lfn;
	fileinfo.lfsize = sizeof lfn;
#endif

  //initsd();

  ret=f_opendir(&dp,path);
  if (ret!=FR_OK)
  {
      path[0]=0;
      ret=f_opendir(&dp,"\\");
      if (ret!=FR_OK) return;
  }

  printf("LS\n");
  for (i=0;i<100;i++)
  {
      ret=f_readdir(&dp,&fileinfo);
      if (fileinfo.fname[0]==0 || ret!=FR_OK) return;
      if (fileinfo.fattrib & AM_SYS) continue;
      else if (fileinfo.fattrib & AM_HID) continue;
      if (strstr(fileinfo.fname, ".G")!=NULL)
        printf("%s\n",fileinfo.fname);
  }
  printf("END\n");
}*/

#if ENABLED(LONG_FILENAME_HOST_SUPPORT)

  /**
   * Get a long pretty path based on a DOS 8.3 path
   */
  void CardReader::printLongPath(char *path) {
    FRESULT fr;
    FILINFO fno;
/*
#if _USE_LFN
    fno.lfname = longFilename;
#endif

    longFilename[0]=0;*/
    fr = f_stat(path, &fno);
    if (fr==FR_OK)
    {
        //if (fno.fattrib & AM_DIR)
        //strcpy(longFilename,fno.lfname);
        SERIAL_ECHOPGM("Full Path: "); SERIAL_ECHOLN(fno.fname);
    }
    else SERIAL_PROTOCOL("???");

    SERIAL_EOL;
    /*
    lsAction = LS_GetFilename;

    int i, pathLen = strlen(path);

    // SERIAL_ECHOPGM("Full Path: "); SERIAL_ECHOLN(path);

    // Zero out slashes to make segments
    for (i = 0; i < pathLen; i++) if (path[i] == '/') path[i] = '\0';

    SdFile diveDir = root; // start from the root for segment 1
    for (i = 0; i < pathLen;) {

      if (path[i] == '\0') i++; // move past a single nul

      char *segment = &path[i]; // The segment after most slashes

      // If a segment is empty (extra-slash) then exit
      if (!*segment) break;

      // Go to the next segment
      while (path[++i]) { }

      // SERIAL_ECHOPGM("Looking for segment: "); SERIAL_ECHOLN(segment);

      // Find the item, setting the long filename
      diveDir.rewind();
      lsDive("", diveDir, segment);

      // Print /LongNamePart to serial output
      SERIAL_PROTOCOLCHAR('/');
      SERIAL_PROTOCOL(longFilename[0] ? longFilename : "???");

      // If the filename was printed then that's it
      if (!filenameIsDir) break;

      // SERIAL_ECHOPGM("Opening dir: "); SERIAL_ECHOLN(segment);

      // Open the sub-item as the new dive parent
      SdFile dir;
      if (!dir.open(diveDir, segment, O_READ)) {
        SERIAL_EOL;
        SERIAL_ECHO_START;
        SERIAL_ECHOPGM(MSG_SD_CANT_OPEN_SUBDIR);
        SERIAL_ECHO(segment);
        break;
      }

      diveDir.close();
      diveDir = dir;

    } // while i<pathLen

    SERIAL_EOL;*/
  }

#endif // LONG_FILENAME_HOST_SUPPORT

extern "C" DSTATUS disk_initialize (BYTE);
void CardReader::initsd() {
  int ret;
  int cnt=0;
  extern char slow_card;

  slow_card=0;
  cardOK = false;
/*  ret=disk_initialize(0);
  if (ret!=RES_OK) ret=disk_initialize(0);
  if (ret!=RES_OK) ret=disk_initialize(0);

  if (ret==RES_OK) {
    ret=f_mount(&fs, "",1);

    ret=f_opendir(&dp,"\\");
    if (ret!=FR_OK) return;

    cardOK = true;
    //SERIAL_ECHO_START;
    //SERIAL_ECHOLNPGM(MSG_SD_CARD_OK);
  }
  else {
    //SERIAL_ERROR_START;
    //SERIAL_ERRORLNPGM(MSG_SD_INIT_FAIL);
  }*/

retry:
  if (cnt>2) slow_card=1;
  if (cnt>5) return;
  cnt++;

  IWDG_ReloadCounter();
  MSD0_SPI_Configuration();

  /*ret=disk_initialize(0);
  if (ret==RES_OK)*/
  ret=f_mount(&fs, "",1);
  if (ret==RES_OK) ret=f_opendir(&dp,"\\");
  if (ret!=FR_OK) goto retry;

  cardOK = true;
}

void CardReader::release() {
  sdprinting = false;
  cardOK = false;
}

void CardReader::startFileprint() {
  extern bool relative_mode;
  relative_mode=false;
  if (cardOK)
    sdprinting = true;

}

void CardReader::pauseSDPrint() {
  if (sdprinting) sdprinting = false;
}

void CardReader::openLogFile(char* name) {
  logging = true;
  openFile(name, false);
}

void CardReader::getAbsFilename(char *t) {
  uint8_t cnt = 0;
  *t = '/'; t++; cnt++;
  for (uint8_t i = 0; i < workDirDepth; i++) {
    //workDirParents[i].getFilename(t); //SDBaseFile.getfilename!
    f_getcwd(t,MAXPATHNAMELENGTH);
    while (*t && cnt < MAXPATHNAMELENGTH) { t++; cnt++; } //crawl counter forward.
  }
  if (cnt < MAXPATHNAMELENGTH - FILENAME_LENGTH)
    for (uint8_t i=0;fileinfo.fname[i]!=0;i++)
    {
        *t++=fileinfo.fname[i];
    }
    //file.getFilename(t);
  //else
    //t[0] = 0;
  *t = 0;
}

bool CardReader::openFile(char* name, bool read, bool replace_current/*=true*/) {
  if (!cardOK) return false;

  if (file.obj.fs != 0) { //replacing current file by new file, or subfile call
    if (!replace_current) {
      if (file_subcall_ctr > SD_PROCEDURE_DEPTH - 1) {
        SERIAL_ERROR_START;
        SERIAL_ERRORPGM("trying to call sub-gcode files with too many levels. MAX level is:");
        SERIAL_ERRORLN(SD_PROCEDURE_DEPTH);
        kill(PSTR(MSG_KILLED));
        return;
      }

      SERIAL_ECHO_START;
      SERIAL_ECHOPGM("SUBROUTINE CALL target:\"");
      SERIAL_ECHO(name);
      SERIAL_ECHOPGM("\" parent:\"");

      //store current filename and position
      getAbsFilename(filenames[file_subcall_ctr]);

      SERIAL_ECHO(filenames[file_subcall_ctr]);
      SERIAL_ECHOPGM("\" pos");
      SERIAL_ECHOLN(sdpos);
      filespos[file_subcall_ctr] = sdpos;
      file_subcall_ctr++;
     }
     else {
      SERIAL_ECHO_START;
      SERIAL_ECHOPGM("Now doing file: ");
      SERIAL_ECHOLN(name);
     }
     //file.close();
     f_close(&file);
  }
  else { //opening fresh file
    file_subcall_ctr = 0; //resetting procedure depth in case user cancels print while in procedure
    SERIAL_ECHO_START;
    SERIAL_ECHOPGM("Now fresh file: ");
    SERIAL_ECHOLN(name);
  }
  sdprinting = false;

  if (read) {
    FRESULT ret;
    //if (file.open(curDir, fname, O_READ)) {
    ret=f_open(&file,name,FA_READ);
    if (ret!=FR_OK) ret=f_open(&file,name,FA_READ);
    if (ret==FR_OK) {
      filesize = file.obj.objsize;

      if (filesize>0)
      {
          SERIAL_PROTOCOLPGM(MSG_SD_FILE_OPENED);
          SERIAL_PROTOCOL(name);
          SERIAL_PROTOCOLPGM(MSG_SD_SIZE);
          SERIAL_PROTOCOLLN(filesize);
          sdpos = 0;

          SERIAL_PROTOCOLLNPGM(MSG_SD_FILE_SELECTED);
          //getfilename(0, name);
          //lcd_setstatus(fileinfo.lfname[0] ? fileinfo.lfname : name);
          return true;
      }
    }
    //else
    {
      SERIAL_PROTOCOLPGM(MSG_SD_OPEN_FILE_FAIL);
      SERIAL_PROTOCOL(name);
      SERIAL_PROTOCOLPGM(".\n");
      return false;
    }
  }
  else { //write
    //if (!file.open(curDir, fname, O_CREAT | O_APPEND | O_WRITE | O_TRUNC)) {
    if (f_open(&file, name, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
      f_stat(name, &fileinfo);
      SERIAL_PROTOCOLPGM(MSG_SD_OPEN_FILE_FAIL);
      SERIAL_PROTOCOL(name);
      SERIAL_PROTOCOLPGM(".\n");
      return false;
    }
    else {
      saving = true;
      SERIAL_PROTOCOLPGM(MSG_SD_WRITE_TO_FILE);
      SERIAL_PROTOCOLLN(name);
      //lcd_setstatus(name);
    }
  }
  return true;
}

void CardReader::removeFile(char* name) {
  f_unlink(name);
  return;
}

void CardReader::getStatus() {
  if (cardOK) {
    SERIAL_PROTOCOLPGM(MSG_SD_PRINTING_BYTE);
    SERIAL_PROTOCOL(sdpos);
    SERIAL_PROTOCOLCHAR('/');
    SERIAL_PROTOCOLLN(filesize);
  }
  else {
    SERIAL_PROTOCOLLNPGM(MSG_SD_NOT_PRINTING);
  }
}

void CardReader::write_command(char *buf) {
  unsigned int written;
  char* begin = buf;
  char* npos = 0;
  char* end = buf + strlen(buf) - 1;

  //file.writeError = false;
  if ((npos = strchr(buf, 'N')) != NULL) {
    begin = strchr(npos, ' ') + 1;
    end = strchr(npos, '*') - 1;
  }
  end[1] = '\r';
  end[2] = '\n';
  end[3] = '\0';
  f_write(&file, begin, (end-begin)+2, &written);
  /*file.write(begin);
  if (file.writeError) {
    SERIAL_ERROR_START;
    SERIAL_ERRORLNPGM(MSG_SD_ERR_WRITE_TO_FILE);
  }*/
}

void CardReader::checkautostart(bool force) {
}

void CardReader::closefile(bool store_location) {
  f_close(&file);
  saving = logging = false;

  if (store_location) {
    //future: store printer state, filename and position for continuing a stopped print
    // so one can unplug the printer and continue printing the next day.
  }
}

/**
 * Get the name of a file in the current directory by index
 */
//const char UP_DUR[]="..";
void CardReader::getfilename(uint16_t nr, const char * const match/*=NULL*/) {
  uint16_t i;
  FRESULT ret;
  //FILINFO fileinfo;

  //fn=NULL;
  //initsd();
  uint8_t retry=0;
r:
  ret=f_opendir(&dp,abspath);
  if (ret!=FR_OK)
  {
      if (retry++<5) goto r;
      return;
  }

  cardOK = true;

  //fileinfo.lfname=longFilename;

  for (i=0;i<=nr;)
  {
      ret=f_readdir(&dp,&fileinfo);

      if (ret!=FR_OK || fileinfo.fname[0]==0)
      {
          filenameIsDir=true;
          //strcpy(filename, "..");
          //fn=(char*)UP_DUR;
          strcpy(fileinfo.fname, "..");
          return;
      }
      if (fileinfo.fattrib & AM_SYS) continue;
      else if (fileinfo.fattrib & AM_HID) continue;
      i++;
  }
  if (fileinfo.fattrib & AM_DIR) filenameIsDir=true;
  else filenameIsDir=false;
  //strcpy(filename, fileinfo.fname);
  //fn=fileinfo.fname;
  return;
}

uint16_t CardReader::getnrfilenames()
{
  uint16_t i;
  uint16_t count;
  FRESULT ret;

  /*
#if _USE_LFN
	fileinfo.lfname = lfn;
	fileinfo.lfsize = sizeof lfn;
#endif
*/

  count=0;
  initsd();

  ret=f_opendir(&dp,abspath);
  if (ret!=FR_OK)
  {
      abspath[0]=0;
      ret=f_opendir(&dp,"\\");
      if (ret!=FR_OK) return;
  }

  do {
      ret=f_readdir(&dp,&fileinfo);
      //SERIAL_PROTOCOLLN(fileinfo.fname);
      if (fileinfo.fattrib & AM_SYS) continue;
      else if (fileinfo.fattrib & AM_HID) continue;
      else count++;
  } while (ret==FR_OK && fileinfo.fname[0]!=0);
  return count;
}

uint16_t CardReader::LcdListFile()
{
  uint16_t count;
  uint8_t retry_cnt=0;
  uint16_t i;
  FRESULT ret;
  extern void wifi_writeString_safe(char message[]);

  /*
#if _USE_LFN
	fileinfo.lfname = lfn;
	fileinfo.lfsize = sizeof lfn;
#endif
*/
  initsd();
  count=0;
retry:
  if (retry_cnt++>2) return 0;
  ret=f_opendir(&dp,abspath);
  if (ret!=FR_OK)
  {
      abspath[0]=0;
      ret=f_opendir(&dp,"\\");
      if (ret!=FR_OK) return;
  }

  do {
      ret=f_readdir(&dp,&fileinfo);
      //SERIAL_PROTOCOLLN(fileinfo.fname);

      if (ret!=FR_OK || fileinfo.fname[0]==0) continue;
      if (fileinfo.fattrib & AM_SYS) continue;
      else if (fileinfo.fattrib & AM_HID) continue;
      if (fileinfo.fattrib & AM_DIR) wifi_writeString_safe((char *)"{DIR:");
      else wifi_writeString_safe((char *)"{FILE:");
      wifi_writeString_safe(fileinfo.fname);
      wifi_writeString_safe("}");
      count++;
  } while (ret==FR_OK && fileinfo.fname[0]!=0);

  if (abspath[0]!=0 && abspath[1]!=0)
  {
      count++;
      wifi_writeString_safe((char *)"{DIR:..}");
  }
  
  if (count==0) goto retry;
  return count;
}

bool CardReader::findautoprint()
{
  uint16_t count;
  uint16_t i;
  FRESULT ret;

  initsd();
  count=0;
  ret=f_opendir(&dp,abspath);
  if (ret!=FR_OK)
  {
      abspath[0]=0;
      ret=f_opendir(&dp,"\\");
      if (ret!=FR_OK) return false;
  }

  do {
      ret=f_readdir(&dp,&fileinfo);

      if (ret!=FR_OK || fileinfo.fname[0]==0) continue;
      if (fileinfo.fattrib & AM_SYS) continue;
      else if (fileinfo.fattrib & AM_HID) continue;
      if (!(fileinfo.fattrib & AM_DIR))
      {
          if (//NULL!=strstr(fileinfo.fname,"auto") &&
              fileinfo.fname[0] == 'a' &&
              fileinfo.fname[1] == 'u' &&
              fileinfo.fname[2] == 't' &&
              fileinfo.fname[3] == 'o' &&
              NULL!=strstr(fileinfo.fname,".g"))
                return true;
      }
      count++;
  } while (ret==FR_OK && fileinfo.fname[0]!=0);

  return false;
}

void CardReader::chdir(const char * relpath) {
    f_chdir(relpath);
    f_getcwd(abspath,64);
}

void CardReader::updir() {
    f_chdir("..");
    f_getcwd(abspath,64);
}

void CardReader::printingHasFinished() {
  st_synchronize();
  if (file_subcall_ctr > 0) { // Heading up to a parent file that called current as a procedure.
    f_close(&file);
    file_subcall_ctr--;
    openFile(filenames[file_subcall_ctr], true, true);
    setIndex(filespos[file_subcall_ctr]);
    startFileprint();
  }
  else {
    f_close(&file);

    if (SD_FINISHED_STEPPERRELEASE) {
      enqueuecommands_P(PSTR(SD_FINISHED_RELEASECOMMAND));
    }

    while (block_buffer_tail!=block_buffer_head)
    {
        //IWDG_ReloadCounter();
        idle();
    }

    autotempShutdown();
    //finishAndDisableSteppers();
    extern void EndBuild();
    EndBuild();
    sdprinting = false;
  }
}

#endif //SDSUPPORT
