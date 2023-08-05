@echo off
setlocal

if "%~1"=="" (
	echo You need to pass a parameter >&2
	echo Example: build.bat 2
	exit /b
)

set root=%cd%

set WARNINGS=/W3 /WX /wd4996
set NAME=%1

pushd build
clang-cl %root%\%NAME%.c /nologo %WARNINGS% /Fe%NAME%.exe /MD
popd