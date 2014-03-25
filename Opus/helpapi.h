/*****************************************************************************
*                                                                            *
*  HELPAPI.H                                                                 *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1989.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Description:  Include file for communicating with help.            *
*                                                                            *
******************************************************************************
*                                                                            *
*  Revision History:  Created by RobertBu 1/12/89                            *
*                                                                            *
*                                                                            *
******************************************************************************
*                                                                            *
*  Known Bugs: None                                                          *
*                                                                            *
*                                                                            *
*                                                                            *
*****************************************************************************/

/*

Communicating with WinHelp involves using Windows SendMessage() function
to pass blocks of information to WinHelp.  The call looks like.

		SendMessage(hwndHelp, wWinHelp, hwndMain, (LONG)hHlp);

Where:

	hwndHelp - the window handle of the help application.  This
				is obtained by enumerating all the windows in the
				system and sending them cmdFind commands.  The
				application may have to load WinHelp.
	wWinHelp - the value obtained from a RegisterWindowMessage()
				szWINHELP
	hwndMain - the handle to the main window of the application
				calling help
	hHlp     - a handle to a block of data with a HLP structure
				at it head.

The data in the handle will look like:

			+-------------------+
			|     cbData        |
			|    usCommand      |
			|     ulTopic       |
			|    ulReserved     |
			|   offszHelpFile   |\     - offsets measured from beginning
		/ |     offaData      | \      of header.
		/  +-------------------| /
		/   |  Help file name   |/
		\   |    and path       |
		\  +-------------------+
		\ |    Other data     |
			|    (keyword)      |
			+-------------------+

The defined commands are:

	cmdQuit    -  Tells WinHelp to terminate
	cmdLast    -  Tells WinHelp to redisplay the last context
	cmdContext -  Tells WinHelp to display the topic defined by ulTopic
	cmdKey     -  Tells WinHelp to display the topic associated with
				a keyword (pointed to by offaData)
	cmdFind    -  WinHelp will return TRUE if it receives this message

For a plug and play package to use this help API, see the HELPCALL
library.

*/

/*****************************************************************************
*                                                                            *
*                               Typedefs                                     *
*                                                                            *
*****************************************************************************/

#define szWINHELP "WM_WHELP"

	typedef struct  {
	unsigned short	cbData;               /* Size of data                     */
	unsigned short	usCommand;            /* Command to execute               */
	unsigned long	ulTopic;              /* Topic/context number (if needed) */
	unsigned long	ulReserved;           /* Reserved (internal use)          */
	unsigned short	offszHelpFile;        /* Offset to help file in block     */
	unsigned short	offabData;            /* Offset to other data in block    */
} HLP;

typedef HLP far *QHLP;


#define cmdFind      1                  /* See above for descriptions       */
#define cmdContext   2
#define cmdKey       3
#define cmdLast      4
#define cmdQuit      5
