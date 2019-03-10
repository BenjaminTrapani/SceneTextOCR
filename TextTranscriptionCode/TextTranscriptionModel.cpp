//
// Created by benjamintrapani on 11/22/17.
//

//#include "pch.h"

#include "stdafx.h"

#include "CNTKLibrary.h"
#include "TextTranscriptionModel.h"
#include "ResNet.h"

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace CNTK;
using namespace std;

class TrainingProgressWriter: public ProgressWriter {
public:
    void OnWriteTrainingUpdate(const std::pair<size_t, size_t>& /*samples*/,
                                                const std::pair<size_t, size_t>& /*updates*/,
                                                const std::pair<double, double>& aggregateLoss,
                                                const std::pair<double, double>& aggregateMetric) override{
    }
    void OnWriteTestUpdate(const std::pair<size_t, size_t>& samples,
                                            const std::pair<size_t, size_t>& /*updates*/,
                                            const std::pair<double, double>& aggregateMetric) override{
		if (samples.second % 8192 == 0) {
			cout << "Test update: " << endl;
			cout << " sample count: " << samples.second << endl;
			cout << " average incorrect character rate = " << aggregateMetric.first / samples.second << ", " << aggregateMetric.second / samples.second << endl;
		}
    }
    void OnWriteTrainingSummary(size_t samples, size_t /*updates*/, size_t /*summaries*/,
                                                 double aggregateLoss, double aggregateMetric,
                                                 size_t /*elapsedMilliseconds*/) override{
		totalSamples += samples;
		++totalSweeps;
        std::cout << "Training summary: " << endl;
		std::cout << " total sweeps: " << totalSweeps << endl;
		std::cout << " total samples: " << totalSamples << endl;
		std::cout << " sample elapsed: " << samples << endl;
		std::cout << " aggregate loss: " << aggregateLoss << endl;
		std::cout << " avg loss per sample: " << aggregateLoss / samples << endl;
		std::cout << " aggregate metric: " << aggregateMetric << endl;
		std::cout << " transcription error: " << aggregateMetric * (double)maxSequenceLen / (double)samples << endl;
    }
	void OnWriteTestSummary(size_t samples, size_t /*updates*/, size_t /*summaries*/,
		double aggregateMetric, size_t /*elapsedMilliseconds*/) override{
		std::cout << "Testing summary: " << endl;
		std::cout << " sample elapsed: " << samples << endl;
		std::cout << " aggregate metric: " << aggregateMetric << endl;
	}
    TrainingProgressWriter(size_t trainingUpdateWriteFrequency, size_t trainingFirstUpdatesToWrite,
                           size_t testUpdateWriteFrequency, size_t testFirstUpdatesToWrite,
                           size_t distributedSyncUpdateWriteFrequency, size_t distributedSyncFirstUpdatesToWrite,
							size_t batchSize, size_t maxSequenceLen):
            ProgressWriter(trainingFirstUpdatesToWrite, trainingFirstUpdatesToWrite, testUpdateWriteFrequency,
                           testFirstUpdatesToWrite, distributedSyncUpdateWriteFrequency, distributedSyncFirstUpdatesToWrite),
			totalSamples(0), totalSweeps(0), batchSize(batchSize), maxSequenceLen(maxSequenceLen){}

private:
	unsigned long long int totalSamples;
	unsigned int totalSweeps;
	size_t batchSize;
	size_t maxSequenceLen;
};

TextTranscriptionModel::TextTranscriptionModel(const unsigned int imageWidth,
                                               const unsigned int imageHeight,
                                               const unsigned int inumClasses,
                                               const unsigned int batchSize,
											   const std::wstring& modelFile):
        imageWidth(imageWidth),
        imageHeight(imageHeight),
        numClasses(inumClasses + 1),
		maxSequenceLen(imageWidth / (unsigned int)pow(2, 2)),
        batchSize(batchSize),
        modelFile(modelFile){
	
	try {
		const unsigned int rnnSize = 256;
		const double glorotInitializerReLU = 1.0;
		const double glorotInitializerOther = 0.8;
		//Reserve the last class for the blank label
		const unsigned int blankTokenID = numClasses - 1;
		imageInput = std::make_shared<Variable>(InputVariable({ imageWidth, imageHeight, 3 },
			DataType::Float, L"imageFeatures", { Axis::DefaultBatchAxis() }));
		labelInput = std::make_shared<Variable>(InputVariable({numClasses}, false,
			DataType::Float, L"inputLabels"));
		
		const auto centeredImage = Minus(*imageInput, Constant::Scalar(DataType::Float, 128.f));
		const auto scaledImage = ElementTimes(centeredImage, Reciprocal(Constant::Scalar(DataType::Float, 128.f)));
		ResNet resNet(scaledImage, 3);
		
		const unsigned int resultFeatureCount = 512;
		const unsigned int adjustedImageWidth = maxSequenceLen;

		auto const convOutWithHeightDropped = Reshape(resNet.getOutput(), { adjustedImageWidth, resultFeatureCount });
		auto const featureMajorTranspose = Transpose(convOutWithHeightDropped);
		//Convert width to feature sequence. Each feature chunk is of size resultFeatureCount
		auto const featureSequence = ToSequenceLike(featureMajorTranspose, *labelInput, L"imageFeatureSequenceToSeqLikeLabels");

		const auto rnnWeightInitializer = GlorotUniformInitializer(glorotInitializerOther);
		const auto weightInitializer = GlorotUniformInitializer(glorotInitializerOther);
		const unsigned int rnnOutputSize = rnnSize * 2;

		const Parameter rnnWeights({ rnnSize, rnnOutputSize }, DataType::Float, rnnWeightInitializer);
		
		const auto rnnOut = OptimizedRNNStack(featureSequence, rnnWeights, rnnSize, 2, true);
		// output is [512, time, batch]
		const auto rnnForwardOut = Slice(rnnOut, { Axis(0) }, { 0 }, { (int)rnnSize });
		const auto rnnBackwardOut = Slice(rnnOut, { Axis(0) }, { (int)rnnSize }, { (int)rnnSize * 2 });
		const Parameter rnnForwardProjWeights({ numClasses, rnnSize }, DataType::Float, weightInitializer);
		const Parameter rnnForwardProjBias({ numClasses }, DataType::Float, weightInitializer);
		const Parameter rnnBackwardProjWeights({ numClasses, rnnSize }, DataType::Float, weightInitializer);
		const Parameter rnnBackwardProjBias({ numClasses }, DataType::Float, weightInitializer);
		const auto forwardProj = Plus(Times(rnnForwardProjWeights, rnnForwardOut), rnnForwardProjBias);
		const auto backwardProj = Plus(Times(rnnBackwardProjWeights, rnnBackwardOut), rnnBackwardProjBias);
		model = Plus(forwardProj, backwardProj);

		auto trainingOp = ForwardBackward(LabelsToGraph(*labelInput), model, blankTokenID, -1);
		
		auto error = TranscriptionError(model, *labelInput, true, {blankTokenID}, L"TranscriptionErrorNode");

		const LearningRateSchedule lr(1);
		const MomentumSchedule beta(0.9); 
		const MomentumSchedule beta2(0.999);
		const auto optimizer = AdamLearner(model->Parameters(), lr, beta, true, beta2);
		ProgressWriterPtr progressWriter(new TrainingProgressWriter(1, 1, 1, 1, 1, 1, batchSize, maxSequenceLen));
		auto modelTrainer = CreateTrainer(model, trainingOp, error, { optimizer }, { progressWriter });

		trainer = modelTrainer;
	}
	catch (exception e) {
		std::cout << e.what() << endl;
	}
}

MinibatchSourcePtr TextTranscriptionModel::getMinibatchSource(const wstring& mapFile,
                                                              const wstring& textFile,
															  const unsigned int numSweeps){
    auto const scaleTransform = ReaderScale(imageWidth, imageHeight, 3, L"linear", L"pad", 0);
	
    auto imageDeserializer = ImageDeserializer(mapFile, L"labelsToIgnore", numClasses, L"image", {scaleTransform});
	StreamConfiguration textLabelConfig(L"textLabel", numClasses, true, L"textLabel");
    const vector<StreamConfiguration> textStreams{ textLabelConfig };
    auto textDeserializer = CTFDeserializer(textFile, textStreams);
    MinibatchSourceConfig sourceConfig({imageDeserializer, textDeserializer}, true);
	sourceConfig.maxSweeps = numSweeps;
    return CreateCompositeMinibatchSource(sourceConfig);
}

string TextTranscriptionModel::decodeOutputs(const std::vector<float>& lineVal){
    //per sequence decoding
    stringstream result;
	unsigned char prevArgmaxChar = 0;
    for(unsigned int j = 0; j < maxSequenceLen; j++){
        float maxActivation = -numeric_limits<float>::max();
        unsigned char argmaxChar = 0;
        for(unsigned int k = 0; k < numClasses; k++){
            const float valHere = lineVal[j*numClasses + k];
            if (valHere > maxActivation){
                argmaxChar = (unsigned char)(k + 'A');
                maxActivation = valHere;
            }
        }
		if ((argmaxChar != prevArgmaxChar || j == 0) && maxActivation > -numeric_limits<float>::max()) {
			if (argmaxChar < 'A' || argmaxChar > 'A' + numClasses - 1) {
				throw std::invalid_argument("Cannot decode character out of ASCII range");
			}
			if (argmaxChar == 'A' + numClasses - 1) {
				result << "_" << " ";
			}
			else {
				result << argmaxChar << " ";
			}
			prevArgmaxChar = argmaxChar;
		}
    }
    return result.str();
}

void TextTranscriptionModel::displayImage(const vector<float>& data, 
	const unsigned int width, 
	const unsigned int height, 
	const int format,
	const float intensityScale) {
	vector<unsigned char> charData(data.size());
	for (int imPos = 0; imPos < data.size(); imPos++) {
		charData[imPos] = (unsigned char)(data[imPos] * intensityScale);
	}
	cv::Mat imageMat(cv::Size(width, height), format);
	memcpy(imageMat.data, charData.data(), data.size());
	cv::namedWindow("Upcoming image");
	cv::imshow("Upcoming image", imageMat);
	cv::waitKey(0);
}

void TextTranscriptionModel::manuallyProcessMinibatch(MinibatchSourcePtr minibatchSource, bool isTesting,
	const size_t maxEvalBatches) {
	const auto imageInputStreamInfo = minibatchSource->StreamInfo(L"image");
	const auto textLabelStreamInfo = minibatchSource->StreamInfo(L"textLabel");
	for (unsigned long long int i = 0; isTesting || i < maxEvalBatches; i++) {
		auto newBatch = minibatchSource->GetNextMinibatch(batchSize);
		if (newBatch.empty()) {
			std::cout << "No more data, exiting" << endl;
			break;
		}
		auto modelOutput = model->Output();
		unordered_map<Variable, ValuePtr> valuesToFetch{ 
			{ *labelInput, nullptr },
		{ modelOutput, nullptr },
		{ *imageInput, nullptr },
		};
		bool shouldExit = false;
		if (!isTesting) {
			shouldExit = !trainer->TrainMinibatch({ { *labelInput, newBatch[textLabelStreamInfo] },
				{ *imageInput, newBatch[imageInputStreamInfo] } }, valuesToFetch);
		}
		else {
			trainer->TestMinibatch({ { *labelInput, newBatch[textLabelStreamInfo] },
				{ *imageInput, newBatch[imageInputStreamInfo] } }, valuesToFetch);
		}
		if (shouldExit) {
			std::cout << "Done manually processing minibatch, exiting" << endl;
			break;
		}
		if (isTesting)
			trainer->SummarizeTestProgress();
		else
			trainer->SummarizeTrainingProgress();
		auto const labelOutput = valuesToFetch[*labelInput];
		auto const predictionOutput = valuesToFetch[modelOutput];
		auto const outputImages = valuesToFetch[*imageInput];
		vector<vector<float>> imagePixelValues;
		if (outputImages != nullptr) {
			wcout << "Image input: " << outputImages->AsString() << endl;
			outputImages->CopyVariableValueTo(*imageInput, imagePixelValues);
		}
		if (labelOutput != nullptr) {
			vector<vector<float>> labelOutputOnCPU;
			vector<vector<float>> predictedOnCPU;
			labelOutput->CopyVariableValueTo(*labelInput, labelOutputOnCPU);
			predictionOutput->CopyVariableValueTo(modelOutput, predictedOnCPU);
			wcout << "Labels after reconcile: " << labelOutput->AsString() << endl;
			std::cout << "CPU label y dim: " << labelOutputOnCPU.size() << endl;
			std::cout << "CPU label x dim: " << labelOutputOnCPU[0].size() << endl;
			wcout << "Predicted labels: " << predictionOutput->AsString() << endl;
			for (unsigned int i = 0; i < labelOutputOnCPU.size(); i++) {
				//per sequence decoding
				const auto line = labelOutputOnCPU[i];
				const auto decodedOut = decodeOutputs(line);
				const auto predLine = predictedOnCPU[i];
				const auto decodedPred = decodeOutputs(predLine);
				std::cout << "Actl:" << decodedOut << endl;
				std::cout << "Pred:" << decodedPred << endl << endl;

				vector<float> redChannel(imageWidth * imageHeight);
				const vector<float>& image = imagePixelValues.at(i);
				copy(image.begin(), image.begin() + redChannel.size(), redChannel.begin());
				displayImage(redChannel, imageWidth, imageHeight, CV_8UC1, 1.0f);
			}
		}
	}
}

void TextTranscriptionModel::Train(const std::wstring& trainMapFile,
	const std::wstring& trainTextFile,
	const std::wstring& testMapFile,
	const std::wstring& testTextFile,
	const int numBatches) {

	MinibatchSourcePtr minibatchSource, testMinibatchSource;
	try {
		minibatchSource = getMinibatchSource(trainMapFile, trainTextFile, 100000000);
		testMinibatchSource = getMinibatchSource(testMapFile, testTextFile, 100000000);
	}
	catch (exception e) {
		cout << "Error loading minibatch sources: " << e.what() << std::endl;
	}

	const auto imageInputStreamInfo = minibatchSource->StreamInfo(L"image");
	const auto textLabelStreamInfo = minibatchSource->StreamInfo(L"textLabel");

	//const auto testImageStreamInfo = testMinibatchSource->StreamInfo(L"image");
	//const auto testLabelStreamInfo = testMinibatchSource->StreamInfo(L"textLabel");

	const unsigned int realBatchSize = maxSequenceLen * batchSize;
	const MinibatchSizeSchedule schedule(realBatchSize);
	
	/*try {
		const CheckpointConfig ckptConfig(modelFile, 1, DataUnit::Sweep, true, false);
		TestConfig testConfig(testMinibatchSource, schedule, {
			{ *imageInput, imageInputStreamInfo },
			{ *labelInput, textLabelStreamInfo }
			});
		auto trainingSession = CreateTrainingSession(trainer, minibatchSource, schedule, {
				{*imageInput, imageInputStreamInfo},
				{*labelInput, textLabelStreamInfo}
			}, numeric_limits<size_t>::max(), 1, DataUnit::Sweep, ckptConfig,
			{ nullptr }, testConfig);

		cout << "Will begin processing model with " << model->Parameters().size() << " parameters" << endl;

		//The model requires GPU support due to batch normalization and optimized RNN
		trainingSession->Train(DeviceDescriptor::GPUDevice(0));
	}
	catch (exception e) {
		cout << e.what();
	}*/


	trainer->RestoreFromCheckpoint(modelFile);
	manuallyProcessMinibatch(minibatchSource, true, 124000000000);
}

void TextTranscriptionModel::loadForEval()
{
	modelFuncForEval = Function::Load(modelFile, DeviceDescriptor::GPUDevice(0));
	evalInputImage = std::make_shared<Variable>(modelFuncForEval->Arguments().at(0));
	evalInputText = std::make_shared<Variable>(modelFuncForEval->Arguments().at(1));
	evalOutput = std::make_shared<Variable>(modelFuncForEval->Outputs().at(0));
}

std::string TextTranscriptionModel::evalImage(std::vector<unsigned char>& imageData)
{
	cv::Mat imageMat(cv::imdecode(cv::Mat(1, imageData.size(), CV_8UC3, imageData.data()),
		CV_LOAD_IMAGE_COLOR));

	cv::resize(imageMat, imageMat, cv::Size(imageWidth, imageHeight), cv::INTER_LINEAR);
	if (!imageMat.isContinuous())
	{
		// According to the contract, dense sequence data 
		// should return continuous data buffer.
		// TODO: This is potentially an expensive operation. Need to do some logging.
		imageMat = imageMat.clone();
	}

	/*unsigned int channelCount = imageMat.channels();
	std::stringstream ss;
	ss << "Upcoming image with " << channelCount << " chans";
	cv::namedWindow(ss.str());
	cv::imshow(ss.str(), imageMat);
	cv::waitKey(0);*/

	
	const int derivedImageSize = imageMat.rows * imageMat.cols * imageMat.channels();
	std::vector<unsigned char> imageDataForCNTK(derivedImageSize);
	unsigned char* rawImageBuffer = imageMat.ptr();
	copy(rawImageBuffer, rawImageBuffer + derivedImageSize, imageDataForCNTK.begin());
	
	// [128 x 32 x 3]
	std::vector<float> imageDataOrderedCorrectly;
	// BGR to BBB ... GGG ... RRR
	for (int channelOffset = 0; channelOffset < 3; ++channelOffset)
	{
		for (int imageIdx = 0; imageIdx < derivedImageSize; imageIdx += 3)
		{
			imageDataOrderedCorrectly.push_back(imageDataForCNTK.at(imageIdx));
		}
	}

	ValuePtr imageInputVal = Value::CreateBatch(evalInputImage->Shape(), imageDataOrderedCorrectly, DeviceDescriptor::GPUDevice(0));
	const std::vector<size_t> dummyTextInputData(maxSequenceLen);
	ValuePtr dummyTextInput = Value::CreateBatchOfSequences<float>(static_cast<size_t>(numClasses), 
		{ dummyTextInputData }, DeviceDescriptor::GPUDevice(0));

	std::unordered_map<Variable, ValuePtr> inputDataMap = { {*evalInputImage, imageInputVal}, {*evalInputText, dummyTextInput} };
	std::unordered_map<Variable, ValuePtr> outputCapture;
	for (auto& modelOutput : modelFuncForEval->Outputs())
	{
		outputCapture[modelOutput] = nullptr;
	}
	
	modelFuncForEval->Evaluate(inputDataMap, outputCapture, DeviceDescriptor::GPUDevice(0));

	std::vector<std::vector<float>> outputOnCPU;
	outputCapture[*evalOutput]->CopyVariableValueTo(*evalOutput, outputOnCPU);
	if (outputOnCPU.size() != 1)
	{
		throw invalid_argument("Expected to get sample of output");
	}
	std::string decoded = decodeOutputs(outputOnCPU[0]);
	return decoded;
}
