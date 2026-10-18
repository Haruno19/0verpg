:TOP
@CLS
@ECHO OFF
:BEGIN
if not exist C:\MingW\bin\0verpg.exe (
COPY 0verpg.exe C:\MingW\bin\0verpg.exe
)
CLS
:loop
C:\MingW\bin\0verpg.exe en
PAUSE
GOTO loop