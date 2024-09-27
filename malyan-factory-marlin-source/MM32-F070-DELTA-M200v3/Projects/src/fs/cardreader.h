#ifndef CARDREADER_H
#define CARDREADER_H

#if ENABLED(SDSUPPORT)

#define MAX_DIR_DEPTH 10          // Maximum folder depth
#define FILENAME_LENGTH 13
#define MAX_VFAT_ENTRIES (2)
#define LONG_FILENAME_LENGTH (FILENAME_LENGTH*MAX_VFAT_ENTRIES+1)

//#include "SdFile.h"
//#include "diskio.h"
#include "ff.h"
#include "integer.h"

extern bool sdprinting;
typedef BYTE	DSTATUS;
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;

enum LsAction { LS_SerialPrint, LS_Count, LS_GetFilename };

class CardReader {
public:
  CardReader();

  void initsd();
  void write_command(char *buf);
  //files auto[0-9].g on the sd card are performed in a row
  //this is to delay autostart and hence the initialisaiton of the sd card to some seconds after the normal init, so the device is available quick after a reset

  void checkautostart(bool x);
  bool openFile(char* name,bool read,bool replace_current=true);
  void openLogFile(char* name);
  void removeFile(char* name);
  void closefile(bool store_location=false);
  void release();
  void startFileprint();
  void pauseSDPrint();
  void getStatus();
  void printingHasFinished();

  #if ENABLED(LONG_FILENAME_HOST_SUPPORT)
    void printLongPath(char *path);
  #endif

  void getfilename(uint16_t nr, const char* const match=NULL);
  uint16_t getnrfilenames();
  uint16_t  LcdListFile();
  bool findautoprint();

  void getAbsFilename(char *t);

  void ls(bool withLongFileNames);
  void ls();
  void swd_ls();
  void chdir(const char *relpath);
  void updir();
  //void setroot();

  FORCE_INLINE bool isFileOpen() {return (file.obj.fs!=0);}//{ return file.isOpen(); }
  FORCE_INLINE bool eof() {return f_eof(&file);}//{ return sdpos >= filesize; }
//  FORCE_INLINE int16_t get() {UINT l;char c[1];sdpos++;f_read(&file,c,1,&l);return c[0];}//{ sdpos = file.curPosition(); return (int16_t)file.read(); }
FORCE_INLINE int16_t get() {UINT l;char c[1];
  uint8_t retry=0;
r:
  f_read(&file,c,1,&l);
  if (file.err!=0)
  {
      uint32_t sdpos_bak;
      if (retry++>3) return 0;
      file.err=0;
      f_close(&file);
      initsd();
      sdpos_bak=sdpos;
    openFile(fileinfo.fname, true);
    sdpos=sdpos_bak;
    f_lseek(&file,sdpos);
    sdprinting=1;
    goto r;
  }
  sdpos++;
  return c[0];
}

  FORCE_INLINE void setIndex(long index) { sdpos = index; f_lseek(&file,index); }
  FORCE_INLINE uint8_t percentDone() { return (isFileOpen() && filesize) ? sdpos / ((filesize + 99) / 100) : 0; }
  FORCE_INLINE char* getWorkDirName() {}//{ workDir.getFilename(filename); return filename; }

public:
  bool saving, logging, cardOK, filenameIsDir;
  //char filename[FILENAME_LENGTH];//, longFilename[LONG_FILENAME_LENGTH];
  //char* fn;
  FILINFO fileinfo;
  //char path[64];
  uint32_t sdpos;
  //int autostart_index;
private:
  FIL file;
  DIR dp;
  FATFS fs;
  //char lfn[_MAX_LFN + 1];
  uint16_t workDirDepth;
  #define SD_PROCEDURE_DEPTH 1
  #define MAXPATHNAMELENGTH (FILENAME_LENGTH*MAX_DIR_DEPTH + MAX_DIR_DEPTH + 1)
  uint8_t file_subcall_ctr;
  uint32_t filespos[SD_PROCEDURE_DEPTH];
  char filenames[SD_PROCEDURE_DEPTH][MAXPATHNAMELENGTH];
  uint32_t filesize;
  millis_t next_autostart_ms;

  bool autostart_stilltocheck; //the sd start is delayed, because otherwise the serial cannot answer fast enought to make contact with the hostsoftware.

  LsAction lsAction; //stored for recursion.
  uint16_t nrFiles; //counter for the files in the current directory and recycled as position counter for getting the nrFiles'th name in the directory.
  char* diveDirName;
  //void lsDive(const char *prepend, SdFile parent, const char * const match=NULL);
};

extern CardReader card;
extern char abspath[];

#define IS_SD_PRINTING (card.sdprinting)

#if PIN_EXISTS(SD_DETECT)
  #if ENABLED(SD_DETECT_INVERTED)
    #define IS_SD_INSERTED (READ(SD_DETECT_PIN) != 0)
  #else
    #define IS_SD_INSERTED (READ(SD_DETECT_PIN) == 0)
  #endif
#else
  //No card detect line? Assume the card is inserted.
  #define IS_SD_INSERTED true
#endif

#else

#define IS_SD_PRINTING (false)

#endif //SDSUPPORT

#endif //__CARDREADER_H
