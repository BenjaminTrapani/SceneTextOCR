#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <fstream>

#include "ImageValidator.h"

ImageValidator::ImageValidator()
{
}


ImageValidator::~ImageValidator()
{
}

bool ImageValidator::isCompleteJPEG(const std::vector<char>& data, long long int size) {
	return (data[0] == (char)0xff &&
		data[1] == (char)0xd8 &&
		data[size - 2] == (char)0xff &&
		data[size - 1] == (char)0xd9);
}

bool ImageValidator::isImageValid(const std::vector<char>& data, const long long int size) {
	bool result;
	if (size < 2 || !ImageValidator::isCompleteJPEG(data, size)) {
		result = false;
	} else {
		cv::Mat dataView(1, (int)size, CV_8UC1, (void*)data.data());
		try {
			auto mat = cv::imdecode(dataView, cv::IMREAD_COLOR);
			result = mat.data != nullptr;
			mat.release();
		}
		catch (cv::Exception e) {
			std::cout << "OpenCV exception: " << e.what() << std::endl;
			result = false;
		}
	}
	return result;
}

struct StreamRedirect {
	StreamRedirect(std::ostream& ibuffer, std::streambuf * new_buffer)
		: buffer(ibuffer), old(buffer.rdbuf(new_buffer))
	{ }

	~StreamRedirect() {
		buffer.rdbuf(old);
	}

private:
	std::ostream& buffer;
	std::streambuf * old;
};

bool ImageValidator::isImageValid(const std::string& path) {
	std::ifstream inputFStream(path, std::ios::binary);
	constexpr size_t blockSize = 1024;
	std::vector<char> buf(blockSize);
	std::vector<char> imageData;

	while (!inputFStream.eof()) {
		inputFStream.read(buf.data(), blockSize);
		const size_t bytesRead = inputFStream.gcount();
		imageData.insert(std::end(imageData), std::begin(buf), std::begin(buf) + bytesRead);
	}
	std::stringstream errorBuffer;
	StreamRedirect redir(std::cerr, errorBuffer.rdbuf());

	if (isImageValid(imageData, imageData.size())) {
		cv::Mat decodedFromFile;
		try {
			decodedFromFile = cv::imread(path, cv::IMREAD_COLOR);
			std::string errorContents = errorBuffer.str();
			if (errorContents.size() > 0) {
				std::cout << "OpenCV wrote error to stdout" << errorContents << std::endl;
				return false;
			}
		}
		catch (...) {
			std::cout << "OpenCV imread exception" << std::endl;
			return false;
		}
		return decodedFromFile.data != nullptr;
	}
	return false;
}