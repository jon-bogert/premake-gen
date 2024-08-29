#include <zipp/ZipReader.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <string>
#include <sstream>
#include <conio.h>

#include "AppData.h"
#include "ProjectSettings.h"
#include "HowTo.h"

#define TAB std::string("    ")

#define PREMAKE_GEN_VERSION "v1.1.0"

#ifdef _DEBUG
//#define DBG_ARGS {"test", "prj", "SFML", "zipp", "yaml-cpp", "-example"}
#define DBG_ARGS {"--list"}
#endif // _DEBUG

struct LibDirectoryInfo
{
    std::string name;
    bool isCompressed = false;
};

std::string path;
std::string libDirectory;
std::vector<std::string> args;
std::vector<LibDirectoryInfo> libManifest;
std::vector<std::string> fileManifest;

void GenerateLibDir();
bool CheckPremakeFolder();
void ParseArgs(int argc, char* argv[]);
void PrintHelp();

void PopulateManifest();
void SetLibDir(const std::string& path);
bool CheckLibDir();
void PrintList();

bool ReadLibInfo(ProjectSettings& settings, const LibDirectoryInfo& lib);
bool ReadLibInfo_Zip(ProjectSettings& settings, const std::string& lib);
bool ReadLibInfo_Folder(ProjectSettings& settings, const std::string& lib);

bool GeneratePremakeFile(const ProjectSettings& settings, const std::string& solution);

bool CopyFiles(const std::string& project, const std::vector<LibDirectoryInfo>& libraries, bool useExamples);
bool CopyFiles_Zip(const std::string& project, const std::string& lib, bool useExamples, bool& firstExample);
bool CopyFiles_Folder(const std::string& project, const std::string& lib, bool useExamples, bool& firstExample);
bool GenerateGitignore();


int main(int argc, char* argv[])
{
    if (!CheckPremakeFolder())
    {
        return 0;
    }
#ifdef _DEBUG
    args = DBG_ARGS;
#else //_DEBUG
    ParseArgs(argc, argv);
#endif // else _DEBUG
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
    if (args[0] == "-appdata" || args[0] == "--appdata")
    {
        std::cout << "Opening AppData Directory...\n";
        system(("explorer \"" + _APPDATA_ + "\\premake-gen\"").c_str());
        return 0;
    }

    if (args[0] == "-libdir" || args[0] == "--libdir")
    {
        if (args.size() >= 2)
        {
            SetLibDir(args[1]);
            return 0;
        }
        if (CheckLibDir())
        {
            std::cout << "Opening Library Directory...\n";
            system(("explorer \"" + libDirectory + "\"").c_str());
            return 0;
        }
        return 1;
    }

    if (!CheckLibDir())
    {
        return 1;
    }

    PopulateManifest();

    GenerateLibDir();

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
    
    std::vector<LibDirectoryInfo> libraries;
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
            continue;
        }
        else if (args[i] == "-windowed")
        {
            settings.kind = ProjectKind::WindowedApp;
            continue;
        }
        else if (args[i] == "-example" || args[i] == "-examples")
        {
            includeExamples = true;
            continue;
        }
        auto iter = std::find_if(libManifest.begin(), libManifest.end(),
            [&](const LibDirectoryInfo& info) { return info.name == args[i]; });
        if (iter != libManifest.end())
        {
            LibDirectoryInfo& lib = libraries.emplace_back(*iter);
            if (!ReadLibInfo(settings, lib))
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

void GenerateLibDir()
{
	if (std::filesystem::exists(libDirectory))
		return;

	std::cout << "Generating Library Directory...\n";
	std::filesystem::create_directories(libDirectory);
}

bool CheckPremakeFolder()
{
    if (!std::filesystem::exists(_APPDATA_ + "/premake-gen/premake"))
    {
        std::filesystem::create_directories(_APPDATA_ + "/premake-gen/premake");
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
    std::cout << "--------------------------------------------------------------------------\n";
    std::cout << "--help               | You're already here ;)\n";
    std::cout << "--version            | See version number\n";
    std::cout << "--list               | Lists all available library names\n";
    std::cout << "--setup              | Instructions on how to setup a new library\n";
    std::cout << "--libdir             | Open the set library directory\n";
    std::cout << "--libdir <directory> | Set the library directory\n";
    std::cout << "--appdata            | Open the AppData directory in File Explorer\n";
    std::cout << "---------------------|----------------------------------------------------\n";
    std::cout << "USAGE: premake-gen <Solution> <Project> <flags>\n\n";
    std::cout << "-dialect <number>    | C++ version override (17 by default)\n";
    std::cout << "-example             | includes the first library example file as Main.cpp\n";
    std::cout << "                     |     with the rest in the 'examples' folder\n";
    std::cout << "<LibName>            | includes that libarary\n";
    std::cout << "--------------------------------------------------------------------------\n";
}

void PopulateManifest()
{
    std::cout << "Finding available libraries...\n";
    std::unordered_set<std::string> added;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(libDirectory))
    {
        if (!entry.is_regular_file())
        {
            std::string name = entry.path().filename().u8string();
            if (added.find(name) != added.end())
            {
                auto iter = std::find_if(libManifest.begin(), libManifest.end(), [&](const LibDirectoryInfo& i) {return i.name == name; });
                iter->isCompressed = false;

                std::cout << "[WARNING] Duplicate library \"" + name + "\" found. premake-gen will use the folder version.\n";
                continue;
            }
            added.insert(name);
            libManifest.push_back({ name, false });
            continue;
        }

        if (!entry.path().has_extension() || entry.path().extension() != ".zip")
        {
            std::cout << "[WARNING] Incompatible file in library directory: \"" + entry.path().filename().u8string() + "\". Ignoring...\n";
            continue;
        }

        std::string name = entry.path().stem().u8string();
        if (added.find(name) != added.end())
        {
            std::cout << "[WARNING] Duplicate library \"" + name + "\" found. premake-gen will use the folder version.\n";
            continue;
        }
        added.insert(name);
        libManifest.push_back({ name, true });
    }
}

void SetLibDir(const std::string& path)
{
    if (!std::filesystem::exists(_APPDATA_ + "/premake-gen"))
        std::filesystem::create_directories(_APPDATA_ + "/premake-gen");
    if (!std::filesystem::exists(path))
        std::filesystem::create_directories(path);

    std::ofstream file(_APPDATA_ + "/premake-gen/settings.info");
    file << "@libDirectory\n" << path << std::endl;
}

bool CheckLibDir()
{
    if (!std::filesystem::exists(_APPDATA_ + "/premake-gen/settings.info"))
    {
        std::cout << "No library directory specified. Use '--libdir <directory>' to specify one." << std::endl;
        return false;
    }

    std::ifstream file(_APPDATA_ + "/premake-gen/settings.info");

    std::string tag;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;
        if (line[0] == '@')
        {
            tag = line.substr(1);
            continue;
        }

        if (tag == "libDirectory")
        {
            libDirectory = line;
        }
    }
    return !libDirectory.empty();
}

void PrintList()
{
    std::cout << "\nPremake Generator -- Listing Available Libraries:\n";
    std::cout << "----------------------------------------------------------------------\n";
    for (const LibDirectoryInfo& lib : libManifest)
    {
        std::cout << "- " << ((lib.isCompressed) ? lib.name + " [*]" : lib.name) << std::endl;
    }
    std::cout << "----------------------------------------------------------------------\n";
    std::cout << "[*] = Compressed in ZIP file\n\n";
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

bool ReadLibInfo(ProjectSettings& settings, const LibDirectoryInfo& lib)
{
    std::cout << "Reading info for Library: " << lib.name << "\n";
    
    return (lib.isCompressed) ?
        ReadLibInfo_Zip(settings, lib.name) :
        ReadLibInfo_Folder(settings, lib.name);
}

bool ReadLibInfo_Zip(ProjectSettings& settings, const std::string& lib)
{
    std::string activeMarker = "";
    std::string line = "";

    zipp::ZipReader zipFile(libDirectory + "/" + lib + ".zip");
    if (!zipFile.IsOpen())
    {
        std::cout << "Could not find or read: " << libDirectory + "/" + lib + ".zip" << std::endl;
        return false;
    }

    if (!zipFile.Contains("library.info"))
    {
        std::cout << "Could not find or read: " << libDirectory + "/" + lib + ".zip/library.info" << std::endl;
        return false;
    }
    zipp::Entry libraryFile = zipFile["library.info"];
    std::string infoData;
    infoData.resize(libraryFile.UncompressedSize() + 1, 0);
    zipFile.ExtractToString("library.info", infoData);

    std::stringstream info(infoData);

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

bool ReadLibInfo_Folder(ProjectSettings& settings, const std::string& lib)
{
    std::string activeMarker = "";
    std::string line = "";

    std::ifstream info(libDirectory + "/" + lib + "/library.info");
    if (!info.is_open())
    {
        std::cout << "Could not find or read: " << libDirectory + "/" + lib + "/library.info" << std::endl;
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
    if (settings.additionalLibDirs.empty())
    {
        //Cleaner version
        file << TAB << "libdirs \"%{prj.name}/lib\"\n\n";
    }
    else 
    {
        // Multiline Version
        file << TAB << "libdirs\n" << TAB << "{\n";
        for (const std::string& str : settings.additionalLibDirs)
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
		defines { "NDEBUG", "_CONSOLE" }
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

bool DoCopy_Zip(zipp::ZipReader& zipFile, const zipp::Path& source, const std::filesystem::path& destination)
{
    try
    {
        std::filesystem::create_directories(destination);

        auto lambda = [&](const zipp::Entry& dirEntry, void* userData)
            {
                const zipp::Path& path = dirEntry.GetPath();
                std::filesystem::path dst = std::filesystem::path(destination.string() + "/" + dirEntry.GetPath().SubDirectory(1).AsString());

                if (!dirEntry.IsFile())
                {
                    std::filesystem::create_directories(dst);
                }
                else
                {
                    CheckLibFile(path.Name().AsString());
                    zipFile.ExtractToFile(dirEntry, dst.u8string());
                }
            };
        zipFile.RecursiveCallback(source, lambda, nullptr, false);
    }
    catch (std::exception&)
    {
        std::cout << "[ERR] Could not copy files from: " << source << " to " << destination << std::endl;
        return false;
    }
    return true;
}

bool DoCopy_Folder(const std::filesystem::path& source, const std::filesystem::path& destination)
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
    catch (std::exception&)
    {
        std::cout << "[ERR] Could not copy files from: " << source << " to " << destination << std::endl;
        return false;
    }
    return true;
}

bool CopyFiles(const std::string& project, const std::vector<LibDirectoryInfo>& libraries, bool useExamples)
{
    std::cout << "Copying additional premake files...\n";
    if (!DoCopy_Folder(_APPDATA_ + "\\premake-gen\\premake", std::filesystem::current_path()))
        return false;

    bool firstExample = true;
    for (const LibDirectoryInfo& lib : libraries)
    {
        if (lib.isCompressed)
        {
            if (!CopyFiles_Zip(project, lib.name, useExamples, firstExample))
                return false;
        }
        else
        {
            if (!CopyFiles_Folder(project, lib.name, useExamples, firstExample))
                return false;
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

bool CopyFiles_Zip(const std::string& project, const std::string& lib, bool useExamples, bool& firstExample)
{
    std::cout << "Copying required files for library: " << lib << "\n";

    zipp::ZipReader zipFile(libDirectory + "/" + lib + ".zip");
    if (!zipFile.IsOpen())
    {
        std::cout << "Could not find or read: " << libDirectory + "/" + lib + ".zip" << std::endl;
        return false;
    }

    if (zipFile.Contains("include"))
    {
        if (!DoCopy_Zip(zipFile, zipp::Path("include"), project + "/include"))
            return false;
    }
    if (zipFile.Contains("lib"))
    {
        if (!DoCopy_Zip(zipFile, zipp::Path("lib"), project + "/lib"))
            return false;
    }
    if (zipFile.Contains("bin"))
    {
        if (!DoCopy_Zip(zipFile, zipp::Path("bin"), project))
            return false;
    }
    if (zipFile.Contains("main.cpp"))
    {
        if (firstExample)
        {
            std::cout << "Generating Main file based on library: " << lib << "\n";

            firstExample = false;
            try
            {
                zipFile.ExtractToFile("main.cpp", project + "/Main.cpp");
            }
            catch (std::exception)
            {
                std::cout << "[ERR] Could not copy file from: " << libDirectory + "/" + lib + ".zip/main.cpp" << " to " << project + "/Main.cpp" << "" << std::endl;
                return false;
            }
        }
        else
        {
            std::cout << "More than one example file found. Sending " + lib + " example to 'examples/' folder.\n";
            try
            {
                std::filesystem::create_directories(project + "/../examples");
                zipFile.ExtractToFile("main.cpp", project + "/../examples/" + lib + ".cpp");
            }
            catch (std::exception)
            {
                std::cout << "[ERR] Could not copy file from: " << libDirectory + "/" + lib + "/main.cpp" << " to " << project + "/" + lib + ".cpp" << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool CopyFiles_Folder(const std::string& project, const std::string& lib, bool useExamples, bool& firstExample)
{
    std::cout << "Copying required files for library: " << lib << "\n";

    if (std::filesystem::exists(libDirectory + "/" + lib + "/include"))
    {
        if (!DoCopy_Folder(libDirectory + "/" + lib + "/include", project + "/include"))
            return false;
    }
    if (std::filesystem::exists(libDirectory + "/" + lib + "/lib"))
    {
        if (!DoCopy_Folder(libDirectory + "/" + lib + "/lib", project + "/lib"))
            return false;
    }
    if (std::filesystem::exists(libDirectory + "/" + lib + "/bin"))
    {
        if (!DoCopy_Folder(libDirectory + "/" + lib + "/bin", project))
            return false;
    }
    if (useExamples && std::filesystem::exists(libDirectory + "/" + lib + "/main.cpp"))
    {
        if (firstExample)
        {
            std::cout << "Generating Main file based on library: " << lib << "\n";

            firstExample = false;
            try
            {
                std::filesystem::copy_file(libDirectory + "/" + lib + "/main.cpp", project + "/Main.cpp");
            }
            catch (std::exception e)
            {
                std::cout << "[ERR] Could not copy file from: " << libDirectory + "/" + lib + "/main.cpp" << " to " << project + "/Main.cpp" << "" << std::endl;
                return false;
            }
        }
        else
        {
            std::cout << "More than one example file found. Sending " + lib + " example to 'examples/' folder.\n";
            try
            {
                std::filesystem::create_directories(project + "/../examples");
                std::filesystem::copy_file(libDirectory + "/" + lib + "/main.cpp", project + "/../examples/" + lib + ".cpp");
            }
            catch (std::exception e)
            {
                std::cout << "[ERR] Could not copy file from: " << libDirectory + "/" + lib + "/main.cpp" << " to " << project + "/" + lib + ".cpp" << std::endl;
                return false;
            }
        }
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
