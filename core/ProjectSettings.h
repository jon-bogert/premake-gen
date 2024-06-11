#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class ProjectKind : uint8_t
{
	ConsoleApp,
	WindowedApp,
	StaticLib,
	SharedLib,
};

struct ProjectSettings
{
	std::string name = "Core";
	ProjectKind kind = ProjectKind::ConsoleApp;
	std::string targetName = "%{prj.name}";
	uint8_t dialect = 17;

	std::vector<std::string> additionalIncludeDirs;
	std::vector<std::string> additionalLibDirs;
	std::vector<std::string> globalLinks;
	std::vector<std::string> debugLinks;
	std::vector<std::string> releaseLinks;
	std::vector<std::string> defines;
};