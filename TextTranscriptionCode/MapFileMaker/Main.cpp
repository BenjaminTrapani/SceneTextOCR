#ifdef WIN32
#include "stdafx.h"
#endif
#include <sstream>
#include <iostream>
#include <climits>

#include "MapFileMaker.hpp"
#include "DirectoryFileFetcher.h"

int main(int argc, char **argv) {
	if (argc != 9) {
		std::cerr << "Usage: MapFileMaker [rootDirectory] [imageMapOutTrain] [textMapOutTrain] ";
		std::cerr << "[imageMapOutTest] [textMapOutTest] [maxSequenceLen] [numTrainImages] [maxImages]" << std::endl;
		return 1;
	}

	const std::string rootDir(argv[1]);
	std::ofstream trainImageFileOut(argv[2]);
	std::ofstream trainTextMapOut(argv[3]);
	std::ofstream testImageFileOut(argv[4]);
	std::ofstream testTextMapOut(argv[5]);

	const unsigned int maxSeqLen = atoi(argv[6]);
	const size_t numTrainImages = (size_t)atoll(argv[7]);
	const size_t maxImages = (size_t)atoll(argv[8]);
	if (maxImages < numTrainImages) {
		std::cerr << "numTrainImages must be <= maxImages" << std::endl;
		return 1;
	}

	MapFileMaker<DirectoryFileFetcher> maker(rootDir, maxSeqLen);

	std::cout << "Writing training images..." << std::endl;
	maker.setMaxNumImages(numTrainImages);
	const bool moreImages = maker.exportMapFile(trainImageFileOut, trainTextMapOut);
	if (moreImages) {
		const size_t numTestImages = maxImages - numTrainImages;
		std::cout << "Finished writing training images. Will write at most " << numTestImages << " test images" << std::endl;
		maker.setMaxNumImages(maxImages);
		maker.exportMapFile(testImageFileOut, testTextMapOut);
	}
	else {
		std::cout << "Wrote " << maker.getNumImagesProcessed() << " training images and none remain for testing" << std::endl;
	}

	return 0;
}
