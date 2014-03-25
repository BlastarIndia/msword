/*  wininfo.h  -- Windows related constants */

/* specific window constants */

/* mac-in-a-box window (mw)*/
#define CBWEMW     2
#define IDWMW      0

/* pane window (ww) */
#define CBWEWW     2
#define IDWWW      0

/* Icon Bar window */
#define CBWEICONBAR     2
#define IDWIBHIBS       0

/* Dde Channel window (used for communication, never shown) */
#define CBWEDDECHNL     2
#define IDWDDECHNLDCL   0

#define CBWEFEDT        sizeof(char **)
#define CBWESTATICEDIT  8

#define CBWEBMI 	8
#define IDWBMIIDRB      0
#define IDWBMIIDCB      2
#define IDWBMIBCM 	4
#define IDWBMIWW	6

/* M E S S A G E S */
#ifndef NOSPECMSG
/* messages specific to our application */

/* Application Modal Messages */

/* if this message is POSTED to the application message queue while any window
	is in Application Modal Mode the mode will terminate and WAppModalHwnd will
	return wParam. */

#define AMM_TERMINATE           WM_USER + 80


#endif /* NOSPECMSG */
