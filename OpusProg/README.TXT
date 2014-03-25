*****************************************************************************
                MICROSOFT WORD FOR WINDOWS SPECIAL INFORMATION
*****************************************************************************

IMPORTANT NOTE
--------------
Microsoft Windows version 2.11 or later is required for Word to run properly.
If you have not yet installed Windows 2.11 or later, it is recommended that 
you do this before running Word for Windows Setup..

This document is not properly formatted for use with Microsoft Word for 
Windows. It is intended for the user who has not yet set up Microsoft Word
for Windows and would like more information. It can be opened in Microsoft
Word for Windows and reformatted if desired.

If Word for Windows does not correctly detect that Windows has been installed
on your system, ensure that Windows is on the PATH= of your AUTOEXEC.BAT 
file. Normal completion of Windows Setup guarantees that the Windows 
directory is on your path. Do not remove the Windows directory from the 
PATH= line if you want Word for Windows Setup to correctly modify the 
WIN.INI file for use with Word for Windows.

If you are running Windows, you should not run Word for Windows Setup due to  
the modifications that must be made to the WIN.INI file. Choose File Exit
from the Windows Program Manager to quit Windows, and then run Setup from the
DOS prompt. Do not run Setup from inside a DOS shell by running the Windows
"DOS Prompt" icon from the Windows Program Manager, or by choosing File Run
from inside the Program Manager, the File Manager (or any other Windows 
application that lets you start COMMAND.COM) while Windows is running.

If you have Windows version 2.x and do not want to upgrade to Windows version
3.x, and you are currently using HIMEM.SYS, please locate HIMEM.SY$ on your 
Word for Windows disk set (for 1.2MB, 1.44MB, and 720KB disk sets, the file 
is on SETUP; for 360KB disk sets, it is on DISK 11). Follow the	directions 
for decompressing the file (see the section below called "COMPRESSED FILES"), 
and replace the Windows 2.x version of HIMEM.SYS with the newer version. If 
you have Windows 3.x, then you should already have this version (or later) 
installed on your system if you have system memory greater than 640KB.

If you use Microsoft LAN Manager, or some other program that appends
multiple PATH= lines in your AUTOEXEC.BAT, Word for Windows Setup may not
correctly add your Word for Windows directory to the PATH= line. If you find 
that C:\WINWORD (or whatever directory you specified) is not on your path, 
you may need to use a text editor to change the PATH= line in your 
AUTOEXEC.BAT file manually.

*****************************************************************************

TO PRINT THIS FILE
------------------
Determine which port you have a printer connected to (LPT1: will be used
for this example). Then type:

    COPY README.TXT LPT1:

and then press ENTER. This file will be printed on your printer.

Note: This procedure will not work with a PostScript printer. If you have
      the full Windows package (not a runtime version of Windows) and it is
      set up to print on a PostScript printer, you can open this document
      in Microsoft Windows Write to print it on your PostScript printer.

*****************************************************************************

ADDITIONAL INFORMATION FILES

The following Word for Windows documents contain additional information. 
If documents with these names exist in the directory you specify in
Setup, they will not be overwritten. It is recommended that you not
install over an existing Word for Windows installation if you wish to read
the new information contained in these documents.


File Name     Contents
----------    ----------------------------------------------
README.DOC    Additional information about Word for Windows,
              including corrections to descriptions in the
              User's Reference.

CONVINFO.DOC  Information about the conversion programs used
              to convert files stored in other file formats
              to Word for Windows file format.

TECHREF.DOC   Summary of the WordBASIC macro language.

EXAMPLES.DOC  Useful example macros.

KEYCAPS.DOC   Table resembling a keyboard that shows Symbol
              font characters.  You can copy and paste the
              characters from KEYCAPS.DOC into your documents.

ISV.DOC       Information for independent software vendors on
              distributing Word for Windows macros.

MACROS.DOC    Useful printing and utility macros.

*****************************************************************************

DISKS IN THE MICROSOFT WORD FOR WINDOWS PACKAGE

Microsoft Word for Windows is available in four disk configurations:

  Configuration    Type of disks
  -------------    --------------------------------------
   1.2 MB           5.25-inch high-density (1.2 MB) disks
  1.44 MB           3.5-inch high-density (1.44 MB) disks
     720K           3.5-inch double-sided (720K) disks 
     360K           5.25-inch low-density (360K) disks

Your Microsoft Word for Windows package includes both the 1.2 MB
and the 1.44 MB configurations. The 720K and 360K configurations are
available only through the fulfillment offer (look for the coupon in your
Microsoft Word for Windows package).

1.2/1.44 MB Disk name |   720K Disk name   |  360K Disk name
---------------------------------------------------------------
Setup                 |Setup               |Disk 1 (Setup)
Utilities             |Program             |Disks 2-11
Conversions           |Help                |
                      |Proofing Tools      |
                      |Conversions         |
                      |Utilities           |

*****************************************************************************

COMPRESSED FILES

Most of the Microsoft Word for Windows files are now in a compressed
form to save disk space. The Setup program automatically decompresses
files for you. However, if for some reason you need to decompress a
file manually, we've provided a program on the Setup disk, called
DECOMP.EXE, which you can use to decompress files.

Compressed files are identified with a special file name extension in
the form *.??$ where "*" is the file name and "??" are letters used to
indicate the file name extension for the decompressed version of the
file. The table below shows the encoded file name extensions for
compressed files and what the corresponding decompressed file name
extension should be.

   Extension    Corresponding
     for a     Extension for a
  Compressed     Decompressed
     file           file
  ----------     -----------
     .EX$          .EXE 	(Executable file)
     .DL$          .DLL         (Dynamic Link Libraries)
     .FL$          .FLT         (Graphics filters)
     .HL$          .HLP         (Help data file)
     .DC$          .DOC         (Word for Windows document)
     .DT$          .DOT         (Word for Windows document template)
     .FO$          .FON         (Font files for installation into Windows)
     .CB$          .CBT         (Computer Based Tutorial data file)
     .LE$          .LES         (Computer Based Tutorial lesson data file)
     .LI$          .LIB         (Computer Based Tutorial library file)
     .TI$          .TIF         (TIFF graphics format files)
     .PF$          .PFM         (HPPCL downloadable Printer Font Metric files)
     .SF$          .SFP         (Screen Font files)

To copy and decompress a file manually:

1. Copy the program DECOMP.EXE from the Setup disk in your disk format
   to the Word for Windows directory on your hard disk.

2. Copy the file(s) you wish to decompress from the appropriate disk to
   the Word for Windows directory on your hard disk.

3. Use DECOMP.EXE to create the decompressed version of the file(s).
   DECOMP.EXE decompresses the file and gives it the correct file name 
   extension.

   DECOMP file_name.fn$

   Where "file_name.fn$" is the name of the compressed file.

=============================================================================

   If you want to decompress the correct files to generate a WINWORD.EXE,
   and you have a floppy drive called A, perform the following steps:

   1.44 MB, 1.2MB,
and 720K versions: Insert the appropriate disk containing WINWORD.EX$ in
                   drive A. Change to the directory in which you installed 
                   Word for Windows.

                   For example, type: 

                   CD\WINWORD
                      
                   and then press ENTER.

                   Copy the compressed file WINWORD.EX$ from the floppy disk
                   to the Word for Windows directory on your hard disk. For 
                   example:

                   COPY A:WINWORD.EX$ C:\WINWORD

                   and then press ENTER.
                   
                   Now type in either uppercase or lowercase:

                   DECOMP WINWORD.EX$

                   If all has gone well, you now have a functional
                   WINWORD.EXE. After you have confirmed that the
                   WINWORD.EXE functions correctly, you should type,
                   very carefully:
 
                   DEL WINWORD.EX$

                   and then press ENTER. Then type:

                   DEL DECOMP.EXE

                   and then press ENTER. This reclaims a considerable amount 
                   of disk space.


     360K version: On the 360K disks, it was necessary for Microsoft to
                   split the file into two pieces so that WINWORD.EXE could
                   be shipped. These instructions will lead you through
                   reassembling the WINWORD.EXE file.  

                   Insert Disk 8 into the floppy drive. Change to
                   the directory in which you installed Word for Windows.
                   For example, type: 

                   CD\WINWORD

                   and then press ENTER.
                      
                   Copy the compressed file WINWORDA.BI$ from the Program 
                   disk to the Word for Windows directory on your hard disk. 
                   For example, type:

                   COPY A:WINWORDA.BI$ C:\WINWORD

                   and then press ENTER.
                   
                   Remove Disk 8, and then insert Disk 9. Type:

                   COPY A:WINWORDB.BI$ C:\WINWORD

                   and then press ENTER.

                   Now type in either uppercase or lowercase:

                   DECOMP WINWORDA.BI$      and press ENTER, followed by
                   DECOMP WINWORDB.BI$      and press ENTER.

                   After the decompression is completed, type:

                   COPY /b WINWORDA.BIN+WINWORDB.BIN WINWORD.EXE

                   and then press ENTER.

                   If all has gone well, you now have a functional
                   WINWORD.EXE. After you have confirmed that the
                   WINWORD.EXE functions correctly, you should type,
                   very carefully:
 
                   DEL WINWORD?.BI$

                   and then press ENTER. Then type:

                   DEL DECOMP.EXE

                   and then press ENTER. This reclaims a considerable amount 
                   of disk space.

If you experience trouble manually decompressing files, we strongly
recommend that you run the Setup program on the Setup disk to have the
files decompressed for you automatically.

*****************************************************************************

LIST OF MICROSOFT WORD FOR WINDOWS FILES

The following table lists all files on the disks in the Microsoft
Word for Windows package. Files are grouped by function. File
distribution is different for each of the three disk configurations
(1.2 MB, 1.44 MB, 720K, 360K).  Note that the compressed file name 
extensions are shown below.

NOTE:  Files marked with an asterisk (*) indicate essential
       files required to run Microsoft Word for Windows.

        FILE          |        DESCRIPTION/COMMENTS
=============================================================================
*WINWORD.EXE          |*WINWORD.EXE PROGRAM FILE
 (1.44 MB, 1.2MB, and |
 720K configurations) |
                      |
*WINWORDA.BI$         |
*WINWORDB.BI$         |
 (360K configuration) |
----------------------+------------------------------------------------------
SETUP.EXE             |SETUP FILES
WWORDSU.INF           |
README.TXT            |
README.DC$            |
----------------------+------------------------------------------------------
WHELP.EX$             |  Microsoft Word for Windows Help
WINWORD.HL$           |  Program and Help Data file
----------------------+------------------------------------------------------
                      |SAMPLER FILES
                      |
ARTICLE.DT$           |- These *.DT$ (document template) files
BASIC.DT$             |  are placed in the directory you specify
BRIEF.DT$             |  when you run the Setup program (usually
BROCHDAT.DT$          |  C:\WINWORD). After you start Word, you
BROCHMRG.DT$          |  can create documents based on these
BROCHURE.DT$          |  templates.
CONTRACT.DT$          |
DATADOC.DT$           |
ENVELOPE.DT$          |  Note: If you install Word for Windows
FRMLTR.DT$            |        in a directory that already contains
LBL1COLT.DT$          |        document templates with these names,
LBL15COT..DT$         |        Setup does not overwrite the existing
LBL2COL.DT$           |        document templates.
LBL2COLT.DT$          |
LBL3COL.DT$           |
LBL3COLT.DT$          |
LETTER.DT$            |
MEMO.DT$              |
NORMAL.DT$            |
PAPER.DT$             |
PCWORD.DT$            |
REPORT.DT$            |
LIBRARY\ARTICLE.DC$   |- These *.DC$ (document) files are placed
LIBRARY\BRIEF.DC$     |  in a subdirectory called LIBRARY in the
LIBRARY\BROCHDAT.DC$  |  directory you gave Setup (e.g., 
LIBRARY\BROCHMRG.DC$  |  C:\WINWORD\LIBRARY). They are placed
LIBRARY\BROCHURE.DC$  |  there so as not to conflict with 
LIBRARY\CONTRACT.DC$  |  documents you may create.
LIBRARY\DATADOC.DC$   |
LIBRARY\FRMLTR.DC$    |  Note: To use them, start Word and choose File
LIBRARY\LETTER.DC$    |        Open. Type LIBRARY and then press ENTER.
LIBRARY\MEMO.DC$      |        This list of documents appears in the Files
LIBRARY\MONIQUE.TI$   |        box, making it easy for you to open a
LIBRARY\PAPER.DC$     |        specific document. (MONIQUE.TIF is a TIFF
LIBRARY\REPORT.DC$    |        graphics image. Use the Insert Picture 
LIBRARY\TABLES.DC$    |        command.)
----------------------+------------------------------------------------------
                      |  These *.DC$ (document) files are placed in the
                      |  directory you specify when you run Setup.
                      |  Because of their important content, they are not
                      |  placed in the LIBRARY subdirectory.
                      |  For more information, see "ADDITIONAL INFORMATION
                      |  FILES" at the beginning of this file.
                      |
MACROS.DC$            |- Contains useful printing and utility macros
EXAMPLES.DC$          |- Contains helpful Word for Windows options
ISV.DC$               |- Information for independent software vendors
KEYCAPS.DC$           |- Shows characters available in the Symbol font
TECHREF.DC$           |- Provides information on WordBASIC, the powerful,
                      |  built-in Word for Windows macro language.
----------------------+------------------------------------------------------
DISSERT.DT$           |- Template for creating academic dissertations.
TERM.DT$              |- Template for creating academic term papers.
----------------------+------------------------------------------------------
LEX-??.DL$            |- Spelling program
LEX-??.DA$            |- Spelling data file
SYN-??.DL$            |- Thesaurus program
SYN-??.DA$            |- Thesaurus data file
----------------------+------------------------------------------------------
HYPH.DA$              |- Hyphenation data file
PCW-RTF.DAT           |- Information on remapping the TmsRmn font. Use for 
                      |  Microsoft Word for DOS documents created with an 
                      |  an HP LaserJet printer and converted to Word for 
                      |  Windows. (Print this file for more information.)
----------------------+------------------------------------------------------
WINWORD.CB$           |MICROSOFT WORD FOR WINDOWS TUTORIAL FILES
WINWORD.LI$           |
WINWORD.LE$           |
                      |
ARTICLE.DC$           |- Example documents and tutorial data
ARTICLE.DT$           |  files (*.DC$, *.DT$, *.TI$, *.LE$)
BOATREP.DC$           |
BRIEF.DT$             |  These files must be in a subdirectory
BROCHDAT.DT$          |  called WINWORD.CBT (which is a
BROCHMRG.DT$          |  subdirectory of the directory
BROCHURE.DT$          |  containing WINWORD.EXE) in order for
CATALOG.DC$           |  the Tutorial to run.
CBTNORM.DT$           |
CONTRACT.DT$          |  These files are shipped in WINWORD.CBT
CUTTER.TI$            |  directories on the following disks:
DATADOC.DT$           |
DOCUMENT.DC$          |  1.2 MB & 1.44 MB disks: CONVERSIONS and UTILITIES
EMPLOY.DC$            |              720K disks: TUTORIAL
EXAMPLE.DT$           |              360K disks: Disks 7-10
FLEET.TI$             |
KITEMEM.DC$           |
KNOTS.DC$             |
LEASEGEN.DC$          |
MACRECO.DC$           |
MAILSYLL.DC$          |
MARKET.DC$            |
MARKNOTE.DC$          |
MEMO.DT$              |
NEPAL.DC$             |
NEWPUNS.DC$           |
NEWS.DC$              |
ORIENT.DC$            |
PACKMOVE.DC$          |
PAPER.DT$             |
PROPOSAL.DT$          |
REVIEW.DC$            |
SAILREP.DC$           |
SKILLS.DC$            |
SUGGSYLL.DC$          |
SWEET.TI$             |
YAWL.TI$              |
----------------------+------------------------------------------------------
HIMEM.SYS             |- Device driver that allows Windows
                      |  to use extended (not expanded) memory.
----------------------+------------------------------------------------------
PREV.FO$              |- Preview screen font for print preview mode
----------------------+------------------------------------------------------
                      |MICROSOFT WORD FOR WINDOWS GRAPHICS
                      |           IMPORT FILTERS
                      |
ADIMPORT.FL$          |- AutoCAD format
CGMMPORT.FL$          |- CompuGraphics Metafile format
DRAW.FL$              |- Micrografx Draw! format
EPSIMP.FL$            |- Encapsulated PostScript EPS format (for use with 
                      |       Windows 3.x only)
HPGLIMP.FL$           |- HPGL format 
IMAMPORT.FL$          |- Zenographics Mirage format
LOTUSPIC.FL$          |- Lotus Graphics (*.PIC) format
PCXIMP.FL$            |- PCX format (for use with Windows 3.x only)
TIFF.FL$              |- Tag Image File Format (TIFF) (for use with Windows
                      |       3.x only)
VIDMPORT.FL$          |- Video Show format
WMF.FL$               |- Windows Metafile format
----------------------+------------------------------------------------------
                      |MICROSOFT WORD FOR WINDOWS DOCUMENT
                      |           CONVERTERS
                      |
CONV-DCA.DL$          |- DCA (RFT) Converter
CONV-MCW.DL$          |- MS Word for the Macintosh 
CONV-MM.DL$           |- MultiMate (3.3, 3.6, Adv. II)
CONV-TX8.DL$          |- Text with Layout (PC-8)
CONV-TXT.DL$          |- Text with Layout (ANSI)
CONV-WP.DL$           |- WordPerfect 4.1, 4.2 
CONV-WP5.DL$          |- WordPerfect 5.0
CONV-WRD.DL$          |- MS Word for DOS
CONV-WRI.DL$          |- MS Windows Write
CONV-WS.DL$           |- MicroPro WordStar
CONV-WWP.DL$          |- MS Works (Word Processor component)
CONVERT.GLY           |- Glossary for use with MS Word for DOS documents
CONVINFO.DC$          |- Conversion information document
----------------------+------------------------------------------------------
                      |MICROSOFT UTILITIES
                      |
MSD.EXE               |- Microsoft Diagnostic program for use during 
                      |  sessions with Microsoft Product Support
DECOMP.EXE            |- Use this in case you want to manually decompress
                      |  any file whose file name ends with "$". See
                      |  "COMPRESSED FILES," earlier in this file, for more 
                      |  information.
                      |   
                      |  Note: This compressed file format is not compatible
                      |        with the COMPRESS/EXPAND format used by
                      |        Windows 3.00 and the Windows 3.00 SDK.
----------------------+------------------------------------------------------
\SYMBOL\*.PF$ *.SF$   |- HP LaserJet Symbol Soft Font files
\SYMBOL.W3\*.PF$      |- HP LaserJet Symbol Soft Font files for Windows 3.00
\SYMBOL\SYMBOL?.FO$   |- Symbol Screen Font files for Windows 2.11
                      |
                      |  Note: Word for Windows Setup will place these files
                      |        in a directory called \PCLFONTS in the root
                      |        of the disk specified while running Setup.
                      |        They are not placed in the directory you 
                      |        specify while running Setup. Setup will make
                      |        the needed soft font entries in WIN.INI for
                      |        you.
======================+======================================================

*****************************************************************************
          END OF MICROSOFT WORD FOR WINDOWS SPECIAL INFORMATION FILE
*****************************************************************************
