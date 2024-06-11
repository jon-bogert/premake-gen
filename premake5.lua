workspace "premake-gen"
	architecture "x64"
	configurations
	{
		"Debug",
		"Release"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "core"
	location "%{prj.name}"
	kind "ConsoleApp"
	language "C++"
	targetname "%{prj.name}"
	targetdir ("bin/".. outputdir)
	objdir ("%{prj.name}/int/".. outputdir)
	cppdialect "C++17"
	staticruntime "Off"

	files
	{
		"**.h",
		"**.cpp"
	}
	
	includedirs
	{
		"%{prj.name}/include",
		"%{prj.name}/src"
	}
	
	links
	{
		"shell32"
	}
	
	filter "system:windows"
		systemversion "latest"
		defines { "WIN32" }
		
	filter "system:linux"
		systemversion "latest"
		defines { "LINUX" }

	filter "configurations:Debug"
		defines { "_DEBUG", "_CONSOLE" }
		symbols "On"
		
	filter "configurations:Release"
		defines { "NDEBUG", "NCONSOLE" }
		optimize "On"
