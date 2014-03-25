/* C O R E . H */
/*  Definitions and structures used for Opus Core load functions
	and functional core loading.
*/



/* code module groups */
#define cmgCore1    	    0
#define cmgCore2    	    1
#define cmgCore3            2
#define cmgCoreLayout	    3
#define cmgCoreStatline     4
#define cmgCoreRibbon	    5
#define cmgCoreRuler        (cmgCoreRibbon) /* same now */
#define cmgCoreWindows	    6
#define cmgCoreWinScroll    7
#define cmgCoreSdmAll       8
#define cmgSaveCode	    9
#define cmgSaveUtil1        10
#define cmgSaveUtil2        11
#define cmgRulerInit        12
#define cmgRulerDisp        13
#define cmgLoad1	    14
#define cmgLoad2            15
#define cmgAdjPane          16
#define cmgSearchCode       17
#define cmgSearchUtil       18
#define cmgReplaceCode      (cmgSearchCode)  /* the same, for now */
#define cmgReplaceUtil      19
#define cmgDlgOld           20
#define cmgPromptUtilNAC    21
#define cmgPromptUtilAC     22
#define	cmgPromptCode       23
#define cmgDlgNew           24
#define cmgApplyProp        25
#define cmgSelect           26
#define cmgInitStuff        27
#define cmgUtilEdit         28


#ifdef WPROC
extern int far pascal wproc_q();            
extern int far pascal N_FORMATLINEDXA();
extern int far pascal N_FETCHCP();
extern int far pascal disp1_q();           
extern int far pascal disp3_q();            
extern int far pascal disp2_q();
extern int far pascal insert_q();           
extern int far pascal dialog2_q();           
extern int far pascal curskeys_q();         
extern int far pascal select_q();           
extern int far pascal scroll_q();           
extern int far pascal fieldcr_q();           
extern int far pascal fieldfmt_q();           
extern int far pascal edit_q();           
extern int far pascal command_q();           
extern int far pascal fetch1_q();
extern int far pascal idle_q();
extern int far pascal clsplc_q();
extern int far pascal dispspec_q();
extern int far pascal cmdcore_q();
extern int far pascal clipbord_q();
extern int far pascal prcsubs_q();
extern int far pascal create2_q();
extern int far pascal inssubs_q();
extern int far pascal layout_q();
extern int far pascal layout1_q();
extern int far pascal layoutap_q();
extern int far pascal printsub_q();
extern int far pascal ihdd_q();
extern int far pascal formatsp_q();
extern int far pascal statline_q();
extern int far pascal menuhelp_q();
extern int far pascal rulerdrw_q();
extern int far pascal savefast_q();
extern int far pascal filewin_q();
extern int far pascal wwchange_q();
extern int far pascal create_q();
extern int far pascal prompt_q();
extern int far pascal create2_q();
extern int far pascal screen2_q();
extern int far pascal dialog1_q();
extern int far pascal raremsg_q();
extern int far pascal editspec_q();
extern int far pascal save_q();
extern int far pascal util2_q();
extern int far pascal sttb_q();
extern int far pascal wwact_q();
extern int far pascal raremsg_q();
extern int far pascal sdmcore_q();
extern int far pascal open_q();
extern int far pascal search_q();
extern int far pascal dlbenum_q();
extern int far pascal menu_q();
extern int far pascal ENUMCHILDWINDOWS();
extern int far pascal BLOAT();
extern int far pascal MENUWNDPROC();
extern int far pascal RSBWndProc();
extern int far pascal CREATEMENU();
extern int far pascal ANYPOPUP();
extern int far pascal LOADCURSORICONHANDLER();
extern int far pascal CHANGEMENU();
extern int far pascal GETNEXTDLGGROUPITEM();
extern int far pascal SCROLLDC();
extern int far pascal EMPTYCLIPBOARD();
extern int far pascal CREATEFONT();
extern int far pascal GETTEXTCHARACTEREXTRA();
extern int far pascal ADDFONTRESOURCE();
extern int far pascal file2_q();
extern int far pascal PPVALLOCCB();
extern int far pascal iconbar1_q();
extern int far pascal iconbar2_q();
extern int far pascal command2_q();
extern int far pascal sdmcore2_q();
extern int far pascal sdmcore3_q();
extern int far pascal sdmcore4_q();
extern int far pascal sdmlistb_q();
extern int far pascal sdmdir_q();
extern int far pascal sdminit_q();
extern int far pascal dialog3_q();


/* This array lists modules that we want to load at some time */
csconst FARPROC mpihcdpfnModules [] = 
{
#define ihcdSdmcore  0
					(FARPROC)sdmcore_q,
#define ihcdSdmcore3 1
					(FARPROC)sdmcore3_q,
#define ihcdSdmcore4 2 
					(FARPROC)sdmcore4_q,
#define ihcdSdmcore2 3
						(FARPROC)sdmcore2_q,
#define ihcdDialog2  4
					(FARPROC)dialog2_q,
#define ihcdDialog3  5
					(FARPROC)dialog3_q,
#define ihcdDialog1  6
			(FARPROC)dialog1_q,
#define ihcdWwact    7
						(FARPROC)wwact_q,
#define ihcdSdmlistb 8
						(FARPROC)sdmlistb_q,
#define ihcdSdmdir   9
						(FARPROC)sdmdir_q,
#define ihcdDisp1    10
					(FARPROC)disp1_q,
#define ihcdDisp3    11
					(FARPROC)disp3_q,
#define ihcdUSER_SCRLWND  12
			(FARPROC)SCROLLDC,		
#define ihcdCommand2 13
					(FARPROC)command2_q,
#define ihcdInsert   14
					(FARPROC)insert_q,
#define ihcdDlbenum  15
						(FARPROC)dlbenum_q,
#define ihcdQuietlyLoadMax    16  /* limit of idle quite-loading of handles */
#define ihcdSelect   16
					(FARPROC)select_q,
#define ihcdScroll   17
					(FARPROC)scroll_q,
#define ihcdFieldcr  18
					(FARPROC)fieldcr_q,
#define ihcdEdit     19
					(FARPROC)edit_q,
#define ihcdCommand  20
					(FARPROC)command_q,
#define ihcdFetch1   21
					(FARPROC)fetch1_q,
#define ihcdIdle     22
					(FARPROC)idle_q,
#define ihcdClsplc   23
						(FARPROC)clsplc_q,
#define ihcdHeap     24
					(FARPROC)PPVALLOCCB,
#define ihcdDispspec 25
					(FARPROC)dispspec_q,
#define ihcdIconbar1 26
					(FARPROC)iconbar1_q,
#define ihcdDisp2    27
					(FARPROC)disp2_q,
#define ihcdCmdcore  28
					(FARPROC)cmdcore_q,
#define ihcdClipbord 29
					(FARPROC)clipbord_q,
#define ihcdPrcsubs  30
					(FARPROC)prcsubs_q,
#define ihcdCreate2  31
					(FARPROC)create2_q,
#define ihcdInssubs  32
					(FARPROC)inssubs_q,
#define ihcdLayout   33
					(FARPROC)layout_q,
#define ihcdLayout1  34
			(FARPROC)layout1_q,
#define ihcdLayoutap 35
			(FARPROC)layoutap_q,
#define ihcdPrintsub 36
					(FARPROC)printsub_q,
#define ihcdIhdd     37
					(FARPROC)ihdd_q,
#define ihcdFormatsp 38
					(FARPROC)formatsp_q,
#define ihcdStatline 39
						(FARPROC)statline_q,
#define ihcdMenuhelp 40
					(FARPROC)menuhelp_q,
#define ihcdRulerdrw 41
					(FARPROC)rulerdrw_q,
#define ihcdSavefast 42
					(FARPROC)savefast_q,
#define ihcdFilewin  43
					(FARPROC)filewin_q,
#define ihcdWwchange 44
					(FARPROC)wwchange_q,
#define ihcdPrompt   45
						(FARPROC)prompt_q,
#define ihcdCreate   46
						(FARPROC)create_q,
#define ihcdScreen2  47
				(FARPROC)screen2_q,
#define ihcdSdminit  48
						(FARPROC)sdminit_q,
#define ihcdIconbar2 49
						(FARPROC)iconbar2_q,
#define ihcdEditspec 50
						(FARPROC)editspec_q,
#define ihcdSave     51
						(FARPROC)save_q,
#define ihcdUtil2    52
						(FARPROC)util2_q,
#define ihcdSttb     53
						(FARPROC)sttb_q,
#define ihcdWproc    54
					(FARPROC)wproc_q,
#define ihcdRaremsg  55
						(FARPROC)raremsg_q,
#define ihcdOpen     56
						(FARPROC)open_q,
#define ihcdSearch   57
			(FARPROC)search_q,
#define ihcdCurskeys 58
					(FARPROC)curskeys_q,
#define ihcdFile2    59
						(FARPROC)file2_q,
#define ihcdMenu     60
			(FARPROC)menu_q,
#define ihcdRsb      61
			(FARPROC)RSBWndProc,
#define ihcdFormat   62
			(FARPROC)N_FORMATLINEDXA,
#define ihcdFetch    63
			(FARPROC)N_FETCHCP,
#define ihcdGDI_INIT 64
						(FARPROC)ADDFONTRESOURCE,
#define ihcdGDI_FONTLOAD  65
			(FARPROC)CREATEFONT,		
#define ihcdUSER_SWITCH   66
			(FARPROC)ENUMCHILDWINDOWS,	
#define ihcdUSER_MENPAINT 67  
			(FARPROC)MENUWNDPROC,		
#define ihcdUSER_SETMENU  68
			(FARPROC)CREATEMENU,		
#define ihcdUSER_WMGR2	  69
			(FARPROC)ANYPOPUP,		
#define ihcdUSER_STUFF	  70
			(FARPROC)LOADCURSORICONHANDLER,	
#define ihcdUSER_MENUCORE 71
			(FARPROC)CHANGEMENU,		
#define ihcdUSER_MDKEY	  72
			(FARPROC)GETNEXTDLGGROUPITEM,   
};
#define ihcdEnd	    255

#define ihcdModulesMax (sizeof(mpihcdpfnModules)/sizeof(FARPROC))

/* This array will get the actual handles as we read them in */
HANDLE rghcdModules [ihcdModulesMax];

/* This defines the code module groups.  It is used by both the compiler and
!  read by coresize utility.
!  
!  Note that modules should be listed in REVERSE order of importance (last
!  module listed will be "newest" after call).
!
!  To add a cmg:  define next icmd above
!  	    	  add modules above if necessary
!  	    	  add an array at the end of this with list of modules
!  	    	  each array ends with ihcdEnd
!
!  format needed for coresize.exe:
!       ! at beginning of line marks comment
!   	!!grouplists must start off
!	each group starts with !<groupname>
!	modules must be of form "ihcd<modulename>,"
!         if anything other than white space preceeds "ihcd" line is ignored
!	if <modulename>.obj is not obj for that module, follow with
!	  @<filespec> where <filespec>.obj is correct
!	ihcdEnd ends the group
!	!!endlists ends the lists
!
*/
csconst CHAR mpcmgrgihcd [][] =
{ /* !!grouplists */
	{ /* !Core1 */
			ihcdCommand2,
			ihcdDialog1,
			ihcdSdmcore,
			ihcdEditspec,
		ihcdClsplc,
		ihcdCommand,
		ihcdEdit,
		ihcdCurskeys,
		ihcdEnd	    },
	{ /* !Core2 */
		ihcdFieldcr,   /* @fieldcr* */
		ihcdSelect,
			ihcdFilewin,    /* @filewin* */
		ihcdDisp1,	    /* @disp1* */
		ihcdDisp2,	    /* @disp2* */
		ihcdEnd 	    },
	{ /* !Core3 */
		ihcdDisp3,	    /* @disp3* */
		ihcdFormat,     /* @formatn* */
		ihcdFetch,	    /* @fetchn* */
		ihcdWproc,      /* @wproc* */
		ihcdEnd 	    },
	{ /* !CoreLayout */
		ihcdLayout,
		ihcdPrintsub,
			ihcdIhdd,
		ihcdEnd 	    },
	{ /* !CoreStatline */
		ihcdStatline,
		ihcdEnd	    },
	{ /* !CoreRibbon */
			ihcdDialog1,
			ihcdSdmcore,
		ihcdRulerdrw,
		ihcdIconbar1,
		ihcdEnd	    },
	{ /* !CoreWindows */
		ihcdUSER_SWITCH,	    /* 00ADp = 2768 bytes win 2.1 pre */
		ihcdUSER_MENPAINT,	    /* 0085p = 2218 bytes win 2.1 pre */
		ihcdUSER_SETMENU,	    /* 00EBp = 3760 bytes win 2.1 pre */
		ihcdUSER_WMGR2,	    /* 00EFp = 3824 bytes win 2.1 pre */
		ihcdUSER_STUFF,	    /* 0053p = 1328 bytes win 2.1 pre */
		ihcdUSER_MENUCORE,	    /* 0059p = 1424 bytes win 2.1 pre */
		ihcdUSER_SCRLWND,	    /* 0071p = 1808 bytes win 2.1 pre */
		ihcdEnd	    },
	{ /* !CoreWinScroll */
		ihcdRsb,
		ihcdEnd 	    },
		{ /* !CoreSdmAll */
			ihcdDialog1,
			ihcdDialog2,
			ihcdDialog3,
			ihcdSdmcore,
			ihcdSdmcore2,
			ihcdSdmcore3,
			ihcdSdmcore4,
			ihcdSdmlistb,
			ihcdSdmdir,
			ihcdDlbenum,
			ihcdEnd         },
	{ /* !SaveCode */
		ihcdWwchange,
		ihcdCreate,
		ihcdFile2,	    /* @file2* */
		ihcdSttb,
		ihcdSavefast,
		ihcdSave,
		ihcdEnd	    },
	{ /* !SaveUtil1 */
		ihcdFetch,	    /* @fetchn* */
		ihcdFetch1,
		ihcdInssubs,    /* @inssubs* */
		ihcdClsplc,
		ihcdFilewin,    /* @filewin* */
		ihcdEnd	    },
	{ /* !SaveUtil2 */
		ihcdEdit,
		ihcdEditspec,
		ihcdIhdd,
		ihcdEnd	    },
	{ /* !RulerInit */
			ihcdSdmcore,
		ihcdWwchange,
		ihcdScreen2,
		ihcdIconbar2,
		ihcdIconbar1,
		ihcdRulerdrw,
		ihcdEnd	    },
	{ /* !RulerDisp */
			ihcdSdmcore,
		ihcdRulerdrw,
		ihcdIconbar1,
		ihcdDialog1,
		ihcdEnd	    },
	{ /* !Load1 */
		ihcdEdit,
		ihcdFetch,	    /* @fetchn* */
			ihcdFieldcr,
			ihcdEditspec,
		ihcdWwact,
		ihcdDisp1,	    /* @disp1* */
			ihcdRsb,
		ihcdDisp3,	    /* @disp3* */
		ihcdFormat,     /* @formatn* */
		ihcdEnd	    },
	{ /* !Load2 */
		ihcdWwchange,
		ihcdFile2,	    /* @file2* */
		ihcdCreate,
		ihcdOpen,
			ihcdClsplc,
		ihcdFilewin,    /* @filewin* */
			ihcdInssubs,
		ihcdCreate2,
			ihcdSttb,
		ihcdEnd	    },
	{ /* !AdjPane */
		ihcdWproc,	    /* @wproc* */
		ihcdRaremsg,
		ihcdClsplc,
		ihcdDisp3,	    /* @disp3* */
		ihcdDisp1,	    /* @disp1* */
		ihcdScroll,
		ihcdEnd	    },
	{ /* !SearchCode */
		ihcdSearch,     /* @search* */
		ihcdEnd	    },
	{ /* !SearchUtil */
		ihcdFilewin,    /* @filewin* */
		ihcdFormat,     /* @formatn* */
		ihcdDisp1,	    /* @disp1* */
		ihcdDisp3,      /* @disp3* */
		ihcdSelect,
		ihcdClsplc,
		ihcdFieldcr,   /* @fieldcr? */
		ihcdFetch,	    /* @fetchn* */
		ihcdEnd	    },
/*  for now, ReplaceCode is same as SearchCode */
	{ /* !ReplaceUtil */
		ihcdFilewin,    /* @filewin* */
		ihcdFormat,     /* @formatn* */
		ihcdDisp1,	    /* @disp1* */
		ihcdDisp3,      /* @disp3* */
		ihcdSelect,
		ihcdInssubs,    /* @inssubs* */
/**/	    ihcdHeap,
		ihcdClsplc,
		ihcdEdit,
		ihcdFieldcr,   /* @fieldcr? */
		ihcdFetch,	    /* @fetchn* */
		ihcdEnd	    },
	{ /* !DlgOld (used in call to OldestCmg) */ 
			ihcdSdmcore3,
			ihcdSdmcore4,
			ihcdDialog2,
		ihcdEnd	    },
	{ /* !PromptUtilNAC (that's No Abort Check) */
		ihcdStatline,
		ihcdWproc,	    /* @wproc* */
		ihcdEnd	    },
	{ /* !PromptUtilAC (that's with Abort Check) */
		ihcdStatline,
		ihcdDisp3,
		ihcdWwact,
		ihcdWproc,	    /* @wproc* */
		ihcdEnd	    },
	{ /* !PromptCode */
		ihcdPrompt,	    /* @prompt* */
		ihcdEnd	    },
		{ /* !DlgNew */
			ihcdSdmcore,
			ihcdDialog1,
			ihcdSdmcore3,
			ihcdSdmcore4,
			ihcdDialog2,
			ihcdWwact,
			ihcdSdmlistb,
			ihcdEnd         },
		{ /* !ApplyProp */
			ihcdFormat,
			ihcdClsplc,
			ihcdEdit,
			ihcdEditspec,
			ihcdGDI_FONTLOAD,
			ihcdPrcsubs,
			ihcdEnd         },
		{ /* !Select */
			ihcdSelect,
			ihcdScroll,
			ihcdIhdd,
			ihcdCurskeys,
		ihcdDisp1,
			ihcdDisp2,
		ihcdDisp3,
			ihcdEnd         },
		{ /* !InitStuff */
			ihcdRaremsg,
			ihcdSdminit,
			ihcdGDI_INIT,
			ihcdEnd         },
		{ /* !UtilEdit */
			ihcdClsplc,
			ihcdEdit,
			ihcdEditspec,
			ihcdFetch1,
			ihcdInssubs,
			ihcdEnd         },
			

	/* !!endlists */
};

#endif /* WPROC */
