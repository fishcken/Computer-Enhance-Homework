@echo off
setlocal
set root=%cd%

set WARNINGS=/W3 /WX /wd4996

pushd build
cl %root%\sim.cpp /nologo %WARNINGS% /Fesim.exe /std:c++14 /MD /link sim86_shared_debug.lib /LIBPATH:%root%
popd