//
// Created by benjamintrapani on 11/22/17.
//

#ifndef TEXTTRANSCRIPTIONCODE_RESNET_H
#define TEXTTRANSCRIPTIONCODE_RESNET_H

#include "CNTKLibrary.h"

class ResNet {
public:
    ResNet(const CNTK::Variable& inputVar, unsigned int channelCount);
    inline const CNTK::Variable& getOutput() {
        return output;
    }
	static CNTK::FunctionPtr buildConvolution(const CNTK::FunctionPtr& var,
		const std::vector<unsigned int>& kernelSize,
		const std::vector<unsigned int>& kernelStride,
		unsigned int prevFilterCount,
		unsigned int newFilterCount,
		double glorotInitScale,
		const std::vector<bool>& usePad);

	static CNTK::Variable buildBatchNormalization(const CNTK::Variable& input, 
		unsigned int filterCount, 
		double glorotInitScale);

private:
    CNTK::Variable buildResnetBasicBlock(const CNTK::FunctionPtr& inputVar,
                                         unsigned int prevFilterCount,
                                         unsigned int filterCount,
										 unsigned int yKernelSize);
    CNTK::Variable buildStackOfBasicBlocks(CNTK::FunctionPtr inputVar,
                                           unsigned int prevFilterCount,
                                           unsigned int newFilterCount,
                                           unsigned int numLayers,
										   unsigned int yKernelSize = 3);
    CNTK::Variable buildDoubleConvBlock(const CNTK::FunctionPtr& inputVar,
                                        unsigned int prevFilterCount,
                                        unsigned int newFilterCount,
										unsigned int xStride = 1,
										unsigned int yStride = 2);
    CNTK::Variable buildInitialLayer(const CNTK::FunctionPtr& inputVar, unsigned int channelCount);
    CNTK::Variable buildIntermediateLayers(const CNTK::FunctionPtr& inputVar);
    CNTK::Variable buildFinalLayer(const CNTK::FunctionPtr& inputVar);

    CNTK::Variable output;
};

#endif //TEXTTRANSCRIPTIONCODE_RESNET_H
