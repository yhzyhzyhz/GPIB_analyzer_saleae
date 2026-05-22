#ifndef GPIB_ANALYZER_H
#define GPIB_ANALYZER_H

#include <Analyzer.h>
#include "GPIBAnalyzerSettings.h"
#include "GPIBAnalyzerResults.h"
#include "GPIBSimulationDataGenerator.h"
#include <memory>
#include <vector>

class ANALYZER_EXPORT GPIBAnalyzer : public Analyzer2
{
public:
	GPIBAnalyzer();
	virtual ~GPIBAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	GPIBAnalyzerSettings mSettings;
	std::unique_ptr<GPIBAnalyzerResults> mResults;
	std::vector<AnalyzerChannelData*> mDataLines;
	AnalyzerChannelData* mAtn;
	AnalyzerChannelData* mDav;
	AnalyzerChannelData* mNrfd;
	AnalyzerChannelData* mNdac;
	AnalyzerChannelData* mEoi;
	AnalyzerChannelData* mIfc;
	AnalyzerChannelData* mRen;
	AnalyzerChannelData* mSrq;
	U32 mSampleRateHz;

	GPIBSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //GPIB_ANALYZER_H
