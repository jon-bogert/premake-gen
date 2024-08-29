#ifndef ZIPP_ZIPREADER_H
#define ZIPP_ZIPREADER_H

#include "Entry.h"
#include "Path.h"

#include <fstream>
#include <functional>
#include <memory>
#include <string>

namespace zipp
{
	class ZipReader
	{
	public:
		ZipReader() = default;
		ZipReader(const std::string& path);
		virtual ~ZipReader();

		ZipReader(const ZipReader&) = delete;
		ZipReader(const ZipReader&&) = delete;
		ZipReader& operator=(const ZipReader&) = delete;
		ZipReader& operator=(const ZipReader&&) = delete;

		bool OpenFile(const std::string& path);
		void Close(); // Close zip file and clear all data
		bool Suspend(); // Keeps info loaded, but closes zip file.
		bool Resume(); // Opens zip file with suspended info

		bool IsOpen() const; // File information is loaded (does not depict if is suspended, check .CanExtract())
		bool IsSuspended() const;
		bool CanExtract() const;

		bool Contains(const Path& path) const;
		bool Contains(const std::string& path) const;

		size_t Size() const;

		Entry& operator[](const Path& path);
		const Entry& operator[](const Path& path) const;
		Entry& operator[](const std::string& path);
		const Entry& operator[](const std::string& path) const;
		Entry& operator[](const size_t index);
		const Entry& operator[](const size_t index) const;

		Entry& Root();

		// Performs callback on only the children within the entry.
		void LevelCallback(Entry& entry, std::function<void(Entry&, void*)> callback, void* userData = nullptr, bool callRoot = false);
		bool LevelCallback(const Path& entryPath, std::function<void(Entry&, void*)> callback, void* userData = nullptr, bool callRoot = false);
		bool LevelCallback(const std::string& entryPath, std::function<void(Entry&, void*)> callback, void* userData = nullptr, bool callRoot = false);

		// Performs callback on only the children within the entry as well as their children and so on.
		void RecursiveCallback(Entry& entry, std::function<void(Entry&, void*)> callback, void* userData = nullptr, bool callRoot = false);
		bool RecursiveCallback(const Path& entryPath, std::function<void(Entry&, void*)> callback, void* userData = nullptr, bool callRoot = false);
		bool RecursiveCallback(const std::string& entryPath, std::function<void(Entry&, void*)> callback, void* userData = nullptr, bool callRoot = false);

		bool ExtractToFile(const Entry& entry, const std::string& filePath);
		bool ExtractToFile(const Path& entryPath, const std::string& filePath);
		bool ExtractToFile(const std::string& entryPath, const std::string& filePath);

		bool ExtractToStream(const Entry& entry, std::ostream& stream);
		bool ExtractToStream(const Path& entryPath, std::ostream& stream);
		bool ExtractToStream(const std::string& entryPath, std::ostream& stream);

		bool ExtractToMemory(const Entry& entry, std::vector<uint8_t>& buffer);
		bool ExtractToMemory(const Path& entryPath, std::vector<uint8_t>& buffer);
		bool ExtractToMemory(const std::string& entryPath, std::vector<uint8_t>& buffer);

	private:
		std::unordered_map<std::string, size_t> m_pathToIndex;
		std::vector<std::unique_ptr<Entry>> m_content;
		std::unique_ptr<Entry> m_rootEntry = nullptr;

		void* m_filePtr = nullptr;
		std::string m_filePath;
		bool m_isSuspended = false;
	};
}

#endif //!ZIPP_ZIPREADER_H
