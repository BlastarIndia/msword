/* automcr.h -- include file for invoking AutoMacros */

#define atmExec		0
#define atmNew		1
#define atmOpen		2
#define atmClose	3
#define atmExit		4

#define atmMax		5


#ifdef MPATMST

csconst char mpatmst [][] =
	{
	StKey("Exec", AutoMcrExec),
	StKey("New", AutoMcrNew),
	StKey("Open", AutoMcrOpen),
	StKey("Close", AutoMcrClose),
	StKey("Exit", AutoMcrExit)
	};

csconst char stAuto [] = StKey("Auto", Auto);

/* 1 for cch, 5 for "Auto", 5 for "Close" (longest in mpatmst) */
#define ichMaxAtm (1 + 5 + 5)

#endif
