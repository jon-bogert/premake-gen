#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <conio.h>

#include "AppData.h"
#include "ProjectSettings.h"
#include "HowTo.h"

#define TAB std::string("    ")

#define LIB_PATH _APPDATA_ + "/premake-gen/libraries/"
#define PREMAKE_GEN_VERSION "v1.0.1"


std::string path;
std::vector<std::string> args;
std::vector<std::string> libManifest;
std::vector<std::string> fileManifest;

bool GenerateDirectory();
bool CheckPremakeFolder();
void ParseArgs(int argc, char* argv[]);
void PrintHelp();
void PopulateManifest();
void PrintList();
bool ReadLibInfo(ProjectSettings& settings, const std::string& lib);
bool GeneratePremakeFile(const ProjectSettings& settings, const std::string& solution);
bool CopyFiles(const std::string& project, const std::vector<std::string>& libraries, bool useExamples);
bool GenerateGitignore();


int main(int argc, char* argv[])
{
    if (GenerateDirectory())
    {
        (void)CheckPremakeFolder();
        return 0;
    }
    if (!CheckPremakeFolder())
    {
        return 0;
    }

    ParseArgs(argc, argv);
    if (args.empty() || args[0] == "-help" || args[0] == "--help")
    {
        PrintHelp();
        return 0;
    }
    if (args[0] == "-setup" || args[0] == "--setup")
    {
        HowTo();
        return 0;
    }
    if (args[0] == "-version" || args[0] == "--version")
    {
        std::cout << "Premake Generator by Jon Bogert (@xepherin): " << PREMAKE_GEN_VERSION << std::endl;
        return 0;
    }
    if (args[0] == "-dir" || args[0] == "--dir")
    {
        std::cout << "Opening AppData Directory...\n";
        system(("explorer \"" + _APPDATA_ + "\\premake-gen\"").c_str());
        return 0;
    }

    PopulateManifest();

    if (args[0] == "-list" || args[0] == "--list")
    {
        PrintList();
        return 0;
    }

    if (args.size() < 2)
    {
        PrintHelp();
        return 0;
    }

    ProjectSettings settings;
    
    std::vector<std::string> libraries;
    std::string sln = args[0];
    settings.name = args[1];

    bool includeExamples = false;
    for (size_t i = 2; i < args.size(); ++i)
    {
        if (args[i] == "-dialect")
        {
            if (i + 1 >= args.size())
            {
                std::cout << "[ERR] No dialect value supplied";
                return 1;
            }
            try
            {
                settings.dialect = std::stoi(args[i + 1]);
            }
            catch (std::exception e)
            {
                std::cout << "[ERR] Could not parse dialect integer from: " << args[i + 1] << std::endl;
                return 1;
            }
            ++i;
        }
        else if (args[i] == "-windowed")
        {
            settings.kind = ProjectKind::WindowedApp;
        }
        else if (args[i] == "-example" || args[i] == "-examples")
        {
            includeExamples = true;
        }
        else if (std::find(libManifest.begin(), libManifest.end(), args[i]) != libManifest.end())
        {
            libraries.push_back(args[i]);
            if (!ReadLibInfo(settings, args[i]))
                return 1;
        }
        else
        {
            std::cout << "[WARN] '" << args[i] << "' was not recognized as a library or argument. Would you like to continue without it? [y/n]: ";
            char c = _getche();
            if (c != 'y' && c != 'Y')
                return 0;
        }
    }
    if (!GeneratePremakeFile(settings, sln))
        return 1;

    if (!CopyFiles(settings.name, libraries, includeExamples))
        return 1;

    if (!GenerateGitignore())
    {
        return 1;
    }

    std::cout << "Done!" << std::endl;
    
    return 0;
}

bool GenerateDirectory()
{
    if (!std::filesystem::exists(LIB_PATH))
    {
        std::cout << "Generating AppData Directory...\n";
        std::filesystem::create_directories(LIB_PATH);
        return true;
    }
    return false;
}

bool CheckPremakeFolder()
{
    if (!std::filesystem::exists(_APPDATA_ + "/premake-gen/premake"))
    {
        std::cout << "Copy files from provided \"premake\" folder to \"%APPDATA%\\premake-gen\\premake\"\n";
        system(("explorer \"" + _APPDATA_ + "\\premake-gen\"").c_str());
        return false;
    }
    return true;
}

void ParseArgs(int argc, char* argv[])
{
    path = argv[0];
    args.resize(argc - 1);
    for (int i = 1; i < argc; ++i)
    {
        args[i - 1] = argv[i];
    }
}

void PrintHelp()
{
    std::cout << "\nPremake Generator -- Help\n";
    std::cout << "----------------------------------------------------------------------\n";
    std::cout << "--help            | You're already here ;)\n";
    std::cout << "--version         | See version number\n";
    std::cout << "--list            | Lists all available library names\n";
    std::cout << "--setup           | Instructions on how to setup a new library\n";
    std::cout << "--dir             | Open the AppData directory in File Explorer\n";
    std::cout << "------------------|---------------------------------------------------\n";
    std::cout << "USAGE: premake-gen <Solution> <Project> <flags>\n\n";
    std::cout << "-dialect <number> | C++ version override (17 by default)\n";
    std::cout << "-example          | includes the first library example file as Main.cpp\n";
    std::cout << "                  |     with the rest in the 'examples' folder\n";
    std::cout << "<LibName>         | includes that libarary\n";
    std::cout << "----------------------------------------------------------------------\n";
}

void PopulateManifest()
{
    std::cout << "Finding available libraries...\n";
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(LIB_PATH))
    {
        libManifest.push_back(entry.path().filename().u8string());
    }
}

void PrintList()
{
    std::cout << "\nPremake Generator -- Listing Available Libraries:\n";
    std::cout << "----------------------------------------------------------------------\n";
    for (const std::string& lib : libManifest)
    {
        std::cout << "- " << lib << std::endl;
    }
    std::cout << "----------------------------------------------------------------------\n";
}

bool IsWhiteSpace(const std::string& str)
{
    if (str.empty())
        return true;
    for (const char c : str)
    {
        if (c != ' ' &&
            c != '\t' &&
            c != '\n')
            return false;
    }
    return true;
}

void CheckAndPush(std::vector<std::string>& vec, const std::string& str)
{
    if (std::find(vec.begin(), vec.end(), str) != vec.end())
        return;

    vec.push_back(str);
}

bool ReadLibInfo(ProjectSettings& settings, const std::string& lib)
{
    std::cout << "Reading info for Library: " << lib << "\n";
    std::string activeMarker = "";
    std::string line = "";

    std::ifstream info(LIB_PATH + lib + "/library.info");
    if (!info.is_open())
    {
        std::cout << "Could not find or read: " << LIB_PATH + lib + "/library.info" << std::endl;
        return false;
    }

    while (std::getline(info, line))
    {
        if (IsWhiteSpace(line))
            continue;
        
        if (line[0] == '@')
        {
            activeMarker = line.substr(1);
            continue;
        }

        if (activeMarker == "defines")
            CheckAndPush(settings.defines, line);
        else if (activeMarker == "additionalIncludeDirs")
            CheckAndPush(settings.additionalIncludeDirs, line);
        else if (activeMarker == "additionalLibDirs")
            CheckAndPush(settings.additionalLibDirs, line);
        else if (activeMarker == "debugLinks")
            CheckAndPush(settings.debugLinks, line);
        else if (activeMarker == "globalLinks")
            CheckAndPush(settings.globalLinks, line);
        else if (activeMarker == "releaseLinks")
            CheckAndPush(settings.releaseLinks, line);
        else
        {
            std::cout << "[ERR] Unidentified marker '" << activeMarker << "'\n";
            return false;
        }
    }

    return true;
}

std::string KindString(ProjectKind kind)
{
    switch (kind)
    {
    case ProjectKind::ConsoleApp:
        return "ConsoleApp";
    case ProjectKind::WindowedApp:
        return "WindowedApp";
    case ProjectKind::StaticLib:
        return "StaticLib";
    case ProjectKind::SharedLib:
        return "SharedLib";
    }
    return "ERR";
}

bool GeneratePremakeFile(const ProjectSettings& settings, const std::string& solution)
{
    std::cout << "Generating premake5.lua\n";

    std::ofstream file("premake5.lua");
    if (!file.is_open())
    {
        std::cout << "[ERR] Could not create or open: " << "premake5.lua\n";
        return false;
    }

    // Workspace
    file << "workspace \"" << solution << "\"\n";
    file << R"(architecture "x64"
    configurations { "Debug", "Release" }
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

)";
    // Project
    file << "project \"" << settings.name << "\"\n";
    file << TAB << "location \"%{prj.name}\"\n";
    file << TAB << "kind \"" << KindString(settings.kind) << "\"\n";
    file << TAB << "language \"C++\"\n";
    file << TAB << "language \"C++\"\n";
    file << TAB << "targetname \"" << settings.targetName << "\"\n";
    file << TAB << "targetdir (\"bin/\".. outputdir)\n";
    file << TAB << "objdir (\"%{prj.name}/int/\" .. outputdir)\n";
    file << TAB << "cppdialect \"C++" << (int)settings.dialect << "\"\n";
    file << TAB << "staticruntime \"Off\"\n\n";

    //File Types
	file << TAB << "files\n" << TAB << "{\n";
	file << TAB + TAB << "\"%{prj.name}/**.h\",\n";
	file << TAB + TAB << "\"%{prj.name}/**.c\",\n";
	file << TAB + TAB << "\"%{prj.name}/**.hpp\"\n,";
	file << TAB + TAB << "\"%{prj.name}/**.cpp\"\n" + TAB << "}\n\n";

    //Include Directories
    file << TAB << "includedirs\n" << TAB << "{\n";
    for (const std::string& str : settings.additionalIncludeDirs)
    {
        file << TAB + TAB << "\"" << str << "\",\n";
    }
    file << TAB + TAB << "\"%{prj.name}/include\",\n";
    file << TAB + TAB << "\"%{prj.name}/src\"\n" << TAB << "}\n\n";

    //Defines
    if (!settings.defines.empty())
    {
        file << TAB << "defines\n" << TAB << "{\n";
        for (size_t i = 0; i < settings.defines.size(); ++i)
        {
            file << TAB + TAB << "\"" << settings.defines[i] << '\"';
            if (i < settings.defines.size() - 1)
                file << ',';
            file << '\n';
        }
        file << TAB << "}\n\n";
    }

    // Library Directories
    if (settings.additionalIncludeDirs.empty())
    {
        //Cleaner version
        file << TAB << "libdirs \"%{prj.name}/lib\"\n\n";
    }
    else 
    {
        // Multiline Version
        file << TAB << "libdirs\n" << TAB << "{\n";
        for (const std::string& str : settings.additionalIncludeDirs)
        {
            file << TAB + TAB << "\"" << str << "\",\n";
        }
        file << TAB + TAB << "\"%{prj.name}/lib\"\n" << TAB << "}\n\n";
    }

    //Global Links
    if (!settings.globalLinks.empty())
    {
        file << TAB << "links\n" << TAB << "{\n";
        for (size_t i = 0; i < settings.globalLinks.size(); ++i)
        {
            file << TAB + TAB << "\"" << settings.globalLinks[i] << '\"';
            if (i < settings.globalLinks.size() - 1)
                file << ',';
            file << '\n';
        }
        file << TAB << "}\n\n";
    }
    
    //Configurations
    file << TAB << R"(filter "system:windows"
		systemversion "latest"
		defines { "WIN32" }

	filter "configurations:Debug"
		defines { "_DEBUG", "_CONSOLE" }
		symbols "On"
)";
    if (!settings.debugLinks.empty())
    {
        file << TAB + TAB << "links\n" << TAB + TAB << "{\n";
        for (size_t i = 0; i < settings.debugLinks.size(); ++i)
        {
            file << TAB + TAB + TAB << "\"" << settings.debugLinks[i] << '\"';
            if (i < settings.debugLinks.size() - 1)
                file << ',';
            file << '\n';
        }
        file << TAB + TAB<< "}\n";
    }
    file << '\n';

    file << TAB << R"(filter "configurations:Release"
		defines { "NDEBUG", "NCONSOLE" }
		optimize "On"
)";
    if (!settings.releaseLinks.empty())
    {
        file << TAB + TAB << "links\n" << TAB + TAB << "{\n";
        for (size_t i = 0; i < settings.releaseLinks.size(); ++i)
        {
            file << TAB + TAB + TAB << "\"" << settings.releaseLinks[i] << '\"';
            if (i < settings.releaseLinks.size() - 1)
                file << ',';
            file << '\n';
        }
        file << TAB + TAB << "}\n\n";
    }

    return true;
}

void CheckLibFile(const std::filesystem::path& file)
{
    if (file.extension() != ".lib" && file.extension() != ".dll")
        return;

    fileManifest.push_back(file.u8string());
}

bool DoCopy(const std::filesystem::path& source, const std::filesystem::path& destination)
{
    try
    {
        std::filesystem::create_directories(destination);

        for (const std::filesystem::directory_entry& dirEntry : std::filesystem::recursive_directory_iterator(source))
        {
            const std::filesystem::path& path = dirEntry.path();
            std::string relativePathStr = path.string().substr(source.string().length());
            std::filesystem::path dst = std::filesystem::path(destination.string() + relativePathStr);

            if (std::filesystem::is_directory(path))
            {
                std::filesystem::create_directories(dst);
            }
            else
            {
                CheckLibFile(path.filename());
                std::filesystem::copy_file(path, dst, std::filesystem::copy_options::overwrite_existing);
            }
        }
    }
    catch (std::exception& e)
    {
        std::cout << "[ERR] Could not copy files from: " << source << " to " << destination << std::endl;
        return false;
    }
    return true;
}


bool CopyFiles(const std::string& project, const std::vector<std::string>& libraries, bool useExamples)
{
    std::cout << "Copying additional premake files...\n";
    if (!DoCopy(_APPDATA_ + "\\premake-gen\\premake", std::filesystem::current_path()))
        return false;

    bool firstExample = true;
    for (const std::string& lib : libraries)
    {
        std::cout << "Copying required files for library: " << lib << "\n";

        if (std::filesystem::exists(LIB_PATH + lib + "/include"))
        {
            if (!DoCopy(LIB_PATH + lib + "/include", project + "/include"))
                return false;
        }
        if (std::filesystem::exists(LIB_PATH + lib + "/lib"))
        {
            if (!DoCopy(LIB_PATH + lib + "/lib", project + "/lib"))
                return false;
        }
        if (std::filesystem::exists(LIB_PATH + lib + "/bin"))
        {
            if (!DoCopy(LIB_PATH + lib + "/bin", project))
                return false;
        }
        if (useExamples && std::filesystem::exists(LIB_PATH + lib + "/main.cpp"))
        {
            if (firstExample)
            {
                std::cout << "Generating Main file based on library: " << lib << "\n";

                firstExample = false;
                try
                {
                    std::filesystem::copy_file(LIB_PATH + lib + "/main.cpp", project + "/Main.cpp");
                }
                catch (std::exception e)
                {
                    std::cout << "[ERR] Could not copy file from: " << LIB_PATH + lib + "/main.cpp" << " to " << project + "/Main.cpp" << "" << std::endl;
                    return false;
                }
            }
            else
            {
                std::cout << "More than one example file found. Sending " + lib + " example to 'examples/' folder.\n";
                try
                {
                    std::filesystem::create_directories(project + "/examples");
                    std::filesystem::copy_file(LIB_PATH + lib + "/main.cpp", project + "/" + lib + ".cpp");
                }
                catch (std::exception e)
                {
                    std::cout << "[ERR] Could not copy file from: " << LIB_PATH + lib + "/main.cpp" << " to " << project + "/" + lib + ".cpp" << std::endl;
                    return false;
                }
            }
        }
    }

    if (!useExamples)
    {
        if (!std::filesystem::exists(project))
            std::filesystem::create_directories(project);

        std::cout << "Generating basic Main file...\n";

        std::ofstream main(project + "/Main.cpp");
        if (!main.is_open())
        {
            std::cout << "[ERR] Couldn't create standard Main file";
            return false;
        }

        main << R"(#include <iostream>

int main (int argc, char* argv[])
{
    
    return 0;
}
)";
    }

    return true;
}

bool GenerateGitignore()
{
    std::cout << "Generating .gitignore file...\n";
    std::ofstream file(".gitignore");
    if (!file.is_open())
    {
        std::cout << "[ERR] could not create .gitignore file\n";
        return false;
    }

    file << R"(# Visual Studio
*.sln
*.vcxproj
*.vcxproj.filters
*.vcxproj.user
/.vs

# Makefile
Makefile

# Build Dirs
*/int
/bin

# Binaries
*.exe
*.lib
*.dll
*.pdb

# Premake Exception
!premake5.exe

# Required Library Exceptions
)";
    for (const std::string& lib : fileManifest)
    {
        file << '!' << lib << '\n';
    }

    return true;
}
