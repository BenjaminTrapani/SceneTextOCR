using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using TextTranscriptionCLRComp;
using System.IO;

namespace TextTranscriptionEvalTest
{
	class Program
	{
		static void Main(string[] args)
		{
			const String basePath = "C:\\Users\\btrap\\source\\repos\\OCR\\SceneTextOCR2\\TextTranscriptionCode\\TextTranscriptionWindows\\";
			TextTranscriptionService testService = new TextTranscriptionService(basePath + "Content\\ResNetCRNNNewCNTK32SeqLenOut");
			byte[] imageBytes = File.ReadAllBytes(basePath + "..\\WebTester\\5_conveyed_16772.jpg");
			String recognizedText = testService.recognizeImage(imageBytes);
			Console.WriteLine(recognizedText);
			Console.ReadKey();
			//System.Diagnostics.Debugger.Break();
		}
	}
}
