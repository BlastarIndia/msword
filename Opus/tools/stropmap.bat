mmem on
linkfast /INC:5000 /MAP @opus.lnk
rc -l opus.res opus.exe opus
mmem off
strpmap opus.map opusnew.map strpmap.str
mv opusnew.map opus.map
mapsym opus
