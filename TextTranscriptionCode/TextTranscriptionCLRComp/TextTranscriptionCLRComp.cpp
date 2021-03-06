#include "stdafx.h"

#include "TextTranscriptionCLRComp.h"
#include <msclr\marshal_cppstd.h>
#include <codecvt>

using namespace TextTranscriptionCLRComp;

TextTranscriptionService::TextTranscriptionService(System::String^ modelFile)
{
	std::wstring unmanagedFile = msclr::interop::marshal_as<std::wstring>(modelFile);
	m_model = new TextTranscriptionModel(128, 32, 26, 64, unmanagedFile);
	m_model->loadForEval();
}

System::String^ TextTranscriptionService::stringToPlatformString(std::string inputString) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring intermediateForm = converter.from_bytes(inputString);
	System::String^ retVal = gcnew System::String(intermediateForm.c_str());

	return retVal;
}

System::String^ TextTranscriptionService::recognizeImage(array<unsigned char>^ dataArray)
{
	std::vector<unsigned char> vectorData(dataArray->Length);
	for (int i = 0; i < vectorData.size(); i++)
	{
		vectorData[i] = dataArray[i];
	}

	std::string transcriptionResult = m_model->evalImage(vectorData);
	return stringToPlatformString(transcriptionResult);
}

TextTranscriptionService::~TextTranscriptionService()
{
	delete m_model;
	m_model = nullptr;
}