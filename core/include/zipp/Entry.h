#ifndef ZIPP_ENTRY_H
#define ZIPP_ENTRY_H

#include "DateTime.h"
#include "Path.h"

#include <vector>

namespace zipp
{
	class Entry
	{
		friend class ZipReader;
		struct FilePosition
		{
			uint64_t PosInDirectory;
			uint64_t NumOfFile;
		};

	public:
		bool IsFile() const;
		const Path& GetPath() const;
		bool HasChildren() const;
		size_t NumChildren() const;
		bool HasParent() const;
		Entry& Parent() const;

		Entry& operator[](const size_t index);
		const Entry& operator[](const size_t index) const;

		size_t UncompressedSize() const;
		size_t CompressedSize() const;
		DateTime GetDateTime() const;

	private:
		FilePosition m_position{};
		Path m_path;
		bool m_isFile = true;

		Entry* m_parent = nullptr;
		std::vector<Entry*> m_children;

		size_t m_uncompSize = 0;
		size_t m_compSize = 0;
		DateTime m_dateTime{};
	};
}

#endif //!ZIPP_ENTRY_H