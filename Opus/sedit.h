#define IDCBSEDIT        1
#define IDCBDLBOX        0

/* -----Static Edit Constants */

#define IDSEDITDSCRP          0
#define IDSEDITDXPCH          2
#define IDSEDITDYPEXTLEAD     4
#define IDSEDITDYPHEIGHT      6

/* -----Static Edit Styles */
#define SES_HYPHENATE           0x0001L  /* The one used in hyphenation */
#define SES_DDL                 0x0002L  /* Used in a drop down list box. */

/* -----Static Edit Style Index */
/* Works only because we have two styles.... */
#define ISesFromStl(stl)        ((((int) (stl)) & 0x000F) - 1)

/* -----Static Edit Messages */
#define SEM_RELMOVE             (WM_USER + 0)
#define SEM_MOUSE               (WM_USER + 1)
#define SEM_SETDSCRP            (WM_USER + 2)
#define SEM_GETDSCRP            (WM_USER + 3)
#define SEM_KILLTIMER           (WM_USER + 4)
#define SEM_RESTARTTIMER        (WM_USER + 5)

#define SEM_SETTEXT             (WM_USER + 6) /* Used by Drop List Box SEdit only */

/* -----Some Macroes for static edit */
/* Compute the left indentation amount based on the width of the system font */
#define DxpLeftSE(dxp)            ((dxp) >> 1)

#define CchMaxWndText   60

typedef LONG STL;               /* Windows style type */

#define FCHECKSTL(l, m)      (((l) & (m)) != 0)
#define FGETSTL(h, m)        (FCHECKSTL(GetWindowLong((h), GWL_STYLE), (m)))
STL StlSetStl( /*- HWND, LONG, BOOL -*/ );

#define EXOR(a, b)              ( (a) ? !(b) : (b) )

