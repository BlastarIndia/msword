/* Error reporting code */

#include "word.h"
DEBUGASSERTSZ            /* WIN - bogus macro for assert string */
#include "error.h"
#include "debug.h"
#ifdef WIN
#include "message.h"
#include "prompt.h"
#include "sdmdefs.h"
#include "sdmver.h"
#include "sdm.h"
#include "sdmtmpl.h"
#include "help.h"
#include "rareflag.h"
#endif /* WIN */

#ifdef MAC
#define WINDOWS
#define DIALOGS
#include "toolbox.h"
#include "dlg.h"

#define FAR	far
#endif /* MAC */


extern struct MERR	vmerr;

#ifdef WIN
extern BOOL		vfInitializing;
extern BOOL		fElActive;
extern int		vcxtHelp;
extern CHAR             szApp[];
extern BOOL             vfLargeFrameEMS;
extern int              vwWinVersion;
extern HWND             vhwndCBT;
extern HANDLE           vhInstance;
#else
extern int		vistg;
#endif /* WIN */

/* I C O N S */
#ifdef WIN
csconst int rgmbIcon [] = 
{
	MB_APPLMODAL,
			MB_ICONASTERISK|MB_APPLMODAL,
			MB_ICONEXCLAMATION|MB_APPLMODAL,
			MB_ICONHAND|MB_SYSTEMMODAL,
};


#endif /* WIN */
#ifdef MAC
csconst int rgaitIcon [] = 
{
	aitNote,
			aitCaution,
			aitStop,
			aitStop,
};


#endif /* MAC */

#define iIconNote   0  /* note */
#define iIconCaut   1  /* minor probelm (astr) */
#define iIconStop   2  /* major problem (excl) */
#define iIconSevere 3  /* severe problem (hand) */



/* E R R O R  M E S S A G E S */

struct EMD /* Error Message Descriptor */
	{
/* FUTURE (rp): add a bit fCBTCanAnticipate because our current heuristic
   for determining whether to put these messages up (will it hose CBT because
   they won't be expecting it) is weak. */
	int eid : 10;       /* error id */
	int fInit : 1;      /* report during initialization */
	int fRepeats : 1;   /* does not set fError, allowing subsequent errors */
	int fMemory : 1;    /* Out of memory message (sets fMemAlert) */
	int iIcon : 3;      /* icon to place in message box */
	CHAR st[];
};

/* Built mst's: first char: 
		\001=> sz
		\002=> int
		\003=> long
		\004=> stFile (leaf name in result)
		\005=> za (allow room in width for units)
		\006=> name of doc (short form)
		\007=> st
		\010=> optional int (omit if 0)
	2nd char is width (right just fixed field for ints, max field for szs) 
*/

csconst struct EMD rgemd [] = 
	{
	/* order of messages does not matter.  place shared messages first. */

	{ eidComplexSession,    fFalse, fFalse,
			fFalse, iIconStop,
	StKey("Session too complex.  Please close a window.",ComplexSession) },
	

	{ eidInvalForFN,        fFalse, fFalse, fFalse, iIconStop,
	StKey("Not a valid action for footnotes",InvalForFN)               },
	

	{ eidInvalForANN,       fFalse, fFalse, fFalse, iIconStop,
	StKey("Not a valid action for annotations",InvalForANN)               },
	

	{ eidMaxCol,            fFalse, fFalse, fFalse, iIconStop,
	StKey("Maximum number of columns exceeded",MaxCol)                 },
	

	{ eidMaxWidth,          fFalse, fFalse, fFalse, iIconStop,
	StKey("Maximum width exceeded",MaxWidth)                           },
	

	{ eidCantDelEOR,        fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot delete end of row",CantDelEOR)                        },
	

	{ eidIllegalPasteToTable,        fFalse, fFalse, fFalse, iIconStop,
	StKey("Illegal paste into table",IllegalPasteToTable)                                    },
	

	{ eidCantPasteTextWChSec, fFalse, fFalse, fFalse, iIconCaut,
	StKey("Text contains section mark; cannot paste.",CantPasteTextWChSec) },
	

	{ eidCantPasteTableInTable, fFalse, fFalse, fFalse, iIconCaut,
	StKey("Cannot paste table into a table cell",CantPasteTableInTable) },
	

	{ eidDocTooBigToSave,   fFalse, fFalse, fFalse, iIconCaut,
	StKey("Document is too large to be saved.  Delete some text before trying to save.",DocTooBigToSave) },
	

	{ eidNotValidEOR,       fFalse, fFalse, fFalse, iIconStop,
	StKey("Not a valid action for end of row",NotValidEOR)    },
	

	{ eidNoSectAllowed,     fFalse, fFalse, fFalse, iIconStop, 
	StKey("Sections not allowed",NoSectAllowed)                         },
	

	{ eidDocNotSaved,       fFalse, fFalse, fFalse, iIconStop,
	StKey("Document not saved",DocNotSaved)                            },
	

	{ eidDiskErrSaveNewName, fFalse, fFalse, fFalse, iIconStop,
	StKey("Original file may be damaged due to serious disk error. Save with a new name.",DiskErrSaveNewName) },
	

	{ eidFormatTooComplex , fFalse, fFalse, fFalse, iIconSevere,
	StKey("Formatting too complex",FormatTooComplex )                  },
	

	{ eidClosingSavedDoc, fFalse,  fFalse, fTrue, iIconSevere,
	StKey("Low memory: closing saved document", ClosingSavedDoc) },
	

	{ eidTooManyEdits, fFalse, fFalse, fFalse, iIconSevere,
	StKey("Too many edits; operation will be incomplete. Save your work.",TooManyEdits) },
	

	{ eidTooWideToSplit, fFalse, fFalse, fFalse, iIconNote,
	StKey("One or more rows are too wide to split.",TooWideToSplit)                 },
	

	{ eidTooWideForOperation, fFalse, fFalse, fFalse, iIconNote,
	StKey("One or more rows are too wide for operation.",TooWideForOperation)                 },
	

	{ eidCantUseSel, fTrue, fFalse, fFalse, iIconStop,
	StKey("Illegal selection for copy or move", CantUseSel) },
	


#ifdef MAC

	{ eidCantTableToEnd, fTrue, fFalse, fFalse, iIconStop,
	StKey("Table cannot be moved to end of document", CantTableToEnd) },
	

	{ eidTableTooWide,      fFalse, fFalse, fFalse, iIconStop,
	StKey("Table too Wide.",TableTooWide)                               },
	

	{ eidMaxCells,          fFalse, fFalse, fFalse, iIconStop,
	StKey("Maximum number of cells exceeded.",MaxCells)                 },
	

	{ eidCopyPasteAreaDiff, fFalse, fFalse, fFalse, iIconStop,
	StKey("Copy and Paste areas are different shapes.",CopyPasteAreaDiff) },
	

	{ eidCpRollOver,        fFalse, fFalse, fFalse, iIconSevere,
	StKey("Serious problem: Your document is too large for Word to handle.",CpRollOver) },
	

	{ eidLowMemCloseWindows, fFalse, fFalse, fTrue,  iIconStop,
	StKey("Low memory.  Close extra windows and save your work.",LowMemCloseWindows) },
	

	{ eidNoMemOperation,    fFalse, fFalse, fTrue,  iIconSevere,
	StKey("complete operation.",NoMemOperation)                         },
	

	{ eidNoMemDisplay,      fFalse, fFalse, fTrue,  iIconSevere,
	StKey("update display.",NoMemDisplay)                               },
	

	{ eidSysLock,           fTrue,  fTrue,  fFalse,  iIconSevere,
	StKey("System disk is locked or full and memory is nearly full. Save your work.",SysLock) },
	

	{ eidSysFull,           fTrue,  fTrue,  fFalse,  iIconSevere,
	StKey("System disk and memory are nearly full. Save your work.",SysFull) },
	

	{ eidMultiRefStyle,     fFalse, fFalse, fFalse, iIconStop,
	StKey("Name refers to more than one style.",MultiRefStyle)          },
	

	{ eidNotStyleName,      fFalse, fFalse, fFalse, iIconStop,
	StKey("Name is not a style name.",NotStyleName)                     },
	

	{ eidInvalStyleName,    fFalse, fFalse, fFalse, iIconStop,
	StKey("Valid style name is required.",InvalStyleName)               },
	

	{ eidIllegalMoveToHeadFoot, fFalse, fFalse, fFalse, iIconStop,
	StKey("Can't move sections into headers/footers.",IllegalMoveToHeadFoot) },
	

	{ eidIllegalFNPlacement, fFalse, fFalse, fFalse, iIconStop,
	StKey("Can't move footnote references there.",IllegalFNPlacement)   },
	

	{ eidCantReplaceFN,      fFalse, fFalse, fFalse, iIconStop,
	StKey("Can't replace footnote references.",CantReplaceFN)           },
	

	{ eidNoConversionText,      fFalse, fFalse, fFalse, iIconStop,
	StKey("No visible text to convert.",NoConversionText)           },
	

	{ eidInvalidSelection,  fFalse, fFalse, fFalse, iIconStop,
	StKey("Not a valid selection",InvalidSelection)                     },
	

	{ eidDocTooBigChange,     fFalse, fFalse, fFalse, iIconSevere,
	StKey("Document is too large to be saved. Change aborted.",DocTooBigChange)           },
	
	
	{ eidMixedSwitchSel,       fFalse, fFalse, fFalse, iIconNote,
	StKey("Mixed table/text selection. Shrinking replacement area.",MixedSwitchSel)    },
	

	{ eidInvalidFmtCells,	fFalse, fFalse, fFalse, iIconStop,
	StKey("Invalid selection to format cells.", InvalidFmtCells)    },
	

	{ eidWholeRowsConverted,	fFalse, fFalse, fFalse, iIconStop,
	StKey("Only whole rows may be converted into text.", WholeRowsConverted)    },
	

	{ eidCannotAgainTable,	fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot perform table action again.", CannotAgainTable)    },
	

	{ eidTableInTable,	fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot insert a table inside a table.", CannotInsTableInTable)    },
	

	{ eidIllegalGlsy,	fFalse, fFalse, fFalse, iIconStop,
	StKey("Illegal glossary entry.", IllegalGlsy)    },
	

#endif /* MAC */
#ifdef WIN

	{ eidCantRunApp,        fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot find or run application",CantRunApp)              },
	

	{ eidCopyPasteAreaDiff, fFalse, fFalse, fFalse, iIconNote,
	StKey("Copy and paste areas are different shapes.",CopyPasteAreaDiff) },
	

	{ eidCpRollOver,        fFalse, fFalse, fFalse, iIconSevere,
	StKey("Your document is too large for Word to handle",CpRollOver) },
	

	{ eidSysLock,           fTrue,  fTrue,  fFalse,  iIconSevere,
	StKey("You are working without a Word work file and memory is nearly full. Save your work.",SysLock) },
	

	{ eidSysFull,           fTrue,  fTrue,  fFalse,  iIconSevere,
	StKey("Word work file and memory are nearly full. Save your work.",SysFull) },
	

	{ eidGotoLineAndFoot,   fFalse, fTrue,  fFalse, iIconNote, /* Caut */
	StKey("Line, footnote, and annotation are mutually exclusive.",GotoLineAndFoot) },
	

	{ eidGotoNothing,       fFalse, fTrue,  fFalse, iIconNote, /* Caut */
	StKey("Undefined Go To operation",GotoNothing)                      },
	

	{ eidNoBkmk,            fFalse, fTrue,  fFalse, iIconNote, /* Caut */
	StKey("Bookmark does not exist.",NoBkmk)                             },
	

	{ eidGotoConflict,      fFalse, fTrue,  fFalse, iIconNote, /* Caut */
	StKey("Go To group character repeated",GotoConflict)                },
	

	{ eidNoPrevSearch,      fFalse, fTrue,  fFalse, iIconNote,
	StKey("No previous Go To or Search",NoPrevSearch)                   },
	

	{ eidInvalBkmkNam,      fFalse, fTrue,  fFalse, iIconNote, /* Caut */
	StKey("Bookmark name not valid",InvalBkmkNam)                       },
	

	{ eidNoMemIndex,        fFalse, fFalse, fTrue,  iIconSevere,
	StKey("compile index",NoMemIndex)                                  },
	

	{ eidNoMemHelp,         fFalse, fFalse, fTrue,  iIconSevere,
	StKey("run Help",NoMemHelp)                                         },
	

	{ eidNoMemLaunchDde,    fFalse, fFalse, fTrue,  iIconSevere,
	StKey("run DDE application",NoMemLaunchDde)                         },
	

	{ eidNoMemRunApp,       fFalse, fFalse, fTrue,  iIconSevere,
	StKey("run application",NoMemRunApp)                                },
	

	{ eidNoMemOperation,    fFalse, fFalse, fTrue,  iIconSevere,
	StKey("complete operation",NoMemOperation)                         },
	

	{ eidNoMemOpRepeats,    fFalse, fTrue, fTrue,  iIconSevere,
	StKey("complete operation",NoMemOpRepeats)                         },
	

	{ eidNoMemDisplay,      fFalse, fFalse, fTrue,  iIconSevere,
	StKey("update display",NoMemDisplay)                               },
	

	{ eidNoMemGlsy,         fFalse, fFalse, fTrue,  iIconSevere,
	StKey("define glossary",NoMemGlsy)                                  },
	

	{ eidNoMemMerge,        fFalse, fFalse, fTrue,  iIconSevere,
	StKey("merge styles",NoMemMerge)                                    },
	

	{ eidNoMemOutline,      fFalse, fFalse, fTrue,  iIconSevere,
	StKey("display outline",NoMemOutline)                               },
	

	{ eidNoMemRuler,        fFalse, fFalse, fTrue,  iIconSevere,
	StKey("display ruler",NoMemRuler)                                    },
	

	{ eidNoMemPict,         fFalse, fFalse, fFalse,  iIconSevere,
	StKey("display/print picture",NoMemPict)                            },
	

	{ eidNoGlsy,            fFalse, fFalse, fFalse, iIconStop,
	StKey("No such glossary",NoGlsy)                                    },
	

	{ eidSDE,               fTrue, fFalse, fFalse, iIconSevere,
	StKey("Serious disk error on file: \004\000",SeriousDiskError) },
	

	{ eidSDN,               fTrue, fTrue,  fFalse, iIconSevere,
	StKey("Unrecoverable disk error on file: \004\000",UnrecoverableDiskError) },
	

	{ eidDiskFull,          fTrue, fFalse, fFalse, iIconSevere,
	StKey("Disk is full trying to write \001\000. Save document on a different disk.", DiskFullERROR_C) },
	

	{ eidNoAvail,           fTrue, fFalse, fFalse, iIconSevere,
	StKey("File is not available: \004\000",NoAvail)                              },
	

	{ eidCantOpen,          fTrue,  fFalse, fFalse, iIconSevere,
	StKey("Cannot open document",CantOpen)                              },
	

	{ eidInvalidStyle,      fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Style name does not exist.",InvalidStyle)                     },
	

	{ eidReplaceFail,       fFalse, fFalse, fTrue, iIconSevere,
	StKey("Low memory: cannot perform Replace",ReplaceFail)           },
	

	{ eidNotFound,          fFalse, fTrue,  fFalse, iIconNote,
	StKey("Search text not found",NotFound)                             },
	

	{ eidRevNotFound,       fFalse, fTrue,  fFalse, iIconNote,
	StKey("Revision marks not found",RevNotFound)                       },
	

	{ eidBadCrop,           fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Dimensions after cropping are too small or too large.",BadCrop) },
	

	{ eidBadScale,          fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Dimensions after scaling are too small or too large.",BadScale) },
	

	{ eidFcTooBig,          fFalse, fFalse, fFalse, iIconSevere,
	StKey("File too large to save; delete some text and try again.",FcTooBig) },
	

	{ eidBogusDotPath,      fTrue,  fTrue,  fFalse, iIconNote,
	StKey("DOT-PATH specified in WIN.INI is not valid: ignoring.",BogusDotPath) },
	

	{ eidBogusIniPath,      fTrue,  fTrue,  fFalse, iIconNote,
	StKey("INI-PATH specified in WIN.INI is not valid: ignoring.",BogusIniPath) },
	

	{ eidBogusUtilPath,     fTrue,  fTrue,  fFalse, iIconNote,
	StKey("UTIL-PATH specified in WIN.INI is not valid: ignoring.",BogusUtilPath) },
	

	{ eidConvtrNoLoad,      fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Cannot start converter: \004\000",ConvtrNoLoad)                        },
	

	{ eidConvtrNoLoadMem,   fTrue, fFalse, fTrue,  iIconSevere,
	StKey("Not enough memory to run converter",ConvtrNoLoadMem)        },
	

	{ eidBogusPassword,     fTrue, fFalse, fFalse, iIconNote,
	StKey("Incorrect password",BogusPassword)                           },
	

	{ eidInvalidHotZ,       fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid hot zone value",InvalidHotZ)                     },
	

	{ eidHypNoDataFile,     fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot open Word hyphenation file",HypNoDataFile)            },
	

	{ eidInvalidDOT,        fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Not a valid document template",InvalidDOT)                   },
	

	{ eidMTL,               fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Margins, column spacing, or paragraph indentations too large",MTL) },
	

	{ eidTabPosTooLarge,    fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Tab position too large",TabPosTooLarge)                      },
	

	{ eidNoPrinter,         fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot print; no printer installed",NoPrinter)               },
	

	{ eidInvalPrintRange,   fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid print range",InvalPrintRange)                    },
	

	{ eidCantPrint,         fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot print",CantPrint)                                     },
	

	{ eidPRFAIL,            fFalse, fFalse, fTrue,  iIconStop,
	StKey("Not enough memory to repaginate/print this document",PRFAIL) },
	

	{ eidPrDiskErr,         fFalse, fFalse, fFalse, iIconStop,
	StKey("Not enough disk space to print this document",PrDiskErr)     },
	

	{ eidSessionComplex,    fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot open window; too many open",SessionComplex)           },
	

	{ eidNOTNUM,            fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid integer",NOTNUM)                                 },
	

	{ eidNOTDXA,            fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid measurement",NOTDXA)                             },
	

	{ eidOutOfRange,        fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Number must be between \002\000 and \002\000.",OutOfRange)   },
	

	{ eidZaOutOfRange,      fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Measurement must be between \001\000 and \001\000.",ZaOutOfRange) },
	

	{ eidCantWriteFile,     fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot write to file \001\000",CantWriteFile)               },
	

	{ eidNoMemory,          fFalse, fFalse, fTrue,  iIconStop,
	StKey("Low memory: save your document now.",NoMemory)              },
	

	{ eidBadFile,           fTrue,  fFalse, fFalse, iIconCaut,
	StKey("Document name or path is not valid.",BadFile)                 },
	

	{ eidBadFileDlg,        fTrue,  fTrue,  fFalse, iIconCaut,
	StKey("Document name or path is not valid.",BadFile)                 },
	

	{ eidBadFilename,       fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid file name",BadFilename) },
	

	{ eidFileExists,        fFalse,  fTrue,  fFalse, iIconCaut,
	StKey("Cannot name a document the same as an open document",FileExists) },
	

	{ eidCantWriteFrgn,     fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Cannot write foreign format over open document",CantWriteFrgn) },
	

	{ eidFileIsReadOnly,    fFalse, fTrue,  fFalse, iIconCaut,
	StKey("File is read-only.",FileIsReadOnly)                           },
	

	{ eidNoSaveTemp,        fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot save; cannot create file",NoSaveTemp)           },
	

	{ eidFields2Deep,       fFalse, fFalse, fFalse, iIconCaut,
	StKey("Fields nested too deeply",Fields2Deep)                       },
	

	{ eidFields2DeepCalc,   fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Fields nested too deeply",Fields2Deep)                       },
	

#ifdef RPTLOWMEM /* no longer wanted */
	{ eidLowMem1,           fTrue,  fTrue,  fFalse,  iIconNote,
	StKey("Low memory: run with background pagination off for better performance.",LowMem1) },
	

	{ eidLowMem2,           fTrue,  fTrue,  fFalse,  iIconNote,
	StKey("Low memory: run with background pagination, ruler, ribbon, and status bar off for better performance.",LowMem2) },
	

	{ eidVeryLowMem,        fTrue,  fTrue,  fFalse,  iIconSevere,
	StKey("Low memory: use draft view for better performance.",VeryLowMem) },
#endif /* RPTLOWMEM */
	

	{ eidCantRunM,          fTrue, fFalse, fFalse,  iIconSevere,
	StKey("Not enough memory to run Word",CantRunM)                     },
	

	{ eidWrongWinOrDos,     fTrue, fFalse, fFalse,  iIconSevere,
	StKey("Cannot run Word: incorrect Windows or DOS version",WrongWinDos)                     },
	

	{ eidWordRunning,       fTrue, fFalse, fFalse,  iIconSevere,
	StKey("Word is already running.",WordRunning)                     },
	

	{ eidBogusGdt,          fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Cannot open existing \004\000",BogusGdt)                  },
	

	{ eidInvalidDate,       fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid date",InvalidDate)                               },
	

	{ eidBadStyle,          fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid style name",BadStyle)                            },
	

	{ eidBadStyleSysModal,  fFalse, fTrue,  fFalse, iIconSevere,
	StKey("Not a valid style name",BadStyle)                            },
	

	{ eidStshFull,          fFalse, fFalse, fFalse, iIconStop,
	StKey("Style sheet is full. Cannot define new style.",StshFull)      },
	

	{ eidStupidBasedOn,     fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Style cannot be based on itself.",StupidBasedOn)              },
	

	{ eidInvalidBasedOn,    fFalse, fTrue, fFalse, iIconCaut,
	StKey("Based On style name does not exist.",InvalidBasedOn)          },
	

	{ eidInvalidNext,       fFalse, fTrue, fFalse, iIconCaut,
	StKey("Next style name does not exist.",InvalidNext)                 },
	

	{ eidCantMerge,         fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Cannot merge the active style sheet",CantMerge)              },
	

	{ eidStyleExists,       fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Style name already exists.",StyleExists)                      },
	

	{ eidFileNotExist,      fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Document does not exist.",FileNotExist)                       },
	

	{ eidClipNoRender,      fFalse, fFalse, fTrue,  iIconSevere,
	StKey("Not enough memory for large Clipboard",ClipNoRender)         },
	

	{ eidIndentTooLarge,    fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Indentation too large",IndentTooLarge)                       },
	

	{ eidParTooWide,        fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Paragraph too wide",ParTooWide)                              },
	

	{ eidRuntimeWindows,    fFalse, fFalse, fFalse, iIconNote,
	StKey("Full Windows required to run another application",RuntimeWindows) },
	

	{ eidNoExtension,       fFalse, fFalse, fFalse, iIconNote,
	StKey("Command name must have extension.",NoExtension)               },
	

	{ eidCantOpenDOT,       fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Cannot open document template",CantOpenDOT)                  },
	

	{ eidDotNotValid,       fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Document template is not valid.",DotNotValid)                 },
	

	{ eidOldFib,            fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Cannot open file: has old file format.  Opening as plain text.",OldFib)             },
	

	{ eidFutureFib,         fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Cannot open file: do not understand file format.  Opening as plain text.",FutureFib) },


	{ eidPmWordFile,        fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Opening PM Word document.  (Documents may be fully converted by saving the document as a Windows Word document from within PM Word)",PmWordFile) },
	

	{ eidOldDot,            fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Cannot open file: saved in old template format.  Opening as plain text.",OldDot) },
	

	{ eidOldFastSavedFib,   fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Cannot open file: fast saved in old format.  Opening as plain text.",OldFastSavedFib) },
	

	{ eidStshFullCopy,      fFalse, fFalse, fFalse, iIconSevere,
	StKey("Style sheet full.  Style of some paragraphs may become Normal.",StshFullCopy) },
	

	{ eidCantInsertSelf,    fFalse, fTrue,  fFalse, iIconStop,
	StKey("Cannot insert file into itself",CantInsertSelf)              },
	

	{ eidEtNoLoadMem,       fFalse, fTrue,  fTrue,  iIconStop,
	StKey("Not enough memory to run Thesaurus",EtNoLoadMem)             },
	

	{ eidEtNoLoad,          fFalse, fTrue,  fFalse, iIconStop,
	StKey("Cannot start Thesaurus",EtNoLoad)                            },
	

	{ eidSpellNoLoadMem,    fFalse, fTrue,  fTrue,  iIconStop,
	StKey("Not enough memory to run Speller",SpellNoLoadMem)           },
	

	{ eidSpellNoDict,       fFalse, fTrue,  fFalse, iIconStop,
	StKey("Cannot find Speller",SpellNoDict)                           },
	

	{ eidSpellNoLoad,       fFalse, fTrue,  fFalse, iIconStop,
	StKey("Cannot start Speller",SpellNoLoad)                            },
	

	{ eidSpellUpdateFail,   fFalse, fTrue, fFalse, iIconStop,
	StKey("Cannot create main update dictionary",SpellUpdateFail)       },
	

	{ eidSpellNotFound,     fFalse, fTrue, fFalse, iIconStop, 
	StKey("Word was not found",SpellNotFound)			   },
	
	
	{ eidSpellDictRO,     fFalse, fTrue, fFalse, iIconStop, 
	StKey("Cannot remove word from dictionary",SpellDictRO)         },
	

	{ eidSpellWdFndMain,    fFalse, fTrue, fFalse, iIconNote,
	StKey("Word was found in main dictionary.",SpellWdFndMain)           },
	

	{ eidSpellWdFndUpdate,  fFalse, fTrue, fFalse, iIconNote,
	StKey("Word was found in main update dictionary.",SpellWdFndUpdate)  },
	

	{ eidSpellWdFndAux,     fFalse, fTrue, fFalse, iIconNote,
	StKey("Word was found in user dictionary.",SpellWdFndAux)            },
	

	{ eidCantFindCbt,       fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot find WINWORD.CBT directory",CantFindCbt)              },
	

	{ eidCantFindHelp,       fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot find Help application",CantFindHelp)                  },
	

	{ eidInvTabPos,         fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid tab position",InvTabPos)                         },
	

	{ eidTooManyTabs,       fFalse, fTrue,  fFalse, iIconNote,
	StKey("Too many tabs set",TooManyTabs)                              },
	

	{ eidTooManyClTabs,     fFalse, fTrue,  fFalse, iIconNote,
	StKey("Too many tabs to be cleared",TooManyClTabs)                  },
	

	{ eidNoEntries,         fFalse, fTrue,  fFalse, iIconNote,
	StKey("No index entries found",NoEntries)                           },
	

	{ eidCantCreateTemp,    fTrue,  fFalse, fFalse, iIconSevere,
	StKey("Cannot create work file",CantCreateTemp)                     },
	

	{ eidBadOpusIni,        fTrue,  fTrue,  fFalse, iIconStop,
	StKey("Not a valid WINWORD.INI file; using defaults",BadOpusIni)   },
	

	{ eidPMCantOpenWind,    fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot open a window for result: close some open windows and try again.",PMCantOpenWind) },
	

	{ eidPMFieldNested,     fFalse, fFalse, fFalse, iIconStop,
	StKey("DATA, NEXT, NEXTIF, and SKIPIF fields may not be nested in any other fields.",PMFieldNested) },
	

	{ eidPMFieldInSubDoc,   fFalse, fFalse, fFalse, iIconStop,
	StKey("DATA, NEXT, NEXTIF, and SKIPIF fields may not be in annotations, headers, footers, or footnotes.",PMFieldInSubDoc) },
	

	{ eidPMDataNotFirstPM,  fFalse, fFalse, fFalse, iIconStop,
	StKey("DATA field must precede NEXT, NEXTIF, or SKIPIF fields.",PMDataNotFirstPM) },
	

	{ eidPMNoDataFileSpec,  fFalse, fFalse, fFalse, iIconStop,
	StKey("DATA field does not contain a data file name.",PMNoDataFileSpec) },
	

	{ eidPMBadFieldCond,    fFalse, fFalse, fFalse, iIconStop,
	StKey("Syntax error in field condition",PMBadFieldCond)            },
	

	{ eidPMNoOpenDataFile,  fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot open data file",PMNoOpenDataFile)                    },
	

	{ eidPMNoOpenHeadFile,  fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot open header file",PMNoOpenHeadFile)                  },
	

	{ eidPMFirstBeyondEnd,  fFalse, fFalse, fFalse, iIconStop,
	StKey("Requested first record is beyond end of data file.",PMFirstBeyondEnd) },
	

	{ eidBadPrinter,        fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Printer error",BadPrinter)                                   },
	

	{ eidNoPrinters,        fFalse, fFalse, fFalse, iIconCaut,
	StKey("Cannot change printer; no printers installed",NoPrinters)    },
	

	{ eidInvalidRPStartAt,  fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid number in Start At",InvalidRPStartAt)            },
	

	{ eidInvalidRPFormat,   fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid number in Format",InvalidRPFormat)               },
	

	{ eidInvalidRPBoth,     fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not valid numbers in Start At and Format",InvalidRPBoth)     },
	

	{ eidNoTocFields,       fFalse, fTrue,  fFalse, iIconNote,
	StKey("No table of contents entry fields found",NoTocFields)        },
	

	{ eidNoOutlineLevels,   fFalse, fTrue,  fFalse, iIconNote,
	StKey("No heading paragraphs found",NoOutlineLevels)                },
	

	{ eidMakeSelection,     fFalse, fFalse, fFalse, iIconCaut,
	StKey("Make selection first.",MakeSelection)                         },
	

	{ eidInvalidSelection,  fFalse, fFalse, fFalse, iIconCaut,
	StKey("Not a valid selection",InvalidSelection)                     },
	

	{ eidWinFailure,        fFalse, fFalse, fTrue,  iIconStop,
	StKey("Low memory: close an application.",WinFailure)       },
	

	{ eidInvalidDMQPath,    fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid search list",InvalidDMQPath)                      },
	

	{ eidInvalidDMSrhExp,   fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid search expression",InvalidDMSrhExp)              },
	

	{ eidInvalidDMDateRange,  fFalse, fTrue,  fFalse, iIconCaut,
	StKey("Not a valid date range",InvalDMDateRange)                    },
	

	{ eidDMIncomplete,      fFalse, fTrue,  fFalse, iIconNote,
	StKey("List may be incomplete.",DMIncomplete)                        },
	

	{ eidLock,              fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot save; \004\000 is locked.",Lock)                      },
	

	{ eidReadonly,          fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot save; \004\000 is read-only.",Readonly)                },
	

	{ eidWindowTooSmall,    fFalse, fFalse, fFalse, iIconStop,
	StKey("Window is too small.",WindowTooSmall)                        },
	

	{ eidNotValidForAnn,    fFalse, fFalse, fFalse, iIconStop,
	StKey("Not a valid action for annotations",NotValidForAnn)         },
	

	{ eidStyleSheetTooLarge, fTrue, fFalse, fTrue, iIconStop,
	StKey("Style sheet definition is too large. Some styles discarded.",StyleSheetTooLarge) },
	

	{ eidNoFNToShow,        fFalse, fFalse, fFalse, iIconStop,
	StKey("No footnotes to show",NoFNToShow)                            },
	

	{ eidNoANNToShow,       fFalse, fFalse, fFalse, iIconStop,
	StKey("No annotations to show",NoANNToShow)                         },
	

	{ eidLowMemIncorrectFonts, fFalse, fFalse, fTrue,  iIconSevere,
	StKey("Low memory: fonts in copied text may be incorrect.",LowMemIncorrectFonts) },
	

	{ eidCantMoveSections,   fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot move sections into destination",CantMoveSections)     },
	

	{ eidCantMoveFNANNRefs,  fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot move footnote/annotation references into destination",CantMoveFNANNRefs) },
	

	{ eidCantReplaceFNANNRefs, fFalse, fFalse, fFalse, iIconStop,
	StKey("Cannot replace footnote/annotation references",CantReplaceFNANNRefs) },
	

	{ eidIllegalTextInTable, fFalse, fFalse, fFalse, iIconStop,
	StKey("Illegal text in table",IllegalTextInTable)                   },
	

	{ eidLowMemCloseWindows, fFalse, fFalse, fTrue,  iIconStop,
	StKey("Low memory: close extra windows and save your work.",LowMemCloseWindows) },
	

	{ eidLowMemMenu,        fFalse, fFalse, fTrue,  iIconSevere,
	StKey("Low memory: close extra windows and save your work.",LowMemMenu) },
	

	{ eidCantRealizeFont,        fFalse, fFalse, fTrue,  iIconStop,
	StKey("Low memory: cannot display requested font",CantRealizeFont) },
	

	{ eidCantCreateTempFile, fFalse, fTrue,  fFalse, iIconSevere,
	StKey("Warning: could not create work file. Check temp environment variable.",CantCreateTempFile) },
	

	{ eidNoMemSort,          fFalse, fFalse, fTrue,  iIconStop,
	StKey("sort",NoMemSort)                                             },
	

	{ eidSortFldCh,          fFalse, fFalse, fFalse, iIconNote,
	StKey("Cannot sort fields in selection",SortFldCh)                  },
	

	{ eidSortNoRec,          fFalse, fFalse, fFalse, iIconNote,
	StKey("No valid records to sort",SortNoRec)                         },
	

	{ eidBasedOnFull,        fFalse, fTrue, fFalse, iIconCaut,
	StKey("Style has too many Based On ancestors.",BasedOnFull)          },
	

	{ eidCircularBasedOn,    fFalse, fTrue, fFalse, iIconCaut,
	StKey("Style has circular Based On list.",CircularBasedOn)           },
	

	{ eidMacroTooBig,        fTrue,  fFalse, fFalse, iIconStop,
	StKey("Macro is too big.", MacroIsTooBig)                            },
	

	{ eidCantChangeMenuHelp, fTrue,  fTrue, fFalse, iIconStop,
	StKey("Cannot change menu help text for built-in commands", CantChangeMenuHelp) },
	

	{ eidCantRenDelCmd,	fTrue, fTrue, fFalse, iIconStop,
	StKey("Cannot rename or delete a built-in macro", CantRenDelCmd)    },
	
		
	{ eidCantRenDelOpenMacro, fTrue, fTrue, fFalse, iIconStop,
	StKey("Cannot rename or delete a macro that is open for editing", CantRenDelOpenMacro) },
	
		
	{ eidCantEdRecording, fTrue, fTrue, fFalse, iIconStop,
	StKey("Cannot edit macro being recorded", CantEdRecording) },
	

	{ eidCantCloseRunningMacro, fTrue,  fFalse, fFalse, iIconStop,
	StKey("Cannot close a running macro", CantCloseRunningMacro)        },
	

	{ eidNoSuchMacro, fTrue,  fTrue, fFalse, iIconStop,
	StKey("No such macro", NoSuchMacro)                                 },
	

	{ eidNoMemForRecord, fTrue,  fFalse, fTrue, iIconStop,
	StKey("record command", NoMemForRecord)                             },
	

	{ eidCantStartCBT, fTrue,  fFalse, fFalse, iIconCaut,
	StKey("Cannot start tutorial", CantStartCBT)                        },
	

	{ eidNoMemCBT, fTrue,  fFalse, fTrue, iIconSevere,
	StKey("run tutorial", NoMemCBT)                                     },
	

	{ eidNoMemTokenize, fTrue,  fFalse, fTrue, iIconSevere,
	StKey("encode macro", NoMemTokenize)                                },
	

	{ eidNoMemRunMacro, fTrue,  fFalse, fTrue, iIconSevere,
	StKey("run macro", NoMemRunMacro)                                   },
	

	{ eidCantReplaceCellMark, fTrue,  fFalse, fTrue, iIconStop,
	StKey("Cannot replace end-of-cell mark", CantReplaceCellMark)       },
	

	{ eidMacroLineTooBig, fTrue, fFalse, fFalse, iIconStop,
	StKey("Macro line is too long.", MacroLineIsTooBig)		    },
	
	
	{ eidMenuTextTooLong, fTrue, fTrue, fFalse, iIconCaut,
	StKey("Menu text is too long.", MenuTextIsTooLong)		    },
	
	
	{ eidCantRecordOverEd, fTrue, fTrue, fFalse, iIconStop,
	StKey("Cannot record over macro being edited", CantRecordOverEditedMacro) },
	

	{ eidRecordingTooBig, fTrue, fFalse, fFalse, iIconStop,
	StKey("Recorded macro was too long and has been truncated.", RecordingTooBig) },
	
		
	{ eidNoMemRecord, fTrue, fFalse, fTrue, iIconStop,
	StKey("Low memory: close extra windows and try again.", NoMemRecord) },
	

	{ eidDxaOutOfRange, fTrue, fFalse, fFalse, iIconStop,
	StKey("Value is out of range.", ValOutOfRange) },
	

	{ eidPicNoOpenDataFile,  fFalse, fFalse, fFalse, iIconSevere,
	StKey("Cannot open data file",PicNoOpenDataFile) },
	

	{ eidSortRecIgnored, fFalse, fFalse, fFalse, iIconNote,
	StKey("Skipping records with incorrect number of fields",SortRecIgnored)  },
	

	{ eidNoStack, fTrue, fTrue, fFalse, iIconSevere,
	StKey("Not enough memory to complete operation",NoStack)            },
		

	{ eidLowMemLBox, fTrue, fTrue, fFalse, iIconStop,
	StKey("Low memory: list may be incomplete.",LowMemLBox)				},
	
	{ eidCantQuitWord,  fFalse, fTrue, fFalse, iIconSevere,
	StKey("Cannot quit Microsoft Word",CantQuitWord) },
	
	{ eidCantRepag,        fFalse, fFalse, fFalse, iIconCaut,
	StKey("Cannot repaginate; no printer installed",NoPrinters)    },

	{ eidPicWrongWin,  fFalse, fFalse, fFalse, iIconSevere,
	StKey("Windows version 3.0 or greater required to display picture",PicWrongWin) },

#endif /* WIN */
#ifdef MKTGPRVW

	{ eidBetaVersion,        fTrue,  fTrue,  fFalse, iIconNote,
	StKey("This Preview Version is out of date--please upgrade to a more recent version.",BetaVersion) },

#endif /* MKTGPRVW */
#ifdef DEMO

	{ eidDemoCantSave,        fTrue,  fTrue,  fFalse, iIconNote,
	StKey("Document is too long to save using this version.",DemoCantSave) },
	
	{ eidDemoCantPrint,        fTrue,  fTrue,  fFalse, iIconNote,
	StKey("Document is too long to print using this version.",DemoCantPrint) },

#endif /* MKTGPRVW */

	{ eidBadPicFormat,        fFalse,  fFalse,  fFalse, iIconSevere,
	St("Cannot display unrecognized picture format.") },

/* this one must be last */
	{ eidNull,          fFalse, fFalse, fFalse, iIconNote,
	St("")                                                              }
	};





/* I E M D  F R O M  E I D */
/* %%Function:IemdFromEid %%Owner:peterj */
NATIVE IemdFromEid(eid)
int eid;
{
	int iemd, eidT;
#ifdef DEBUG
	static BOOL fEidsChecked = fFalse;
	if (!fEidsChecked)
		{
		CkEids();
		fEidsChecked = fTrue;
		}
#endif /* DEBUG */
	Assert(eid != eidNil);
	for (iemd = 0; (eidT = rgemd[iemd].eid) != eidNull && eidT != eid; iemd++);
	return iemd;
}


#ifdef DEBUG
/* assure there are no duplicates */
/* %%Function:CkEids %%Owner:peterj */
CkEids()
{
	int iemd1, iemd2, eid1, eid2;
	for (iemd1 = 0; (eid1 = rgemd[iemd1].eid) != eidNull; iemd1++)
		for (iemd2 = iemd1+1; (eid2 = rgemd[iemd2].eid) != eidNull; iemd2++)
			if (eid2 == eid1)
				{
				Assert(fFalse);
#ifdef WIN
				CommSzNum(SzShared("iemd1 = "), iemd1);
				CommSzNum(SzShared("iemd2 = "), iemd2);
				CommSzNum(SzShared("eid = "), eid1);
#endif /* WIN */
				}
}


#endif /* DEBUG */


/* E R R O R  I E M D  S Z  I I C O N */
/*  Work horse for error reporting.  Report message sz with icon iIcon.
*/
/* %%Function:ErrorIemdSzIIcon %%Owner:peterj */
ErrorIemdSzIIcon(iemd, sz, iIcon)
int iemd;
CHAR *sz;
int iIcon;
{
#ifdef WIN
	extern BOOL vnErrorLevel;
#endif

	if ((!rgemd[iemd].fRepeats && vmerr.fErrorAlert) || 
			(WinMac(vfInitializing, vistg == istgInit) && !rgemd[iemd].fInit)
			|| vmerr.fInhibit )
		{
		return;
		}

	if (rgemd[iemd].fMemory && vmerr.fMemFail) /* repeated "out-of-memory" ? */
		if (vmerr.fHadMemAlert)
			return;
		else 
			vmerr.fHadMemAlert = fTrue;

	if (!rgemd[iemd].fRepeats)
		vmerr.fErrorAlert = fTrue;

#ifdef WIN
		{
		extern HWND vhwndStatLine;
		extern HWND vhwndPrompt;
		extern BOOL vfHelpPromptOn;
		int mb = rgmbIcon[iIcon]|MB_OK;
		int cxtT;
		BOOL fSetPrompt = (iIcon != iIconSevere && vhwndStatLine && !vfHelpPromptOn && !vhwndPrompt
				&& rgemd[iemd].eid != eidCantFindHelp);

		cxtT = vcxtHelp;
		vcxtHelp = CxtFromEid((int)rgemd[iemd].eid);

		/* WINBUG! */
		if (iIcon == iIconSevere && vfLargeFrameEMS && vwWinVersion < 0x0299)
			mb = mb & ~MB_ICONHAND;

		if (fSetPrompt)
			DisplayHelpPrompt(fTrue);

		IdPromptBoxSz(sz, mb);

		if (fSetPrompt)
			DisplayHelpPrompt(fFalse);

		vcxtHelp = cxtT;
		}

	if (fElActive)
		/* if vnErrorLevel >= 1 then the tester does not want the macro 
		to stop when it gets to an error */
		Debug( if (vnErrorLevel < 1) )
			FlagRtError((int)rgemd[iemd].eid);

#ifdef DEBUG 
	EnterDebug();
	if (iIcon >= iIconSevere && vdbs.fRipOnError)
		/* offer to do stack trace for sys modal errors */
		if (IdMessageBoxMstRgwMb(mstStackTraceErr, NULL, MB_SYSTEMMODAL|MB_YESNO)
				== IDYES)
			DoStackTrace(SzFrame("System modal error"));
	ExitDebug();
#endif /* DEBUG */
#endif /* WIN */
#ifdef MAC
	if (iIcon >= iIconStop)
		SystemBeep(1);
	TmcAlert(attOk, rgaitIcon[iIcon], sz, 0);
#endif /* MAC */
}


/* E R R O R  E I D  P R O C */
/* Put up error message eid. */
#ifdef DEBUG
EXPORT ErrorEidProc(eid, szFunc)
char *szFunc;
#else
/* %%Function:ErrorEidProc %%Owner:peterj */
EXPORT ErrorEidProc(eid)
#endif /* DEBUG */
int eid;
{
	int iemd = IemdFromEid(eid);
	int cch = rgemd[iemd].st[0];
	char sz[cchMaxSz];

	bltbx((CHAR FAR *)&rgemd[iemd].st[1], (CHAR FAR *)sz, cch);
	sz[cch] = 0;

#ifdef WIN	/* Mac doesn't want this */
#ifdef DEBUG
	if (rgemd[iemd].iIcon < iIconSevere)
		{
		SzSzAppend(sz, SzShared(" //"));
		SzSzAppend(sz, szFunc);
		}
#endif /* DEBUG */
#endif /* WIN */

	/* Terminate CBT, since they aren't expecting and thus can't handle memory
		or severe errors */														if (!vhwndCBT && eid == eidStupidBasedOn) vrf.fRibbonCBT = fFalse;
	if (vhwndCBT && (rgemd[iemd].fMemory || rgemd[iemd].iIcon == iIconSevere))
		{
		CBTMemError(eid);
		}
	else if (!vmerr.fCBTMacroMemErr) /* eidNoMemCBT pending, don't put up another message... */
		ErrorIemdSzIIcon(iemd, sz, rgemd[iemd].iIcon);
}


#ifdef WIN  /* code depends on the Opus mst code (for building strings from
multiple elements) */

/* E R R O R  E I D  R G W  P R O C */

#ifdef DEBUG
ErrorEidRgwProc(eid, rgw, szFunc)
char *szFunc;
#else
/* %%Function:ErrorEidRgwProc %%Owner:peterj */
ErrorEidRgwProc(eid, rgw)
#endif /* DEBUG */
int eid;
int rgw[];
{
	int iemd = IemdFromEid(eid);
	CHAR szMsg[cchMaxSz];
	CHAR stMst[cchMaxSt];

	CopyCsSt(rgemd[iemd].st, stMst);
	BuildStMstRgw( Mst(0,stMst) , rgw, szMsg, cchMaxSt, hNil);
	StToSzInPlace(szMsg);

#ifdef DEBUG
	if (rgemd[iemd].iIcon < iIconSevere)
		{
		SzSzAppend(szMsg, SzShared(" //"));
		SzSzAppend(szMsg, szFunc);
		}
#endif /* DEBUG */

	ErrorIemdSzIIcon(iemd, szMsg, rgemd[iemd].iIcon);
}


/* E R R O R  E I D  W  P R O C */

#ifdef DEBUG
ErrorEidWProc(eid, w, szFunc)
char *szFunc;
#else
/* %%Function:ErrorEidWProc %%Owner:peterj */
ErrorEidWProc(eid, w)
#endif /* DEBUG */
int eid;
int w;
{
	int iemd = IemdFromEid(eid);
	CHAR szMsg[cchMaxSz];
	CHAR stMst[cchMaxSt];

	CopyCsSt(rgemd[iemd].st, stMst);
	BuildStMstRgw( Mst(0,stMst) , &w, szMsg, cchMaxSt, hNil);
	StToSzInPlace(szMsg);

#ifdef DEBUG
	if (rgemd[iemd].iIcon < iIconSevere)
		{
		SzSzAppend(szMsg, SzShared(" //"));
		SzSzAppend(szMsg, szFunc);
		}
#endif /* DEBUG */

	ErrorIemdSzIIcon(iemd, szMsg, rgemd[iemd].iIcon);
}


#endif /* WIN */


#ifdef WIN
/* E R R O R  E I D  S Z  P R O C */
/* Puts up a message box with sz as the text.  Icon, help, memory and disk
are controled by the eid. */
#ifdef DEBUG
ErrorEidSzProc(eid, sz, szFunc)
char *szFunc;
#else
/* %%Function:ErrorEidSzProc %%Owner:peterj */
ErrorEidSzProc(eid, sz)
#endif /* DEBUG */
int eid;
CHAR *sz;
{
	int iemd = IemdFromEid(eid);
	CHAR szMsg[cchMaxSz];

	szMsg[0] = 0;
	SzSzAppend(szMsg, sz);
#ifdef DEBUG
	if (rgemd[iemd].iIcon < iIconSevere)
		{
		SzSzAppend(szMsg, SzShared(" //"));
		SzSzAppend(szMsg, szFunc);
		}
#endif /* DEBUG */

	ErrorIemdSzIIcon(iemd, szMsg, rgemd[iemd].iIcon);
}


#endif /* WIN */

csconst CHAR stNoMemPrefix[] = StKey("Not enough memory to ",NoMemPrefix);

/* E R R O R  N O  M E M O R Y */
/* Put up an out of memory message for a task (Not emough memory to <foo>).
<foo> is defined by eid. */
/* %%Function:ErrorNoMemory %%Owner:peterj */
EXPORT ErrorNoMemory(eid)
int eid;
{
	int iemd = IemdFromEid(eid);
	int cch;
	CHAR szMsg[cchMaxSz];

	bltbx(stNoMemPrefix + 1, szMsg, (cch = *stNoMemPrefix));
	bltbx(rgemd[iemd].st + 1, szMsg + cch, *rgemd[iemd].st);
	cch += *rgemd[iemd].st;
	szMsg[cch] = 0;

	/* Terminate CBT, since they aren't expecting and thus can't handle memory
		or severe errors */
	if (vhwndCBT && (vmerr.mat == matCBT || !vmerr.fSentCBTMemErr))
		CBTMemError(eid);
	else
		ErrorIemdSzIIcon(iemd, szMsg, rgemd[iemd].iIcon);
}


/*  C B T  M E M  E R R O R  */
/* %%Function: CBTMemError %%Owner: rosiep */
/* If we have an out of memory error in the middle of CBT, CBT is not
   expecting it, so it will get hosed, so we must terminate CBT.  If
   we're in a macro, this must wait until the macro is cleaned up,
   because the macro could be in CBT's cbtnorm.dot, which we throw away
   when we terminate CBT.
*/
CBTMemError(eid)
int eid;
{
	if (fElActive)
		{
		/* wait to terminate CBT until macro cleaned up */
		vmerr.fCBTMacroMemErr = fTrue;
		FlagRtError(eid);  /* force macro to stop */
		}
	else
		{
		if (IsWindow(vhwndCBT) && !vmerr.fSentCBTMemErr)
			{
			SendMessage(vhwndCBT, WM_CBTTERM, vhInstance,
				MAKELONG(0, ctrOutOfMemory /* reason */));
			vmerr.fSentCBTMemErr = fTrue;
			}
		}
	vmerr.mat = matCBT;
}



/* R e p o r t  P e n d i n g  A l e r t s */
/* if an out-of-memory alert has been entered into vmerr.mat and no
	error has yet been reported, report one and set the "I reported the
	error" flag. This suppresses future error reports without clearing
	the error condition.*/
/* Called when the error should be reported, but higher level procs still need
	to know that an error has taken place */

/* %%Function:ReportPendingAlerts %%Owner:peterj */
ReportPendingAlerts()
{
	if (!vmerr.fHadMemAlert)
		{
		switch (vmerr.mat)
			{
		case matMem:
			ErrorEid(eidLowMemCloseWindows,"ReportPendingAlerts");
			break;
		case matReplace:
			ErrorEid(eidTooManyEdits,"ReportPendingAlerts");
			SetUndoNil();
			break;
		case matLow:
			ErrorNoMemory(eidNoMemOperation);
			break;
		case matDisp:
			/* we only report this once per session because otherwise
				we'd be reporting it over and over */
			if (!vmerr.fHadDispAlert)
				{
				ErrorNoMemory(eidNoMemDisplay);
				vmerr.fHadDispAlert = fTrue;
				}
			break;
		case matMenu:
			ErrorEid(eidLowMemMenu, "menu");
			break;
		case matFont:
			/* we only report this once per session because otherwise
				we'd be reporting it over and over */
			if (!vmerr.fHadDispAlert)
				{
				ErrorEid(eidCantRealizeFont, "ReportPendingAlerts");
				vmerr.fHadDispAlert = fTrue;
				}
			break;
		case matCBT:
			/* clear this now so ErrorNoMemory lets message come up */
			vmerr.mat = matNil;
			ErrorNoMemory(eidNoMemCBT);
			break;
			}
		}

	vmerr.fHadMemAlert = fTrue;
}


/* F l u s h  P e n d i n g  A l e r t s */
/* Similar to ReportPendingAlerts, but clears the error condition */
/* Called when the error report is the only action we need to take
	to act on the error */

/* %%Function:FlushPendingAlerts %%Owner:peterj */
FlushPendingAlerts()
{
	ReportPendingAlerts();
	vmerr.mat = matNil;
	vmerr.fHadMemAlert = fFalse;
}



#ifdef WIN

/* S e t  E r r o r  M a t  P r o c */
/* debugging trap for what is in the nondebug version a macro that is
	vmerr.mat = mat.  This allows us to trap the point at which the error 
	condition actually occurred. */
/* %%Function:SetErrorMatProc %%Owner:peterj */
EXPORT SetErrorMatProc(mat)
int mat;
{
	int id;
	EnterDebug();

	if (vhwndCBT && mat != matNil)
		CBTMemError(eidNoMemCBT /* anything non-zero */);
	else
		vmerr.mat = mat;
#ifdef PCJ
	CommSz(SzShared("SetErrorMat called!\r\n"));
#endif /* PCJ */

#ifdef DEBUG
	if (vdbs.fRipOnError)
		if ((id = IdMessageBoxMstRgwMb(mstStackTraceMat, &mat, 
				MB_SYSTEMMODAL|MB_YESNOCANCEL))
				== IDYES)
			{
			DoStackTrace(SzFrame("SetErrorMatProc error Trap"));
			}
		else  if (id == IDCANCEL)
			vdbs.fRipOnError = fFalse;

	ExitDebug();
#endif /* DEBUG */
}

#endif  /* WIN */


#ifdef MAC
/* S Z   S Z   A P P E N D */
/* %%Function:SzSzAppend %%Owner:NOTUSED */
SzSzAppend(szTo, szFrom)
char *szTo, *szFrom;
{
	for (; *szTo; szTo++)
		;
	while (*szTo++ = *szFrom++)
		;
}


#endif /* MAC */

/* E R R O R  E I D  S T A R T U P */
/* At startup putting up a message box may be dangerous through the normal
mechanism.  Call MessageBox directly to avoid problems. */
/* %%function:ErrorEidStartup %%owner:peterj */
ErrorEidStartup(eid)
int eid;
{
	int iemd;
	CHAR szMsg[cchMaxSz];

	if (!vmerr.fErrorAlert)
        {
	    iemd = IemdFromEid(eid);
    	Assert(rgemd[iemd].st[0] > 0 && rgemd[iemd].st[0] < cchMaxSz);
    	CopyCsSt(rgemd[iemd].st, szMsg);
    	StToSzInPlace(szMsg);
    	Yield();/* windows bug work around */
    	MessageBox(NULL, (LPSTR)szMsg, (LPSTR)szApp, MB_OK|MB_SYSTEMMODAL);
    	Yield();/* windows bug work around */
        }
}

#ifdef DEBUG
/* C M D  S C A N  E R R O R S */
/* Allows for interactive execution of error messages */
/* %%Function:CmdScanErrors %%Owner:peterj */
CMD  CmdScanErrors (pcmb)
CMB  *pcmb;
{
	extern BOOL vfHelpPromptOn;
	int  iemd;
	int  rgw [2];
	vfHelpPromptOn = fTrue;

	for (iemd=IGetStart(); rgemd[iemd].eid != eidNull; iemd++)
		{
		rgw[0] = iemd;
		rgw[1] = rgemd[iemd].eid;
		SetPromptRgwMst (mstDbgErrorIemd, rgw, pdcNotify);
		ErrorEid (rgemd[iemd].eid, "ScanErrors");
		vmerr.fErrorAlert = fFalse;
		vmerr.fHadMemAlert = fFalse;
		if ( iemd>0 && !(iemd%10) && (!FContinueScan(iemd)) )
			break;
		}
	vfHelpPromptOn = fFalse;
	return cmdOK;
}


/* %%Function:IGetStart %%Owner:peterj */
IGetStart () /* Prompts user for starting iemd value */
{
	CHAR  sz[10];
	sz[0]=0;
	if (TmcInputPmtMst (mstDbgErrorStart, sz, 10, bcmNil, mmoNormal) == tmcOK)
		{
		return max(0, WFromSzNumber(sz));
		}
	return 0;
}


/* %%Function:FContinueScan %%Owner:peterj */
FContinueScan (iemd) /*Prompts user to continue every 10th message*/
int iemd;
{
	int mb;
	int rgw[2];

	rgw[0] = iemd+1;
	rgw[1] = IemdMax();
	mb = MB_ICONQUESTION | MB_DEFBUTTON1 | MB_YESNO;
	return (IdMessageBoxMstRgwMb (mstDbgScanErrors, rgw, mb) == IDYES);
}


/* %%Function:IemdMax %%Owner:peterj */
IemdMax () /*Computes total number of Iemd*/
{
	int iemd;

	for (iemd = 0; rgemd[iemd].eid != eidNull; iemd++);
	return --iemd;
}


#endif  /* DEBUG */
