/* MS-Windows specific definitions */

#define szStatOn ("1")  /* strings for status item value in win.ini */
#define szStatOff ("0")

/* Some useful composite message box codes */

#define MB_MESSAGE        (MB_OK | MB_APPLMODAL | MB_ICONASTERISK)
#define MB_YESNOQUESTION  (MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION)
#define MB_ERROR          (MB_OK | MB_ICONEXCLAMATION)
#define MB_TROUBLE        (MB_OK | MB_SYSTEMMODAL | MB_ICONHAND)
#define MB_DEFYESQUESTION (MB_YESNOCANCEL | MB_APPLMODAL | MB_ICONQUESTION)
#define MB_DEFNOQUESTION  (MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_APPLMODAL | MB_ICONQUESTION)
