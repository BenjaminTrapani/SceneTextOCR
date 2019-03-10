#pragma once
#include <string>
#include "archive.h"
#include "archive_entry.h"
#include "ImageFileMetadata.h"
#include <vector>

class TarGZFileFetcher
{
public:
	TarGZFileFetcher(const std::string& rootPath);
	~TarGZFileFetcher() noexcept(false);

	bool getNextMetadata(ImageFileMetadata& metadata);

private:
	const std::string rootPath;
	struct archive* pArchiveToRead;
	struct archive_entry* pAE;
};