@ECHO OFF
SETLOCAL ENABLEEXTENSIONS
SET me=%~n0
SET parent=%~dp0
REM The root tree of the context distro, i.e.
REM SET CTXDIR=C:\data\context\standalone
SET CTXDIR=C:\data\context\context-setup-win64\context

PUSHD %CTXDIR% && POPD || (ECHO "%CTXDIR% not found" && EXIT /B )

call %CTXDIR%\tex\setuptex.bat

C:\Python27\python.exe command_primitives_api.py ^
-m mtxrun.exe ^
-c %CTXDIR%\tex\texmf-context\tex\context\base\mkiv\mult-prm.lua ^
-i %CTXDIR%\tex\texmf-context\tex\context\interface\mkiv\context-en.xml ^
-d %CTXDIR%\tex\texmf-context\tex\context\base\mkiv\char-def.lua 


ECHO "Searching xmllint.exe..."
where /q xmllint.exe
IF ERRORLEVEL 1  (
 ECHO " Not found, could not check if xml files are well-formed."
) ELSE (
  xmllint.exe -noout ConTeXt.xml & ECHO "ConTeXt.xml is well-formed"  
  xmllint.exe -noout APIs/context.xml & ECHO "APIs/context.xml is well-formed"  
)
