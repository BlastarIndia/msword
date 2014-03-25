#include "opuscmd.h"

/* phony bcms for Undo */

#define bcmTyping	-100
#define bcmDeleteSel	-101
#define bcmFormatting	-102
#define bcmCutBlock	-103
#define bcmPasteBlock	-104
#define bcmChangeBkmk	-105
#define bcmInsertSect	-106
#define bcmMove		-107
#define bcmInsertFld    -108
#define bcmTypingAgain  -109
#define bcmMoveUpDown	-110
#define bcmRevMark      -111
#define bcmOpenInsert	-112
#define bcmReturnSame	-113
#define bcmInsRows            -114    
#define bcmInsColumns         -115    
#define bcmInsCellsHoriz      -116
#define bcmInsCellsVert       -117
#define bcmDeleteRows         -118
#define bcmDeleteColumns      -119
#define bcmDeleteCellsHoriz   -120
#define bcmDeleteCellsVert    -121
#define bcmSplitCells         -122
#define bcmMergeCells         -123
#define bcmOtlValMouse	-124
#define bcmOtlLvlMouse	-125
#define bcmSplitTable   -126

/* for Mac compatibility, define ucms */

#define ucmNil		bcmNil
#define ucmPromote	bcmPromote
#define ucmTyping	bcmTyping
#define ucmOpenInsert	bcmOpenInsert
#define ucmReturnSame	bcmReturnSame
#define ucmPaste	bcmPaste
#define ucmCut		bcmCut
#define ucmCopy		bcmCopy
#define ucmDeleteSel	bcmDeleteSel
#define ucmFormatting	bcmFormatting
#define ucmCutBlock	bcmCutBlock
#define ucmPasteBlock	bcmPasteBlock
#define ucmChangeBkmk	bcmChangeBkmk
#define ucmInsertSect	bcmInsertSect
#define ucmFootnote	bcmInsFootnote
#define ucmMove		bcmMove
#define ucmMoveUpDown	bcmMoveUpDown
#define ucmMoveSp	bcmMoveSp
#define ucmCopySp	bcmCopySp
#define ucmDiaCopyLooks	bcmCopyLooks
#define ucmRevMark      bcmRevMark
#define ucmOutlining    imiOutline
#define ucmNewTable     imiInsTable
#define ucmFormatPosition	bcmPosition
#define ucmParagraphs           bcmParagraph
#define ucmSections             bcmSection
#define ucmDocument             bcmDocument
#define ucmInsRows              bcmInsRows
#define ucmInsColumns           bcmInsColumns
#define ucmInsCellsHoriz        bcmInsCellsHoriz
#define ucmInsCellsVert         bcmInsCellsVert
#define ucmDeleteRows           bcmDeleteRows
#define ucmDeleteColumns        bcmDeleteColumns
#define ucmDeleteCellsHoriz     bcmDeleteCellsHoriz
#define ucmDeleteCellsVert      bcmDeleteCellsVert
#define ucmSplitCells           bcmSplitCells
#define ucmMergeCells           bcmMergeCells
#define ucmFormatTable          bcmFormatTable
#define ucmOtlUp		bcmMoveUp
#define ucmOtlDown		bcmMoveDown
#define ucmOtlLeft		bcmPromote
#define ucmOtlRight		bcmDemote
#define ucmOtlSHBody		bcmToggleEllip
#define ucmOtlAll		bcmExpandAll
#define ucmOtlPlus		bcmExpand
#define ucmOtlMinus		bcmCollapse
#define ucmOtlBody		bcmConvertToBody
#define ucmOtlVal		bcmOtlValMouse
#define ucmOtlLvl		bcmOtlLvlMouse
#define ucmClear		bcmClear
#define ucmNextDr               bcmNextDr
#define ucmPrevDr               bcmPrevDr
#define ucmScrollUp             bcmPrevDr

/* The following ucm's are used by CmdOutlnUcm internally. */
#define ucmOtlBodyLeftRight	-1
#define ucmOtlPass2		-2
#define ucmOtlMinusDirect	-3

/* until we have these functions */
/* REVIEW bradch(pj) */
#define ucmDrLeft               (-1)
#define ucmDrRight              (-2)
#define ucmDrUp                 (-3)
#define ucmDrDown               (-4)
#define ucmDrFirst              (-5)
#define ucmDrLast               (-6)
#define ucmOtlMinusAll          (-7)
