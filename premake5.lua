workspace "MyWorkspace"
   configurations { "Debug", "Release" }
   platforms { "x64" }
   location "build"


project "SpaceInvaders"
   objdir ("build/obj/%{cfg.platform}/%{cfg.buildcfg}")
   targetdir ("build/bin/%{cfg.platform}/%{cfg.buildcfg}")
   debugdir "data"

   kind "ConsoleApp"
   language "C++"

   files {"code/**.cpp" }

    -- Common settings
   libdirs { "vendor/sdl2/lib"}
   links {"SDL2", "SDL2main", "SDL2_mixer", "SDL2_ttf", "shell32"}
   includedirs {"code", "vendor/sdl2/include"}

   -- Compiler-specific settings
   filter "toolset:gcc or toolset:clang"
      buildoptions { "-Wall", "-Wextra" }
      linkoptions { "-pthread" }

   filter "toolset:msc"
      buildoptions { "/W4" }
      linkoptions { "/SUBSYSTEM:CONSOLE" }

   filter "platforms:x64"
      architecture "x64"

    -- Configuration-specific settings
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

   postbuildcommands {
     "{COPYFILE} ../build/SDL2.dll %{cfg.targetdir}",
     "{COPYFILE} ../build/SDL2_mixer.dll %{cfg.targetdir}",
     "{COPYFILE} ../build/SDL2_ttf.dll %{cfg.targetdir}"
   }


--    copy /Y "$(SolutionDir)SDL2.dll" "$(SolutionDir)bin\x64\Debug\"
-- copy /Y "$(SolutionDir)SDL2_mixer.dll" "$(SolutionDir)bin\x64\Debug\"
-- copy /Y "$(SolutionDir)SDL2_ttf.dll" "$(SolutionDir)bin\x64\Debug\"