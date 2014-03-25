typedef struct _dosfbuf {
					char    reserved[21];
					char    Attrib;
					char    time[2];
					char    date[2];
					long    cbFileSize;
					char    szFName[13];
				}  DOSFBUF, *PDOSFBUF;

#define NORMAL      0x00
#define RONLY       0x01
#define HIDDEN      0x02
#define SYSFILE     0x04
#define VOLLABEL    0x08
#define SUBDIR      0x10
#define ARCHIVE     0x20
