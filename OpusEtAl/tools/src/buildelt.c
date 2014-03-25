//***************************************************************************
//
// This program builds the "elt" tables in the proper order -- by means of
// some very simple minded code (ie. not very efficient).
// Table one is in alphabetic order with respect to the keyword, and
// maps the keyword to their corresponding elt name.
// Table two is in elt number order, and maps the elt to its entry in the
// first table.
// Table three maps the first charcter of the keyword to its entry in the
// first table.
// The Elts should be #defined in a seperate file "ELTFILE", and the keywords
// should appear in a one to one correspondence with the Elts in a seperate
// file "DEFFILE".  The keyword elt pair should appear on the same line
// seperated by blanks and/or tabs -- one pair per line.
//
// elt definition file:
// --------------------
// #define eltfirst  1         all elt's must be defined,
// #define eltThird  3         but they may be in any order.
// #define eltsecond 2
//  ...
//
//
//
// keyword file:
// -------------
// eltfirst    one             this is where some of the elt's are matched
// eltThird    three           with their corresponding keywords.  Not all
//                             of the elt's may have keywords.
//
//
//
// destination file:
// -----------------
// #define itkrNil       -1
//
// csconst TKR csrgtkr[] =              this array is in alphabetical order
//    {                                 with respect to the keywords.
//    eltfirst    St("one"),
//    eltThird    St("three"),
//    };
//
// csconst BYTE csmpeltitkr[] =
//    {                                 this array is in elt order.
//    1,          /* eltfirst  1 */
//    itkrNil,    /* eltSecond 2 */
//    3,          /* eltThrid  3 */
//    };


//
// csconst BYTE csmpdchitkr[] =
// 	{
	// 	0, 	 /* a */
	// 	0, 	 /* b */
	// 	0, 	 /* c */                    this is an array of indexes into
			// 	0, 	 /* d */                    the first array based on the first
			// 	0, 	 /* e */                    letter in the keyword.
			// 	0, 	 /* f */
	// 	0, 	 /* g */
	// 	0, 	 /* h */
	// 	0, 	 /* i */
	// 	0, 	 /* j */
	// 	0, 	 /* k */
	// 	0, 	 /* l */
	// 	0, 	 /* m */
	// 	0, 	 /* n */
	// 	0, 	 /* o */
	// 	1, 	 /* p */
	// 	1, 	 /* q */
	// 	1, 	 /* r */
	// 	1, 	 /* s */
	// 	1, 	 /* t */
	// 	2, 	 /* u */
	// 	2, 	 /* v */
	// 	2, 	 /* w */
	// 	2, 	 /* x */
	// 	2, 	 /* y */
	// 	2, 	 /* z */
	// 	};


//
// #define itkrMaxAlpha 	 2
// #define itkrMax	(sizeof (csrgtkr) / sizeof (TKR))
//
// NOTE: 
The first two files may have any number of blank lines or lines 
//       of comment.  These lines are simply skipped over.
//
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <search.h>

#define  BUFFSIZE    256
#define  CSAVEMAX    256

char        *StrSave(char *);
int         Compare(char **, char **);

typedef struct {
         char  *pszElt;
         char  *pszName;
         } SAVE;

static   SAVE     rgSave[CSAVEMAX];
static   char     *rgszInOrder[CSAVEMAX];

main(argc, argv)
int   argc;
char  *argv[];
{
   FILE     *hElt;         // handle to the file containing the #define elts
   FILE     *hDef;         // handle to the file containing the keywords
   FILE     *hOut;         // handle to the output file
   char     *pszEltFile;   // name of file containing #define elts
   char     *pszDefFile;   // name of file containing keywords
   char     *pszOutFile;   // name of the output file
   char     rgchBuff[BUFFSIZE];  // input line buffer
   char     *pszFirst;     // first word of the current input line
   char     *pszSecond;    // second word of the current input line
   char     *pszThird;     // third word of the current input line
   int      i;             // loop index
   int      j;             // loop index
   int      itkr;
   int      iPrevj;        // used to determine last alphabetic element
   int      iSave = 0;     // index into the Save array
   int      iLast  = 0;    // last non-empty element in the Save array 

   if (argc != 4)
      {
      fprintf(stderr, "BUILDELT: inproper number of arguments!\n");
      fprintf(stderr, "USAGE: ");
      fprintf(stderr, "buildelt <deffile> <keywordfile> <destinationfile>\n");
      exit(-1);
      }

   pszEltFile = argv[1];
   pszDefFile = argv[2];
   pszOutFile = argv[3];

   for (i = 0 ; i < CSAVEMAX ; i++)                // initialize array
      rgSave[i]. pszName = rgSave[i].pszElt = NULL;

   if ((hElt = fopen(pszEltFile, "r")) == NULL)    // open file containning
      {                                            // #define elt*'s
      fprintf(stderr, "BUILDELT: can't open %s\n", pszEltFile);
      exit(-2);
      }

   if ((hDef = fopen(pszDefFile, "r")) == NULL)    // open file which match
      {                                            // elt*'s and keyword's
      fprintf(stderr, "BUILDELT: can't open %s\n", pszDefFile);
      exit(-3);
      }

   // get the elt's name and corresponding number from the first file and
   //store it into the array location corresponding to that elt's number.
   while (fgets(rgchBuff, BUFFSIZE, hElt) != NULL)
      {
      if ((pszFirst = strtok(rgchBuff, " \t\n")) == NULL
         || strcmp(pszFirst, "#define") != 0)
         continue;

      if ((pszSecond = strtok(NULL, " \t\n")) == NULL
         || strlen(pszSecond) < 4
         || *pszSecond       != 'e'
         || *(pszSecond + 1) != 'l'
         || *(pszSecond + 2) != 't' )
         continue;

      if ((pszThird = strtok(NULL, " \t\n")) == NULL || !isdigit(*pszThird))
         {
         fprintf(stderr, "BUILDELT: error in input file %s/n", pszEltFile);
         fprintf(stderr, "line: %s %s %s\n", pszFirst, pszSecond, pszThird);
         exit(-4);
         }

      if ((iLast = max(iSave = atoi(pszThird), iLast)) >= CSAVEMAX)
         {
         fprintf(stderr, "BUILDELT: static array size rgSave[] exceeded\n");
         exit(-5);
         }

      if (rgSave[iSave].pszElt)     // if elt is already defined barf!
         {
         fprintf(stderr, "BUILDELT: multiply defined elt* !!\n");
         fprintf(stderr, "%s %s\n", rgSave[iSave].pszElt, pszSecond);
         exit(-6);
         }

      rgSave[iSave].pszElt = StrSave(pszSecond);
      }

   fclose(hElt);

   // get the elt and keyword pair from the second file, search for the
   // elt in the Save array, and then store the keyword into its field in 
   // that array.
   while (fgets(rgchBuff, BUFFSIZE, hDef) != NULL)
      {
      if ((pszFirst = strtok(rgchBuff, " \t\n")) == NULL
         || strlen(pszFirst) < 4
         || *pszFirst       != 'e'
         || *(pszFirst + 1) != 'l'
         || *(pszFirst + 2) != 't' )
         continue;


      if ((pszSecond = strtok(NULL, " \t\n")) == NULL)    // no szName !!
         {
         fprintf(stderr, "BUILDELT: error in input file %s\n", pszDefFile);
         fprintf(stderr, "error with line: %s\n", pszFirst);
         exit(-7);
         }

      for ( i = 1 ; i <= iLast ; i++)
         {
         if (strcmp(pszFirst, rgSave[i].pszElt) == 0)
            break;
         }

      if (i > iLast)           // elt was not #defineded in pszEltFile
         {
         fprintf(stderr, "BUILDELT: file content mismatch!!\n");
         fprintf(stderr, "%s not defined in %s file.\n",
                                                      pszFirst, pszEltFile);
         exit(-8);
         }

      if (rgSave[i].pszName)
         {
         fprintf(stderr, "BUILDELT: multiply defined name!!\n");
         fprintf(stderr, "%s %s\n", rgSave[i].pszName, pszSecond);
         exit(-9);
         }

      rgSave[i].pszName = rgszInOrder[i] = StrSave(pszSecond);
      }

   fclose(hDef);

   qsort((void *) rgszInOrder, (size_t)iLast + 1, sizeof(char *), Compare);

   if ((hOut = fopen(pszOutFile, "w")) == NULL)
      {
      fprintf(stderr, "BUILDELT: can't open %s for writing.\n", pszOutFile);
      exit(-10);
      }

   fprintf(hOut, "#define itkrNil \t\t\t -1\n\n\n");

   // output the first table which is in alphabetic order with respect to
   // the keywords.  The table consists of the elt's name followed by
   // that elt's keyword seperated by commas and an unspecified number
   // of tabs or spaces. Note that the keyword is quoted and inclosed
   // as an argument to the function St() (ie. eltAlias, St("Alias"), ).
   fprintf(hOut, "csconst TKR csrgtkr[] =\n");
   fprintf(hOut, "\t{\n");
   for (i = 0 ; i <= iLast ; i++)
      {
      for (j = 0 ; j <= iLast ; j++)
         {
         if (rgszInOrder[i] == rgSave[j].pszName)
            break;
         }
      if (rgSave[j].pszName != NULL)
         fprintf(hOut, "\t%s, \t\t St(\"%s\"),\n",
                        rgSave[j].pszElt, rgSave[j].pszName);
      }
   fprintf(hOut, "\t};\n\n\n");

   // output the second table which is in numeric order with respect to the
   // elts.  This table maps the elt to its entry in the first table. itkrNil
   // indecates that the elt has no entry in the first table.
   fprintf(hOut, "csconst BYTE csmpeltitkr[] =\n");
   fprintf(hOut, "\t{\n");
   for (i = 0 ; i <= iLast ; i++)
      {
      itkr = 0;
      for (j = 0 ; j <= iLast ; j++)
         {
         if (rgSave[i].pszName == rgszInOrder[j])
            break;
         if (rgszInOrder[j] != NULL)
	    itkr += 1;
         }
      if (rgSave[i].pszName == NULL)
         fprintf(hOut, "\titkrNil,  /* %s  %d */\n", rgSave[i].pszElt, i);
      else
         fprintf(hOut, "\t%d,  \t\t /* %s  %d */\n",
                                                itkr, rgSave[i].pszElt, i);
      }

   fprintf(hOut, "\t};


\n\n\n");

   // output the final table which maps the first character of the keyword to
   // its entry in the first table.
   fprintf(hOut, "csconst BYTE csmpdchitkr[] =\n");
   fprintf(hOut, "\t{
	\n");
	
	   i = j = 0;
	   do
	      {
	      iPrevj = j;
	      fprintf(hOut, "\t%d, \t /* %c */ \n", min(j, iLast), i + 'a');
	      while(j <= iLast 
	         &&    ((*rgszInOrder[j] >= 'a'
	                 && *rgszInOrder[j] <= 'z'
	                 && *rgszInOrder[j] <= (char) i + 'a')
	            ||  (*rgszInOrder[j] >= 'A'
	                 && *rgszInOrder[j] <= 'Z'
	                 && *rgszInOrder[j] <= (char) i + 'A') ))
		      {
	         j++;
		      }
	      i++;
	      } while(i < 26);
	
	   fprintf(hOut, "\t};


\n\n\n");

   fprintf(hOut, "#define itkrMaxAlpha \t %d\n", iPrevj);
fprintf(hOut, "#define itkrMax\t(sizeof (csrgtkr) / sizeof (TKR))\n");

fclose(hOut);
return 0;    
// normal termination of program.
}


//***************************************************************************
//
// StrSave - allocates enough memory to save the passed in string and 
//           returns a pointer to that string.  StrSave prints an error
//           message on error and calls exit();
//
//***************************************************************************

char *StrSave(pszToSave)
char  *pszToSave;
{
   char  *pszAlloc;

   if ((pszAlloc = (char *) malloc( (strlen(pszToSave) + 1) * sizeof(char) )) 
                                                                     == NULL)
      {
      fprintf(stderr, "BUILDELT: out of memory!!\n");
      fprintf(stderr, "unable to allocate sufficient memory.\n");
      exit(-11);
      }

   return strcpy(pszAlloc, pszToSave);
}

//***************************************************************************
//
// Compare - This is the compare function for the qsort() routine.  It simply
//           compares two strings and returns less than zero if *pszFirst is
//           less than *pszSecond, zero if *pszFirst is equal to *pszSecond,
//           and greater than zero if *pszFirst is breater than *pszSecond.
//
//***************************************************************************
int Compare(pszFirst, pszSecond)
char **pszFirst;
char **pszSecond;
{
   if(isalpha(**pszFirst) && isalpha(**pszSecond))
      return strcmp(*pszFirst, *pszSecond);

   if(isalpha(**pszFirst))
      return -1;

   if(isalpha(**pszSecond))
      return 1;

   return strcmp(*pszFirst,*pszSecond);
}
