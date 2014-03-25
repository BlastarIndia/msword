/*
  -- maketool.h
*/


/* system dependencies X86 => XENIX */
#ifndef X86
#define szROText "rt"
#define szWOText "wt"
#else
#define szROText "r"
#define szWOText "w"
#endif

typedef int	BOOL;
#define	TRUE	1
#define	FALSE	0

#define	VOID	void
