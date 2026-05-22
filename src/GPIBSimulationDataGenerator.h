#ifndef GPIB_SIMULATION_DATA_GENERATOR
#define GPIB_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>

#include "GPIBAnalyzerSettings.h"

#include <memory>
#include <array>
#include <string>

class GPIBSimulationDataGenerator
{
public:
	GPIBSimulationDataGenerator();
	~GPIBSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, GPIBAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	GPIBAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;
	U32 mMessageIndex;
	U64 mCurrentSample;

	std::unique_ptr<SimulationChannelDescriptorGroup> mSimulationData;
	std::array<SimulationChannelDescriptor*, GPIBAnalyzerSettings::kDataLineCount> mDataLines;
	std::array<SimulationChannelDescriptor*, GPIBAnalyzerSettings::kControlLineCount> mControlLines;
	SimulationChannelDescriptor* mDav;

	std::array<U32, GPIBAnalyzerSettings::kDataLineCount> mDataLineIndices;
	std::array<U32, GPIBAnalyzerSettings::kControlLineCount> mControlLineIndices;
	U32 mDavIndex;

	std::string mMessage;

};
#endif //GPIB_SIMULATION_DATA_GENERATOR