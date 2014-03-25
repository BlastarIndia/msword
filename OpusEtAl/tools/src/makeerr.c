#include <stdio.h>
#include <stdlib.h>


main(cArg, rgpchArg)
int cArg;
char *rgpchArg[];
{
	fprintf(stderr, "Make process failure!!\n");
	fprintf(stdout, "Make process failure!!\n");

	if (cArg > 1)
		{
		FILE *fp = fopen(rgpchArg[1], "w");
		if (fp == NULL)
			fprintf(stderr, "Cannot open %s.\n", rgpchArg[1]);
		else
			fclose(fp);
		}
}


