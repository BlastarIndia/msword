/time/ b fixit
/Log/  b fixit
/enlist/s/^.*//
/^.* in  / b check
b
:fixit
s/^.*//
$!N
s/\n//
b
:check
/\.[Cc] / b obj
/\.[Aa][Ss][Mm] / b obj
/\.[Tt][Xx][Tt] / b obj
/\.[Rr][Cc] / b resid
/\.[Cc][Mm][Dd] / b opcmd
/\\[Pp][Mm][Aa][Pp]\.[Tt][Xx][Tt] / b pmap
s/^.*//
b
:obj
s/\...*/\.obj/
s/^.* in      /copy %opusdrv%/
s/$/ \./
b
:resid
s/\...*/\.res/
s/^..*/copy %opusdrv%\\cashmere\\resource\\opus\.res  %mprivate%\\private\\obj\\resource\\opus\.res/
/[Tt][Oo][Oo][Ll][Bb][Oo][Xx]/ b addon
b
:opcmd
s/^..*/copy %opusdrv%\\cashmere\\resource\\opuscmd\.obj  %mprivate%\\private\\obj\\resource\\opuscmd\.obj /
a\
copy %opusdrv%\\cashmere\\resource\\opuscmd\.h %mprivate%\\private\\obj\\resource\\opuscmd.h
b
:pmap
s/\\[^\\\.]*\.[Tt][Xx][Tt] /\\pcodemap\.obj /
s/^/copy %opusdrv%/
s/$/ \./
b
:addon
a\
copy %opusdrv%toolbox.txt %mprivate%
