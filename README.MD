# Premake Generator - Readme

## Description

Premake Generator is a command line tool for generating C++ projects using the [premake5](https://premake.github.io/) build system. This tool also allows for the inclusion of pre-packaged libraries provided by the user.

## Setup
1. Place `premake-gen.exe` into a folder that is accessible by your system PATH variable or add the location to PATH.
2. Place `premake5.exe`, `premake-licence.txt`, and `generate-vs2022.bat` (included in binary download) into `%APPDATA%\premake-gen\premake` (You will be prompted to do this by the tool if you haven't done it already).
3. You can set your library prefered library path using `premake-gen --libdir <directory>`.

## Usage

The following action flags are available:
- `--help`: Show help dialogue
- `--version`: See version number
- `--list`: Lists all available library names
- `--setup`: Instructions on how to setup a new library
- `--libdir`: Open the set library directory
- `--libdir <directory>`: Set the library directory
- `--appdata`: Open the AppData directory in File Explorer

The main usage structure is `premake-gen` followed by the solution name and then project name. After this you can include the names of any libraries you've added to you library directory as well as any other flags.

- `-dialect <number>`: changes the C++ version (C++ 17 is default).
- `-example`: includes the first library's example file as `Main.cpp` with the rest in the 'examples' folder if available.

`premake-gen <SolutionName> <ProjectName> <Lib(s)> <flag(s)>`

Example (C++ 20 solution named "MyApp" with a project named "Core" that includes the ImGui and yaml-cpp libraries. ImGui's example file will be set to Main.cpp and yaml-cpp's example will be included in an "example" folder):

`premake-gen MyApp Core ImGui yaml-cpp -example -dialect 20`

## Libraries

Libaries can be placed as folders or ZIP files within your specified library directory. Use the following directions to add your library (also available through the tool's `--setup` flag):

1. Build your libraries - This tool only works with built Static (.lib) and Dynamic (.dll) Libraries
2. If using for the first time, Set a location for your  libraies using the  `--libdir <directory>` flag. Example: `premake-gen --libdir "C:\premake-gen\libraries"`
3. Create a new folder or ZIP file in your library directory named after the library you are adding.
3. Place required include headers into a "include" folder in your library folder/ZIP file.
4. Place required static library files (.lib) into a "lib" folder in your library folder/ZIP file.
5. Place required dynamic library files (.dll) into a "bin" folder in your library folder/ZIP file.
6. Create a text file named "library.info" in the root for your library Folder/ZIP File (make sure the extension is ".info" not ".txt").
7. in "library.info", place required project settings all with new lines under @-tagged headings:
    - @defines - list any required preprocessor defines
    - @globalLinks - list any required libraries that need to be linked globaly (without .lib extension)
    - @debugLinks - list any required libraries that need to be linked in debug mode (without .lib extension)
    - @releaseLinks - list any required libraries that need to be linked in release mode (without .lib extension)
    - @additionalIncludeDirs - list include directories/ sub-directories that are not %{prj.name}/include
    - @additionalLibDirs - list library file directories/ sub-directories that are not %{prj.name}/lib
8. Place an example main file into the library folder/ZIP file named `main.cpp`

## Additional Info

For additional information on how to edit your premake5 project, see the [premake5 documentation](https://premake.github.io/docs/).

## Repository Info

This repository uses the [premake5](https://premake.github.io/) build system. Execute `build-vs2022.bat` to generate a Visual Studio 2022 solution.
