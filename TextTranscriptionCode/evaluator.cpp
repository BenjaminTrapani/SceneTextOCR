#include "TextTranscriptionModel.h"
#include <iostream>
#include <locale>
#include <codecvt>
#include <string>
#include <fstream>

namespace {
  std::vector<unsigned char> readAllBytesFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)),
				      std::istreambuf_iterator<char>());
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Expected one argument: path to model file" << std::endl;
    return 1;
  }

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wideModelFile = converter.from_bytes(std::string(argv[1]));
  TextTranscriptionModel model(128, 32, 26, 64, wideModelFile);
  model.loadForEval();
  std::cin.clear();
  std::cin.ignore();
  
  bool shouldQuit = false;
  while (!shouldQuit) {
    std::string line;
    while (!std::getline(std::cin, line)) {
      std::cin.clear();
    }
    if (line == "q") {
      shouldQuit = true;
    } else {
      auto imageData = readAllBytesFromFile(line);
      const auto recognizedText = model.evalImage(imageData);
      std::cout << recognizedText << std::endl;
    }
  }

  return 0;
}
