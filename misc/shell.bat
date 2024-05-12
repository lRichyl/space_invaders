@echo off
subst w: c:\work
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
set path = w:\work\handmade\misc;%path%