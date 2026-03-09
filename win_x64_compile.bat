@echo off
echo ==========================================================
echo compile obs-xxq 64bit start
SETLOCAL

::rem 设置当前目录
::set curDir=%cd%
::rem 设置环境变量
::set DepsPath=%curDir%\xxq-obs-deps
::set QTDIR=C:\Qt\Qt5.14.2_64\5.14.2\msvc2017_64
::mkdir build & cd build
::cmake -A "x64" -G "Visual Studio 15 2017" -DDepsPath=%QTDIR% -DQTDIR=%QTDIR% ..

set PATH=C:\Program Files\CMake\bin;%PATH%

:: 切换到脚本所在目录
cd /d %~dp0

cmake --preset=obs_64

set Local_Path=%~dp0%
echo LocalPath: %Local_Path%
cd %Local_Path%
set VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build
set PATH=%VCINSTALLDIR%\bin;%PATH%
call "%VCINSTALLDIR%\vcvarsall.bat" x64

msbuild .\build64\obs-studio.sln /t:Rebuild /p:Configuration=Debug /p:Platform=x64 /p:SubsystemVersion=5.1
msbuild .\build64\obs-studio.sln /t:Rebuild /p:Configuration=RelWithDebInfo /p:Platform=x64 /p:SubsystemVersion=5.1

echo compile obs-xxq 64bit successfully

endlocal
