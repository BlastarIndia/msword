/* contains resources used by DanDraw */

#define OEMRESOURCE /* for OBM_CLOSE */
#include <windows.h>
#include "resource.h"



BMDS	rgbmds[] =
{
#ifdef RESOURCE
#include "OTLPAT.hb"
#include "SHOWVIS.hb"
#ifdef EIGHT
#include "8RULTGLS.hg"
#include "8paralig.hg"
#include "8ribbon.hg"
#endif /* EIGHT */
#include "8SCRPTPO.hg"
#include "8hdr.hg"
#include "8pageico.hg"
#include "CHDR.hg"
#include "COUTLINE.hg"
#include "CPAGEICO.hg"
#include "CPARALIG.hg"
#include "CRIBBON.hg"
#include "CRULMARK.hg"
#include "CRULTGLS.hg"
#include "CSCRPTPO.hg"
#include "EHDR.hg"
#ifdef EIGHT
#include "EOUTLINE.hg"
#include "VOUTLINE.hg"
#include "VRULMARK.hg"
#endif 
#include "EPAGEICO.hg"
#include "EPARALIG.hg"
#include "ERIBBON.hg"
#include "ERULMARK.hg"
#include "ERULTGLS.hg"
#include "ESCRPTPO.hg"
#include "VHDR.hg"
#include "VPAGEICO.hg"
#include "VPARALIG.hg"
#include "VRIBBON.hg"
#include "VRULTGLS.hg"
#include "VSCRPTPO.hg"
#else
#include "MUSTANG.hg"
#endif
};


RCDS	rgrcdsCur[] =
{
#ifdef RESOURCE
#include "CROSS.hc"
#include "COLUMN.hc"
#include "HELP.hc"
#include "HORZOUT.hc"
#include "LRDRAG.hc"
#include "MIC.hc"
#include "MWHIRES.hc"
#include "MWLORES.hc"
#include "PMSCUR.hc"
#include "PRVWCRS.hc"
#include "SPLIT.hc"
#include "STYNAME.hc"
#include "VERTOUT.hc"
#else
#include "MUSTANG.hc"
#endif
};


RCDS	rgrcdsIcon[] =
{
#ifdef RESOURCE
#include "OLDWORD.hi"
#include "OPUS.hi"
#include "WINWORD.hi"
#include "MW.hi"
#include "DRP.hi"
#include "DT.hi"
#else
#include "MUSTANG.hi"
#endif
};


