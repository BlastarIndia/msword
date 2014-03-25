#include <stdio.h>
#include <string.h>
#include <direct.h>

#define szMax 125

FILE *pfToCmd;
	char szCmdFile[] = { 
	"tocmd.cmd" 	};


main (int argc, char *argv[])

{
	if ((pfToCmd = fopen(szCmdFile, "wt")) == NULL)
		fprintf (stderr, "Error opening %s\n", szCmdFile);
	else if (argc != 2)
		{
		fprintf (stderr, "Correct usage of this version of to is:\n\n");
		fprintf (stderr, "\tto [pathname]\n");
		}
	else
		{
		char szPath[szMax];

		strcpy (szPath, argv[1]);
		if (chdir(szPath) != 0)
			fprintf (stderr, "newtoexe:\t\t%s is not a valid path\n", szPath);
		else
			{
			fprintf (pfToCmd, "cd %s\n", szPath);
			if (szPath[1] == ':')
				{
				szPath[2] = '\0';
				fprintf (pfToCmd, "%s\n", szPath);
				}
			}
		}
	fclose(pfToCmd);
}


