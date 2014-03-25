/* B C M  M A P . H */
/*  Used by mkbcmmap.c and dumprsh.c. */

#define ichBcmNameMax 20

struct BME /* Bcm Map Element */
	{
	unsigned bcm;
	char szName[ichBcmNameMax];
	};

#define cbBME sizeof(struct BME)



