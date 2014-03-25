/* D E B U G . H */

/* W I N D O W S  A L L O C  F A I L U R E S */
#include "debugwin.h"

/* this is to ease the pain of who has and who doesn't have PROFILE
	defined for putobj/takeobj purposes.
*/
#ifdef DEBUG
#ifndef PROFILE
#define PROFILE
#endif /* PROFILE */
#endif /* DEBUG */
#ifdef HYBRID
#ifndef PROFILE
#define PROFILE
#endif /* PROFILE */
#endif /* HYBRID */
#ifndef DEBUG
#ifndef HYBRID
#ifdef PROFILE
#undef PROFILE
#endif /* PROFILE */
#endif /* !HYBRID */
#endif /* !DEBUG */

#define wUnusedHeapWord 0xFFFF  /* sorry, BV, needed an odd number */

/* number of characters of extra global space for debugging routines */
#define cchDebug        2048

#ifdef DEBUG
#define ShowLbs(a, b) (!vdbs.fShowLbs ? 0 : ShowLbsProc(a, b))
#else
#define ShowLbs(a, b)
#endif
#ifdef DEBUG

#define Scribble(a, b) (!vdbs.grpfScribble ? 0 : ScribbleProc(a, b))
#else
#define Scribble(a, b)
#endif

/* S C R I B B L E  P O S I T I O N S */
#define ispNil          -1   /* set isp to ispNil to disable */
/* continious operations */
#define ispIdle1        1
#define ispIdle2        0
#define ispIdleComplete 2
#define ispInsertLine   3

/* frequent operations */
#define ispInsertMode   7
#define ispUpdateWw     6
#define ispQuickSave    5
#define ispSavePass     6
#define ispInitStructs  6
#define ispDisplayFli   5
#define ispAdjustCp     5
#define ispFormatLine   5
#define ispWireFont     4

/* format operations */
#define ispFieldFormat1 4
#define ispFieldFormat2 3
#define ispFieldParse   3

/* layout/pagination/printing operations */
#define ispLayout1      11
#define ispLayout2      10
#define ispLayout3      9
#define ispLayout4      8
#define ispUpdateHplcpgd 13
#define ispDispLayoutRC 12

/* infrequent operations */
#define ispDdeIdle      13
#define ispDdeWait      12
#define ispTest         14
#define ispRefreshSpeed 14
#define ispFieldCalc1   15
#define ispFieldCalc2   14
#define ispGraphics     12

/* following can be removed when all DebugFileHeaders are gone */
#define DebugFileHeader

/* A S S E R T */
/* Assert will stop if the argument is false */
#ifdef DEBUG
#define Assert(f) ((f) ? 0 : AssertProc((CHAR *)szAssertFile,__LINE__))
#define AssertX(f) Assert(f)
#define AssertSz(f,sz) ((f) ? 0 : AssertSzProc(SzFrame(sz),(CHAR *)szAssertFile,__LINE__))
#define AssertH(h) AssertHSzLn(h,(CHAR *)szAssertFile,__LINE__)
#define AssertDo(f) Assert(f)
#else
#define AssertX(f)  	(0)
#define Assert(f)
#define AssertH(h)
#define AssertDo(f)     (f)
#define AssertSz(f,sz)
#endif

/* A S S E R T  P E N V */
#ifdef DEBUG
#define AssertPenv(penv) \
{   	    	    	    \
extern ENV *penvBase;	    \
Assert (penv != penvBase);  \
Assert (*(int *)penv);	    \
}
#else
#define AssertPenv(penv)
#endif

/* R E P O R T*/
#ifdef DEBUG
#define ReportSz(sz) ( ReportSzProc(SzFrame(sz), (CHAR *)szAssertFile,__LINE__))
#else
#define ReportSz(sz)
#endif

/* R E P O R T F */
/* ReportF will stop if the argument is false */
#ifdef DEBUG
#define ReportF(f, id) ((f) ? 0 : ReportProc(id, (CHAR *)szAssertFile,__LINE__))
#else
#define ReportF(f, id)
#endif

/* N A T I V E   V E R I F I C A T I O N */

#ifdef DEBUG
#define FetchCp(doc,cp,fcm) \
		S_FetchCp(doc,cp,fcm)
#define CachePara(doc,cp) \
		S_CachePara(doc,cp)
#define FormatLineDxa(ww,doc,cp,dxa) \
		S_FormatLineDxa(ww,doc,cp,dxa)
#define LoadFont(pchp, fWidth) \
		S_LoadFont(pchp, fWidth)
#define CpSearchSz(pbmib, cpFirst, cpLim, pcpNextRpt, pppr) \
		S_CpSearchSz(pbmib, cpFirst, cpLim, pcpNextRpt, pppr)
#define CpSearchSzBackward(pbmib, cpFirst, cpLim) \
		S_CpSearchSzBackward(pbmib, cpFirst, cpLim)
#define FfcFormatFieldPdcp(pdcp, ww, doc, cp, ch) \
		S_FfcFormatFieldPdcp(pdcp, ww, doc, cp, ch)
#define DcpSkipFieldChPflcd(ch, pflcd, fShowResult, fFetch) \
		S_DcpSkipFieldChPflcd(ch, pflcd, fShowResult, fFetch)
#define FShowResultPflcdFvc(pflcd, fvc) \
		S_FShowResultPflcdFvc(pflcd, fvc)
#define IfldFromDocCp(doc, cp, fMatch) \
		S_IfldFromDocCp(doc, cp, fMatch)
#define FillIfldFlcd(hplcfld, ifld, pflcd) \
		S_FillIfldFlcd(hplcfld, ifld, pflcd)
#define GetIfldFlcd(doc, ifld, pflcd) \
		S_GetIfldFlcd(doc, ifld, pflcd)
#define SetFlcdCh(doc, pflcd, ch) \
		S_SetFlcdCh(doc, pflcd, ch)
#define IfldInsertDocCp(doc, cp) \
		S_IfldInsertDocCp(doc, cp)
#define FetchVisibleRgch(pfvb, fvc, fProps, fNested) \
		S_FetchVisibleRgch(pfvb, fvc, fProps, fNested)
#define FetchCpPccpVisible(doc, cp, pccp, fvc, fNested) \
		S_FetchCpPccpVisible(doc, cp, pccp, fvc, fNested)
#define CpVisibleCpField(doc, cp, fvc, fIns) \
		S_CpVisibleCpField(doc, cp, fvc, fIns)
#define FVisibleCp(ww, doc, cp) \
		S_FVisibleCp(ww, doc, cp)
#define DisplayFliCore( ww, rcClip, xl, yl ) \
		S_DisplayFliCore( ww, rcClip, xl, yl )
#define FMarkLine(psel, hpldr, idr, pedl, cpFirstSel, cpLimSel, cpFirstLine) \
		S_FMarkLine(psel, hpldr, idr, pedl, cpFirstSel, cpLimSel, cpFirstLine)
#define XpFromDcp(dcp1, dcp2, pxpFirst, pich) \
		S_XpFromDcp(dcp1, dcp2, pxpFirst, pich)
#define MapStc(pdod, stc, pchp, ppap) \
		S_MapStc(pdod, stc, pchp, ppap)
#define AddVisiSpaces(hdc, xpLine, ypLine, prcw) \
		S_AddVisiSpaces(hdc, xpLine, ypLine, prcw)
#define ApplyPrlSgc(hpprl, cch, prgbProps, sgc) \
		S_ApplyPrlSgc(hpprl, cch, prgbProps, sgc)
#define FGetParaState(fAll, fAbortOk) \
		S_FGetParaState(fAll, fAbortOk)
#define FGraphicsFcidToPlf(fcid, plf, fPrinterFont) \
		S_FGraphicsFcidToPlf(fcid, plf, fPrinterFont)
#define PnFromPlcbteFc(hplcbte, fc) \
		S_PnFromPlcbteFc(hplcbte, fc)
#define BFromFc(hpfkp, fc, pfcFirst, pfcLim, pifc) \
		S_BFromFc(hpfkp, fc, pfcFirst, pfcLim, pifc)
#define HpchFromFc(fn, fc) \
		S_HpchFromFc(fn, fc)
#define HpchGetPn(fn, pn) \
		S_HpchGetPn(fn, pn)
#define IbpCacheFilePage(fn, pn) \
		S_IbpCacheFilePage(fn, pn)
#define DoPrmSgc(prm, prgbProps, sgc) \
		S_DoPrmSgc(prm, prgbProps, sgc)
#define LoadFcidFull( fcid ) \
		S_LoadFcidFull( fcid )
#define ResetFont(fPrinterFont) \
		S_ResetFont(fPrinterFont)
#define DisplayFli( ww, hpldr, idr, dl, ypLine ) \
		S_DisplayFli( ww, hpldr, idr, dl, ypLine )
#define FormatLine( ww, doc, cp ) \
		S_FormatLine( ww, doc, cp )
#define FormatLineDr( ww, cp, pdr ) \
		S_FormatDrLine( ww, cp, pdr )
#define FAddRun(fn, fc, pch, cch, pfkpd, fPara, fAllocMac, fPlcMustExp, fWord3) \
		FAddRunCkProc(fn, fc, pch, cch, pfkpd, fPara, fAllocMac, fPlcMustExp)
#define AddRun(fn, fc, pch, cch, pfkpd, fPara, fAllocMac, fPlcMustExp, fWord3) \
		C_AddRun(fn, fc, pch, cch, pfkpd, fPara, fAllocMac, fPlcMustExp)
#define WriteRgchToFn(fn, pch, cch) \
		S_ScanFnForBytes(fn, (char HUGE *)pch, cch, fTrue /* fWrite */)
#define ReadRgchFromFn(fn, pch, cch) \
		S_ScanFnForBytes(fn, (char HUGE *)pch, cch, fFalse /* fWrite */)
#define WriteHprgchToFn(fn, hpch, cch) \
		S_ScanFnForBytes(fn, hpch, cch, fTrue /* fWrite */)
#define ReadHprgchFromFn(fn, hpch, cch) \
		S_ScanFnForBytes(fn, hpch, cch, fFalse /* fWrite */)
#define FInTableDocCp(doc, cp) \
		S_FInTableDocCp(doc, cp)
#define AdjustCp(pca, dcpIns) \
		S_AdjustCp(pca, dcpIns)
#define IbpLru() \
		S_IbpLru()
#define ClFormatLines(plbs, cpLim, dylFill, dylMax, clLim, dxl, f1, f2) \
		S_ClFormatLines(plbs, cpLim, dylFill, dylMax, clLim, dxl, f1, f2)
#define CacheParaL(doc, cp, fOutline) \
		S_CacheParaL(doc, cp, fOutline)
#define CacheSectL(doc, cp, fOutline) \
		S_CacheSectL(doc, cp, fOutline)
#define FAbortLayout(fOutline, plbs) \
		S_FAbortLayout(fOutline, plbs)
#define PdrFetch(hpldr, idr, pdrf) \
		S_PdrFetch(hpldr, idr, pdrf)
#define PdrFetchAndFree(hpldr, idr, pdrf) \
		S_PdrFetchAndFree(hpldr, idr, pdrf)
#define PdrFreeAndFetch(hpldr, idr, pdrf) \
		S_PdrFreeAndFetch(hpldr, idr, pdrf)
#define FreePdrf(pdrf) \
		S_FreePdrf(pdrf)
#define PtOrigin(hpldr, idr) \
		S_PtOrigin(hpldr, idr)
#define GetCpFirstCpLimDisplayPara(ww, doc, cp, pcpFirst, pcpLim) \
		S_GetCpFirstCpLimDisplayPara(ww, doc, cp, pcpFirst, pcpLim)
#define CpFormatFrom(ww, doc, cp) \
		S_CpFormatFrom(ww, doc, cp)
#define CpVisibleBackCpField(doc, cp, fvc) \
		S_CpVisibleBackCpField(doc, cp, fvc)
#define FAssignLr(plbs, dylFill, fEmptyOK) \
		S_FAssignLr(plbs, dylFill, fEmptyOK)
#define FWidowControl(plbs, plbsNew, dylFill, fEmptyOK) \
		S_FWidowControl(plbs, plbsNew, dylFill, fEmptyOK)
#define IfrdGatherFtnRef(plbs, plbsNew, ifrd, fNormal, ylReject, ylAccept) \
		S_IfrdGatherFtnRef(plbs, plbsNew, ifrd, fNormal, ylReject, ylAccept)
#define FGetFtnBreak(plbs, ifrl, pfrl, fpc) \
		S_FGetFtnBreak(plbs, ifrl, pfrl, fpc)
#define CopyHdtLrs(ihdt, plbs, yl, plbsAbs) \
		S_CopyHdtLrs(ihdt, plbs, yl, plbsAbs)
#define CopyLrs(hpllrFrom, plbsTo, ilrMac, yl, fCopyIgnore, xl) \
		S_CopyLrs(hpllrFrom, plbsTo, ilrMac, yl, fCopyIgnore, xl)
#define ReplaceInPllr(hpllr, ilr, plr) \
		S_ReplaceInPllr(hpllr, ilr, plr)
#define PushLbs(plbsFrom, plbsTo) \
		S_PushLbs(plbsFrom, plbsTo)
#define PopLbs(plbsId, plbsTo) \
		S_PopLbs(plbsId, plbsTo)
#define CopyLbs(plbsFrom, plbsTo) \
		S_CopyLbs(plbsFrom, plbsTo)
#define LbcFormatPara(plbs, dylFill, clLim) \
		S_LbcFormatPara(plbs, dylFill, clLim)
#define FUpdateDr(ww, hpldr, idr, rcwInval, fAbortOK, udmod, cpUpd) \
		S_FUpdateDr(ww, hpldr, idr, rcwInval, fAbortOK, udmod, cpUpd)
#define ScrollDrUp(ww, hpldr, idr, h2, dlFrom, dlTo, yp1, yp2, yp3, yp4) \
		S_ScrollDrUp(ww, hpldr, idr, h2, dlFrom, dlTo, yp1, yp2, yp3, yp4)
#define MiscPlcLoops(hplc, iFirst, iLim, pResult, wRoutine) \
		S_MiscPlcLoops(hplc, iFirst, iLim, pResult, wRoutine)
#define FFillRgwWithSeqLevs(doc, cp, ipad, ifld, hplcpad, hplcfld, rgw) \
		S_FFillRgwWithSeqLevs(doc, cp, ipad, ifld, hplcpad, hplcfld, rgw)
#define AdjustHplcedlCps(hplcedl, cp, dcp) \
		S_AdjustHplcedlCps(hplcedl, cp, dcp)
#define WidthHeightFromBrc(brc, grpf) \
		S_WidthHeightFromBrc(brc, grpf)
#define PxsInit(pxs, ixsMax, pxbc) \
		S_PxsInit(pxs, ixsMax, pxbc)
#define PostTn(pxbc, tnt, h, c) \
		S_PostTn(pxbc, tnt, h, c)
#define FDoTns(pxbc) \
		S_FDoTns(pxbc)
#define CloseTns(pxbc) \
		S_CloseTns(pxbc)
#define FReplace(pca, fn, fc, dfc) \
		S_FReplace(pca, fn, fc, dfc)
#define XReplace(fPlan, pxbc, pxsr) \
		S_XReplace(fPlan, pxbc, pxsr)
#define IpcdSplit(hplcpcd, cp) \
		S_IpcdSplit(hplcpcd, cp)
#define XRepl1(fPlan, pxbc, pxsr) \
		S_XRepl1(fPlan, pxbc, pxsr)
#define InvalCaFierce() \
		S_InvalCaFierce()
#define FCpVisiInOutline(ww, doc, cp, ccp, pcpFirstInvisi) \
		S_FCpVisiInOutline(ww, doc, cp, ccp, pcpFirstInvisi)
#define DrawInsertLine(psel) \
		S_DrawInsertLine(psel)
#define FInsertInPlc(hplc, i, cp, pch) \
		S_FInsertInPlc(hplc, i, cp, pch)
#define FOpenPlc(hplc, i, di) \
		S_FOpenPlc(hplc, i, di)
#define ShrinkPlc(hplc, iMaxNew, i, di) \
		S_ShrinkPlc(hplc, iMaxNew, i, di)
#define FStretchPlc(hplc, di) \
		S_FStretchPlc(hplc, di)
#define FNewChpIns(doc, cp, pchp, stc) \
		S_FNewChpIns(doc, cp, pchp, stc)
#define FInsertRgch(doc, cp, rgch, cch, pchp, ppap) \
		S_FInsertRgch(doc, cp, rgch, cch, pchp, ppap)
#define FcAppendRgchToFn(fn, pch, cch) \
		S_FcAppendRgchToFn(fn, pch, cch)
#define CbGrpprlProp(f, p1, cb, p2, p3, cw, mpiwspx, mpiwspxW3) \
		S_CbGrpprlProp(f, p1, cb, p2, p3, cw, mpiwspx)
#define CbGenPrl(pprop, ppropBase, sprm, rgb) \
		S_CbGenPrl(pprop, ppropBase, sprm, rgb)
#define CbGenChpxFromChp(pchpResult, pchp, pchpBase, fWord3) \
		S_CbGenChpxFromChp(pchpResult, pchp, pchpBase)
#define CbGenPapxFromPap(ppapx, ppap, ppapBase, fWord3) \
		S_CbGenPapxFromPap(ppapx, ppap, ppapBase)
#define ItcGetTcxCache(ww, doc, cp, ptap, itc, ptcx) \
		S_ItcGetTcxCache(ww, doc, cp, ptap, itc, ptcx)
#define ItcGetTcx(ww, ptap, itc, ptcx) \
		S_ItcGetTcx(ww, ptap, itc, ptcx)
#define FUpdateTable(ww, doc, h1, idr, pcp, pyp, h2, dl1, dl2, dl3, yp1, yp2, yp3, rc, f) \
		S_FUpdateTable(ww, doc, h1, idr, pcp, pyp, h2, dl1, dl2, dl3, yp1, yp2, yp3, rc, f)
#define FUpdTableDr(ww, hpldr, idr) \
		S_FUpdTableDr(ww, hpldr, idr)
#define FrameEasyTable(ww, doc, cp, hpldr, prclDrawn, pdr, dyl, f1, f2, f3, f4) \
		S_FrameEasyTable(ww, doc, cp, hpldr, prclDrawn, pdr, dyl, f1, f2, f3, f4)
#define RtfIn(hribl, pch, cch) \
		S_RtfIn(hribl, pch, cch)
#define ChMapSpecChar(ch, chs) \
		S_ChMapSpecChar(ch, chs)
#define FSearchRgrsym(sz, pirsym) \
		S_FSearchRgrsym(sz, pirsym)
#define ValFromPropSprm(prgbProps, sprm) \
		S_ValFromPropSprm(prgbProps, sprm)
#define XDelFndSedPgdPad(fPlan, pxbc, pxsr) \
		S_XDelFndSedPgdPad(fPlan, pxbc, pxsr)
#define PchSzRtfMove(iszRtf, pch) \
		S_PchSzRtfMove(iszRtf, pch)
#define FRepl1(pca, fn, fc, dfc) \
		S_FRepl1(pca, fn, fc, dfc)
#define FReplaceCps(pcaDel, pcaIns) \
		S_FReplaceCps(pcaDel, pcaIns)
#define XReplaceCps(fPlan, pxbc, pxsr) \
		S_XReplaceCps(fPlan, pxbc, pxsr)
#define WCompSzSrt(psz1,psz2,fCase) \
		S_WCompSzSrt(psz1,psz2,fCase)
#define WCompChCh(ch1, ch2) \
		S_WCompChCh(ch1, ch2)
#define WCompRgchIndex(hpch1, cch1, hpch2, cch2) \
		S_WCompRgchIndex(hpch1, cch1, hpch2, cch2)
#define IbstFindSzFfn(hsttb, pffn) \
		S_IbstFindSzFfn(hsttb, pffn)
#else /* !DEBUG */
#define FetchCp(doc,cp,fcm) \
		N_FetchCp(doc,cp,fcm)
#define CachePara(doc,cp) \
		N_CachePara(doc,cp)
#define FormatLineDxa(ww,doc,cp,dxa) \
		N_FormatLineDxa(ww,doc,cp,dxa)
#define LoadFont(pchp, fWidth) \
		N_LoadFont(pchp, fWidth)
#define CpSearchSz(pbmib, cpFirst, cpLim, pcpNextRpt, pppr) \
		N_CpSearchSz(pbmib, cpFirst, cpLim, pcpNextRpt, pppr)
#define CpSearchSzBackward(pbmib, cpFirst, cpLim) \
		N_CpSearchSzBackward(pbmib, cpFirst, cpLim)
#define FfcFormatFieldPdcp(pdcp, ww, doc, cp, ch) \
		N_FfcFormatFieldPdcp(pdcp, ww, doc, cp, ch)
#define DcpSkipFieldChPflcd(ch, pflcd, fShowResult, fFetch) \
		N_DcpSkipFieldChPflcd(ch, pflcd, fShowResult, fFetch)
#define FShowResultPflcdFvc(pflcd, fvc) \
		N_FShowResultPflcdFvc(pflcd, fvc)
#define IfldFromDocCp(doc, cp, fMatch) \
		N_IfldFromDocCp(doc, cp, fMatch)
#define FillIfldFlcd(hplcfld, ifld, pflcd) \
		N_FillIfldFlcd(hplcfld, ifld, pflcd)
#define GetIfldFlcd(doc, ifld, pflcd) \
		N_GetIfldFlcd(doc, ifld, pflcd)
#define SetFlcdCh(doc, pflcd, ch) \
		N_SetFlcdCh(doc, pflcd, ch)
#define IfldInsertDocCp(doc, cp) \
		N_IfldInsertDocCp(doc, cp)
#define FetchVisibleRgch(pfvb, fvc, fProps, fNested) \
		N_FetchVisibleRgch(pfvb, fvc, fProps, fNested)
#define FetchCpPccpVisible(doc, cp, pccp, fvc, fNested) \
		N_FetchCpPccpVisible(doc, cp, pccp, fvc, fNested)
#define CpVisibleCpField(doc, cp, fvc, fIns) \
		N_CpVisibleCpField(doc, cp, fvc, fIns)
#define FVisibleCp(ww, doc, cp) \
		N_FVisibleCp(ww, doc, cp)
#define DisplayFliCore( ww, rcClip, xl, yl ) \
		N_DisplayFliCore( ww, rcClip, xl, yl )
#define FMarkLine(psel, hpldr, idr, pedl, cpFirstSel, cpLimSel, cpFirstLine) \
		N_FMarkLine(psel, hpldr, idr, pedl, cpFirstSel, cpLimSel, cpFirstLine)
#define XpFromDcp(dcp1, dcp2, pxpFirst, pich) \
		N_XpFromDcp(dcp1, dcp2, pxpFirst, pich)
#define MapStc(pdod, stc, pchp, ppap) \
		N_MapStc(pdod, stc, pchp, ppap)
#define AddVisiSpaces(hdc, xpLine, ypLine, prcw) \
		N_AddVisiSpaces(hdc, xpLine, ypLine, prcw)
#define ApplyPrlSgc(hpprl, cch, prgbProps, sgc) \
		N_ApplyPrlSgc(hpprl, cch, prgbProps, sgc)
#define FGetParaState(fAll, fAbortOk) \
		N_FGetParaState(fAll, fAbortOk)
#define FGraphicsFcidToPlf(fcid, plf, fPrinterFont) \
		N_FGraphicsFcidToPlf(fcid, plf, fPrinterFont)
#define PnFromPlcbteFc(hplcbte, fc) \
		N_PnFromPlcbteFc(hplcbte, fc)
#define BFromFc(hpfkp, fc, pfcFirst, pfcLim, pifc) \
		N_BFromFc(hpfkp, fc, pfcFirst, pfcLim, pifc)
#define HpchFromFc(fn, fc) \
		N_HpchFromFc(fn, fc)
#define HpchGetPn(fn, pn) \
		N_HpchGetPn(fn, pn)
#define IbpCacheFilePage(fn, pn) \
		N_IbpCacheFilePage(fn, pn)
#define DoPrmSgc(prm, prgbProps, sgc) \
		N_DoPrmSgc(prm, prgbProps, sgc)
#define LoadFcidFull( fcid ) \
		N_LoadFcidFull( fcid )
#define ResetFont(fPrinterFont) \
		N_ResetFont(fPrinterFont)
#define DisplayFli( ww, hpldr, idr, dl, ypLine ) \
		N_DisplayFli( ww, hpldr, idr, dl, ypLine )
#define FormatLine( ww, doc, cp ) \
		N_FormatLine( ww, doc, cp )
#define FormatLineDr( ww, cp, pdr ) \
		N_FormatDrLine( ww, cp, pdr )
#define FAddRun(fn, fc, pch, cch, pfkpd, fPara, fAllocMac, fPlcMustExp, fWord3) \
		N_FAddRun(fn, fc, pch, cch, pfkpd, (fPara << 2) + (fAllocMac << 1) + fPlcMustExp)
#define AddRun(fn, fc, pch, cch, pfkpd, fPara, fAllocMac, fPlcMustExp, fWord3) \
		N_FAddRun(fn, fc, pch, cch, pfkpd, (fPara << 2) + (fAllocMac << 1) + fPlcMustExp)
#define WriteRgchToFn(fn, pch, cch) \
		N_ScanFnForBytes(fn, (char HUGE *)pch, cch, fTrue /* fWrite */)
#define ReadRgchFromFn(fn, pch, cch) \
		N_ScanFnForBytes(fn, (char HUGE *)pch, cch, fFalse /* fWrite */)
#define WriteHprgchToFn(fn, hpch, cch) \
		N_ScanFnForBytes(fn, hpch, cch, fTrue /* fWrite */)
#define ReadHprgchFromFn(fn, hpch, cch) \
		N_ScanFnForBytes(fn, hpch, cch, fFalse /* fWrite */)
#define FInTableDocCp(doc, cp) \
		N_FInTableDocCp(doc, cp)
#define AdjustCp(pca, dcpIns) \
		N_AdjustCp(pca, dcpIns)
#define IbpLru() \
		N_IbpLru()
#define ClFormatLines(plbs, cpLim, dylFill, dylMax, clLim, dxl, f1, f2) \
		N_ClFormatLines(plbs, cpLim, dylFill, dylMax, clLim, dxl, f1, f2)
#define CacheParaL(doc, cp, fOutline) \
		N_CacheParaL(doc, cp, fOutline)
#define CacheSectL(doc, cp, fOutline) \
		N_CacheSectL(doc, cp, fOutline)
#define FAbortLayout(fOutline, plbs) \
		N_FAbortLayout(fOutline, plbs)
#define PdrFetch(hpldr, idr, pdrf) \
		N_PdrFetch(hpldr, idr, pdrf)
#define PdrFetchAndFree(hpldr, idr, pdrf) \
		N_PdrFetchAndFree(hpldr, idr, pdrf)
#define PdrFreeAndFetch(hpldr, idr, pdrf) \
		N_PdrFreeAndFetch(hpldr, idr, pdrf)
#define FreePdrf(pdrf) \
		N_FreePdrf(pdrf)
#define PtOrigin(hpldr, idr) \
		N_PtOrigin(hpldr, idr)
#define GetCpFirstCpLimDisplayPara(ww, doc, cp, pcpFirst, pcpLim) \
		N_GetCpFirstCpLimDisplayPara(ww, doc, cp, pcpFirst, pcpLim)
#define CpFormatFrom(ww, doc, cp) \
		N_CpFormatFrom(ww, doc, cp)
#define CpVisibleBackCpField(doc, cp, fvc) \
		N_CpVisibleBackCpField(doc, cp, fvc)
#define FAssignLr(plbs, dylFill, fEmptyOK) \
		N_FAssignLr(plbs, dylFill, fEmptyOK)
#define FWidowControl(plbs, plbsNew, dylFill, fEmptyOK) \
		N_FWidowControl(plbs, plbsNew, dylFill, fEmptyOK)
#define IfrdGatherFtnRef(plbs, plbsNew, ifrd, fNormal, ylReject, ylAccept) \
		N_IfrdGatherFtnRef(plbs, plbsNew, ifrd, fNormal, ylReject, ylAccept)
#define FGetFtnBreak(plbs, ifrl, pfrl, fpc) \
		N_FGetFtnBreak(plbs, ifrl, pfrl, fpc)
#define CopyHdtLrs(ihdt, plbs, yl, plbsAbs) \
		N_CopyHdtLrs(ihdt, plbs, yl, plbsAbs)
#define CopyLrs(hpllrFrom, plbsTo, ilrMac, yl, fCopyIgnore, xl) \
		N_CopyLrs(hpllrFrom, plbsTo, ilrMac, yl, fCopyIgnore, xl)
#define ReplaceInPllr(hpllr, ilr, plr) \
		N_ReplaceInPllr(hpllr, ilr, plr)
#define PushLbs(plbsFrom, plbsTo) \
		N_PushLbs(plbsFrom, plbsTo)
#define PopLbs(plbsId, plbsTo) \
		N_PopLbs(plbsId, plbsTo)
#define CopyLbs(plbsFrom, plbsTo) \
		N_CopyLbs(plbsFrom, plbsTo)
#define LbcFormatPara(plbs, dylFill, clLim) \
		N_LbcFormatPara(plbs, dylFill, clLim)
#define FUpdateDr(ww, hpldr, idr, rcwInval, fAbortOK, udmod, cpUpd) \
		N_FUpdateDr(ww, hpldr, idr, rcwInval, fAbortOK, udmod, cpUpd)
#define ScrollDrUp(ww, hpldr, idr, h2, dlFrom, dlTo, yp1, yp2, yp3, yp4) \
		N_ScrollDrUp(ww, hpldr, idr, h2, dlFrom, dlTo, yp1, yp2, yp3, yp4)
#define MiscPlcLoops(hplc, iFirst, iLim, pResult, wRoutine) \
		N_MiscPlcLoops(hplc, iFirst, iLim, pResult, wRoutine)
#define FFillRgwWithSeqLevs(doc, cp, ipad, ifld, hplcpad, hplcfld, rgw) \
		N_FFillRgwWithSeqLevs(doc, cp, ipad, ifld, hplcpad, hplcfld, rgw)
#define WidthHeightFromBrc(brc, grpf) \
		N_WidthHeightFromBrc(brc, grpf)
#define PxsInit(pxs, ixsMax, pxbc) \
		N_PxsInit(pxs, ixsMax, pxbc)
#define PostTn(pxbc, tnt, h, c) \
		N_PostTn(pxbc, tnt, h, c)
#define FDoTns(pxbc) \
		N_FDoTns(pxbc)
#define CloseTns(pxbc) \
		N_CloseTns(pxbc)
#define FReplace(pca, fn, fc, dfc) \
		N_FReplace(pca, fn, fc, dfc)
#define XReplace(fPlan, pxbc, pxsr) \
		N_XReplace(fPlan, pxbc, pxsr)
#define IpcdSplit(hplcpcd, cp) \
		N_IpcdSplit(hplcpcd, cp)
#define XRepl1(fPlan, pxbc, pxsr) \
		N_XRepl1(fPlan, pxbc, pxsr)
#define InvalCaFierce() \
		N_InvalCaFierce()
#define FCpVisiInOutline(ww, doc, cp, ccp, pcpFirstInvisi) \
		N_FCpVisiInOutline(ww, doc, cp, ccp, pcpFirstInvisi)
#define DrawInsertLine(psel) \
		N_DrawInsertLine(psel)
#define FInsertInPlc(hplc, i, cp, pch) \
		N_FInsertInPlc(hplc, i, cp, pch)
#define FOpenPlc(hplc, i, di) \
		N_FOpenPlc(hplc, i, di)
#define ShrinkPlc(hplc, iMaxNew, i, di) \
		N_ShrinkPlc(hplc, iMaxNew, i, di)
#define FStretchPlc(hplc, di) \
		N_FStretchPlc(hplc, di)
#define FNewChpIns(doc, cp, pchp, stc) \
		N_FNewChpIns(doc, cp, pchp, stc)
#define FInsertRgch(doc, cp, rgch, cch, pchp, ppap) \
		N_FInsertRgch(doc, cp, rgch, cch, pchp, ppap)
#define FcAppendRgchToFn(fn, pch, cch) \
		N_FcAppendRgchToFn(fn, pch, cch)
#define CbGrpprlProp(f, p1, cb, p2, p3, cw, mpiwspx, mpiwspxW3) \
		N_CbGrpprlProp(f, p1, cb, p2, p3, cw, mpiwspx)
#define CbGenPrl(pprop, ppropBase, sprm, rgb) \
		N_CbGenPrl(pprop, ppropBase, sprm, rgb)
#define CbGenChpxFromChp(pchpResult, pchp, pchpBase, fWord3) \
		N_CbGenChpxFromChp(pchpResult, pchp, pchpBase)
#define CbGenPapxFromPap(ppapx, ppap, ppapBase, fWord3) \
		N_CbGenPapxFromPap(ppapx, ppap, ppapBase)
#define ItcGetTcxCache(ww, doc, cp, ptap, itc, ptcx) \
		N_ItcGetTcxCache(ww, doc, cp, ptap, itc, ptcx)
#define ItcGetTcx(ww, ptap, itc, ptcx) \
		N_ItcGetTcx(ww, ptap, itc, ptcx)
#define FUpdateTable(ww, doc, h1, idr, pcp, pyp, h2, dl1, dl2, dl3, yp1, yp2, yp3, rc, f) \
		N_FUpdateTable(ww, doc, h1, idr, pcp, pyp, h2, dl1, dl2, dl3, yp1, yp2, yp3, rc, f)
#define FUpdTableDr(ww, hpldr, idr) \
		N_FUpdTableDr(ww, hpldr, idr)
#define FrameEasyTable(ww, doc, cp, hpldr, prclDrawn, pdr, dyl, f1, f2, f3, f4) \
		N_FrameEasyTable(ww, doc, cp, hpldr, prclDrawn, pdr, dyl, f1, f2, f3, f4)
#define RtfIn(hribl, pch, cch) \
		N_RtfIn(hribl, pch, cch)
#define ChMapSpecChar(ch, chs) \
		N_ChMapSpecChar(ch, chs)
#define FSearchRgrsym(sz, pirsym) \
		N_FSearchRgrsym(sz, pirsym)
#define ValFromPropSprm(prgbProps, sprm) \
		N_ValFromPropSprm(prgbProps, sprm)
#define XDelFndSedPgdPad(fPlan, pxbc, pxsr) \
		N_XDelFndSedPgdPad(fPlan, pxbc, pxsr)
#define PchSzRtfMove(iszRtf, pch) \
		N_PchSzRtfMove(iszRtf, pch)
#define FRepl1(pca, fn, fc, dfc) \
		N_FRepl1(pca, fn, fc, dfc)
#define FReplaceCps(pcaDel, pcaIns) \
		N_FReplaceCps(pcaDel, pcaIns)
#define XReplaceCps(fPlan, pxbc, pxsr) \
		N_XReplaceCps(fPlan, pxbc, pxsr)
#define WCompSzSrt(psz1,psz2,fCase) \
		N_WCompSzSrt(psz1,psz2,fCase)
#define WCompChCh(ch1, ch2) \
		N_WCompChCh(ch1, ch2)
#define WCompRgchIndex(hpch1, cch1, hpch2, cch2) \
		N_WCompRgchIndex(hpch1, cch1, hpch2, cch2)
#define IbstFindSzFfn(hsttb, pffn) \
		N_IbstFindSzFfn(hsttb, pffn)
#endif /* !DEBUG */

/* Temporary macros to eliminate IInPlcMult, IInPlc2Mult */
#define IInPlcMult(hplc, cp) \
	IInPlc(hplc, cp)
#define IInPlc2Mult(hplc, cp, iFirst) \
	IInPlc2(hplc, cp, iFirst)
/* Macros to get rid of unneeded argument in DxaFromDxp, DxpFromDxa for WIN */
#define DxaFromDxp(pwwd, dxp) \
		N_DxaFromDxp(dxp)
#define DxpFromDxa(pwwd, dxa) \
		N_DxpFromDxa(dxa)
/* Macro to get rid of assembler error message in resident.asm */
#define abs(x) N_abs(x)

/* D E B U G */
/* Debug and Debug0 are just ways to make an expression conditional */
/* used for assignment to j variables e.g. Debug(vjClip = jClipWw) */
/* Debug0 is useful when null expression disallowed, as in the use
	of comma operator in pic.c e.g., if ((Debug(i=2), foo))  
*/

#ifdef DEBUG
#define Debug(e) e
#define Debug0(e) e
#else
#define Debug(e)
#define Debug0(e) 0
#endif

/* B A T C H */
/* Batch mode code is defined under #ifdef BATCH, so it can be used */
/* in non-debug versions.  In the debug version, BATCH is always defined, */
/* but in non-debug versions BATCH must be supplied as an explicit compile */
/* flag. */
#ifdef DEBUG
#define BATCH
#endif

/* Batch is similar to the Debug conditional expression macro */
#ifdef BATCH
#define Batch(e) e
#else
#define Batch(e)
#endif

/* M A R K */
/* call MarkProc conditionally */
#ifdef DEBUG
#define Mark(e) MarkProc(e)
#else
#define Mark(e)
#endif

/* F  R A R E  T */
/* identifies a value that is rarely true, but if vjRare is set to
the identifying number, a true value will be substituted for testing
purposes.
*/
#ifdef DEBUG
#define FRareT(n, f) ((f) || FRareProc(n))
#else
#define FRareT(n, f) (f)
#endif

/* F  R A R E  F */
/* as above, except the value is rarely false.
*/
#ifdef DEBUG
#define FRareF(n, f) ((f) && !FRareProc(n))
#else
#define FRareF(n, f) (f)
#endif

/* E R R O R S */

#ifdef DEBUG /* ERRORSTRINGS */
#define DiskError(idpmt, fn, msg)       DiskErrorProc(idpmt, fn, SzShared(msg))
#else
#define DiskError(idpmt, fn, msg)       DiskErrorProc(idpmt, fn)
#endif

#define SetErrorMat(mat)		SetErrorMatProc(mat)



/* C A  T E M P */
#ifdef DEBUG
#define AcquireCaAdjust() { extern int cCaAdjust; ++cCaAdjust; \
								Assert (caAdjust.doc == docNil); \
								Assert (cCaAdjust == 1); }
#define ReleaseCaAdjust() { extern int cCaAdjust; --cCaAdjust; \
								Assert (cCaAdjust == 0); \
								caAdjust.doc = docNil; }
#define AcquireCaAdjustL() { extern int cCaAdjustL; ++cCaAdjustL; \
								Assert (caAdjustL.doc == docNil); \
								Assert (cCaAdjustL == 1); }
#define ReleaseCaAdjustL() { extern int cCaAdjustL; --cCaAdjustL; \
								Assert (cCaAdjustL == 0); \
								caAdjustL.doc = docNil; }
#else
#define AcquireCaAdjust()
#define ReleaseCaAdjust()
#define AcquireCaAdjustL()
#define ReleaseCaAdjustL()
#endif

/* D O C  S C R A T C H */
#ifdef DEBUG
extern BOOL fDocScratchInUse;
#define ReleaseDocScratch() Assert (!(fDocScratchInUse = !fDocScratchInUse))
#else
#define ReleaseDocScratch()
#endif /* DEBUG */

/* S T A T I C */
/* STATIC is a way of declaring a procedure that we will only reference within */
/* a module, but whose symbol we don't want to hide from the debugger. */
#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif



#ifdef DEBUG

#define dbctFli 0
#define dbctDisplay 1
#define dbctLr 2

#define cTestsPrefs	36
#define pchTestMin   	((char *)&vdbs.fCkDoc)
#define pchPrefMin  	((char *)&vdbs.fCommSz)
#define pchUseCMin	((char *)&vdbs.CFetchCp)

#define cchDbgBuf1      320
#define cchDbgBuf2      80

#define nDbsuVerCur	7

struct DBS              /* DeBug State */
		{
	struct DBSU
		/* This structure forms the stored file, it must be first */
		{
		/* version */
/* 0 */	    int nDbsuVer;

			/* Consistency checking routine "on/off" switches */
/* 2 */     char fCkDoc;/* must be first */
			char fCkFn;
			char fCkWw;
			char fCkBptb;

			char fCkFkp;
			char fCkPlcbte;
/* 8 */     char fCkFli;
			char fCkScc;

			char fCkPrm;
			char fCkDlg;
			char fCkHeap;
			char fShakeHeap;

			char fCkStruct;
			char fCkFont;
/* 10 */    char fCkMwd;
		char fCk16;

			char fCkFldDel;
			char fCkFldIdle;
			char fCkLHRUpd;
			char fCkScroll;          /* see scroll.c */

			char fCkPaph;  
			char fCkText;
/* 18 */    char fCkTlbx;
		char fCkTableSel;
#define cTestsUsed  24

		char fCk25;
		char fCk26;
		char fCk27;
		char fCk28;

		char fCk29;
		char fCk30;
/* 20 */    char fCk31;
		char fCk32;

		char fCk33;
		char fCk34;
		char fCk35;
		char fCk36;


		/* debugging preferences */
			char fCommSz; /* must be first */
			char fMoreComm;
/* 28 */    char fRipToDbgScreen;
			char fRetryDef;

			char fReports;
		char fRipOnError;
			char fFailInMcr;
			char fReportHeap;

			char fFixedMem;
			char fOutsideCmd;
/* 30 */    char fHeapAct;
			char fWinAct;

			char fTraceGDI;
			char fNoCoreLoad;
		char fNoInfoRTF;
		char fFail8087;

		char fCommDde1;
		char fCommDde2;
/* 38 */    char fNoEmm;
		char fBreakOnFail;

		char fNoFillBlock;
		char fCallSetFlm;
		char fCommPrint;
		char fCoverActive;

		char fShowDirty;
		char fCompressOften; /* causes compression to occur more often */
/* 40 */    char fTrackGdiHandles;
		char fUseTextOut;	/* Use TextOut instead of ExtTextOut() */

		char fShowLbs;    /* show layout rectangles during layout process */
		char fSmallFileCache;	/* File cache same size as if no EMM */
		char fNoRtfConv;
		char fAllGraphicsBands; /* Mark all bands as graphics bands */

		char fExtTextOutNull;	/* Use ExtTextOut() but make extra values null */
		char fDumpMessages;		/* Output messages in wprocn.asm */
/* 48 */    char fVerboseHeapInfo;
		char fDumpPicInfo;
#define cPrefsUsed 36


		/* use C versions flags */
/* 4A */    char CFetchCp;  /* must be first */
		char CCachePara;
		char CFormatLine;
		char CLoadFont;

		char CCpSearchSz;
		char CCpSearchSzBackward;
/* 50 */    char CFfcFormatField;
		char CDcpSkipFieldChPflcd;

		char CFShowResultPflcdFvc;
		char CIfldFromDocCp;
		char CFillIfldFlcd;
		char CGetIfldFlcd;

		char CSetFlcdCh;
		char CIfldInsertDocCp;
/* 58 */    char CFetchVisibleRgch;
		char CFetchCpPccpVisible;

		char CCpVisibleCpField;
		char CFVisibleCp;
		char CDisplayFliCore;
		char CFMarkLine;

		char CXpFromDcp;
		char CMapStc;
/* 60 */    char CAddVisiSpaces;
		char CApplyPrlSgc;

		char CFGetParaState;
		char CFGraphicsFcidToPlf;
		char CPnFromPlcbteFc;
		char CBFromFc;

		char CHpchFromFc;
		char CHpchGetPn;
/* 68 */    char CIbpCacheFilePage;
		char CDoPrmSgc;

		char CLoadFcidFull;
		char CFDisplayFli;
		char CResetFont;
		char unused2;
#define cUseCUsed   36

		/* scribbles */
		int grpfScribble;

/* 70 */    int rgwSpare [4];

			/* end stored part of DBS */
		};

	/* Following are session based only */

	/* allocation failure control */
	/* code in initwin.c depends on the order of these */
	int cLmemSucceed;
	int cLmemFail;
	int cWinSucceed;
	int cWinFail;
		int cLmemCalls;
		int cWinCalls;

		int fInfoAssert;    /* RTF */
		int fSprmTablesChecked;
		int StatLine;
		int cdlInval;
		int fScan;              /* true iff scanning dialogs */
		/* number of lines output to provide debugging status info.
		Needed to implement scrolling when outputting status info. */
		int jc1;
		int jnScc;              /* Scc test number to skip to */
		int fJoinBelow;         /* return value from CheckSccs */
		int fJoinAbove;         /* return value from CheckSccs */
		/* global stuff to be used with the debugging file read routines */
		FC fcCur;
		int osfn;
		char stRoutineName[30];
		char stFile[256];
		int fFlushFailed;

		int cbHeapWantedCmd;/* amount of heap wanted for cmd */
		VOID **hDebug;/* heap pointer to wasted heap */

		/* used to turn off an assert in CkPbptb during quicksave. */
		int fQuicksave;
	
		/* More failures; stuck at the end to avoid mass make */
		int cBkgndSucceed;
	int cBkgndFail;
		};

extern struct DBS vdbs;
extern int cHpFreeze;
#endif  /* DEBUG */

#ifdef WIN
typedef struct _aob
	{
	char aot;
	char ifr;
	int x, y;
	int irgb;
	int dxMove;
	int dyMove;
	int ddy;
	int yGuess;
	union
		{
		struct
			{
			int xGuess;
			char dx, dy;
			};
		
		struct
			{
			char * st;
			int spare;
			};
		};
	} AOB;

#define iaobMax		50
#endif

#ifdef BATCH
extern BOOL vfBatchMode;
extern CHAR szBatchString[];
extern BOOL vfBatchIdle;
#endif  /* BATCH */

/* R A R E   E V E N T S */

#ifdef DEBUG
#define reNULL            0     /* RESERVED */
#define reMemAlert        1     /* used in UpdateWw */
#define reChngSizeH       2     /* used in FExpandGrpchr */
#define reMsgPresent1     4     /* used in UpdateWw */
#define reMsgPresent2     5     /* used in UpdateWw */
#define reExpandPbmi     27     /* used in FExpandPbmi - can't create bitmap */
#define reMemAlert1      55     /* used in DocCreateSub */
#define reMemAlert2      56     /* used in DocCreateSub */
#define reMemAlert3      57     /* used in DocCreateSub */
#define reMemAlert4      58     /* used in FChangeAll */
#define reMemAlert5      59     /* used in CpDoReplace */
#define reMemAlert6      60     /* used in CpDoReplace */
#define reMemAlert7      61     /* used in DlgfSearchRepl */
#define reOpenPlc       123     /* used in AddToHplcEdit */
#define rePchrNew       324     /* used in FNewChrfChr */
#define reRibbon        100     /* allocation failure in ribbon */
#define reRuler         101     /* allocation failure in ruler */

/* Rare Event Codes for Save */ 
#define reNoSaveHeaders 500     /* could not build superdoc */
#define reNoSaveTemp    501     /* could not create temp file to save to */
#define reNoRenameTemp  502     /* could not rename temp file to dest name */
#define reNoAccessBak   503     /* could not open/delete file that's in the way of the save */ 
#define reNoRenameToBak 504     /* could not rename file to .BAK/.TMP */
#define reNoDelBak      505     /* could not delete old .BAK */

/* Rare Event Codes for GDI failure */

#define reMiscGDI       600
#define reGetDC         601
#define reSelectObject  602
#define reSaveDC        603

#define RareEventMax     10     /* count of concurrent rare events */

typedef struct
		{
		int     wEventNumber;       /* identifier of event */
		int     wRepetitionsMac;    /* stop repetition count */
		int     wRepetitions;       /* current repetition count */
		int     fOn;                /* count on/off flag */
		} ARG_RAREEVENTS;

#endif  /* DEBUG */

/* P R O F I L I N G */

#ifdef PROFILE
#define Profile(x)      x
extern int vpfi;
#define StartProf(n) (PcodeStartProfiler(),StartProfiler(n,NULL))
#define StopProf()   (StopProfiler(),PcodeStopProfiler())
#else /* NO PROFILE */
#define Profile(x)
#define StartProf(n)
#define StopProf()
#undef StartUMeas
#undef StopUMeas
#define StartUMeas(n)
#define StopUMeas(n)
#undef StartProfiler
#undef StopProfiler
#define StartProfiler(n,m)
#define StopProfiler()
#endif /* NO PROFILE */

/* profile id's -- allow profiling of major events based on vdbs.pfi */

#define pfiInit         0       /* FInitWinInfo */
#define pfiScroll       1       /* scrolling */
#define pfiParaDlg      2       /* bringing up paragraph dialog */
#define pfiTyping       3       /* typing */
#define pfiSearch       4       /* Cmd Search */
#define pfiReplace      5       /* Cmd Replace */
#define pfiPgUp         6       /* Key Curs PgUp */
#define pfiDebugUpdateRuler 7   /* testing ruler updates */
#define pfiDebugFieldsDisplay 8 /* testing fields display */
#define pfiCpSearchSz   9       /* low level search */
#define pfiMenu         10      /* drop down menu */
#define pfiFieldRef     11      /* Refresh of fields */
#define pfiPictDrag     12      /* drag the picture with mouse */
#define pfiPictDraw     13      /* draw a pic */
#define pfiPictFmt      14      /* get pic info for formatting */
#define pfiPictPaste    15      /* paste a pict */
#define pfiRepag        16      /* CmdRepaginateNow */
#define pfiInitCmds     17      /* initialize commands */

#define pfiSaveFile     40      /* SaveFile */
#define pfiPromptProf   41
#define pfiUntitled     42

/* User-defined measurement codes */

#ifdef PROFILE
#define umTextOut           201
#define umSelectFont        202
#define umPeekMessage       203
#define umGetMessage        204
#define umCreatePrinterDC   205
#define umBitBlt            206
#define umCreateBitmap      207
#define umRuler1            211
#define umRuler2            212
#define umRuler3            213
#define umRuler4            214
#define umRuler5            215
#define umRuler6            216
#define umField1            221
#define umField2            222
#define umField3            223
#define umOsDateTime        240
#define umSavePrompt        241
#define umGetCodeHandle     242
#define umGlobalLru         243
#define umAssignFnFc1       250
#define umAssignFnFc2       251
#define umAssignFnFc3       252
#define umPmtRelDC    	    260
#define umPmtETO2           261
#define umPmtGCR            262
#define umPmtETO    	    263
#define umGetMenuStr        270
#define umChangeMenu        271
#define umScanFnForBytes    272
#define umPromptCreateWin   273
#define umPromptUpdateWin   274
#define umStatlineTextOut   275
#define umStatlineDestroy   276
#define umFileRead          280
#define umFileWrite         281
#define umNormPath          291
#endif


#ifdef DEBUG
/* Do-nothing functions, for breakpointing */
int Dummy();
NATIVE NatDummy();
#endif

#ifndef DEBUG
#ifdef PCJ
#undef PCJ
#endif
#endif

#ifdef DEBUG
extern int vfInDebug;
#define EnterDebug()    (vfInDebug++)
#define ExitDebug()     (vfInDebug--)
#else
#define EnterDebug()
#define ExitDebug() 
#endif /* DEBUG */

/* G D I   H A N D L E	 T R A C K I N G */

#ifdef DEBUG
#ifndef NOVHGDIS
extern int vhgdis;
#endif /* NOVHGDIS */
#define LogGdiHandle(hGdi,wId) (vhgdis ? LogGdiHandleProc(hGdi,wId) : 0)
#define UnlogGdiHandle(hGdi,wId) (vhgdis ? UnlogGdiHandleProc(hGdi,wId) : 0)
#else
#define LogGdiHandle(hGdi,wId)
#define UnlogGdiHandle(hGdi,wId)
#endif /* DEBUG */

#ifdef RSH
#ifdef RSHDEFS
extern int vfnUa;

#define uaCursor     0x2000
#define uaCursExtend 0x3000
#define uaLooksKey   0x5000
#define uaDragMark   0x6000
#define uaAppMessage 0x7400
#define uaMwdMessage 0x7800
#define uaWwdMessage 0x7C00
#define uaAppCommand 0x8400
#define uaMwdCommand 0x8800
#define uaWwdCommand 0x8C00
#define uaKeyState   0x9000 

#define uaBcmFollows 0xf001
#define uaTimesFollow 0xf002

#define uaCxtHelp    0x0000
#define uaTempMacro  0x0001
/* #define uaGlobalMacro 0x0002 */
#define uaInsert     0x0003
#define uaCancelOrError 0x0004
#define uaIBDlgGetFocus 0x0005
#define uaIBDlgLoseFocus 0x0006
#define uaBackspace    0x0007
#define uaApplyStyleIb 0x0008
#define uaFontPtIb     0x0009
#define uaQuickSave    0x000a
#define uaFullSave     0x000b
#define uaIdle         0x000c
#define uaStartDialogSession	0x000d
#define uaStopDialogSession	0x000e


/* timing values */

#define uamNull -1
#define uamIdle 0
#define uamCommand 1
#define uamNavigation 2
#define uamTyping 3
#define uamMax 4

#define uasMenu 0
#define uasDialogOvrhd 1
#define uasDialogSession 2
#define uasMax 3

struct UAT /* user action timer */
	{
	int uam; /* user action mode */
	long msecStartMode; /* time entered this mode */
	long msecTimeout;   /* time to enter idle mode */
	long msecStopped;   /* time timer was stopped (0 if running) */
	long msecNextReport;/* time to log next report */
	long msecSessionStart;	/* time this WinWord session begun */
	long mpuammsec[uamMax];	/* total times for each mode */
	long mpuasmsec[uasMax];	/* total times for sub-timers */
	long mpuasmsecStarted[uasMax]; /* when running, time sub-timer was started */
	};

#define msecModeTimeout (3*1000L)
#define msecReports (5*60*1000L)

extern struct UAT vuat;

#endif /* RSHDEFS */
#endif /* RSH */
