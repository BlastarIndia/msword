# pmap.awk should be run on three files at once: $segs $funcs pmap.txt
# pmap.txt must be the last file, the other two may be in either order
# $segs and $funcs are created by pmap.bat (see it for explanations)

{
if (FILENAME != prevfile)
    {
    if (FILENAME == "$segs")
        fsegs = 1
    
    if (FILENAME == "$funcs")
        ffuncs = 1

    prevfile = FILENAME
    }

# create array with the names of all pcode segments which have
#  hand native code in them

if (FILENAME == "$segs")
    {
    segs[++cSegs] = $1
    next
    exit
    }

# create array with names of native functions in these segments

if (FILENAME == "$funcs")
    {
    funcs[++cFuncs] = $1
    next
    exit
    }

if (FILENAME != "pmap.txt")
    {
    print "Error: pmap.txt not specified on command line"
    exit
    }

if (fsegs == 0)
    {
    print "Error: $segs not specified before pmap.txt"
    exit
    }

if (ffuncs == 0)
    {
    print "Error: $funcs not specified before pmap.txt"
    exit
    }

# if we get this far, we are processing pmap.txt

found = "no"

# find out if the current line is a segment which has hand native code
# it will be in the file $segs (thus the array segs) if so

for (i = 1; i <= cSegs; i++)
    {
    if (segs[i] == $1)
        {
        found = "yes"
        break
        }
    }

# if no hand native code in this segment, just output the name as is

if (found == "no")
    print $1

# otherwise, output the name followed by a colon and the names of the
# native functions that are in the same segment, separated by commas

else
    {
    state = "ignore"
    lineout = sprintf ("%s:", $1)

    for (j = 1; j <= cFuncs; j++)
        {
        # we are in the group of functions in this segment and have already
        #  output a function name
        if (state == "inseg")
            {
            if (funcs[j] == "$")  # $ on line by itself terminates the group
                break
            else if (length(lineout funcs[j]) + 2 < 244)
                lineout = lineout sprintf (", %s", funcs[j])
            else  # line too long; ignore remaining functions
                continue
            }

        # we have found the seg name but have not output any function names
        if (state == "first")
            {
            if (funcs[j] == "$")
                break
            else
                {
                lineout = lineout sprintf ("%s", funcs[j])
                state = "inseg"
                }
            }

        # we are not in the group of functions in this segment
        if (state == "ignore" && funcs[j] == $1)
            state = "first"   # we just found this segment
        }
    print lineout
    }
}


