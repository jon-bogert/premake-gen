#pragma once

void HowTo()
{
    std::cout << R"(
Premake Generator -- How to Setup a Library
----------------------------------------------------------------------
1. Build your libraries - This tool only works with built
   Static (.lib) and Dynamic (.dll) Libraries
2. If using for the first time, Set a location for your libraies using 
   the  "--libdir <directory>" flag.
   Example > premake-gen --libdir "C:\premake-gen\libraries"
3. Create a new folder in your library directoy named after
   the library you are adding.
3. Place required include headers into a "include" folder in your
   library folder.
4. Place required static library files (.lib) into a "lib" folder in
   your library folder.
5. Place required dynamic library files (.dll) into a "bin" folder in
   your library folder.
6. Create a text file named "library.info" (make sure the extension is
   ".info" not ".txt")
7. Place required project settings all with new lines under @-tagged
   headings
    a. @defines - list any required preprocessor defines
    b. @globalLinks - list any required libraries that need to be
       linked globaly (without .lib extension)
    c. @debugLinks - list any required libraries that need to be
       linked in debug mode (without .lib extension)
    d. @releaseLinks - list any required libraries that need to be
       linked in release mode (without .lib extension)
    e. @additionalIncludeDirs - list include directories/
       sub-directories that are not %{prj.name}/include
    e. @additionalLibDirs - list library file directories/
       sub-directories that are not %{prj.name}/lib
8. Place an example main file into the library folder named "main.cpp"
----------------------------------------------------------------------
)";
}