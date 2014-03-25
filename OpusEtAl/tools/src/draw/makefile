# makefile for Mustang

#
# NOTE: in order to build Mustang.exe, you must copy
#         \cashmere\resource\*.bmp *.ico *.cur to
#       this directory.
#

NAME = mustang

OBJECTS = \
	mustang.obj resource.obj ddprint.obj ddesc.obj ddedit.obj

HEADERS = \
	mustang.hg mustang.hc mustang.hi
#	8hdr.hg 8outline.hg 8pageico.hg 8paralig.hg 8ribbon.hg 8rulmark.hg \
#	8rultgls.hg 8scrptpo.hg chdr.hg coutline.hg cpageico.hg cparalig.hg \
#	crultgls.hg cscrptpo.hg ehdr.hg eoutline.hg epageico.hg eparalig.hg \
#	erultgls.hg escrptpo.hg otlpat.hb showvis.hb vhdr.hg voutline.hg \
#	vribbon.hg vrulmark.hg vrultgls.hg vscrptpo.hg \
#	column.hc cross.hc horzout.hc lrdrag.hc mic.hc mwhires.hc mwlores.hc \
#	styname.hc pmscur.hc vertout.hc oldword.hi opus.hi winword.hi \
#	mw.hi dt.hi drp.hi \
#	cribbon.hg crulmark.hg eribbon.hg erulmark.hg vpageico.hg vparalig.hg\
#	help.hc prvwcrs.hc split.hc

# Settings...

MAKEFILE = makefile

CC = cc
CCFLAGS = -UM_I86 -UMSDOS -UM_I86SM -d -c -Adsn -Gsw -Oas -Zpe

LINK = link4
LFLAGS = /map/li/nod
LIBS = libh slibw slibc/noe

MAPSYM = mapsym

RC = rc
RCFLAGS = -r

SED = sed
SEDFLAGS = -n -f


# Make Executable File

$(NAME).exe: $(OBJECTS) $(NAME).def $(NAME).lnk $(NAME).res
	$(LINK) @$(NAME).lnk
	$(MAPSYM) $(NAME)
	$(RC) $(NAME).res $(NAME).exe
	mv $(NAME).exe ..\..


# Make Resources

$(NAME).res: $(NAME).rc $(NAME).ico
	$(RC) $(RCFLAGS) $(NAME).rc


# Make Link File

$(NAME).lnk: $(MAKEFILE) link.sed
	$(SED) $(SEDFLAGS) link.sed <$(MAKEFILE) >$(NAME).lnk
	echo $(NAME) >>$(NAME).lnk
	echo $(NAME) $(LFLAGS) >>$(NAME).lnk
	echo $(LIBS) >>$(NAME).lnk
	echo $(NAME).def >>$(NAME).lnk

resource.obj:resource.c $(HEADERS)
	$(CC) $(CCFLAGS) $*.c

# Transformations

.SUFFIXES: .obj .lst .def .sym .c .exe .map .hb .hi .hc .hg .bmp .ico .cur


.c.obj:
	$(CC) $(CCFLAGS) $*.c

.rc.res:
	$(RC) $(RCFLAGS) $*.rc

.bmp.hb:
	bitapp $*.bmp $*.hb

.ico.hi:
	bitapp $*.ico $*.hi

.cur.hc:
	bitapp $*.cur $*.hc

.bmp.hg:
	bitapp /g $*.bmp $*.hg

# end of makefile

