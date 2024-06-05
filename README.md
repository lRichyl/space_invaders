# Space Invaders Emulator - 1978 Classic (Intel 8080)
This emulator faithfully recreates the 1978 arcade classic, Space Invaders, allowing you to relive the authentic retro gaming experience. Originally developed by Tomohiro Nishikado and released by Taito, Space Invaders ran on the Intel 8080 microprocessor, a significant piece of computing history. Our emulator reproduces the original gameplay mechanics, graphics, and sounds, ensuring an authentic and nostalgic experience. With an accurate emulation of the Intel 8080 microprocessor, you'll enjoy the same performance characteristics as the original arcade machine.
[space_invaders.webm](https://github.com/lRichyl/space_invaders/assets/66743720/4c64b589-fb23-42b3-a086-5c4ab7048d5f)

## Controls
**Add Credit** - Z
### Player 1
**Move left**  - Left arrow key  
**Move right** - Right arrow key  
**Start**      - Enter  
**Shoot**      - Right Control  

### Player 2
**Move left**  - A  
**Move right** - D  
**Start**      - Right shift  
**Shoot**      - Space  

## Build
To build the project run the following premake5 command to generate the visual studio solution.
```Bash
premake5 vs2022
```
You can also use the MinGW compiler using the following command to generate a makefile.
```Bash
premake5 gmake2
```
The build files are generated in the build folder.
After compilation the executable is copied to the space_invaders folder.
