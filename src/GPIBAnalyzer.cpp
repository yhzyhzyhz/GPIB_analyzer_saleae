#include "GPIBAnalyzer.h"

#include <AnalyzerChannelData.h>

#include <vector>

namespace
{
const U32 kDataLineCount = GPIBAnalyzerSettings::kDataLineCount;

BitState GetLineStateAtSample( AnalyzerChannelData* line, U64 sample_number )
{
	line->AdvanceToAbsPosition( sample_number );
	return line->GetBitState();
}

bool IsAssertedAtSample( AnalyzerChannelData* line, U64 sample_number )
{
	return line != NULL && GetLineStateAtSample( line, sample_number ) == BIT_LOW;
}

U8 ReadByteAtSample( std::vector<AnalyzerChannelData*>& data_lines, U64 sample_number )
{
	U8 value = 0;
	for( U32 i = 0; i < data_lines.size(); i++ )
	{
		if( GetLineStateAtSample( data_lines[ i ], sample_number ) == BIT_LOW )
		{
			value |= ( 1 << i );
		}
	}
	return value;
}
}

GPIBAnalyzer::GPIBAnalyzer()
: 	Analyzer2(),
	mSettings(),
	mResults(),
	mDataLines(),
	mAtn( NULL ),
	mDav( NULL ),
	mNrfd( NULL ),
	mNdac( NULL ),
	mEoi( NULL ),
	mIfc( NULL ),
	mRen( NULL ),
	mSrq( NULL ),
	mSampleRateHz( 0 ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( &mSettings );
}

GPIBAnalyzer::~GPIBAnalyzer()
{
	KillThread();
}

void GPIBAnalyzer::SetupResults()
{
	mResults.reset( new GPIBAnalyzerResults( this, &mSettings ) );
	SetAnalyzerResults( mResults.get() );
	if( mSettings.mDavChannel != UNDEFINED_CHANNEL )
	{
		mResults->AddChannelBubblesWillAppearOn( mSettings.mDavChannel );
	}
}

void GPIBAnalyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();
	mDataLines.clear();
	mDataLines.reserve( kDataLineCount );
	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		mDataLines.push_back( GetAnalyzerChannelData( mSettings.mDataChannels[ i ] ) );
	}

	mAtn = mSettings.mAtnChannel != UNDEFINED_CHANNEL ? GetAnalyzerChannelData( mSettings.mAtnChannel ) : NULL;
	mDav = mSettings.mDavChannel != UNDEFINED_CHANNEL ? GetAnalyzerChannelData( mSettings.mDavChannel ) : NULL;
	mNrfd = mSettings.mNrfdChannel != UNDEFINED_CHANNEL ? GetAnalyzerChannelData( mSettings.mNrfdChannel ) : NULL;
	mNdac = mSettings.mNdacChannel != UNDEFINED_CHANNEL ? GetAnalyzerChannelData( mSettings.mNdacChannel ) : NULL;
	mEoi = mSettings.mEoiChannel != UNDEFINED_CHANNEL ? GetAnalyzerChannelData( mSettings.mEoiChannel ) : NULL;
	mIfc = mSettings.mIfcChannel != UNDEFINED_CHANNEL ? GetAnalyzerChannelData( mSettings.mIfcChannel ) : NULL;
	mRen = mSettings.mRenChannel != UNDEFINED_CHANNEL ? GetAnalyzerChannelData( mSettings.mRenChannel ) : NULL;
	mSrq = mSettings.mSrqChannel != UNDEFINED_CHANNEL ? GetAnalyzerChannelData( mSettings.mSrqChannel ) : NULL;

	for( ;; )
	{
		if( mDav == NULL )
			return;

		for( U32 i = 0; i < mDataLines.size(); i++ )
		{
			if( mDataLines[ i ] == NULL )
				return;
		}

		mDav->AdvanceToNextEdge();
		if( mDav->GetBitState() != BIT_LOW )
		{
			continue;
		}

		U64 start_sample = mDav->GetSampleNumber();
		U64 sample_to_read = start_sample + 1;
		U8 data = ReadByteAtSample( mDataLines, sample_to_read );
		bool command = IsAssertedAtSample( mAtn, sample_to_read );
		bool eoi = IsAssertedAtSample( mEoi, sample_to_read );

		mResults->AddMarker( start_sample, AnalyzerResults::DownArrow, mSettings.mDavChannel );

		mDav->AdvanceToNextEdge();
		U64 end_sample = mDav->GetSampleNumber();

		Frame frame;
		frame.mData1 = data;
		frame.mData2 = ( command ? 1 : 0 ) | ( eoi ? 2 : 0 );
		frame.mFlags = 0;
		frame.mStartingSampleInclusive = start_sample;
		frame.mEndingSampleInclusive = end_sample;

		mResults->AddFrame( frame );
		mResults->CommitResults();
		ReportProgress( frame.mEndingSampleInclusive );
	}
}

bool GPIBAnalyzer::NeedsRerun()
{
	return false;
}

U32 GPIBAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), &mSettings );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 GPIBAnalyzer::GetMinimumSampleRateHz()
{
	return 1000000;
}

const char* GPIBAnalyzer::GetAnalyzerName() const
{
	return "GPIB Analyzer";
}

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName()
{
	return "GPIB Analyzer";
}

extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer()
{
	return new GPIBAnalyzer();
}

extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}