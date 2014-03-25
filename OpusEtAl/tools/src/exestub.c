/*
	COW : Character Oriented Windows

	fixstub.c : stub fixing utility

	NOTE : this file contains hard-coded values from the
		COW version of newexe.inc.
*/

#include <stdio.h>
#include <fcntl.h>

#define cbCopy (14*4)		/* 14 important words */

#define	dlfaReserve	(64-10)	/* see newexe.inc */


void cdecl Fatal(sz, p1, p2, p3, p4, p5)
/*
	-- print fatal error
*/
char *sz;	/* string or format string */
int p1, p2, p3, p4, p5;	/* optional parameter */
{
	printf("fixstub : ");
	printf(sz, p1, p2, p3, p4, p5);
	printf("\n");
	exit(1);
}



main(argc, argv)
int argc;
char *argv[];
{
	int fdMain, fdStub;
	long lfa;
	char rgbCopy[cbCopy];
	int fSetCparaReserve = 0;

	if (argc != 3)
		{
		printf("usage: fixstub <.exe file> <stub file>\n");
		exit(1);
		}

	if ((fdMain = open(argv[1], O_RDWR | O_BINARY)) == -1)
		Fatal("can't open %s", argv[1]);

	if ((fdStub = open(argv[2], O_RDONLY | O_BINARY)) == -1)
		Fatal("can't open %s", argv[2]);

	if (read(fdStub, rgbCopy, cbCopy) != cbCopy)
		Fatal("read error");

	if (write(fdMain, rgbCopy, cbCopy) != cbCopy)
		Fatal("write error");

	close(fdMain);
	close(fdStub);
}


