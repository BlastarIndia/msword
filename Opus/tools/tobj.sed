/\.[Cc]$/ b obj
/\.[Aa][Ss][Mm]$/ b obj

/\\[Pp][Mm][Aa][Pp]\.[Tt][Xx][Tt]$/ { 
s/\\[^\\\.]*\.[Tt][Xx][Tt]$/\\pcodemap\.obj/
s/^/del /
w $$pmap.snt
b
}

/\.[Tt][Xx][Tt]$/ b obj

/\.[Cc][Mm][Dd]$/  { 
s/\\[^\\\.]*\.[Cc][Mm][Dd]$/\\opus\.res/
s/^/del /
w $$res.snt
s/\\opus\.res/\\opuscmd\.obj/
w $$cmd.snt
b
}

/\.[Rr][Cc]$/ { 
s/\\[^\\\.]*\.[Rr][Cc]$/\\opus\.res/
s/^/del /
w $$res.snt
b
}

b

:obj
s/\...*$/\.obj/
s/.*\\//
s/^/del /
p


