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
	mSimulationData( nullptr ),
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
	if( settings == NULL )
		return;

	mSettings = settings;
	mSimulationSampleRateHz = simulation_sample_rate;
	mMessageIndex = 0;
	mCurrentSample = 0;
	mSimulationData.reset( new SimulationChannelDescriptorGroup() );
	mDataLines.fill( NULL );
	mControlLines.fill( NULL );
	mDataLineIndices.fill( static_cast<U32>( -1 ) );
	mControlLineIndices.fill( static_cast<U32>( -1 ) );
	mDav = NULL;
	mDavIndex = static_cast<U32>( -1 );

	U32 next_index = 0;
	for( U32 i = 0; i < GPIBAnalyzerSettings::kDataLineCount; i++ )
	{
		mSimulationData->Add( mSettings->mDataChannels[ i ], mSimulationSampleRateHz, BIT_HIGH );
		mDataLineIndices[ i ] = next_index++;
	}

	if( mSettings->mDavChannel != UNDEFINED_CHANNEL )
	{
		mSimulationData->Add( mSettings->mDavChannel, mSimulationSampleRateHz, BIT_HIGH );
		mDavIndex = next_index++;
	}

	if( mSettings->mAtnChannel != UNDEFINED_CHANNEL )
	{
		mSimulationData->Add( mSettings->mAtnChannel, mSimulationSampleRateHz, BIT_HIGH );
		mControlLineIndices[ 0 ] = next_index++;
	}
	if( mSettings->mNrfdChannel != UNDEFINED_CHANNEL )
	{
		mSimulationData->Add( mSettings->mNrfdChannel, mSimulationSampleRateHz, BIT_HIGH );
		mControlLineIndices[ 2 ] = next_index++;
	}
	if( mSettings->mNdacChannel != UNDEFINED_CHANNEL )
	{
		mSimulationData->Add( mSettings->mNdacChannel, mSimulationSampleRateHz, BIT_HIGH );
		mControlLineIndices[ 3 ] = next_index++;
	}
	if( mSettings->mEoiChannel != UNDEFINED_CHANNEL )
	{
		mSimulationData->Add( mSettings->mEoiChannel, mSimulationSampleRateHz, BIT_HIGH );
		mControlLineIndices[ 4 ] = next_index++;
	}
	if( mSettings->mIfcChannel != UNDEFINED_CHANNEL )
	{
		mSimulationData->Add( mSettings->mIfcChannel, mSimulationSampleRateHz, BIT_HIGH );
		mControlLineIndices[ 5 ] = next_index++;
	}
	if( mSettings->mRenChannel != UNDEFINED_CHANNEL )
	{
		mSimulationData->Add( mSettings->mRenChannel, mSimulationSampleRateHz, BIT_HIGH );
		mControlLineIndices[ 6 ] = next_index++;
	}
	if( mSettings->mSrqChannel != UNDEFINED_CHANNEL )
	{
		mSimulationData->Add( mSettings->mSrqChannel, mSimulationSampleRateHz, BIT_HIGH );
		mControlLineIndices[ 7 ] = next_index++;
	}

	SimulationChannelDescriptor* descriptors = mSimulationData->GetArray();
	for( U32 i = 0; i < GPIBAnalyzerSettings::kDataLineCount; i++ )
	{
		if( mDataLineIndices[ i ] != static_cast<U32>( -1 ) )
			mDataLines[ i ] = &descriptors[ mDataLineIndices[ i ] ];
	}

	if( mDavIndex != static_cast<U32>( -1 ) )
		mDav = &descriptors[ mDavIndex ];

	for( U32 i = 0; i < GPIBAnalyzerSettings::kControlLineCount; i++ )
	{
		if( mControlLineIndices[ i ] != static_cast<U32>( -1 ) )
			mControlLines[ i ] = &descriptors[ mControlLineIndices[ i ] ];
	}

	mMessage = kMessage;
}

U32 GPIBSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
	if( sample_rate == 0 && mSimulationSampleRateHz != 0 )
	{
		sample_rate = mSimulationSampleRateHz;
	}

	if( sample_rate == 0 || mSimulationSampleRateHz == 0 )
	{
		*simulation_channel = mSimulationData->GetArray();
		return mSimulationData->GetCount();
	}

	U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

	while( mCurrentSample < adjusted_largest_sample_requested )
	{
		mSimulationData->AdvanceAll( 1000 );
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
				// GPIB data lines are active-low, so asserted data bits are driven low.
				mDataLines[ i ]->TransitionIfNeeded( BIT_LOW );
			}
			else
			{
				mDataLines[ i ]->TransitionIfNeeded( BIT_HIGH );
			}
		}

		bool is_command = ( mMessageIndex == 1 );
		bool is_eoi = ( mMessageIndex == 0 );

		if( mControlLines[ 0 ] != NULL )
		{
			mControlLines[ 0 ]->TransitionIfNeeded( is_command ? BIT_LOW : BIT_HIGH );
		}
		if( mControlLines[ 4 ] != NULL )
		{
			mControlLines[ 4 ]->TransitionIfNeeded( is_eoi ? BIT_LOW : BIT_HIGH );
		}
		if( mDav != NULL )
		{
			mDav->TransitionIfNeeded( BIT_LOW );
		}
		mSimulationData->AdvanceAll( 400 );
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
			mControlLines[ 4 ]->TransitionIfNeeded( BIT_HIGH );
		}
	}

	*simulation_channel = mSimulationData->GetArray();
	return mSimulationData->GetCount();
}
