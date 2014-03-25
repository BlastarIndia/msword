/^BUILD_OBJ = /,/[^\\]$/{s/^.UILD_OBJ = //
s/^	//
s/\\//
s/$/+/
p
}
/^WORDTECH_OBJ = /,/[^\\]$/{s/^.ORDTECH_OBJ = //
s/^	//
s/\\//
s/$/+/
p
}
/^CASHMERE_OBJ = /,/[^\\]$/{s/^.ASHMERE_OBJ = //
s/^	//
s/\\//
s/$/+/
p
}
/^ASM_OBJ = /,/[^\\]$/{s/^.SM_OBJ = //
s/^	//
s/\\$/+/
p
}
