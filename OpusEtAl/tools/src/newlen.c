#include <string.h>
#include <stdio.h>

/*	This program is designed to mimic what the opus tool length does.
	The reason we had to have a new program is that, in trying to convert
	makeopus to OS\2, we could not find the source code for length to
	compile a protected mode version - john loftin (t-johnlo)           */

main()

{
	char Buff[100];
	char *ALine;
	size_t LineLen;

	while ((ALine = gets(Buff)) != NULL)
		{
		LineLen = strlen( ALine );
		printf("%05u\t%s\n", LineLen, ALine);
		}
}


