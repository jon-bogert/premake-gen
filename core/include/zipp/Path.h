#ifndef ZIPP_PATH_H
#define ZIPP_PATH_H

#include <ostream>
#include <string>
#include <vector>

namespace zipp
{
	class Path
	{
	public:
		Path() = default;
		Path(const Path&) = default;
		Path(std::string path);
		Path(const std::vector<std::string> path);
		Path(std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator end);

		~Path() = default;

		Path& operator=(const Path& other) = default;
		Path& operator=(std::string path);

		//Add with separator "folder/file" + ".ext" = "folder/file/.ext"
		Path operator/(const Path& other);
		Path operator/(const std::string& path);

		Path& operator/=(const Path& other);
		Path& operator/=(const std::string& path);

		//Add without separator "folder/file" + ".ext" = "folder/file.ext"
		Path operator+(const Path& other);
		Path operator+(const std::string& path);

		Path& operator+=(const Path& other);
		Path& operator+=(const std::string& path);

		std::string AsString() const;
		Path Name() const;				// includes extension if available
		Path Extension() const;			// includes '.'
		Path Stem() const;				// Name without extension
		Path Parent() const;

		Path SubDirectory(size_t offset, size_t count = SIZE_MAX) const;

		bool HasParent() const;
		bool HasExtension() const;		// Use Extension() and check .empty() if
										// you need the extention contents anyways
		bool Empty() const;
		void Clear();
		size_t DirectoryCount() const;

		friend std::ostream& operator<<(std::ostream& os, const Path& path)
		{
			os << path.AsString();
			return os;
		}

	private:
		std::vector<std::string> m_content;
	};
}

#endif //!ZIPP_PATH_H
