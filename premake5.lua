 workspace "MyWorkspace"
   configurations { "Debug", "Release" }
   platforms { "x64" }
   location "build"


project "SpaceInvaders"
   objdir ("build/obj/%{cfg.platform}/%{cfg.buildcfg}")
   targetdir ("build/bin/%{cfg.platform}/%{cfg.buildcfg}")
   debugdir ""

   kind "WindowedApp"
   language "C++"

   files {"code/**.cpp" }

    -- Common settings
   libdirs { "vendor/sdl2/lib"}
   -- links {"SDL2main", "SDL2",  "SDL2_mixer", "SDL2_ttf", "shell32"}
   includedirs {"code", "vendor/sdl2/include"}

   postbuildcommands {
     "{COPYFILE} %{wks.location}/../vendor/sdl2/lib/SDL2.dll %{wks.location}/%{cfg.targetdir}",
     "{COPYFILE} %{wks.location}/../vendor/sdl2/lib/SDL2_mixer.dll %{wks.location}/%{cfg.targetdir}",
     "{COPYFILE} %{wks.location}/../vendor/sdl2/lib/SDL2_ttf.dll %{wks.location}/%{cfg.targetdir}",
     "{COPYFILE} %{wks.location}/%{cfg.targetdir}/SpaceInvaders.exe %{wks.location}/.."
   }

   -- Compiler-specific settings
   filter "toolset:gcc or toolset:clang"
      links {"mingw32", "SDL2main", "SDL2",  "SDL2_mixer", "SDL2_ttf", "shell32"}
      buildoptions { "-Wall", "-Wextra"}
      linkoptions { "-pthread", "-mwindows" }

   filter "toolset:msc*"
      links {"SDL2main", "SDL2",  "SDL2_mixer", "SDL2_ttf", "shell32"}
      buildoptions { "/W4" }
      linkoptions { "/SUBSYSTEM:WINDOWS" }

   filter "platforms:x64"
      architecture "x64"

    -- Configuration-specific settings
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

   


--    copy /Y "$(SolutionDir)SDL2.dll" "$(SolutionDir)bin\x64\Debug\"
-- copy /Y "$(SolutionDir)SDL2_mixer.dll" "$(SolutionDir)bin\x64\Debug\"
-- copy /Y "$(SolutionDir)SDL2_ttf.dll" "$(SolutionDir)bin\x64\Debug\"