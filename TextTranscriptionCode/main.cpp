#ifdef WIN32
#include "stdafx.h"
#endif

#include <iostream>
#include "TextTranscriptionModel.h"
#include <climits>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 6){
        cout << "Usage: TextTranscriptionModel [trainImageMapFile.txt] [trainTextMapFile.txt] [evalImageMapFile.txt] [evalTextMapFile.txt] [ModelFileName]" << endl;
    }
    const string mapFileName(argv[1]);
    const string textFileName(argv[2]);
	const string evalMapFile(argv[3]);
	const string evalTextFile(argv[4]);
	const string modelFileName(argv[5]);

    const wstring wideMapFileName(mapFileName.begin(), mapFileName.end());
    const wstring wideTextMapFileName(textFileName.begin(), textFileName.end());
	const wstring wideEvalMapFile(evalMapFile.begin(), evalMapFile.end());
	const wstring wideEvalTextFile(evalTextFile.begin(), evalTextFile.end());
	const wstring wideModelFileName(modelFileName.begin(), modelFileName.end());

	TextTranscriptionModel model(128, 32, 26, 64, wideModelFileName);
    model.Train(wideMapFileName, wideTextMapFileName, wideEvalMapFile, wideEvalTextFile);

    return 0;
}
