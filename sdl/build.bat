@echo off
cd /d %~dp0

SET common_dir=\common\rpc\

setlocal enabledelayedexpansion

for /r %%i in (*.sdl) do (
 set v=%%~dpi
 call :loop
 set parent=!%~dp0!
 set parent=!parent:~0,-4!
 set son=!v:%~dp0=!
 if not exist !parent!%common_dir%!son! (mkdir !parent!%common_dir%!son!)
)

for /r %%i in (*.sdl) do (
 set v=%%i
 call :loop
 set in=!v:%~dp0=!

 set common_out=..%common_dir%!in!
 set common_out=!common_out:.sdl=.h!

 echo compile !in!
 sscc -i !in! -o !common_out! -l cpp
)

setlocal disabledelayedexpansion
pause

:loop
if "!v:~-1!"==" " set "v=!v:~0,-1!" & goto loop
