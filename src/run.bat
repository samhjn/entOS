
cd /d "%~dp0"
call ..\tools\setpath.bat
call mount.bat
make
call unmount.bat
pause
call boot.bat
rem call mount.bat