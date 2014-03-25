/* 	SDMDEFS.H						*\
*								*
*	to be included for all dialogs converted to SDM 2.1	*
*	do NOT mix old and new dialogs in the same module	*
\*								*/



#define BltDlt(dltSrc, pdltDest) \
	bltbx((char FAR *) &dltSrc, (char FAR *) pdltDest, sizeof (dltSrc))

#define DLG_CONST csconst

#undef  PcabFromHcab
#define PcabFromHcab(hcab)	(*(hcab))



/* the following definitions will be used during the conversion to SDM 2.1 *\
\* in order to preserve functionality without having to rename utilities   */

#define GrayButtonOnBlank	GrayButtonOnBlank_sdm21


#undef tmcOK
#undef tmcCancel


#define PcmbDlgCur()	((CMB *) WRefDlgCur())
