echo off
echo  Name          Seg     Size  > %1.siz
echo  ----          ---     ----  >> %1.siz
map2siz %1.map -2 | sort >> %1.siz
echo. Size   Name           Seg >> %1.siz
echo  ----   ----           --- >> %1.siz
map2siz %1.map -1 | sort >> %1.siz
echo. Seg  Name               Size >> %1.siz
echo  ---  ----               ---- >> %1.siz
map2siz %1.map -0 | sort >> %1.siz
