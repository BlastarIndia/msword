/* * Include file for SetJmp / DoJmp */


typedef struct
	{
	char fPcodeEnv;
	char snEnv;
	unsigned bpcEnv;
	int cbEnv;
	int *fEnv;
	} ENV;

/* Use this to indicate an invalid ENV: fPcodeEnv=snEnv=0. */
#define InvalidateEnv(penv)	(*(int *)(penv)=0)

#ifdef CC	/* Cmerge */
void far pascal DoJmp(ENV near *, int);

#else		/* Cs */
sys int WSYS_SETJMP();
#define SetJmp(penv) WSYS_SETJMP(3, (ENV *) penv)
sys void VSYS_SETJMP();
#define DoJmp(penv,rv) VSYS_SETJMP(6, (ENV *) penv, (int) rv)

#endif
