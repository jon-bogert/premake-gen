workspace "sln"
architecture "x64"
    configurations { "Debug", "Release" }
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "prj"
    location "%{prj.name}"
    kind "ConsoleApp"
    language "C++"
    language "C++"
    targetname "%{prj.name}"
    targetdir ("bin/".. outputdir)
    objdir ("{prj.name}/int/" .. outputdir)
    cppdialect "C++17"
    staticruntime "Off"

    files
    {
        "{prj.name}/**.h",
        "{prj.name}/**.c",
        "{prj.name}/**.hpp"
,        "{prj.name}/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/include",
        "%{prj.name}/src"
    }

    libdirs "lib"

    filter "system:windows"
		systemversion "latest"
		defines { "WIN32" }

	filter "configurations:Debug"
		defines { "_DEBUG", "_CONSOLE" }
		symbols "On"

    filter "configurations:Release"
		defines { "NDEBUG", "NCONSOLE" }
		optimize "On"
