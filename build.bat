@echo off

IF NOT EXIST build mkdir build 
pushd build
cl /Fe"SpaceInvaders" -Zi -FC -W3 -DBUILD_SLOW=1 ..\code\*.cpp ..\vendor\sdl2\lib\SDL2.lib ..\vendor\sdl2\lib\SDL2main.lib ..\vendor\sdl2\lib\SDL2_mixer.lib ..\vendor\sdl2\lib\SDL2_ttf.lib shell32.lib  -I..\vendor\sdl2\include /link /SUBSYSTEM:CONSOLE

popd
