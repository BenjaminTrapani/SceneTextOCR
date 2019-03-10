//
// Created by benjamintrapani on 11/22/17.
//

#ifndef TEXTTRANSCRIPTIONCODE_TEXTTRANSCRIPTIONMODEL_H
#define TEXTTRANSCRIPTIONCODE_TEXTTRANSCRIPTIONMODEL_H
#include <vector>

namespace CNTK
{
	class Trainer;
	typedef std::shared_ptr<Trainer> TrainerPtr;
	
	class Variable;
	
	class Function;
	typedef std::shared_ptr<Function> FunctionPtr;

	class MinibatchSource;
	typedef std::shared_ptr<MinibatchSource> MinibatchSourcePtr;
}

using VariablePtr = std::shared_ptr<CNTK::Variable>;

class __declspec(dllexport) TextTranscriptionModel {
public:
    TextTranscriptionModel(unsigned int imageWidth,
                           unsigned int imageHeight,
                           unsigned int numClasses,
                           unsigned int batchSize,
						   const std::wstring& modelFile);
    void Train(const std::wstring& trainMapFile,
		const std::wstring& trainTextFile,
		const std::wstring& testMapFile,
		const std::wstring& testTextFile, 
		int numBatches=-1);

	void loadForEval();
	std::string evalImage(std::vector<unsigned char>& data);

private:
    CNTK::MinibatchSourcePtr getMinibatchSource(const std::wstring& mapFile, 
		const std::wstring& textFile,
		unsigned int numSweeps);
    std::string decodeOutputs(const std::vector<float>& lineVal);
	void manuallyProcessMinibatch(CNTK::MinibatchSourcePtr minibatchSource, 
		bool isTesting, size_t maxEvalBatches);
	void displayImage(const std::vector<float>& data, 
		unsigned int width, 
		unsigned int height, 
		int format,
		float intensityScale);

    const unsigned int imageWidth;
    const unsigned int imageHeight;
    const unsigned int numClasses;
	const unsigned int maxSequenceLen;
    const unsigned int batchSize;
	const std::wstring modelFile;
	
    CNTK::TrainerPtr trainer;
	VariablePtr imageInput;
	VariablePtr labelInput;
	VariablePtr convStackOutput;
    CNTK::FunctionPtr model;
    CNTK::FunctionPtr reconciledLabels;

	CNTK::FunctionPtr modelFuncForEval;
	VariablePtr evalInputImage;
	VariablePtr evalInputText;
	VariablePtr evalOutput;
};

#endif //TEXTTRANSCRIPTIONCODE_TEXTTRANSCRIPTIONMODEL_H

