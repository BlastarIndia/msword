i\
echo ******************************************************
h
s/^/echo /
p
g
s/^/command \/c normrtf /
s/\.[Dd][Oo][Cc]/\.rt1/
s/$/ > nul /
p
g
s/^/command \/c normrtf /
s/\.[Dd][Oo][Cc]/\.rt2/
s/$/ > nul /
p
g
s/^/diff /
s/\.[Dd][Oo][Cc]/\.rt1 /
G
s/\n/ /
s/\.[Dd][Oo][Cc]/\.rt2/
s/$/ > ~$$$.$$$/
p
g
s/^/if not errorlevel 1 goto /
s/\.//
p
i\
echo  > ~xxx.xxx
i\
echo.
i\
echo the files are different
i\
cat ~xxx.xxx
i\
cat ~$$$.$$$
i\
if exist ~xxx.xxx rm ~xxx.xxx
i\
if exist ~$$$.$$$ rm ~$$$.$$$
g
s/^/:/
s/\.//

