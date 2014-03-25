#define WFromPpvB(ppv, b) *((int *) PvParseArg(ppv, bArg))
#define SetPpvBToW(ppv, b, w) *((int *) PvParseArg(ppv, bArg)) = w


/* Dialog Parse Values */
typedef int DPV;
#define dpvError	0x00
#define dpvNormal	0x01
#define dpvBlank	0x02
#define dpvAuto		0x04
#define dpvDouble	0x08
#define dpvSpaces	0x10

/* Opus Parse Types */
/* These values are stored in the dialog structures as the 
	wParam to pass to WParseOpt(). */
#define optNil		0 /* Passed if wParam is not given in de! */
#define optAnyInt      	1
#define optPos  	2
#define optNZPos	4
#define optAnyUnit     	8
#define optLineUnit    	16
#define optAuto		32
	/* 2 more slots avail - not > 128 for elsubs table limit */

#define optPosInt		(optPos | optAnyInt)
#define optPosNZInt		(optNZPos | optAnyInt)
#define optPosUnit		(optPos | optAnyUnit)
#define optPosNZUnit		(optNZPos | optAnyUnit)
#define optPosNZPos   		(optPos | optNZPos)

#define optAutoAnyInt		(optAuto | optAnyInt)
#define optAutoPosInt		(optAuto | optAnyInt | optPos)

#define optAutoAnyUnit		(optAuto | optAnyUnit)
#define optAutoPosUnit		(optAuto | optAnyUnit | optPos)
#define optAutoLineUnit		(optAuto | optAnyUnit | optLineUnit)

#define optPosLineUnit		(optPos | optLineUnit | optAnyUnit)


#define FIntOpt(opt)	((opt) & optAnyInt)
#define FUnitOpt(opt)	((opt) & optAnyUnit)
#define FPosOpt(opt)	((opt) & optPos)
#define FNonNegOpt(opt)	((opt) & optPosNZPos)
#define FNZPosOpt(opt)	((opt) & optNZPos)
#define FAutoOpt(opt)	((opt) & optAuto)
#define FLineOpt(opt)	((opt) & optLineUnit)


extern DPV DpvParseFdxa(int *, TMC, char *, int, int, DPV, int, int, int);
extern DPV DpvPdxaSzTmcWLow(int *, char *, TMC, WORD);
extern WORD WParseIntRange(TMM, char *, void **, WORD, TMC, WORD, WORD);
extern WORD WParsePosIntNZ(TMM, char *, void **, WORD, TMC, WORD);
extern WORD WParseUnit(TMM, char *, void **, WORD, TMC, WORD);
extern WORD WParseAnyInt(TMM, char *, void **, WORD, TMC, WORD);
extern WORD WParseAutoUnit(TMM, char *, void **, WORD, TMC, WORD);
extern WORD WParseOptRange(TMM, char *, void **, WORD, TMC, WORD, WORD, WORD);
