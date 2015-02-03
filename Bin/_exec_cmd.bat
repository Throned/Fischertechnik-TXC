@echo off
if "%1"/ == ""/ exit
if "%ROOT_BATCH%"/ == ""/ %1\_start go %0 %1 %2 %3

set PATH=%1;%PATH%
set COM_PORT=%3
if "%3"/ == ""/ call set_port

4cmd_ft %2 %COM_PORT%
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
