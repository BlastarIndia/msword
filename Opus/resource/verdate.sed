{
s/Jan/January/
s/Feb/February/
s/Mar/March/
s/Apr/April/
s/May/May/
s/Jun/June/
s/Jul/July/
s/Aug/August/
s/Sep/September/
s/Oct/October/
s/Nov/November/
s/Dec/December/
s/\([^0-9]\)0/\1/g
s/^/#define szVerDateDef "/
s/ *$/"/
}

