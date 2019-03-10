//
// Created by benjamintrapani on 11/27/17.
//

#ifndef TEXTTRANSCRIPTIONCODE_MAPFILEMAKER_HPP
#define TEXTTRANSCRIPTIONCODE_MAPFILEMAKER_HPP

#include <string>
#include <sstream>
#include <fstream>
#include "ImageFileMetadata.h"

template<typename FileIterator>
class MapFileMaker {
public:
	MapFileMaker(const std::string& rootDir, const int isequenceLen) :
		maxSequenceLen(isequenceLen), maxNumImages(0), numImagesProcessed(0), fileIterator(rootDir) {}

	// Returns true if there are more files in the file iterator instance and false otherwise
	bool exportMapFile(std::ofstream& imageMapOut,
		std::ofstream& textOut) {
		unsigned int sequenceID = 0;
		ImageFileMetadata metadata;
		while (fileIterator.getNextMetadata(metadata) && numImagesProcessed < maxNumImages) {
			std::string label = getLabelStringFromImageName(metadata.fileName);
			if (label.size() * 2 <= maxSequenceLen) {
				transform(label.begin(), label.end(), label.begin(), ::toupper);
				if (writeLabel(sequenceID, label, textOut)) {
					writeSequencesForImage(sequenceID, metadata.filePath, imageMapOut);
					++sequenceID;
					++numImagesProcessed;
				}
			}
		}
		return numImagesProcessed >= maxNumImages;
	}

	inline void setMaxNumImages(const size_t val) {
		maxNumImages = val;
	}
	inline void setMaxSequenceLen(const int val) {
		maxSequenceLen = val;
	}
	inline void setDirectory(const std::string& dir) {
		fileIterator = FileIterator(dir);
	}
	inline size_t getNumImagesProcessed() const {
		return numImagesProcessed;
	}

private:
	int maxSequenceLen;
	size_t maxNumImages;
	size_t numImagesProcessed;
	FileIterator fileIterator;

	void writeSequencesForImage(const size_t sequenceID,
		const std::string &path,
		std::ofstream& imageMapOut) {
		writeSequenceEntry(sequenceID, (unsigned char)0, path, imageMapOut);
	}

	void writeSequenceEntry(const size_t sequenceID,
		const unsigned char val,
		const std::string &filePath,
		std::ofstream& imageMapOut) {
		const auto label = (unsigned int)val;
		imageMapOut << sequenceID << '\t' << filePath << '\t' << label << std::endl;
	}

	bool writeLabel(const size_t sequenceID,
		const std::string& label,
		std::ofstream& textOutput) {
		constexpr unsigned char minASCI = 'A';
		constexpr unsigned char maxASCI = 'Z';

		for (unsigned int i = 0; i < label.size(); i++) {
			const auto character = label[i];
			if (character < minASCI || character > maxASCI) {
				return false;
			}
		}

		// the blank character is 26
		const unsigned int padChar = maxASCI + 1 - minASCI;
		for (unsigned int i = 0; i < label.size(); i++) {
			const auto character = label[i] - minASCI;
			writeTextSequenceLine(sequenceID, character, 2, textOutput);
			writeTextSequenceLine(sequenceID, padChar, 1, textOutput);
		}

		const int charsToPadd = maxSequenceLen - (int)label.size() * 2;
		for (int i = 0; i < charsToPadd; i++) {
			writeTextSequenceLine(sequenceID, padChar, 1, textOutput);
		}

		return true;
	}

	void writeTextSequenceLine(const size_t sequenceID,
		const unsigned int value,
		const unsigned int encoding,
		std::ofstream& textOutput) {
		textOutput << sequenceID << " |textLabel " << value << ":" << encoding << std::endl;
	}

	std::string getLabelStringFromImageName(const std::string &imageName) {
		unsigned int underscoreCount = 0;
		std::stringstream result;
		for (auto iter = imageName.begin(); iter != imageName.end() && underscoreCount < 2; ++iter) {
			const auto charHere = *iter;
			if (charHere == '_') {
				underscoreCount++;
			}
			else if (underscoreCount > 0) {
				result << *iter;
			}
		}

		return result.str();
	}
};

#endif //TEXTTRANSCRIPTIONCODE_MAPFILEMAKER_HPP
