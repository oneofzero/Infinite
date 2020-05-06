echo off
setlocal enabledelayedexpansion
set projs[0] =crypto
set projs[1] =ssl
set rtpath=%cd%
echo rootpath is %rtpath%
for /f "tokens=2* delims=.=" %%a in ('set projs') do (
echo %%a
cd %rtpath%
cd %%a
luamake android_arm %1%
rem echo !ERRORLEVEL!
if !ERRORLEVEL!== 0 (echo build ok) else (goto error)

luamake android_x86 %1%
if !ERRORLEVEL!== 0 (echo build ok) else (goto error)

  )
echo allbuild success!
goto end

:error
echo ERROR!

:end
pause
