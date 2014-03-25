/^SHARE\.OBJ = /,/[^\\]$/{s/^.HARE\.OBJ = //
s/^	//
s/\\//
s/$/+/
p
}
/^SHAREDBG\.OBJ = /,/[^\\]$/{s/^.HAREDBG\.OBJ = //
s/^	//
s/\\//
s/$/+/
p
}
/^INTERP\.OBJ = /,/[^\\]$/{s/^.NTERP\.OBJ = //
s/^	//
s/\\//
s/$/+/
p
}
/^MALL\.OBJ = /,/[^\\]$/{s/^.ALL\.OBJ = //
s/^	//
s/\\//
s/$/+/
p
}
/^ASM\.OBJ = /,/[^\\]$/{s/^.SM\.OBJ = //
s/^	//
s/\\//
s/$/+/
p
}
/^MALLDBG\.OBJ = /,/[^\\]$/{s/^.ALLDBG\.OBJ = //
s/^	//
s/\\$/+/
p
}
