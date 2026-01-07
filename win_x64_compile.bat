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

:: 打印系统 PATH 环境变量（不含用户 PATH）
echo ==========================================================
echo System PATH only:
reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path
echo ==========================================================
