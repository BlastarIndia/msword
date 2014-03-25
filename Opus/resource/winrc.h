/* Window styles */
#define WS_TILED        0x00000000L
#define WS_POPUP        0x80000000L
#define WS_CHILD        0x40000000L
#define WS_ICONIC       0x20000000L
#define WS_VISIBLE      0x10000000L
#define WS_DISABLED     0x08000000L
#define WS_CLIPSIBLINGS 0x04000000L
#define WS_CLIPCHILDREN 0x02000000L

#define WS_BORDER       0x00800000L
#define WS_CAPTION      0x00c00000L
#define WS_DLGFRAME     0x00400000L
#define WS_VSCROLL      0x00200000L
#define WS_HSCROLL      0x00100000L
#define WS_SYSMENU      0x00080000L
#define WS_SIZEBOX      0x00040000L
#define WS_GROUP        0x00020000L
#define WS_TABSTOP      0x00010000L

/* Shorthand for the common cases */
#define WS_TILEDWINDOW   (WS_TILED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX)
#define WS_POPUPWINDOW   (WS_POPUP | WS_BORDER | WS_SYSMENU)
#define WS_CHILDWINDOW   (WS_CHILD)

/* Conventional dialog box and message box command IDs */
#define IDOK         1
#define IDCANCEL     2
#define IDABORT      3
#define IDRETRY      4
#define IDIGNORE     5
#define IDYES        6
#define IDNO         7

/* Edit control styles */
#define ES_LEFT           0L
#define ES_CENTER         1L
#define ES_RIGHT          2L
#define ES_MULTILINE      4L
#define ES_AUTOVSCROLL    64L
#define ES_AUTOHSCROLL    128L
#define ES_NOHIDESEL      256L

/* Button control styles */
#define BS_PUSHBUTTON    0L
#define BS_DEFPUSHBUTTON 1L
#define BS_CHECKBOX      2L
#define BS_AUTOCHECKBOX  3L
#define BS_RADIOBUTTON   4L
#define BS_3STATE        5L
#define BS_AUTO3STATE    6L
#define BS_GROUPBOX      7L
#define BS_USERBUTTON    8L

/* Static control constants */
#define SS_LEFT       0L
#define SS_CENTER     1L
#define SS_RIGHT      2L
#define SS_ICON       3L
#define SS_BLACKRECT  4L
#define SS_GRAYRECT   5L
#define SS_WHITERECT  6L
#define SS_BLACKFRAME 7L
#define SS_GRAYFRAME  8L
#define SS_WHITEFRAME 9L
#define SS_USERITEM   10L

/* Listbox style bits */
#define LBS_NOTIFY        0x0001L
#define LBS_SORT          0x0002L
#define LBS_NOREDRAW      0x0004L
#define LBS_MULTIPLESEL   0x0008L
#define LBS_STANDARD      (LBS_NOTIFY | LBS_SORT | WS_VSCROLL | WS_BORDER)

/* Scroll bar styles */
#define SBS_HORZ                    0x0000L
#define SBS_VERT                    0x0001L
#define SBS_TOPALIGN                0x0002L
#define SBS_LEFTALIGN               0x0002L
#define SBS_BOTTOMALIGN             0x0004L
#define SBS_RIGHTALIGN              0x0004L
#define SBS_SIZEBOXTOPLEFTALIGN     0x0002L
#define SBS_SIZEBOXBOTTOMRIGHTALIGN 0x0004L
#define SBS_SIZEBOX                 0x0008L
