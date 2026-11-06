@echo off
rem Build ca hai ban: build.bat       Chay kiem thu: build.bat test
setlocal
cd /d "%~dp0"
if not exist build mkdir build

set "LOGIC=src\nhaplieu.c src\suatchieu.c src\ve.c src\thongke.c src\tepdulieu.c"
set "APP=src\giaodien.c src\dohoa.c %LOGIC%"
set "CONSOLE=src\main.c src\manhinh.c src\ui.c %LOGIC%"
set "TEST=tests\kiemthu.c %LOGIC%"

where gcc >nul 2>nul
if %errorlevel%==0 goto dung_gcc

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto khong_co
set "VSPATH="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
if not defined VSPATH goto khong_co
call "%VSPATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>nul

set "CO=/nologo /utf-8 /W4 /O2 /D_CRT_SECURE_NO_WARNINGS /Fo:build\"
if /i "%~1"=="test" goto test_cl
if exist src\giaodien.c cl %CO% %APP% /Fe:rapchieuphim.exe /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib ole32.lib windowscodecs.lib uuid.lib || exit /b 1
cl %CO% %CONSOLE% /Fe:rapchieuphim_console.exe || exit /b 1
echo.
echo Da build xong
exit /b 0

:test_cl
cl %CO% %TEST% /Fe:build\kiemthu.exe || exit /b 1
"%~dp0build\kiemthu.exe"
exit /b %errorlevel%

:dung_gcc
set "CO=-std=c11 -Wall -Wextra -O2 -finput-charset=UTF-8 -fexec-charset=UTF-8"
if /i "%~1"=="test" goto test_gcc
if exist src\giaodien.c gcc %CO% -municode -mwindows %APP:\=/% -o rapchieuphim.exe -lgdi32 -lole32 -lwindowscodecs -luuid -lm || exit /b 1
gcc %CO% %CONSOLE:\=/% -o rapchieuphim_console.exe || exit /b 1
echo.
echo Da build xong
exit /b 0

:test_gcc
gcc %CO% %TEST:\=/% -o build/kiemthu.exe || exit /b 1
"%~dp0build\kiemthu.exe"
exit /b %errorlevel%

:khong_co
echo Khong tim thay trinh bien dich C. Hay cai MinGW-w64 hoac Visual Studio Build Tools.
exit /b 1
