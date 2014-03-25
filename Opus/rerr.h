/* R E R R . H */
/*  App specific runtime error strings for macro statements
*/

#define rerrAppMin	1000
#define rerrMiscMin	500
#define rerrInterpMin	100

#define rerrNil		0
#define rerrHalt	-1	/* halt request, not an error */
#define rerrStop	-2
#define rerrSuspend	-3

#define rerrReturnWithoutGosub		3
#define rerrOutOfData			4
#define rerrIllegalFunctionCall		5
#define rerrOverflow			6
#define rerrOutOfMemory			7
#define rerrSubscriptOutOfRange		9
#define rerrDivisionByZero		11
#define rerrOutOfStringSpace		14
#define rerrStringFormulaTooComplex	16
#define rerrNoResume			19
#define rerrResumeWithoutError		20
#define rerrDeviceTimeout		24
#define rerrDeviceFault			25
#define rerrOutOfPaper			27
#define rerrCaseElseExpected		39
#define rerrVariableRequired		40
#define rerrFieldOverflow		50
#define rerrInternalError		51
#define rerrBadFileNameOrNumber		52
#define rerrFileNotFound		53
#define rerrBadFileMode			54
#define rerrFileAlreadyOpen		55
#define rerrFieldStatementActive	56
#define rerrDeviceIOError		57
#define rerrFileAlreadyExists		58
#define rerrBadRecordLength		59
#define rerrDiskFull			61
#define rerrInputPastEndOfFile		62
#define rerrBadRecordNumber		63
#define rerrBadFileName			64
#define rerrTooManyFiles		67
#define rerrDeviceUnavailable		68
#define rerrCommunicationBufferOverflow	69
#define rerrPermissionDenied		70
#define rerrDiskNotReady		71
#define rerrDiskMediaError		72
#define rerrAdvancedFeatureUnavailable	73
#define rerrRenameAcrossDisks		74
#define rerrPathFileAccessError		75
#define rerrPathNotFound		76

#define rerrSyntaxError			rerrInterpMin + 0
#define rerrCommaMissing		rerrInterpMin + 1
#define rerrCommandFailed		rerrInterpMin + 2
#define rerrDlgRecVarExpected		rerrInterpMin + 3
#define rerrElseWithoutIf		rerrInterpMin + 4
#define rerrEndIfWithoutIf		rerrInterpMin + 5
#define rerrInputMissing		rerrInterpMin + 9
#define rerrNoResumeNext		rerrInterpMin + 10
#define rerrExpressionTooComplex	rerrInterpMin + 11
#define rerrIdentifierExpected		rerrInterpMin + 12
#define rerrDuplicateLabel		rerrInterpMin + 13
#define rerrLabelNotFound		rerrInterpMin + 14
#define rerrMissingRightParen		rerrInterpMin + 15
#define rerrArgCountMismatch		rerrInterpMin + 16
#define rerrMissingNextOrWend		rerrInterpMin + 17
#define rerrNestedSubOrFunc		rerrInterpMin + 18
#define rerrNextWithoutFor		rerrInterpMin + 19
#define rerrArrayAlreadyDimensioned	rerrInterpMin + 20
#define rerrDuplicateDef		rerrInterpMin + 21
#define rerrTypeMismatch		rerrInterpMin + 22
#define rerrUndefDlgRecField		rerrInterpMin + 23
#define rerrSubprogNotDefined		rerrInterpMin + 24
#define rerrUnexpectedEndOfMacro	rerrInterpMin + 25
#define rerrWendWithoutWhile		rerrInterpMin + 26
#define rerrWrongNumOfDimensions	rerrInterpMin + 27
#define rerrNoCurOptionButtonGroup	rerrInterpMin + 28
#define rerrTooManyCtrlStructs		rerrInterpMin + 29
#define rerrMissingEndSelect		rerrInterpMin + 30

#define rerrCannotInitiate      	rerrMiscMin + 0
#define rerrBogusDcl	    		rerrMiscMin + 1
#define rerrNoResponse   		rerrMiscMin + 2
#define rerrAppNacked	    		rerrMiscMin + 3
#define rerrNoSuchWin	    		rerrMiscMin + 4
#define rerrCantActivate		rerrMiscMin + 5
#define rerrCannotSendKeys   		rerrMiscMin + 6
#define rerrAppBusyed	    		rerrMiscMin + 8
#define rerrModeError			rerrMiscMin + 9
#define rerrBadName			rerrMiscMin + 10
#define rerrNoMacro			rerrMiscMin + 11
#define rerrOutOfRange			rerrMiscMin + 12
#define rerrStringTooBig		rerrMiscMin + 13
#define rerrDocNotOpen			rerrMiscMin + 14
#define rerrBadParam			rerrIllegalFunctionCall
#define rerrUnableToLoadSpeller		rerrMiscMin + 28
#define rerrCannotOpenDictionary	rerrMiscMin + 29
#define rerrDlgBoxTooComplex		rerrMiscMin + 30


#ifdef RERRSTRINGS
typedef struct _erd
	{
	int rerr;
	CHAR sz [];
	} ERD;


csconst ERD rgerd [] =
	{
	/* 0 */
	/* 1 */
	/* 2 */
	rerrReturnWithoutGosub,      SzKey("RETURN without GOSUB",ReturnWithoutGosub),
	rerrOutOfData,		SzKey("Out of DATA",OutOfData),
	rerrIllegalFunctionCall,	SzKey("Illegal function call",IllegalFunctionCall),
	rerrOverflow,		SzKey("Overflow",Overflow),
	rerrOutOfMemory,		SzKey("Out of memory",OutOfMemory),
	/* 8 */
	rerrSubscriptOutOfRange,	SzKey("Subscript out of range",SubscriptOutOfRange),
	/* 10 */
	rerrDivisionByZero,		SzKey("Division by zero",DivisionByZero),
	/* 12 */
	/* 13 */
	rerrOutOfStringSpace,	SzKey("Out of string space",OutOfStringSpace),
	/* 15 */
	rerrStringFormulaTooComplex,	SzKey("String formula too complex",StringFormulaTooComplex),
	/* 17 */
	/* 18 */
	rerrNoResume,		SzKey("No RESUME",NoResume),
	rerrResumeWithoutError,	SzKey("RESUME without ERROR",ResumeWithoutError),
	/* 21 */
	/* 22 */
	/* 23 */
	rerrDeviceTimeout,		SzKey("Device timeout",DeviceTimeout),
	rerrDeviceFault,		SzKey("Device fault",DeviceFault),
	/* 26 */
	rerrOutOfPaper,		SzKey("Out of paper",OutOfPaper),
	/* 28 */
	/* 29 */
	/* 30 */
	/* 31 */
	/* 32 */
	/* 33 */
	/* 34 */
	/* 35 */
	/* 36 */
	/* 37 */
	/* 38 */
	rerrCaseElseExpected,	SzKey("CASE ELSE expected",CaseElseExpected),
	rerrVariableRequired,	SzKey("Variable required",VariableRequired),
	/* 41 */
	/* 42 */
	/* 43 */
	/* 44 */
	/* 45 */
	/* 46 */
	/* 47 */
	/* 48 */
	/* 49 */
	rerrFieldOverflow,		SzKey("FIELD overflow",FieldOverflow),
	rerrInternalError,		SzKey("Internal error",InternalError),
	rerrBadFileNameOrNumber,	SzKey("Bad file name or number",BadFileNameOrNumber),
	rerrFileNotFound,		SzKey("File not found",FileNotFound),
	rerrBadFileMode,		SzKey("Bad file mode",BadFileMode),
	rerrFileAlreadyOpen,		SzKey("File already open",FileAlreadyOpen),
	rerrFieldStatementActive,	SzKey("FIELD statement active",FieldStatementActive),
	rerrDeviceIOError,		SzKey("Device I/O error",DeviceIOError),
	rerrFileAlreadyExists,	SzKey("File already exists",FileAlreadyExists),
	rerrBadRecordLength,		SzKey("Bad record length",BadRecordLength),
	/* 60 */
	rerrDiskFull,		SzKey("Disk full",DiskFull),
	rerrInputPastEndOfFile,	SzKey("Input past end of file",InputPastEndOfFile),
	rerrBadRecordNumber,		SzKey("Bad record number",BadRecordNumber),
	rerrBadFileName,		SzKey("Bad file name",BadFileName),
	/* 65 */
	/* 66 */
	rerrTooManyFiles,		SzKey("Too many files",TooManyFiles),
	rerrDeviceUnavailable,	SzKey("Device unavailable",DeviceUnavailable),
	rerrCommunicationBufferOverflow, SzKey("Communication-buffer overflow",CommBufferOverflow),
	rerrPermissionDenied,	SzKey("Permission denied",PermissionDenied),
	rerrDiskNotReady,		SzKey("Disk not ready",DiskNotReady),
	rerrDiskMediaError,		SzKey("Disk-media error",DiskMediaError),
	rerrAdvancedFeatureUnavailable,  SzKey("Advanced feature unavailable",AdvancedFeatureUnavailable),
	rerrRenameAcrossDisks,	SzKey("Rename across disks",RenameAcrossDisks),
	rerrPathFileAccessError,	SzKey("Path/File access error",PathFileAccessError),
	rerrPathNotFound,		SzKey("Path not found",PathNotFound),

	rerrSyntaxError,		SzKey("Syntax error",SyntaxError),
	rerrCommaMissing,		SzKey("Comma missing",CommaMissing),
	rerrCommandFailed,		SzKey("Command failed",CommandFailed),
	rerrDlgRecVarExpected,	SzKey("Dialog record variable expected",DlgRecVarExpected),
	rerrElseWithoutIf,		SzKey("ELSE without IF",ElseWithoutIf),
	rerrEndIfWithoutIf,		SzKey("END IF without IF",EndIfWithoutIf),
	rerrInputMissing,		SzKey("INPUT missing",InputMissing),
	rerrNoResumeNext,		SzKey("No RESUME NEXT",NoResumeNext),
	rerrExpressionTooComplex,	SzKey("Expression too complex",ExpressionTooComplex),
	rerrIdentifierExpected,	SzKey("Identifier expected",IdentifierExpected),
	rerrDuplicateLabel,		SzKey("Duplicate label",DuplicateLabel),
	rerrLabelNotFound,		SzKey("Label not found",LabelNotFound),
	rerrMissingRightParen,	SzKey("Right parenthesis missing",MissingRightParen),
	rerrArgCountMismatch,    	SzKey("Argument-count mismatch",ArgCountMismatch),
	rerrMissingNextOrWend,	SzKey("Missing NEXT or WEND",MissingNextOrWend),
	rerrNestedSubOrFunc,		SzKey("Nested SUB or FUNCTION definitions",NestedSubOrFunc),
	rerrNextWithoutFor,		SzKey("NEXT without FOR",NextWithoutFor),
	rerrArrayAlreadyDimensioned,	SzKey("Array already dimensioned",ArrayAlreadyDimensioned),
	rerrDuplicateDef,		SzKey("Duplicate definition",DuplicateDef),
	rerrTypeMismatch,		SzKey("Type mismatch",TypeMismatch),
	rerrUndefDlgRecField,    	SzKey("Undefined dialog record field",UndefDlgRecField),
	rerrSubprogNotDefined,	SzKey("Undefined SUB or FUNCTION",SubprogNotDefined),
	rerrUnexpectedEndOfMacro,	SzKey("Unexpected end of macro",UnexpectedEndOfMacro),
	rerrWendWithoutWhile,    	SzKey("WEND without WHILE",WendWithoutWhile),
	rerrWrongNumOfDimensions,	SzKey("Wrong number of dimensions",WrongNumOfDimensions),
	rerrNoCurOptionButtonGroup,	SzKey("No current option button group",NoCurOptionButtonGroup),
	rerrTooManyCtrlStructs,	SzKey("Too many nested control structures", TooManyCtrlStricts),
	rerrMissingEndSelect,	SzKey("SELECT without END SELECT", MissingEndSelect),

	/* rerrMiscMin */
	rerrCannotInitiate,      	SzKey("Cannot initiate link",CannotInitiateLink),
	rerrBogusDcl,    		SzKey("Invalid channel number",InvalidChannelNumber),
	rerrNoResponse,   		SzKey("Application does not respond",AppDoesNotRespond),
	rerrAppNacked,	    	SzKey("Process failed in other application",ProcessFailOtherApp),
	rerrNoSuchWin,	    	SzKey("Window does not exist",WindowDoesNotExist),
	rerrCantActivate,		SzKey("Cannot activate application",CannotActivateApp),
	rerrCannotSendKeys,      	SzKey("Cannot send keys",CannotSendKeys),
	rerrAppBusyed,	    	SzKey("Other application is busy",OtherAppBusy),
	rerrModeError,	    	SzKey("Command is unavailable",OperationOutOfContext),
	rerrBadName,			SzKey("Illegal name",IllegalName),
	rerrNoMacro,			SzKey("No such macro",NoSuchMacro),
	rerrOutOfRange,	    	SzKey("Value out of range",ValueOutOfRange),
	rerrStringTooBig,		SzKey("String too long",StringTooLong),
	rerrDocNotOpen,	    	SzKey("Document not open",DocumentNotOpen),
	rerrBadParam,	    	SzKey("Bad parameter",BadParameter),
	rerrUnableToLoadSpeller, 	SzKey("Unable to load Speller",UnableToLoadSpeller),
	rerrCannotOpenDictionary, 	SzKey("Cannot open dictionary",CannotOpenDictionary),
	rerrDlgBoxTooComplex,	SzKey("Dialog Box description too complex",DlgBoxTooComplex),

	-1, "" /* END MARKER */
	};
#endif	/* RERRSTRINGS */




