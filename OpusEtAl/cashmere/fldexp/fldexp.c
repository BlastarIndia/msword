/*
* Created by CSD YACC (IBM PC) from "fldexp.grm" */

# line 2
BOOL    fInUnrfnc;
# define COMSEP 257
# define EQ 258
# define LT 259
# define LE 260
# define GT 261
# define GE 262
# define NE 263
# define NUMBER 264
# define BOOKMARK 265
# define RDCFNC 266
# define BINFNC 267
# define UNRFNC 268
# define IF 269
# define ROW 270
# define COL 271
# define INTEGER 272
# define FLDESC 273
# define PRE_CURRENCY 274
# define POST_CURRENCY 275
# define NUMINPAREN 276
# define BOOL 277
# define UMINUS 278
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

#line 120

	short yyexca[] ={
	-1, 1,
	0, -1,
	-2, 0,
	};


# define YYNPROD 59
# define YYLAST 311
	short yyact[]={

	10,  18,  89,  41,  88,   6,  40,  80,   3,  19,
	42,  20, 101,  17,  10,  89,  73,  88,  43,   6,
	100,  71,  72,   2,  98,   4,  68,  32,  77,  85,
	80,  62,  97,  30,  59, 110,  30,  28,  31,  29,
	56,  31,  70,  30,  28,  60,  29,  95,  31,  66,
	84,  62, 103,  30,  28,  64,  29,  39,  31, 102,
	30,  28,  37,  29,  36,  31,  16,  30,  28,  15,
	29,  57,  31,  75,  30,  28,  38,  29,  34,  31,
	55,  30,  28,   8,  29,  32,  31,   1,  32,  30,
	28,   0,  29,  69,  31,  32,  30,  28,  83,  29,
	78,  31,  90,  81,  67,  32,   0,  94,   0,  82,
	79,   0,  32,   0,   0,   0,  99,   0,   0,  32,
	0,   0,   0,   0,   0, 106,  32,  96, 107,   0,
	0,   0,   0,  32, 109,   0,   0,   0, 105,   0,
	61,  32,   0,   0,   5,  21,   0,  33,  32,   0,
	0,  35,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,  44,  45,  46,  47,  48,  49,  50,
	51,  52,  53,  54,   0,   0,   0,   0,  63,  73,
	65,   0,   0,   0,  71,  72,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,  91,  92,  93,   0,   0,
	0,   0,   0,   0,  18,  58,  11,  12,  13,  14,
	86,  87,  19,   0,  20,   0,  17,   7,  18,   9,
	11,  12,  13,  14,  87, 108,  19,   0,  20,   0,
	17,   7,  22,  23,  24,  25,  26,  27, 104,  22,
	23,  24,  25,  26,  27,   0,   0,   0,   0,  22,
	23,  24,  25,  26,  27,   0,  22,  23,  24,  25,
	26,  27,  76,  22,  23,  24,  25,  26,  27,  74,
	22,  23,  24,  25,  26,  27,   0,  22,  23,  24,
	25,  26,  27,   0,   0,  22,  23,  24,  25,  26,
	27 	};


	short yypact[]={

	-250,-1000,-233, -26, -26,  47, -26,-1000,-263,-1000,
	-26,  24,  22,-1000,  17,-1000,-1000,-1000,-269,-272,
	-254,  47, -26, -26, -26, -26, -26, -26, -26, -26,
	-26, -26, -26,-1000,-1000,  39, -40, -26,  15, -26,
	-1000,-1000,-1000,-1000,  54,  54,  54,  54,  54,  54,
	-9,  -9, -67, -67, -67,-1000,   8,-1000, -60,-231,
	-249,  47,-1000,  32,-1000,  25,-1000, -86, -40, -63,
	-8, -41, -28,-1000, -26, -26, -26,-1000, -63, -11,
	-1000,-1000,-1000,-1000,-249,-247, -28,-1000,-252,-260,
	-1000,  18,  11,   1,-1000,-249, -63,-1000, -28,-1000,
	-1000,-1000,-1000,-1000, -26, -63,-1000,-1000,  -6,-1000,
	-1000 	};


	short yypgo[]={

	0,  87, 140,  83,  40,  76,  73,  71,  45,  28,
	34,  32,  42,  29,  66,  69 	};


	short yyr1[]={

	0,   1,   1,   2,   2,   2,   2,   2,   2,   2,
	2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
	2,   5,   6,   2,   2,   4,   4,   4,   8,   9,
	7,   7,   7,   7,  11,  11,  11,  11,  11,  11,
	11,  11,  12,  12,  13,  13,  13,  10,  10,  14,
	14,  14,  14,  15,  15,  15,  15,   3,   3 	};


	short yyr2[]={

	0,   3,   2,   3,   3,   3,   3,   3,   3,   3,
	3,   3,   3,   3,   2,   1,   1,   1,   3,   4,
	6,   0,   0,   6,   8,   1,   3,   3,   1,   1,
	4,   6,   3,   5,   4,   2,   2,   3,   3,   2,
	1,   1,   1,   1,   1,   2,   2,   1,   1,   2,
	2,   2,   2,   1,   1,   1,   1,   2,   1 	};


	short yychk[]={

	-1000,  -1, 273, 258, 258,  -2,  45, 277,  -3, 265,
	40, 266, 267, 268, 269, -15, -14, 276, 264, 272,
	274,  -2, 258, 259, 260, 261, 262, 263,  43,  45,
	42,  47,  94,  -2, -15,  -2,  40,  40,  -5,  40,
	275, 275, 264, 272,  -2,  -2,  -2,  -2,  -2,  -2,
	-2,  -2,  -2,  -2,  -2,  41,  -4,  -7, 265, -10,
	-8,  -2,  91,  -2,  40,  -2,  41,  -8, 257, -11,
	-12, 270, 271, 265, 257,  -6, 257,  -9, -11, -12,
	93, -10,  -4,  -9,  58, -13, 271, 272,  45,  43,
	-13,  -2,  -2,  -2,  -9,  58, -12, -11, 271, -13,
	272, 272,  41,  41, 257, -12,  -9, -13,  -2,  -9,
	41 	};


	short yydef[]={

	0,  -2,   0,   0,   0,   2,   0,  15,  16,  17,
	0,   0,   0,  21,   0,  58,  53,  54,  55,  56,
	0,   1,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,  14,  57,   0,   0,   0,   0,   0,
	50,  52,  49,  51,   3,   4,   5,   6,   7,   8,
	9,  10,  11,  12,  13,  18,  47,  25,  17,   0,
	0,  48,  28,   0,  22,   0,  19,   0,   0,  42,
	0,  40,  41,  43,   0,   0,   0,  26,  42,   0,
	29,  27,  47,  32,   0,  35,  39,  44,   0,   0,
	36,   0,   0,   0,  30,   0,   0,  42,  38,  37,
	45,  46,  20,  23,   0,   0,  33,  34,   0,  31,
	24 	};


# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

#ifdef YYDEBUG				/* RRR - 10/9/85 */
#define yyprintf(a, b, c) printf(a, b, c)
#else
#define yyprintf(a, b, c)
#endif

/*      parser for yacc output  */

YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse()
{

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

yystack:    /* put a state and value onto the stack */

	yyprintf( "state %d, char 0%o\n", yystate, yychar );
	if ( ++yyps> &yys[YYMAXDEPTH] )
		{
		yyerror( "yacc stack overflow" );
		return(1);
		}
	*yyps = yystate;
	++yypv;
	*yypv = yyval;
yynewstate:

	yyn = yypact[yystate];

	if ( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if ( yychar<0 ) if ( (yychar=yylex())<0 ) yychar=0;
	if ( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if ( yychk[ yyn=yyact[ yyn ] ] == yychar )
		{
		/* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if ( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}
yydefault:
	/* default state action */

	if ( (yyn=yydef[yystate]) == -2 )
		{
		if ( yychar<0 ) if ( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for ( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		for (yyxi+=2; *yyxi >= 0; yyxi+=2)
			{
			if ( *yyxi == yychar ) break;
			}
		if ( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if ( yyn == 0 )
		{
		/* error */
		/* error ... attempt to resume parsing */

		switch ( yyerrflag )
			{

		case 0:   /* brand new error */

			yyerror( "syntax error" );
yyerrlab:
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys )
				{
				yyn = yypact[*yyps] + YYERRCODE;
				if ( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE )
					{
					yystate = yyact[yyn];  /* simulate a shift of "error" */
					goto yystack;
					}
				yyn = yypact[*yyps];

			/* the current yyps has no shift onn "error", pop stack */

				yyprintf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
				--yyps;
				--yypv;
				}

			/* there is no state on the stack with an error shift ... abort */

yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */
			yyprintf( "error recovery discards char %d\n", yychar, 0 );

			if ( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

	yyprintf("reduce %d\n",yyn, 0);
	yyps -= yyr2[yyn];
	yypvt = yypv;
	yypv -= yyr2[yyn];
	yyval = yypv[1];
	yym=yyn;
	/* consult goto table to find next state */
	yyn = yyr1[yyn];
	yyj = yypgo[yyn] + *yyps + 1;
	if ( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
	switch (yym)
		{

	case 21:
# line 58
			{
			fInUnrfnc = fTrue;
			} 
		break;
	case 22:
# line 60
			{
			fInUnrfnc = fFalse;
			} 
		break;/* End of actions */
		}
	goto yystack;  /* stack new state and value */

}


