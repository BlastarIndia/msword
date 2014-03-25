/* definitions for user preference file */
/* The user preference file is laid out as follows:

	PREF structure	     \
	header info	     /	PREFD structure

	file cache
	doc mgmt query path
	printer name sz and driver name sz
	printer metrics for vpri
	font name sttb
	printer font enumeration STTB
	Font descriptors (portion of rgfce) (size prefd.cbFontCache)
	Font Width Table (512 bytes for each !fFixedWidth FCE)
*/

#define cchMaxFileCache 	(ibstMaxFileCache*ichMaxFile)

/* NOTE : If you change this structure, change nPrefdVer */
#define nPrefdVerCur   	    12
#define nPrefPrefdVerCur    (nPrefdVerCur+nPrefVerCur)

/* The following two defines are to allow transparent conversion from the
	ini file format used in WinWord 1.00 to that used in 1.00a.  The only
	change between them was printer information.  FReadOpusIni uses these.
*/
#define nPrefPrefdVer100	29	/* ini file version for shipped WinWord 1.00 */
#define nPrefPrefdVer100a	30	/* ini file version for shipped WinWord 1.00a */

struct PREFD {
/* version first so it is invariant between versions! */
	int	   nPrefPrefdVer; /* contains nPrefdVer + nPrefVer */
	struct PREF 	pref;
/* room on file for PREF expansion in future versions */
/* This is one spare allocation that should NOT be deleted when we ship */
	int	  rgwSpare [16];	
/* fields describing remainder of preference file */
		int	   cbSttbFileCache;
	int	   cbStDMQPath;
	int	   cbPrNameAndDriver;
	int	   cbPrinterMetrics;	
	int	   cbPrenv;
	int	   cbSttbFont;
	int	   cbSttbPaf;
	int	   cbRgfce;
	int	   cbFontWidths;
	/* struct STTB  sttbFileCache;          file MRU list */
	/* CHAR         stSMQPath[]             document management path */
	/* CHAR		grpszPrNameAndDriver 	printer name and driver name */
	/* int		rgwPrinterMetrics[];	device  metrics for printer */
	/* struct PRENV	prenv;			printer environment */
	/* struct STTB  sttbFont;		font name table */
	/* struct STTB  sttbPaf;		printer font enumeration */
	/* CHAR		grpstFonts[]; 		Face names & PAF from printer enum */
	/* struct FCE	rgfce[]; 		Printer font cache */
	/* int		rgrgdxp [] [256]; 	Widths for cache */
	};
/* NOTE : If you change this structure, change nPrefdVer (above) */


#define cbPREFD (sizeof(struct PREFD))
