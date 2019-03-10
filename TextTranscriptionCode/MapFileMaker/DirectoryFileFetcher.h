#pragma once

#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include <string>

#include "ImageFileMetadata.h"

class DirectoryFileFetcher
{
public:
	DirectoryFileFetcher(const std::string& rootDir);
	bool getNextMetadata(ImageFileMetadata& metadata);

private:
	boost::filesystem::recursive_directory_iterator dirIter;
	const boost::filesystem::recursive_directory_iterator terminalDirectory;
	const std::string rootDir;
};
