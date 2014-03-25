/*
	uops.h * Include file for Windows UOPS *
	Compiles with CS only !
*/

/*
	Macros Defined :

	LowWord(l)		: return low word (int) of long
	HighWord(l)		: return high word (int) of long
	MakeLong(lo, hi)	: make a long out of two words

	BLTB(pb1, pb2, cb)	: move from p1 to p2 (near pointers)
	BLTBX(lpb1, lpb2, cb)	: move from lp1 to lp2 (far pointers)

	BLTCX(w, lpw, cw)	: fill *lpw++ with w (cw = word count)
	BLTBCX(b, lpb, cb)	: fill *lpb++ with b (cb = byte count)

	BLTBV(pb1, pb2, cb)	: like BLTB but return pb2+cb
	BLTBXV(lpb1, lpb2, cb)	: like BLTBXV but return lpb2+cb

	see "huge.h" for BLTBH uop.
*/

	
uop void _UOP_VOID();
uop int _UOP_INT();
uop long _UOP_LONG();
uop char *_UOP_PB();
uop char far *_UOP_LPB();

#define LowWord(l) _UOP_INT(2,0xe0,0xda, (long) (l))
#define HighWord(l) _UOP_INT(1,0xda, (long) (l))
#define MakeLong(lo,hi) _UOP_LONG(0, (int) (hi), (int) (lo))

#define BLTB(p1,p2,cb) _UOP_VOID(2,0xe6,0xe, (char *) (p1), (char *) (p2), (int) (cb))
#define BLTBX(lp1,lp2,cb) _UOP_VOID(2,0xe6,0xf, (char far *) (lp1), (char far *) (lp2), (int) (cb))

#define BLTCX(w,lpw,cw) _UOP_VOID(2,0xe6,0xc, (int) (w), (int far *) (lpw), (int) (cw))
#define BLTBCX(b,lpb,cb) _UOP_VOID(2,0xe6,0xd, (char) (b),(char far *) (lpb), (int) (cb))

#define BLTBV(p1,p2,cb) _UOP_PB(2,0xe6,0x20, (char *) (p1), (char *) (p2), (int) (cb))
#define BLTBXV(lp1,lp2,cb) _UOP_LPB(2,0xe6,0x24, (char far *) (lp1), (char far *) (lp2), (int) (cb))
