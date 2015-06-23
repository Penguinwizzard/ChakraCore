setlocal
call %*


cd %~dp0\..\..\..\..\..
echo %CD%
set JSCRIPT_ROOT=%CD%
call :GetName %CD%
set OBJECT_JSCRIPT_DIR=%OBJECT_ROOT%\inetcore\%_NAME%

echo JSCRIPT_ROOT=%JSCRIPT_ROOT%
echo OBJECT_JSCRIPT_DIR=%OBJECT_JSCRIPT_DIR%


set _SKIP_BYTE_CODE_VERIFY=1
build -cz -dir %JSCRIPT_ROOT%\core\manifests;%JSCRIPT_ROOT%\core\lib\backend;%JSCRIPT_ROOT%\core\lib\common;%JSCRIPT_ROOT%\core\lib\parser\release;%JSCRIPT_ROOT%\core\lib\runtime\bytecode\release;%JSCRIPT_ROOT%\core\lib\runtime\math;%JSCRIPT_ROOT%\core\lib\runtime\language;%JSCRIPT_ROOT%\core\lib\runtime\library;%JSCRIPT_ROOT%\core\lib\runtime\types;%JSCRIPT_ROOT%\private\lib\winrt;%JSCRIPT_ROOT%\private\bin\chakra\test;%JSCRIPT_ROOT%\private\bin\jshost;%JSCRIPT_ROOT%\private\lib\memprotectheap\release

exit /B 0


:GetName
set _NAME=%~n1
exit /B0
