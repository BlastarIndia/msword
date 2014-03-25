/* S T R T B L . C */
/*  Implementation of a standard string look-up table using CS based
	data.
*/


#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "field.h"
#include "ch.h"
#include "file.h"
#include "dde.h"
#include "debug.h"
#include "renum.h"

#define STRTBL
#include "fltexp.h"
#include "strtbl.h"
#include "strus.h"




#ifdef PROTOTYPE
#include "strtbl.cpt"
#endif /* PROTOTYPE */

/* rgste and mpszgiste are both defined in strtbl.h */


/* C C H  S Z  F R O M  S Z G  I D */
/*  Copy the string with id id in szg to sz.  Write no more than cchMax
	characters.  Return number of characters written, including null
	terminator.
*/

/* %%Function:CchSzFromSzgId   %%Owner:peterj */
CchSzFromSzgId (sz, szg, id, cchMax)
CHAR *sz;
int szg, id, cchMax;

{
	int iste = mpszgiste [szg];
	int isteLim = mpszgiste [szg+1];

	while (iste < isteLim && rgste [iste].id != id)
		iste++;

	if (iste < isteLim)
		/* desired string found */
		{
		CHAR *pch = sz;
		int ich = 0;

		while (ich < cchMax)
			if (!(*pch++ = rgste [iste].sz [ich++]))
				return ich;

		/*  overflow */
		sz [cchMax-1] = '\0';
		return cchMax;
		}

	sz [0] = 0;
	return 0; /* not found */
}


/* W  C O M P  S Z  I S T E */
/*  String compare sz to iste (not case sensitive).  Return zero if equal,
	negative if sz preceeds iste and positive if iste preceeds sz.
	Note: uses a plain ASCII sort, not the sort table.

	Tables are now in upper case to do "french" (drop accents)
	case-insensitive lookup. (jurgenl 08-26-89)
*/

/* %%Function:WCompSzIste   %%Owner:peterj */
NATIVE WCompSzIste (psz, iste)
CHAR *psz;
int iste;

{
	int ich = 0;
	int ch1, ch2;

	while ((ch1 = ChUpperLookup(*psz++)) == (ch2 = rgste[iste].sz[ich++]))
		if (ch1 == 0)
			return 0;

	return ch1 - ch2;
}



/* I D  F R O M  S Z G  S Z */
/*  Look up sz in subtable szg and return the corresponding id (or idNil if
	not found).  Uses a binary search.
*/

/* %%Function:IdFromSzgSz   %%Owner:peterj */
IdFromSzgSz (szg, sz)
int szg;
CHAR *sz;

{
	int isteMin = mpszgiste [szg];
	int isteMac = mpszgiste [szg+1];
	int iste;
	int wCompGuess;
#ifdef DEBUG
	static BOOL fTableChecked = fFalse;
	if (!fTableChecked)
		{
		fTableChecked = fTrue;
		CkRgSte();
		}
#endif /* DEBUG */

	/*  perform binary search for sz */
	while (isteMin < isteMac)
		{
		iste = (isteMin + isteMac) >> 1;
		if ((wCompGuess = WCompSzIste (sz, iste)) == 0)
			return rgste [iste].id;

		else  if (wCompGuess < 0)
			isteMac = iste;

		else
			isteMin = iste + 1;
		}

	/* not found */
	return idNil;
}




#ifdef DEBUG
/* C K  R G S T E */
/* Assure that rgste is properly formed */

/* %%Function:CkRgSte   %%Owner:peterj */
CkRgSte ()
{
	int iste;
	int isteLast;
	int szg;
	CHAR sz [cchMaxSz];

	for (szg = 0; szg < szgMax; szg++)
		{
		iste = mpszgiste [szg];
		isteLast = mpszgiste [szg+1] -1;

		Assert (iste <= isteLast);

		while (iste < isteLast)
			{
			CHAR *pch = sz;
			int ich = 0;

			do
				{
				*pch = rgste [iste].sz[ich++];
				Assert (ChUpperLookup (*pch) == *pch);
				} 
			while (*pch++);

			Assert (WCompSzIste (sz, ++iste) < 0);
			}
		}
}


#endif /* DEBUG */


/* The following functions have been added for localization purposes. */

/* C C H  S Z  E N G  F R O M  S Z G  I D  */
/*  Copy the string with id id in szg to sz.  Write no more than cchMax
	characters.  Return number of characters written.
	It always outputs English strings.
*/

/* %%Function:CchSzEngFromSzgId   %%Owner:peterj */
CchSzEngFromSzgId (sz, szg, id, cchMax)
CHAR *sz;
int szg, id, cchMax;

{
	int iste = mpszgiste [szg];
	int isteLim = mpszgiste [szg+1];

	while (iste < isteLim && rgsteEng [iste].id != id)
		iste++;

	if (iste < isteLim)
		/* desired string found */
		{
		CHAR *pch = sz;
		int ich = 0;

		while (ich < cchMax)
			if (!(*pch++ = rgsteEng [iste].sz [ich++]))
				return ich;

		/*  overflow */
		sz [cchMax-1] = '\0';
		return cchMax;
		}

	sz [0] = 0;
	return 0; /* not found */
}


/* I D  E N G  F R O M  S Z G  S Z */
/*  Look up sz in subtable szg and return the corresponding id (or idNil if
	not found).  Uses a binary search. Looks up only English fields.
*/

/* %%Function:IdEngFromSzgSz   %%Owner:peterj */
IdEngFromSzgSz (szg, sz)
int szg;
CHAR *sz;

{
	int isteMin = mpszgiste [szg];
	int isteMac = mpszgiste [szg+1];
	int iste;
	int wCompGuess;

	/*  perform binary search for sz */
	while (isteMin < isteMac)
		{
		iste = (isteMin + isteMac) >> 1;
		if ((wCompGuess = WCompEngSzIste (sz, iste)) == 0)
			return rgsteEng [iste].id;

		else  if (wCompGuess < 0)
			isteMac = iste;

		else
			isteMin = iste + 1;
		}

	/* not found */
	return idNil;
}



/* W  C O M P  E N G  S Z  I S T E */
/*  String compare sz to iste (not case sensitive).  Return zero if equal,
	negative if sz preceeds iste and positive if iste preceeds sz.
	Note: uses a plain ASCII sort, not the sort table.
	Checks for only English fields.
*/

/* %%Function:WCompEngSzIste   %%Owner:peterj */
NATIVE WCompEngSzIste (psz, iste)
CHAR *psz;
int iste;

{
	int ich = 0;
	int ch1, ch2;

	while ((ch1 = ChLower(*psz++)) == (ch2 = rgsteEng[iste].sz[ich++]))
		if (ch1 == 0)
			return 0;

	return ch1 - ch2;
}


/*************************************************************************/
