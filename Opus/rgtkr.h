#define itkrNil 			 -1


csconst TKR csrgtkr[] =
	{
	eltAlias, 		 St("Alias"),
	eltAnd, 		 St("And"),
	eltAny, 		 St("Any"),
	eltAppend, 		 St("Append"),
	eltAs, 		 St("As"),
	eltBegin, 		 St("Begin"),
	eltCall, 		 St("Call"),
	eltCase, 		 St("Case"),
	eltCdecl, 		 St("Cdecl"),
	eltCheckBox, 		 St("CheckBox"),
	eltClose, 		 St("Close"),
	eltComboBox, 		 St("ComboBox"),
	eltDeclare, 		 St("Declare"),
	eltDialog, 		 St("Dialog"),
	eltDim, 		 St("Dim"),
	eltDouble, 		 St("Double"),
	eltElse, 		 St("Else"),
	eltElseif, 		 St("ElseIf"),
	eltEnd, 		 St("End"),
	eltError, 		 St("Error"),
	eltFor, 		 St("For"),
	eltFunction, 		 St("Function"),
	eltGetCurVals, 		 St("GetCurValues"),
	eltGoto, 		 St("Goto"),
	eltIf, 		 St("If"),
	eltInput, 		 St("Input"),
	eltInteger, 		 St("Integer"),
	eltIs, 		 St("Is"),
	eltLet, 		 St("Let"),
	eltLib, 		 St("Lib"),
	eltLine, 		 St("Line"),
	eltListBox, 		 St("ListBox"),
	eltLong, 		 St("Long"),
	eltMod, 		 St("Mod"),
	eltName, 		 St("Name"),
	eltNext, 		 St("Next"),
	eltNot, 		 St("Not"),
	eltOn, 		 St("On"),
	eltOpen, 		 St("Open"),
	eltOptGroup, 		 St("OptionGroup"),
	eltOr, 		 St("Or"),
	eltOutput, 		 St("Output"),
	eltPrint, 		 St("Print"),
	eltRead, 		 St("Read"),
	eltRedim, 		 St("Redim"),
	eltRem, 		 St("Rem"),
	eltResume, 		 St("Resume"),
	eltSelect, 		 St("Select"),
	eltShared, 		 St("Shared"),
	eltSingle, 		 St("Single"),
	eltStep, 		 St("Step"),
	eltStop, 		 St("Stop"),
	eltStringT, 		 St("String"),
	eltSub, 		 St("Sub"),
	eltSuper, 		 St("Super"),
	eltTextBox, 		 St("TextBox"),
	eltThen, 		 St("Then"),
	eltTo, 		 St("To"),
	eltSpell, 		 St("UtilGetSpelling"),
	eltThes, 		 St("UtilGetSynonyms"),
	eltWend, 		 St("Wend"),
	eltWhile, 		 St("While"),
	eltWrite, 		 St("Write"),
	eltLParen, 		 St("("),
	eltRParen, 		 St(")"),
	eltMul, 		 St("*"),
	eltAdd, 		 St("+"),
	eltComma, 		 St(","),
	eltSubtract, 		 St("-"),
	eltMinus, 		 St("-"),
	eltDiv, 		 St("/"),
	eltColon, 		 St(":"),
	eltSemi, 		 St(";"),
	eltLt, 		 St("<"),
	eltLe, 		 St("<="),
	eltNe, 		 St("<>"),
	eltEq, 		 St("="),
	eltGt, 		 St(">"),
	eltGe, 		 St(">="),
	};


csconst BYTE csmpeltitkr[] =
	{
	itkrNil,  /* eltNil  0 */
	69,  		 /* eltMinus  1 */
	36,  		 /* eltNot  2 */
	1,  		 /* eltAnd  3 */
	40,  		 /* eltOr  4 */
	63,  		 /* eltLParen  5 */
	64,  		 /* eltRParen  6 */
	66,  		 /* eltAdd  7 */
	68,  		 /* eltSubtract  8 */
	70,  		 /* eltDiv  9 */
	65,  		 /* eltMul  10 */
	33,  		 /* eltMod  11 */
	76,  		 /* eltEq  12 */
	75,  		 /* eltNe  13 */
	73,  		 /* eltLt  14 */
	77,  		 /* eltGt  15 */
	74,  		 /* eltLe  16 */
	78,  		 /* eltGe  17 */
	67,  		 /* eltComma  18 */
	itkrNil,  /* eltSubscript  19 */
	itkrNil,  /* eltOperand  20 */
	itkrNil,  /* eltElkGets  21 */
	itkrNil,  /* eltEndExp  22 */
	itkrNil,  /* eltEol  23 */
	46,  		 /* eltResume  24 */
	71,  		 /* eltColon  25 */
	18,  		 /* eltEnd  26 */
	53,  		 /* eltSub  27 */
	21,  		 /* eltFunction  28 */
	24,  		 /* eltIf  29 */
	56,  		 /* eltThen  30 */
	17,  		 /* eltElseif  31 */
	16,  		 /* eltElse  32 */
	61,  		 /* eltWhile  33 */
	60,  		 /* eltWend  34 */
	20,  		 /* eltFor  35 */
	57,  		 /* eltTo  36 */
	50,  		 /* eltStep  37 */
	35,  		 /* eltNext  38 */
	itkrNil,  /* eltExit  39 */
	72,  		 /* eltSemi  40 */
	6,  		 /* eltCall  41 */
	23,  		 /* eltGoto  42 */
	51,  		 /* eltStop  43 */
	37,  		 /* eltOn  44 */
	19,  		 /* eltError  45 */
	28,  		 /* eltLet  46 */
	14,  		 /* eltDim  47 */
	48,  		 /* eltShared  48 */
	47,  		 /* eltSelect  49 */
	27,  		 /* eltIs  50 */
	7,  		 /* eltCase  51 */
	4,  		 /* eltAs  52 */
	44,  		 /* eltRedim  53 */
	42,  		 /* eltPrint  54 */
	25,  		 /* eltInput  55 */
	30,  		 /* eltLine  56 */
	62,  		 /* eltWrite  57 */
	34,  		 /* eltName  58 */
	41,  		 /* eltOutput  59 */
	3,  		 /* eltAppend  60 */
	38,  		 /* eltOpen  61 */
	22,  		 /* eltGetCurVals  62 */
	13,  		 /* eltDialog  63 */
	54,  		 /* eltSuper  64 */
	12,  		 /* eltDeclare  65 */
	15,  		 /* eltDouble  66 */
	26,  		 /* eltInteger  67 */
	32,  		 /* eltLong  68 */
	49,  		 /* eltSingle  69 */
	52,  		 /* eltStringT  70 */
	8,  		 /* eltCdecl  71 */
	0,  		 /* eltAlias  72 */
	2,  		 /* eltAny  73 */
	58,  		 /* eltSpell  74 */
	59,  		 /* eltThes  75 */
	31,  		 /* eltListBox  76 */
	11,  		 /* eltComboBox  77 */
	10,  		 /* eltClose  78 */
	5,  		 /* eltBegin  79 */
	29,  		 /* eltLib  80 */
	55,  		 /* eltTextBox  81 */
	39,  		 /* eltOptGroup  82 */
	9,  		 /* eltCheckBox  83 */
	43,  		 /* eltRead  84 */
	itkrNil,  /* eltLineNolabel  85 */
	itkrNil,  /* eltLineIdent  86 */
	itkrNil,  /* eltLineInt  87 */
	itkrNil,  /* eltElt  88 */
	itkrNil,  /* eltElx  89 */
	itkrNil,  /* eltElk  90 */
	itkrNil,  /* eltNum  91 */
	itkrNil,  /* eltIdent  92 */
	itkrNil,  /* eltString  93 */
	itkrNil,  /* eltComment  94 */
	itkrNil,  /* eltInt  95 */
	itkrNil,  /* eltEof  96 */
	itkrNil,  /* eltStmtSep  97 */
	itkrNil,  /* eltBadSyntax  98 */
	itkrNil,  /* eltSpace  99 */
	45,  		 /* eltRem  100 */
	itkrNil,  /* eltHash  101 */
	itkrNil,  /* eltUserElk  102 */
	};


csconst BYTE csmpdchitkr[] =
	{
	0, 	 /* a */ 
	5, 	 /* b */ 
	6, 	 /* c */ 
	12, 	 /* d */ 
	16, 	 /* e */ 
	20, 	 /* f */ 
	22, 	 /* g */ 
	24, 	 /* h */ 
	24, 	 /* i */ 
	28, 	 /* j */ 
	28, 	 /* k */ 
	28, 	 /* l */ 
	33, 	 /* m */ 
	34, 	 /* n */ 
	37, 	 /* o */ 
	42, 	 /* p */ 
	43, 	 /* q */ 
	43, 	 /* r */ 
	47, 	 /* s */ 
	55, 	 /* t */ 
	58, 	 /* u */ 
	60, 	 /* v */ 
	60, 	 /* w */ 
	63, 	 /* x */ 
	63, 	 /* y */ 
	63, 	 /* z */
	63 
	};


#define itkrMaxAlpha 	 63
#define itkrMax	(sizeof (csrgtkr) / sizeof (TKR))
