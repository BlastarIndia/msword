/* Speller Word Codes */

#define spcNotFound    0    /* - word not found                               */
#define spcMainDict    1    /* - word is in main dictionary                   */
#define spcIgnore      2    /* - word found in aux cache - ignore mode        */
#define spcReplace     3    /* - word found in aux cache - replace mode       */
#define spcReplaceXact 4    /* - word found in aux cache - exact replace mode */
#define spcUpdateDict  5    /* - word is in update dictionary                 */
#define spcUserDict    6    /* - word is in user dictionary                   */


/*----------------------------------------------------------------------
	These added to define what happens with chCase array.
	This is used to indicate what properties different characters
	have (numeric, alpha, special-word-reserved, etc) and is done
	by having each bit indicate something special.
--------------------------------------------------------------pault---*/

#define CASE_NONE           0  /* not a legal letter in chCase array */
#define CASE_LOWER        0x1  /* bit 0 indicates LOWERCASE in chCase array */
#define CASE_UPPER        0x2  /*  "  1     "     UPPERCASE */
#define CASE_START        0x4  /*  "  2     "     valid start character */
#define CASE_VALID        0x8  /*  "  3     "     valid string character */
#define CASE_BREAK       0x10  /*  "  4     "     word-break character */
#define CASE_SPECIAL     0x20  /*  "  5     "     special chars (hyphens, etc) */
#define CASE_PUNCT       0x40  /*  "  6     "     punctuation mark which is
													allowed to precede a word,
													e.g. "(hushed)" */
#define CASE_SPACE       0x80  /*  "  7     "     whitespace characters */

#define CASE_MASK         0x3  /* mask off other info, only look at
												upper/lower case bits */

#define islegal(c)  (chCase[c] & CASE_VALID)
#define upperonly(c)    ((c & CASE_UPPER) && !(c & CASE_LOWER))
#define loweronly(c)    ((c & CASE_LOWER) && !(c & CASE_UPPER))
#define ischupper(c) upperonly(chCase[c])
#define ischlower(c) loweronly(chCase[c])


#define TOO_LONG             -5     /* word is too long to check */
#define REPEAT_WORD         -19     /* only used when DOUB_WRD is SET, the
										word was found more than once in a row */
#define CAP_REQUIRED        -30     /* Word exists, but dict says it
										requires a cap.  E.g., "london" */
#define ALLCAPS_REQUIRED    -32     /* Just added.  What will they think of
										next (i.e. why did they change this)?? */
#define IMPROP_CHAR         -40     /* word contains invalid dict chars */
#define IMPROP_CAP          -41     /* Not sure if word exists, but word
										definitely has a capitalization
										problem.  E.g., "LOnden" */
#define INVALID_COMP        -50     /* word is a compoud, juxtaposed by
										hyphens or slashes, but at least one
										part of the compound was not found */
#define INVALID_CHAR        -60     /* ? */
#define NOT_FOUND          -500     /* word located in NEITHER the compressed
										dictionary or the user dictionaries */
#define ABORT_SPELLER         1     /* error occurred - abort the speller */


#define ichMaxSplWord		31  /* longest word which may be sent to speller
					includes terminator */
					
struct GHD {
	unsigned ghsz;
	int ichMac;
	int ichMax;
	};
