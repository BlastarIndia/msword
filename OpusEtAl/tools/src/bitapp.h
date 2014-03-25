#define FALSE	0
#define TRUE	1
#define NULL	0

#define FIG_CURSOR	1
#define FIG_BITMAP	2
#define FIG_ICON	3

#define EVEN_MASK	0xaaaa
#define ODD_MASK	0x5555

#define WRONGPARAM	1
#define BADFILE		2
#define BADFIGURE	3
#define BADEOF		4
#define BADSWITCH	5

#define FAR	far
#define NEAR
#define LONG	long
#define VOID	void

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long  DWORD;
typedef int	  BOOL;
typedef char	 *PSTR;
typedef char NEAR *NPSTR;
typedef char FAR *LPSTR;
typedef int  FAR *LPINT;

typedef struct tagBITMAP {
	short      bmType;
	short      bmWidth;
	short      bmHeight;
	short      bmWidthBytes;
	BYTE       bmPlanes;
	BYTE       bmBitsPixel;
	LPSTR      bmBits;
} BITMAP;

typedef struct {
	short      xHotspot;
	short      yHotspot;
	short      cx;
	short      cy;
	short      WidthBytes;
	short      clr;
} RCI;
