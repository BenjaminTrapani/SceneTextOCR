#include "stdafx.h"
#include <iostream>
#include "DirectoryFileFetcher.h"
#include "ImageValidator.h"


DirectoryFileFetcher::DirectoryFileFetcher(const std::string& rootDir) {
	boost::system::error_code completeErrorCode;
	dirIter = boost::filesystem::recursive_directory_iterator(rootDir, completeErrorCode);
	if (completeErrorCode) {
		throw std::invalid_argument("Unable to open root dir for reading");
	}
}

bool DirectoryFileFetcher::getNextMetadata(ImageFileMetadata& metadata) {
	auto const basePath = boost::filesystem::current_path();
	for (; dirIter != terminalDirectory; ++dirIter) {
		const auto frontierPath = dirIter->path();
		if (frontierPath.extension() == ".jpg") {
			const auto frontierPathFname = frontierPath.filename().string();
			const auto absolutePath = canonical(frontierPath, basePath);
			if (ImageValidator::isImageValid(absolutePath.string())) {
				metadata.fileName = frontierPathFname;
				metadata.filePath = absolutePath.string();
				if (dirIter != terminalDirectory) {
					++dirIter;
				}
				return true;
			}
			else {
				std::cout << "Corrupt image at " << absolutePath << std::endl;
			}
		}
	}
	return false;
}