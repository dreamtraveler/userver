@echo off
cd /d %~dp0


setlocal enabledelayedexpansion

for /r %%i in (*.sdl) do (
 set v=%%i
 call :loop
 set in=!v:%~dp0=!
 dos2unix !in!
)

setlocal disabledelayedexpansion
pause

:loop
if "!v:~-1!"==" " set "v=!v:~0,-1!" & goto loop
