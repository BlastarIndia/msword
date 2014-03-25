/* set of character and string definitions for fields which are related to US 
	and which may be localized.
	included in strtbl.h 
*/

/* numeric strings */
#define	chThousSepUS	','
#define chDecimalUS	'.'
#define chListUS	','
#define chCurrUS	'$'

/* date time strings */
#define chDayUS		'd'
#define chMonthUS	'M'
#define chYearUS	'y'
#define chHourUS	'h'
#define chMinuteUS	'm'

#define	chDateSepUS	'/'
#define chTimeSepUS	':'
#define szAmUS		SzShared("AM")
#define szPmUS		SzShared("PM")

/* switch characters */
#define chFldXeUSBold		'b'
#define chFldXeUSItalic		'i'
#define chFldTocUSOutline	'o'
#define chFldTocUSFields	'f'

#ifdef STRTBL

/* the default field strings for US */
struct STE
		{
		int id;
		CHAR sz[];
		};

csconst struct STE rgsteEng [] =
	{  /* id,           sz              */

/* S Z G  F L T  S I N G L E */
		/*  these strings are special field keywords that are only
			one character and don't have to be followed by a space */
		{ fltExp,       "="             },

/* S Z G  F L T  N O R M A L */
		/*  these are the mainstream field keywords */
		{ fltAsk,       "ask"           },
		{ fltAuthor,    "author"        },
		{ fltSeqLevNum, "autonum"       },
		{ fltSeqLevLeg, "autonumlgl"    },
		{ fltSeqLevOut, "autonumout"    },

		{ fltComments,  "comments"      },
		{ fltCreateDate,"createdate"    },
		{ fltPMData,    "data"          },
		{ fltDate,      "date"          },
		{ fltDde,       "dde"           },
		
		{ fltDdeHot,    "ddeauto"       },
		{ fltEditTime,  "edittime"      },
		{ fltFormula,   "eq"            },
		{ fltFileName,  "filename"      },
		{ fltFillIn,    "fillin"        },

		{ fltFtnRef,    "ftnref"        },
		{ fltGlsy,      "glossary"      },
		{ fltHyperText, "gotobutton"    },
		{ fltIf,        "if"            },
		{ fltImport,    "import"        },
		
		{ fltInclude,   "include"       },
		{ fltIndex,     "index"         },
		{ fltInfo,      "info"          },
		{ fltKeyWords,  "keywords"      },
		{ fltLastRevBy, "lastsavedby"   },
		
		{ fltMacroText, "macrobutton"   },
		{ fltPMRec,     "mergerec"      },
		{ fltPMNext,    "next"          },
		{ fltPMNextIf,  "nextif"        },
		{ fltNumChars,  "numchars"      },
		
		{ fltNumPages,  "numpages"      },
		{ fltNumWords,  "numwords"      },
		{ fltPage,      "page"          },
		{ fltPageRef,   "pageref"       },
		{ fltPrint,     "print"         },
		
		{ fltPrintDate, "printdate"     },
		{ fltQuote,     "quote"         },
		{ fltRefDoc,    "rd"            },
		{ fltBkmkRef,   "ref"           },
		{ fltRevNum,    "revnum"        },
		
		{ fltRevDate,   "savedate"      },
		{ fltSequence,  "seq"           },
		{ fltSet,       "set"           },
		{ fltPMSkipIf,  "skipif"        },
		{ fltStyleRef,  "styleref"      },
		
		{ fltSubject,   "subject"       },
		{ fltTce,       "tc"            },
		{ fltDot,       "template"      },
		{ fltTime,      "time"          },
		{ fltTitle,     "title"         },
		
		{ fltToc,       "toc"           },
		{ fltXe,        "xe"            },

/* S Z G  F O R M A T  F U N C */
		/* these are the arguments to the field format switch */
		{ ffnAlphabetic,"alphabetic"    },
		{ ffnArabic,    "arabic"        },
		{ ffnCaps,      "caps"          },
		{ ffnCardtext,  "cardtext"      },
		{ ffnCharFormat,"charformat"    },
		{ ffnDollartext,"dollartext"    },
		{ ffnFirstcap,  "firstcap"      },
		{ ffnHex,       "hex"           },
		{ ffnLower,     "lower"         },
		{ ffnMergeFormat,"mergeformat"  },
		{ ffnOrdinal,   "ordinal"       },
		{ ffnOrdtext,   "ordtext"       },
		{ ffnRoman,     "roman"         },
		{ ffnUpper,     "upper"         },

/* S Z G  F L T  E X P  F U N C */
		/*  these are the functions available for the expression field */
		{ tfiABS,       "abs"           },
		{ tfiAND,       "and"           },
		{ tfiAVERAGE,   "average"       },
		{ tfiCOUNT,     "count"         },
		{ tfiDEFINED,   "defined"       },
		{ tfiFALSE,     "false"         },
		{ tfiIF,        "if"            },
		{ tfiINT,       "int"           },
		{ tfiMAX,       "max"           },
		{ tfiMIN,       "min"           },
		{ tfiMOD,       "mod"           },
		{ tfiNOT,       "not"           },
		{ tfiOR,        "or"            },
		{ tfiPRODUCT,   "product"       },
		{ tfiROUND,     "round"         },
		{ tfiSIGN,      "sign"          },
		{ tfiSUM,       "sum"           },
		{ tfiTRUE,      "true"          },

/* S Z G  C O N D  O P  C O D E S */
		/*  these are the op codes allowed in conditional fields */
		{ cocLT,        "<"             },
		{ cocLE,        "<="            },
		{ cocNE,        "<>"            },
		{ cocEQ,        "="             },
		{ cocGT,        ">"             },
		{ cocGE,        ">="            },

/* S Z G  D D E  S Y S  T O P I C */
		/* these are the system topic items we offer for dde */
		{ stiFormats,   "formats"       },
		{ stiSysItems,  "sysitems"      },
		{ stiTopics,    "topics"        },
		
/* S Z G  R E N U M  F O R M A T */
		/* formats for the renumber combo box */
		{ rpfLearn,     "learn"         },
		{ rpfLegal,     "legal"         },
		{ rpfOutline,   "outline"       },
		{ rpfSequence,  "sequence"      },

/* S Z G  P A R S E  K E Y S */
		/*  keywords recognized for SendKeys and Execute */
	{ VK_BACK,	"backspace"	},
	{ VK_BACK,      "bksp"	    	},
	{ VK_CANCEL,	"break"	    	},
	{ VK_BACK,	"bs"	    	},
	{ VK_CAPITAL,	"capslock"  	},
	{ VK_CLEAR,	"clear"	    	},
	{ VK_DELETE,	"del"	    	},
	{ VK_DELETE,	"delete"    	},
	{ VK_DOWN,	"down"	    	},
	{ VK_END,	"end"	    	},
	{ VK_RETURN,	"enter"	    	},
	{ VK_ESCAPE,	"esc"	    	},
	{ VK_ESCAPE,	"escape"    	},
	{ VK_F1,	"f1"	    	},
	{ VK_F10,	"f10"	    	},
	{ VK_F11,	"f11"	    	},
	{ VK_F12,	"f12"	    	},
	{ VK_F13,	"f13"	    	},
	{ VK_F14,	"f14"	    	},
	{ VK_F15,	"f15"	    	},
	{ VK_F16,	"f16"	    	},
	{ VK_F2,	"f2"	    	},
	{ VK_F3,	"f3"	    	},
	{ VK_F4,	"f4"	    	},
	{ VK_F5,	"f5"	    	},
	{ VK_F6,	"f6"	    	},
	{ VK_F7,	"f7"	    	},
	{ VK_F8,	"f8"	    	},
	{ VK_F9,	"f9"	    	},
	{ VK_HELP,	"help"	    	},
	{ VK_HOME,	"home"	    	},
	{ VK_INSERT,	"insert"    	},
	{ VK_LEFT,	"left"	    	},
	{ VK_NUMLOCK,	"numlock"   	},
	{ VK_NEXT,	"pgdn"	    	},
	{ VK_PRIOR,	"pgup"	    	},
	{ VK_PRINT,	"prtsc"	    	},
	{ VK_RIGHT,	"right"	    	},
	{ VK_TAB,	"tab"	    	},
	{ VK_UP,	"up"	    	},

/* S Z G  B K M K  S P E C I A L */
		/*  Names of special bookmarks */
	{ ibkmkCell,    "\\cell"        },
		{ ibkmkChar,    "\\char"        },
		{ ibkmkDoc,     "\\doc"         },
	{ ibkmkEODoc,   "\\endofdoc"    },
	{ ibkmkEOSel,   "\\endofsel"    },
	{ ibkmkHeading, "\\headinglevel"},
		{ ibkmkLine,    "\\line"        },
		{ ibkmkPage,    "\\page"        },
		{ ibkmkPar,     "\\para"        },
		{ ibkmkPrevSel1,"\\prevsel1"    },
		{ ibkmkPrevSel2,"\\prevsel2"    },
		{ ibkmkSect,    "\\section"     },
		{ ibkmkSel,     "\\sel"         },
	{ ibkmkSODoc,   "\\startofdoc"  },
	{ ibkmkSOSel,   "\\startofsel"  },
	{ ibkmkTable,   "\\table"       },

/* S Z G  P R I N T  P S */
		/* postscript arguments to the print field */
	{ idPrintPsCell, "cell"		},
	{ idPrintPsDict, "dict"		},
		{ idPrintPsPage, "page"         },
		{ idPrintPsPara, "para"         },
		{ idPrintPsPic,  "pic"          }, 
	{ idPrintPsRow,	 "row"		},
	};

#endif /* STRTBL */


/* stores info for translation of switches */
struct FSITL
	{
	int flt;
	char chSwUS;
	char chSwLocal;
	};

/* function declarations */
CHAR ChSpFetchSw();
CHAR ChItlSw();
