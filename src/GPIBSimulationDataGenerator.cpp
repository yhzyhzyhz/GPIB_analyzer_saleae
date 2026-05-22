#include "GPIBSimulationDataGenerator.h"

#include "GPIBAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

namespace
{
const char* kMessage = "GPIB ANALYZER ";
}

GPIBSimulationDataGenerator::GPIBSimulationDataGenerator()
: 	mSettings( NULL ),
	mSimulationSampleRateHz( 0 ),
	mMessageIndex( 0 ),
	mCurrentSample( 0 ),
	mSimulationData(),
	mDataLines(),
	mControlLines(),
	mDav( NULL ),
	mMessage( kMessage )
{
	mDataLines.fill( NULL );
	mControlLines.fill( NULL );
}

GPIBSimulationDataGenerator::~GPIBSimulationDataGenerator()
{
}

void GPIBSimulationDataGenerator::Initialize( U32 simulation_sample_rate, GPIBAnalyzerSettings* settings )
{
	mSettings = settings;
	mSimulationSampleRateHz = simulation_sample_rate;
	mMessageIndex = 0;
	mCurrentSample = 0;
	mSimulationData = SimulationChannelDescriptorGroup();
	mDataLines.fill( NULL );
	mControlLines.fill( NULL );
	mDav = NULL;

	for( U32 i = 0; i < GPIBAnalyzerSettings::kDataLineCount; i++ )
	{
		mDataLines[ i ] = mSimulationData.Add( mSettings->mDataChannels[ i ], mSimulationSampleRateHz, BIT_HIGH );
	}

	if( mSettings->mDavChannel != UNDEFINED_CHANNEL )
	{
		mDav = mSimulationData.Add( mSettings->mDavChannel, mSimulationSampleRateHz, BIT_HIGH );
	}

	if( mSettings->mAtnChannel != UNDEFINED_CHANNEL )
	{
		mControlLines[ 0 ] = mSimulationData.Add( mSettings->mAtnChannel, mSimulationSampleRateHz, BIT_HIGH );
	}
	if( mSettings->mNrfdChannel != UNDEFINED_CHANNEL )
	{
		mControlLines[ 2 ] = mSimulationData.Add( mSettings->mNrfdChannel, mSimulationSampleRateHz, BIT_HIGH );
	}
	if( mSettings->mNdacChannel != UNDEFINED_CHANNEL )
	{
		mControlLines[ 3 ] = mSimulationData.Add( mSettings->mNdacChannel, mSimulationSampleRateHz, BIT_HIGH );
	}
	if( mSettings->mEoiChannel != UNDEFINED_CHANNEL )
	{
		mControlLines[ 4 ] = mSimulationData.Add( mSettings->mEoiChannel, mSimulationSampleRateHz, BIT_HIGH );
	}
	if( mSettings->mIfcChannel != UNDEFINED_CHANNEL )
	{
		mControlLines[ 5 ] = mSimulationData.Add( mSettings->mIfcChannel, mSimulationSampleRateHz, BIT_HIGH );
	}
	if( mSettings->mRenChannel != UNDEFINED_CHANNEL )
	{
		mControlLines[ 6 ] = mSimulationData.Add( mSettings->mRenChannel, mSimulationSampleRateHz, BIT_HIGH );
	}
	if( mSettings->mSrqChannel != UNDEFINED_CHANNEL )
	{
		mControlLines[ 7 ] = mSimulationData.Add( mSettings->mSrqChannel, mSimulationSampleRateHz, BIT_HIGH );
	}

	mMessage = kMessage;
}

U32 GPIBSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
	U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

	while( mCurrentSample < adjusted_largest_sample_requested )
	{
		mSimulationData.AdvanceAll( 1000 );
		mCurrentSample += 1000;

		U8 byte = static_cast<U8>( mMessage[ mMessageIndex ] );
		mMessageIndex++;
		if( mMessageIndex >= mMessage.size() )
		{
			mMessageIndex = 0;
		}

		for( U32 i = 0; i < GPIBAnalyzerSettings::kDataLineCount; i++ )
		{
			if( mDataLines[ i ] == NULL )
			{
				continue;
			}
			if( ( byte & ( 1 << i ) ) != 0 )
			{
				mDataLines[ i ]->TransitionIfNeeded( BIT_HIGH );
			}
			else
			{
				mDataLines[ i ]->TransitionIfNeeded( BIT_LOW );
			}
		}

		if( mControlLines[ 0 ] != NULL )
		{
			mControlLines[ 0 ]->TransitionIfNeeded( mMessageIndex == 1 ? BIT_LOW : BIT_HIGH );
		}
		if( mDav != NULL )
		{
			mDav->TransitionIfNeeded( BIT_LOW );
		}
		mSimulationData.AdvanceAll( 400 );
		mCurrentSample += 400;
		if( mDav != NULL )
		{
			mDav->TransitionIfNeeded( BIT_HIGH );
		}
		if( mControlLines[ 0 ] != NULL )
		{
			mControlLines[ 0 ]->TransitionIfNeeded( BIT_HIGH );
		}

		if( mControlLines[ 4 ] != NULL )
		{
			mControlLines[ 4 ]->TransitionIfNeeded( mMessageIndex == 0 ? BIT_LOW : BIT_HIGH );
		}
	}

	*simulation_channel = mSimulationData.GetArray();
	return mSimulationData.GetCount();
}
