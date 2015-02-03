@echo off
if "%1"/ == ""/ exit
if "%ROOT_BATCH%"/ == ""/ %1\_start go %0 %1 %2

set PATH=%1;%PATH%
set COM_PORT=%2
if "%2"/ == ""/ call set_port

echo Loading program file ...
4load_ft *.bin %COM_PORT%
if errorlevel 1 goto end
goto exit

:end
echo.
echo    *****   ****    ****     ***    ****
echo    *       *   *   *   *   *   *   *   *
echo    ***     ****    ****    *   *   ****
echo    *       *  *    *  *    *   *   *  *
echo    *****   *   *   *   *    ***    *   *
echo.

:exit
if exist "%1\ftlib.log" del "%1\ftlib.log"
if exist "%1\ftlib.bak" del "%1\ftlib.bak"
