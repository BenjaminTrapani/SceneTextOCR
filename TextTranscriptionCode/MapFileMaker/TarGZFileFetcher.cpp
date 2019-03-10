#include "stdafx.h"
#include "TarGZFileFetcher.h"
#include "boost/filesystem/path.hpp"
#include <sstream>
#include <iostream>
#include "ImageValidator.h"

TarGZFileFetcher::TarGZFileFetcher(const std::string& rootPath): rootPath(rootPath), pArchiveToRead(archive_read_new())
{
	archive_read_support_compression_all(pArchiveToRead);
	archive_read_support_format_all(pArchiveToRead);
	const int status = archive_read_open_filename(pArchiveToRead, rootPath.c_str(), 16384);
	if (status != ARCHIVE_OK) {
		throw std::invalid_argument("Unable to open archive file for reading");
	}
	pAE = archive_entry_new();
}

TarGZFileFetcher::~TarGZFileFetcher() noexcept(false)
{
	archive_entry_free(pAE);
	if (archive_read_close(pArchiveToRead) != ARCHIVE_OK) {
		throw std::ios_base::failure("Unable to close archive file");
	}
}

bool TarGZFileFetcher::getNextMetadata(ImageFileMetadata& metadata) {
	int status;
	bool foundFile = false;
	while ((status = archive_read_next_header2(pArchiveToRead, pAE)) == ARCHIVE_OK && !foundFile) {
		// full path format: rootPath@entryPath
		// File name is last path component of entry_pathname
		const char* pathname = archive_entry_pathname(pAE);
		const boost::filesystem::path boostPath(pathname);
		if (boostPath.extension() == ".jpg") {
			// check if image is valid format
			const auto entrySize = archive_entry_size(pAE);
			if (entrySize < 0) {
				std::cout << "Error fetching archive entry size for archive " << pathname << std::endl;
			}
			else {
				std::vector<char> dataBuff(entrySize);
				const auto bytesRead = archive_read_data(pArchiveToRead, &dataBuff.data()[0], entrySize);
				if (!ImageValidator::isImageValid(dataBuff, bytesRead)) {
					std::cout << "Corrupt image at " << pathname << std::endl;
				}
				else {
					const std::string filename = boostPath.filename().string();
					std::stringstream fullPath;
					fullPath << rootPath << "@" << pathname;

					metadata.fileName = filename;
					metadata.filePath = fullPath.str();
					foundFile = true;
				}
			}
		}
	}

	if ((status != ARCHIVE_OK && status != ARCHIVE_EOF)) {
		const char* archiveErrorStr = archive_error_string(pArchiveToRead);
		std::stringstream errorStream;
		errorStream << "Unable to read archive header: " << archiveErrorStr;
		throw std::invalid_argument(errorStream.str());
	}

	return status == ARCHIVE_OK && foundFile;
}