//
// Created by benjamintrapani on 11/22/17.
//

//#include "pch.h"
#include "stdafx.h"
#include "ResNet.h"

#include <exception>

using namespace CNTK;
using namespace std;

FunctionPtr ResNet::buildConvolution(const FunctionPtr& var,
	const std::vector<unsigned int>& kernelSize,
	const std::vector<unsigned int>& kernelStride,
	const unsigned int prevFilterCount,
	const unsigned int newFilterCount,
	const double glorotInitScale,
	const vector<bool>& usePad){
	if (kernelStride.size() != 2) {
		throw std::invalid_argument("Cannot specify kernel stride in a dimension other than 2");
	}
	if (kernelSize.size() != 2) {
		throw std::invalid_argument("Cannot specify kernel size in a dimension other than 2");
	}
    const auto paramInitializer = GlorotUniformInitializer(glorotInitScale);
    const Parameter convMap({kernelSize.at(0), kernelSize.at(1), prevFilterCount, newFilterCount},
                             DataType::Float, paramInitializer);
	const Parameter convBias({ 1, 1, newFilterCount }, DataType::Float, paramInitializer);
    const FunctionPtr convResult = Plus(Convolution(convMap,
                                      var,
		{ kernelStride.at(0), kernelStride.at(1), prevFilterCount }, { true }, usePad), convBias);
	
	const FunctionPtr resultAfterBatchNorm = buildBatchNormalization(convResult, newFilterCount, 1);
	return resultAfterBatchNorm;
}

Variable ResNet::buildBatchNormalization(const CNTK::Variable& input, const unsigned int filterCount, const double glorotInitScale) {
	const auto paramInitializer = GlorotUniformInitializer(glorotInitScale);
	Parameter scale({ filterCount }, DataType::Float, paramInitializer);
	Parameter bias({ filterCount }, DataType::Float, paramInitializer);
	Parameter runningMean({ filterCount }, DataType::Float, paramInitializer);
	Parameter runningStdDev({ filterCount }, DataType::Float, paramInitializer);
	Parameter runningCount({ 1 }, DataType::Float, paramInitializer);
	Variable batchNormalizationResult = BatchNormalization(input, scale, bias, runningMean, runningStdDev,
		runningCount, true, 4096, 0.0, 1e-05, true, true);
	return batchNormalizationResult;
}

Variable ResNet::buildResnetBasicBlock(const FunctionPtr& inputVar,
                                       const unsigned int prevFilterCount,
                                       const unsigned int filterCount,
									   const unsigned int yKernelSize){
	auto conv1 = buildConvolution(inputVar, { 3, 3 }, { 1, 1 }, prevFilterCount, filterCount, 1.0, { true });
    auto reluConv1 = ReLU(conv1);
	auto conv2 = buildConvolution(reluConv1, { 3, 3 }, { 1, 1 }, filterCount, filterCount, 1.0, { true });
    auto loopedBack = ReLU(Plus(conv2, inputVar));
    return loopedBack;
}

Variable ResNet::buildStackOfBasicBlocks(FunctionPtr inputVar,
                                         const unsigned int prevFilterCount,
                                         const unsigned int newFilterCount,
                                         const unsigned int numLayers,
										 const unsigned int yKernelSize){
    inputVar = buildResnetBasicBlock(inputVar, prevFilterCount, newFilterCount, yKernelSize);
    for(unsigned int i = 0; i < numLayers-1; i++){
        inputVar = buildResnetBasicBlock(inputVar, newFilterCount, newFilterCount, yKernelSize);
    }
    return inputVar;
}

Variable ResNet::buildDoubleConvBlock(const FunctionPtr& inputVar,
                                      const unsigned int prevFilterCount,
                                      const unsigned int newFilterCount,
									  const unsigned int xStride,
									  const unsigned int yStride){
	auto conv1 = buildConvolution(inputVar, { 3, 3 }, { xStride, yStride }, prevFilterCount, newFilterCount, 1.0, { true });
    auto reluConv1 = ReLU(conv1);

	auto conv2 = buildConvolution(reluConv1, { 3, 3 }, { 1, 1 }, newFilterCount, newFilterCount, 1.0, { true });
	auto shortcutConv = buildConvolution(inputVar, { 1, 1 }, { xStride, yStride }, prevFilterCount, newFilterCount, 1.0, { true });
    auto concatedResult = ReLU(Plus(conv2, shortcutConv));
    return concatedResult;
}

Variable ResNet::buildInitialLayer(const FunctionPtr& inputVar, const unsigned int channelCount) {
	// [128, 32, 3]
	auto initialConv = buildConvolution(inputVar, { 5, 5 }, { 2, 2 }, channelCount, 64, 1.0, { true });
	auto initialActivation = ReLU(initialConv);
	// [64, 16, 64]
	auto firstStack = buildStackOfBasicBlocks(initialActivation, 64, 64, 4);
	auto firstReduce = buildDoubleConvBlock(firstStack, 64, 128);
	// [64, 8, 128]
	return firstReduce;
}

Variable ResNet::buildIntermediateLayers(const FunctionPtr& inputVar) {
	// [64, 8, 128]
    auto layer128 = buildStackOfBasicBlocks(inputVar, 128, 128, 4);
	auto layer128Reduced = buildDoubleConvBlock(layer128, 128, 256, 2);
	// [32, 4, 256]
    auto layer256 = buildStackOfBasicBlocks(layer128Reduced, 256, 256, 4);
    auto layer256Reduced = buildDoubleConvBlock(layer256, 256, 512, 1, 1);
	// [32, 4, 256]
    auto layer512 = buildStackOfBasicBlocks(layer256Reduced, 512, 512, 4, 4);
	auto finalConv = ReLU(buildConvolution(layer512, { 1, 4 }, { 1, 4 }, 512, 512, 1.0, { false }));
	// [16 x 1 x 512]
    return finalConv;
}

Variable ResNet::buildFinalLayer(const FunctionPtr& inputVar){
    auto averagePool = Pooling(inputVar, PoolingType::Average, {7, 7});
    return averagePool;
}

// Build the 18-layer version of the ResNet neural network.
// Image is reduced along y by 2^5 and along x by 2^3, resulting in a 1 x 16 receptive field
// The final 7x7 average pool specified in the paper is omitted to preserve image size.
ResNet::ResNet(const Variable& inputVar, const unsigned int channelCount) {
    auto initialLayerOut = buildInitialLayer(inputVar, channelCount);
    auto intermediateLayerOut = buildIntermediateLayers(initialLayerOut);
    output = intermediateLayerOut;
}
