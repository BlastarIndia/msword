// sdmver.h : SDM Version file
// Opus Version

////// Environment //////
#define SDM_ENV_WIN
// #define SDM_ENV_CW
// #define SDM_ENV_MAC
// #define SDM_ENV_PM

////// Multiple SBs ? //////
// #define SDM_MULTI_SB
// #define LBOX_MULTI_SB

////// DropDown's ? //////
#define SDM_DROPDOWN

////// OOM DoJmp()'s ? //////
// #define SDM_ERRJMP

////// CallBack Procedures //////
#define SDM_CALLBACK_PD
#define LBOX_CALLBACK_PD

////// Strings //////
#define SDM_STR_SZ
// #define SDM_STR_STZ

////// FEDT ? //////
#define SDM_FEDT

////// Debug Cover Functions? //////
#define SDM_COVER_DEBUG

////// P-Code Native Generation ? //////
#define SDM_NATIVE	NATIVE
#define LBOX_NATIVE	NATIVE
#define	BEGIN_NATIVE_BLOCK	{{
#define END_NATIVE_BLOCK	}}
#define BEGIN_PCODE_BLOCK	{{
#define	END_PCODE_BLOCK		}}


/////// temporary renaming during the OPUS conversion period //////
#define OPUSCONVERSION
#define InitCab		InitCab_sdm21
#define HcabAlloc	HcabAlloc_sdm21
#define SetTmcVal	SetTmcVal_sdm21
#define GetTmcText	GetTmcText_sdm21
#define SetTmcText	SetTmcText_sdm21
#define TmcDoDlg	TmcDoDlg_sdm21
#define FInitSdm	FInitSdm_sdm21
#define EnableTmc	EnableTmc_sdm21
#define FEnabledTmc	FEnabledTmc_sdm21


// for temporary API extensions for Opus only
#define OPUSBOGUS
