/* _exp.h: private EXP definitions.
*/

/* ELVC -- EL Variable-type Cast (doesn't work for elvRecord types)
*/
#define ElvcOfElvElv(elvFrom, elvTo)	((elvFrom) * elvFirstIeldi + (elvTo))

#define elvcNumToInt	ElvcOfElvElv(elvNum, elvInt)
#define elvcIntToNum	ElvcOfElvElv(elvInt, elvNum)

#define RgeltStack()	((ELT huge *)					\
				HpOfSbIb(sbTds, *HpOfSbIb(sbTds, ElGlobal(heltStack))))
#define RgevStack()	(ElGlobal(rgev))

#define IeltStackPtr()	ElGlobal(ieltStackPtr)

#define cevPoolQuantum	256

#define cchDlgStringMax	128
