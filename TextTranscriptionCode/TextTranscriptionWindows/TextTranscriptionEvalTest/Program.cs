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
			if (args.Length != 2)
			{
				Console.WriteLine("Usage: TextTranscriptionEvalTest [ModelFile] [TestImage]");
			}
			else
			{
				String modelFile = args[0];
				String testImage = args[1];
				TextTranscriptionService testService = new TextTranscriptionService(modelFile);
				byte[] imageBytes = File.ReadAllBytes(testImage);
				String recognizedText = testService.recognizeImage(imageBytes);
				Console.WriteLine(recognizedText);
				Console.ReadKey();
			}
		}
	}
}
