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

:: 打印当前 PATH 环境变量
echo ==========================================================
echo Current PATH:
echo %PATH%
echo ==========================================================
