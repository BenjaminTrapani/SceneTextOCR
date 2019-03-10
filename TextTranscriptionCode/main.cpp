#include "stdafx.h"

#include <iostream>
#include "TextTranscriptionModel.h"
#include <climits>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 5){
        cout << "Usage: TextTranscriptionModel [trainImageMapFile.txt] [trainTextMapFile.txt] [evalImageMapFile.txt] [evalTextMapFile.txt]" << endl;
    }
    TextTranscriptionModel model(128, 32, 26, 64, 
		L"C:\\Users\\btrap\\source\\repos\\OCR\\SceneTextOCR\\TextTranscriptionCode\\TextTranscriptionWindows\\Content\\ResNetCRNNNewCNTK32SeqLenOut");
    const string mapFileName(argv[1]);
    const string textFileName(argv[2]);
	const string evalMapFile(argv[3]);
	const string evalTextFile(argv[4]);

    const wstring wideMapFileName(mapFileName.begin(), mapFileName.end());
    const wstring wideTextMapFileName(textFileName.begin(), textFileName.end());
	const wstring wideEvalMapFile(evalMapFile.begin(), evalMapFile.end());
	const wstring wideEvalTextFile(evalTextFile.begin(), evalTextFile.end());
	
    model.Train(wideMapFileName, wideTextMapFileName, wideEvalMapFile, wideEvalTextFile);
    return 0;
}