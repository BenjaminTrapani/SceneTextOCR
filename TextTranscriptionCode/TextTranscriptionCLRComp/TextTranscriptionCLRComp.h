#pragma once
#include <string>
#include "TextTranscriptionModel.h"

namespace TextTranscriptionCLRComp {
	public ref class TextTranscriptionService sealed
	{
	public:
		TextTranscriptionService(System::String^ modelFile);
		virtual ~TextTranscriptionService();
		System::String^ recognizeImage(array<unsigned char>^ dataArray);
	private:
		static System::String^ TextTranscriptionService::stringToPlatformString(std::string inputString);

		TextTranscriptionModel* m_model;
	};
}
