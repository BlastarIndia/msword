if not exist %1 goto finis
echoerr Decoding %1
decode %1 | dec2sym >> %2
:finis
