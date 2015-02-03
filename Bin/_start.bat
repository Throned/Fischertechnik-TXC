@echo off
if "%1"/ == ""/ exit
if "%1"/ == "begin"/ goto start
if "%OS%"/ == "Windows_NT"/ goto start

rem start second command shell to avoid problems
rem with environment size, then start batch again
rem with additional first parameter 'begin', goto
rem begin, readjust parameter list and proceed
 
command.com  /E:4096 /C %0 begin %2 %3 %4 %5
exit

:start
set ROOT_BATCH=RUNNING
set START_PATH=%PATH%
call %2 %3 %4 %5
set ROOT_BATCH=

if "%OS%"/ == "Windows_NT"/ pause
