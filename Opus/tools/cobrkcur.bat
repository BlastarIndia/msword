echo off
if "%1"=="" goto usego
echo Make file %1.bat which will break links on all the files
echo   currently checked out in this directory (for mass compile use)
status -l | sed -e "s/^/attrib -r /"  >%1.bat
echo Results of this operation: %1.bat contains:
type %1.bat
goto finis
:usego
echo Make file getout.bat which will break links on all the files
echo   currently checked out in this directory (for mass compile use)
status -l | sed -e "s/^/attrib /"  >getout.bat
echo Results of this operation: getout.bat contains:
type getout.bat
:finis
