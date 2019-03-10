#pragma once
#include <vector>

class ImageValidator
{
public:
	ImageValidator();
	~ImageValidator();
	static bool isImageValid(const std::vector<char>& data, long long int size);
	static bool isImageValid(const std::string& path);
private:
	static bool isCompleteJPEG(const std::vector<char>& data, long long int size);
};

