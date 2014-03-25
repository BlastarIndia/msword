/^OBJECTS = /,/[^\\]$/{s/^.BJECTS = //
s/^	//
s/\\$/+/
p
}
