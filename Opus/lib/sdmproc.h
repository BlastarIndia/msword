///////////////////////////////////////////////////////////////////////////////
// File sdmproc.h:							     //
// -- SDM Procedure templates for all Public SDM entries.		     //
///////////////////////////////////////////////////////////////////////////////

#ifndef NO_SDM

///////////////////////////////////////////////////////////////////////////////
// Common calls.

// Initialization and main control.
#ifdef	DEBUG
VOID	SDMPUBLIC	EnableReports(BOOL);
#else	//DEBUG
#define			EnableReports(f)
#endif	//!DEBUG

#ifdef	SDM_ENV_PM
FTME	SDMPUBLIC	FtmeIsSdmMessage(PQMSG);
#endif	//SDM_ENV_PM
#ifdef	SDM_ENV_WIN
FTME	SDMPUBLIC	FtmeIsSdmMessage(LPMSG);
#endif	//SDM_ENV_WIN
#ifdef	SDM_ENV_MAC
FTME	SDMPUBLIC	FtmeIsSdmMessage(EventPtr);
#endif	//SDM_ENV_MAC

#ifdef	SDM_ENV_WIN_PM
VOID	SDMPUBLIC	ChangeColors(void);
#else	//SDM_ENV_WIN_PM
#define	ChangeColors()			AssertSz(fFalse, "Not yet implemented")
#endif	//!SDM_ENV_WIN_PM

BOOL	SDMPUBLIC	FInitSdm(SDI *);
VOID	SDMPUBLIC	EndSdm(void);

// Help Support.
WORD	SDMPUBLIC	HidOfDlg(HDLG);

// CAB Control.
HCAB	SDMPUBLIC	HcabAlloc(WORD);
VOID	SDMPUBLIC	InitCab(HCAB, WORD);
VOID	SDMPUBLIC	ReinitCab(HCAB, WORD);
VOID	SDMPUBLIC	FreeCab(HCAB);
VOID	SDMPUBLIC	FreeCabData(HCAB);
PCAB	SDMPUBLIC	PcabLockCab(HCAB);
VOID	SDMPUBLIC	UnlockCab(HCAB);
BOOL	SDMPUBLIC	FSetCabSt(HCAB, STZ_CAB, WORD);
VOID	SDMPUBLIC	GetCabStz(HCAB, STZ_CAB, WORD, WORD);
BOOL	SDMPUBLIC	FSetCabSz(HCAB, SZ_CAB, WORD);
VOID	SDMPUBLIC	GetCabSz(HCAB, SZ_CAB, WORD, WORD);
BOOL	SDMPUBLIC	FSetCabRgb(HCAB, RGB_CAB, WORD, WORD);
VOID	SDMPUBLIC	GetCabRgb(HCAB, RGB_CAB, WORD, WORD);
VOID	SDMPUBLIC	GetCabSt(HCAB, STZ_CAB, WORD, WORD);

// Dialog Control.
#ifdef	SDM_ENV_WIN_PM
WORD	SDMPUBLIC	IdDoMsgBox(char FAR *, char FAR *, WORD);
BOOL	SDMPUBLIC	FSetDlgSab(WORD);
#else	//SDM_ENV_WIN_PM
#define	IdDoMsgBox(lszText, lszCaption, mb)	\
	AssertSz(fFalse, "Not yet implemented");
#define FSetDlgSab(sab)			AssertSz(fFalse, "Not yet impleneted")
#endif	//!SDM_ENV_WIN_PM

TMC	SDMPUBLIC	TmcDoDlg(struct _dlt **, HCAB, BYTE *);
TMC	SDMPUBLIC	TmcDoDlgDli(struct _dlt **, HCAB, DLI *);
HDLG	SDMPUBLIC	HdlgStartDlg(struct _dlt **, HCAB, DLI *);
VOID	SDMPUBLIC	EndDlg(TMC);
BOOL	SDMPUBLIC	FFreeDlg(void);
WORD	SDMPUBLIC	SabGetDlg(void);

VOID	SDMPUBLIC	SetTmcVal(TMC, WORD);
WORD	SDMPUBLIC	ValGetTmc(TMC);
VOID	SDMPUBLIC	GetTmcLargeVal(TMC, VOID *, WORD);
BOOL	SDMPUBLIC	FSetTmcLargeVal(TMC, VOID *);
VOID	SDMPUBLIC	SetTmcText(TMC, char *);
VOID	SDMPUBLIC	GetTmcText(TMC, char *, WORD);
WORD	SDMPUBLIC	CchGetTmcText(TMC, STR, WORD);

VOID	SDMPUBLIC	SetFocusTmc(TMC);
TMC	SDMPUBLIC	TmcGetFocus(VOID);

VOID	SDMPUBLIC	SetTmcTxs(TMC, TXS);
TXS	SDMPUBLIC	TxsGetTmc(TMC);
VOID	SDMPUBLIC	RedisplayTmc(TMC);

BOOL	SDMPUBLIC	FEnabledTmc(TMC);
VOID	SDMPUBLIC	EnableTmc(TMC, BOOL);
VOID	SDMPUBLIC	EnableNoninteractiveTmc(TMC, BOOL);



///////////////////////////////////////////////////////////////////////////////
// Rare Calls.

// CAB Control.
HCAB	SDMPUBLIC	HcabFromDlg(BOOL);
VOID	SDMPUBLIC	NinchCab(HCAB);

// Dialog Control.
HDLG	SDMPUBLIC	HdlgSetCurDlg(HDLG);
VOID	SDMPUBLIC	ShowDlg(BOOL);
BOOL	SDMPUBLIC	FVisibleDlg(void);
VOID	SDMPUBLIC	ResizeDlg(int, int);
VOID	SDMPUBLIC	MoveDlg(int, int);
VOID	SDMPUBLIC	SdmScaleRec(REC *);
HDLG	SDMPUBLIC	HdlgSetFocusDlg(HDLG);

VOID	SDMPUBLIC	GetListBoxEntry(TMC, WORD, STR, WORD);
WORD	SDMPUBLIC	CchGetListBoxEntry(TMC, WORD, STR, WORD);

VOID	SDMPUBLIC	AddListBoxEntry(TMC, STR);
VOID	SDMPUBLIC	InsertListBoxEntry(TMC, STR, WORD);
VOID	SDMPUBLIC	DeleteListBoxEntry(TMC, WORD);
WORD	SDMPUBLIC	CentryListBoxTmc(TMC);
VOID	SDMPUBLIC	StartListBoxUpdate(TMC);
VOID	SDMPUBLIC	BeginListBoxUpdate(TMC, BOOL);
VOID	SDMPUBLIC	EndListBoxUpdate(TMC);
WORD	SDMPUBLIC	IEntryFindListBox(TMC, STR, WORD *);



///////////////////////////////////////////////////////////////////////////////
// Very Rare.

// CAB Control.
HCAB	SDMPUBLIC	HcabDupeCab(HCAB);

// Dialog Control.
BOOL	SDMPUBLIC	FKillDlgFocus(void);
BOOL	SDMPUBLIC	FModalDlg(HDLG);
BOOL	SDMPUBLIC	FIsDlgDying(VOID);
VOID	SDMPUBLIC	ClearListError(HDLG);

WORD	SDMPUBLIC	CselListBoxTmc(TMC);
TMV	SDMPUBLIC	TmvGetTmc(TMC);
BOOL	SDMPUBLIC	FReturnDlgControl(TMC, BOOL);
VOID	SDMPUBLIC	SetDefaultTmc(TMC);
TMC	SDMPUBLIC	TmcGetDefault(BOOL);

#ifdef	SDM_DROPDOWN
TMC	SDMPUBLIC	TmcGetDropped(VOID);
#endif	//SDM_DROPDOWN

#ifdef	SDM_FEDT
VOID	SDMPUBLIC	SetSecretEditTmc(TMC);
#endif	//SDM_FEDT

#ifdef	SDM_ENV_WIN_PM
FLBF_LBOX SDMPUBLIC	FlbfFillDirListTmc(char *, char *, TMC, TMC, TMC, FDIR,
			    FDIR);
#endif	//SDM_ENV_WIN_PM

BOOL	SDMPUBLIC	FDirSelectTmc(TMC, STR);
BOOL	SDMPUBLIC	FSetDirText(STR);
char **	SDMPUBLIC	PszGetDirText(VOID);
VOID	SDMPUBLIC	CompleteComboTmc(TMC);
VOID	SDMPUBLIC	LimitTextTmc(TMC, WORD);
VOID	SDMPUBLIC	SetVisibleTmc(TMC, BOOL);
BOOL	SDMPUBLIC	FIsVisibleTmc(TMC);
VOID	SDMPUBLIC	GetTmcRec(TMC, REC *);
HWND	SDMPUBLIC	HwndOfTmc(TMC);
BOOL	SDMPUBLIC	FIsDlgInteractive(VOID);
VOID	SDMPUBLIC	SetDlgCaption(char *);

#ifdef	SDM_ENV_WIN_PM
HLBX	SDMPUBLIC	HlbxFromTmc(TMC);
HDLG	SDMPUBLIC	HdlgFromHwnd(HWND);
HWND	SDMPUBLIC	HwndSwapSdmParent(HWND);
HWND	SDMPUBLIC	HwndFromDlg(HDLG);
WORD	SDMPUBLIC	CchGetTmc(TMC);

#ifdef	SDM_ENV_WIN
VOID	SDMPUBLIC	SetEditTmcHandle(TMC, HANDLE);
HANDLE	SDMPUBLIC	HGetEditTmc(TMC);
#endif	//SDM_ENV_WIN
#else	//SDM_ENV_WIN_PM
#define	HdlgFromHwnd(hwnd)	AssertSz(fFalse, "Not yet implemented")
#define	HwndSwapSdmParent(hwnd)	AssertSz(fFalse, "Not yet implemented")
#define HwndFromDlg(hdlg)	AssertSZ(fFalse, "Not yet implemented")
#endif	//!SDM_ENV_WIN



///////////////////////////////////////////////////////////////////////////////
// Restore state.

BOOL	SDMPUBLIC	FRestoreDlg(BOOL);
BOOL	SDMPUBLIC	FRestoreTmc(TMC, BOOL);



///////////////////////////////////////////////////////////////////////////////
// EB/EL Support.

VOID	SDMPUBLIC	SaveCabs(PFN_SAVECAB, BOOL);
BOOL	SDMPUBLIC	FSetNoninteractive(WORD, TMC);
VOID	SDMPUBLIC	EndSdmTranscription(VOID);
BOOL	SDMPUBLIC	FExecutable(VOID);



///////////////////////////////////////////////////////////////////////////////
// CBT Support.

VOID	SDMPUBLIC	CBTState(BOOL);



///////////////////////////////////////////////////////////////////////////////
// Profiling support.

#ifdef	SDM_ENV_WIN
#ifdef	PROFILE
VOID	SDMPUBLIC	MeasureTime(VOID);
VOID	SDMPUBLIC	ReportTime(VOID);
VOID	SDMPUBLIC	FlushSegs(VOID);
#endif	//PROFILE
#endif	//SDM_ENV_WIN

#endif //!NO_SDM



///////////////////////////////////////////////////////////////////////////////
// Common interface to button drawing code.

#ifdef	SDM_ENV_WIN_PM
// Win 3.0 / PM 1.2 (and higher) PushButton drawer.
WORD SDM_CALLBACK	WRenderNewPush(TMM, RDS *, FTMS, FTMS, TMC, WORD);

WORD SDM_CALLBACK	WRenderPush(TMM, RDS *, FTMS, FTMS, TMC, WORD);
WORD SDM_CALLBACK	WRenderRadio(TMM, RDS *, FTMS, FTMS, TMC, WORD);
WORD SDM_CALLBACK	WRenderCheck(TMM, RDS *, FTMS, FTMS, TMC, WORD);
WORD SDM_CALLBACK	WRenderGroup(TMM, RDS *, FTMS, FTMS, TMC, WORD);
#endif	//SDM_ENV_WIN_PM
