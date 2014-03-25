typedef struct
	{
	BITMAP	bm;
	char	rgchBits[550];
	int		cb;
	}  BMDS;

typedef struct
	{
	char	rgchBits[1050];
	int		cb;
	}  RCDS;

#ifdef RESOURCE
#define	MAXBITMAP  26
#define MAXCURSOR  13
#define	MAXICON    6
#else
#define MAXBITMAP  1
#define MAXCURSOR  1
#define MAXICON    1
#endif
