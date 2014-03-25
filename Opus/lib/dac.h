typedef WORD HINST;
typedef WORD HFNT;		/* Font to use for display (system if null). */
typedef struct
	{
	HINST	hinst;
	int	wScrollRate;	/* rate of scrolling */
	int	dxBorder;	/* thickness of vertical border */
	int	dyBorder;	/* heigth of horizontal border */
	int	dyCaption;	/* height of caption bar */
	int	dxVScroll;	/* width of vertical scroll bar */

	BOOL	fNewVis;	/* "New" version of windows? (3.00 or higher). */

	/* Leave these here at the end, since we don't want to include them  */
	/* in the GetInstanceData() call of FInitSdm(). */

#define	cbDacNonInstanceData	((WORD)&(((DAC_SDM *)0)->clrWindow))

	DWORD	clrWindow;		/* Window color. */
	DWORD	clrWindowFrame;		/* Window frame color. */
	DWORD	clrWindowText;		/* Window text color. */
	DWORD	clrButton;		/* Button face color. */
	DWORD	clrButtonText;		/* Button text color. */
	DWORD	clrButtonShadow;	/* Button shadow color. */
	DWORD	clrButtonReflection;	/* Button reflection color (top&left). */
	DWORD	clrButtonFrame;		/* Button outline color. */
	DWORD	clrHighLight;		/* Selection background. */
	DWORD	clrHighLightText;	/* Selection text. */

	int	dxSysFontChar;		/* Unadjusted average character width. */
	int	dySysFontChar;		/* Exact character height, NOT adjusted */
	int	dySysFontAscent;	/* Ascent from TextMetrics. */

	int	dyExternalLeading;
	HBRUSH	hbrsGray;		/* Gray brush for drawing carets. */
	HBRUSH	hbrsHighLightText;	/* Brush for disabled and highlighted. */
	HBRUSH	hbrsButtonText;		/* Brush with same color as button text. */
	HBRUSH	hbrsWindowFrame;	/* Brush with same color as windowframe. */

	HFNT	hfnt;		/* Font to use for display (system if null). */
	} DAC_SDM;

extern DAC_SDM dac;
