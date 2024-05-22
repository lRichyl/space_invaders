@echo off

IF NOT EXIST build mkdir build 
pushd build
cl /Fe"SpaceInvaders" -Zi -FC -O2 -W3 -DBUILD_SLOW=1 ..\code\main.cpp ..\vendor\sdl2\lib\SDL2.lib ..\vendor\sdl2\lib\SDL2main.lib shell32.lib -I..\vendor\sdl2\include /link /SUBSYSTEM:CONSOLE

popd
