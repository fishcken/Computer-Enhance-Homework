@echo off
setlocal

set root=%cd%

set WARNINGS=/W3 /WX /wd4996

pushd build
clang-cl %root%\decode.c /nologo %WARNINGS% /Fedecode.exe /MD /std:c11
popd