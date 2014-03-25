
/* Standard Boolean CAB */

typedef struct
	{
	BOOL f;
	} CABBOOL;


#define EL


/* Command Function Type */

typedef int CMD;
#define cmdOK		0
#define cmdError       -1
#define cmdCancelled   -2
  /* we do not differentiate these, and -1 is 1 byte special value,
     so we map cmdNoMemory to cmdError  bz
  */
#define cmdNoMemory    cmdError


/* Command Table Offset Type */

typedef unsigned BCM;
#define bcmNil	       0xffff

/* NOTE: fExtend is ignored! */
#define FExecUcm(ucm, chm, fExtend) (CmdExecBcmKc((ucm), (chm)) == cmdOK)
#define CmdExecUcm(ucm, hcab) (CmdExecBcm((ucm), (hcab)))


typedef struct _cmi
	{
	int kc;
	BCM bcm;
	void ** hcab;
	WORD tmc;
	} CMI;
	

typedef struct _cmb
	{
	union {
		int cmm;
		struct {
			int fAction : 1;
			int fDialog : 1;
			int fDefaults : 1;
			int fRepeated : 1;
			int fNoHelp : 1;
			int fCheck : 1;
			int fBuiltIn : 1;
			int fSuper : 1;
			};
		};
		
	union
		{
		CMI cmi;
		struct {
			int kc;
                        union   {
                                int ucm;
			        BCM bcm;
                                };
			void ** hcab;
			WORD tmc;
			};
		};
		
	union {
		CMI cmiPrev;
		struct {
			union	{
				int kcPrev;
				};
                        union   {
                                int ucmPrev;
			        BCM bcmPrev;
                                };
			void ** hcabPrev;
			WORD tmcPrev;
			};
		};
	
	union {
		void * pv;	/* Points to cab extensions */
		BOOL fOn;	/* Used by toggling commands */
		WORD wParam;	/* Arbitrarily used */
		};
	} CMB;


/* REVIEW iftime: these should be expanded in-line to avoid confusion */
#define FCmdFillCab()	(pcmb->fDefaults)
#define FCmdDialog()	(pcmb->fDialog)
#define FCmdAction()	(pcmb->fAction)


/* Setup command mode before calling FExecCmd()! */
#define SetCmdMode(cmmNew)	(vcmb.cmm = (cmmNew))
#define cmmDefaults	(4)	/* Cmd is to fill the CAB with defaults */
#define cmmDialog	(2)	/* Cmd is to bring up the dialog box */
#define cmmAction	(1)	/* Cmd is to perform its action */
#define cmmNoHelp       (cmmNormal | 16)
#define cmmNormal	(cmmDefaults | cmmDialog | cmmAction)
#define cmmCheck	(32)
#define cmmBuiltIn	(64)
#define cmmSuper	(128)
