#include "GPIBAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

namespace
{
const char* kDataNames[ GPIBAnalyzerSettings::kDataLineCount ] = { "DIO1", "DIO2", "DIO3", "DIO4", "DIO5", "DIO6", "DIO7", "DIO8" };
}

GPIBAnalyzerSettings::GPIBAnalyzerSettings()
: 	mDataChannels( kDataLineCount, UNDEFINED_CHANNEL ),
	mAtnChannel( UNDEFINED_CHANNEL ),
	mDavChannel( UNDEFINED_CHANNEL ),
	mNrfdChannel( UNDEFINED_CHANNEL ),
	mNdacChannel( UNDEFINED_CHANNEL ),
	mEoiChannel( UNDEFINED_CHANNEL ),
	mIfcChannel( UNDEFINED_CHANNEL ),
	mRenChannel( UNDEFINED_CHANNEL ),
	mSrqChannel( UNDEFINED_CHANNEL )
{
	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		AnalyzerSettingInterfaceChannel* data_channel_interface = new AnalyzerSettingInterfaceChannel();
		data_channel_interface->SetTitleAndTooltip( kDataNames[ i ], kDataNames[ i ] );
		data_channel_interface->SetChannel( mDataChannels[ i ] );
		data_channel_interface->SetSelectionOfNoneIsAllowed( false );
		mDataChannelInterfaces.push_back( data_channel_interface );
	}

	mAtnChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mAtnChannelInterface->SetTitleAndTooltip( "ATN", "Attention line, active low" );
	mAtnChannelInterface->SetChannel( mAtnChannel );
	mAtnChannelInterface->SetSelectionOfNoneIsAllowed( true );

	mDavChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mDavChannelInterface->SetTitleAndTooltip( "DAV", "Data valid line, active low" );
	mDavChannelInterface->SetChannel( mDavChannel );
	mDavChannelInterface->SetSelectionOfNoneIsAllowed( false );

	mNrfdChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mNrfdChannelInterface->SetTitleAndTooltip( "NRFD", "Not ready for data, active low" );
	mNrfdChannelInterface->SetChannel( mNrfdChannel );
	mNrfdChannelInterface->SetSelectionOfNoneIsAllowed( true );

	mNdacChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mNdacChannelInterface->SetTitleAndTooltip( "NDAC", "Not data accepted, active low" );
	mNdacChannelInterface->SetChannel( mNdacChannel );
	mNdacChannelInterface->SetSelectionOfNoneIsAllowed( true );

	mEoiChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mEoiChannelInterface->SetTitleAndTooltip( "EOI", "End or identify, active low" );
	mEoiChannelInterface->SetChannel( mEoiChannel );
	mEoiChannelInterface->SetSelectionOfNoneIsAllowed( true );

	mIfcChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mIfcChannelInterface->SetTitleAndTooltip( "IFC", "Interface clear, active low" );
	mIfcChannelInterface->SetChannel( mIfcChannel );
	mIfcChannelInterface->SetSelectionOfNoneIsAllowed( true );

	mRenChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mRenChannelInterface->SetTitleAndTooltip( "REN", "Remote enable, active low" );
	mRenChannelInterface->SetChannel( mRenChannel );
	mRenChannelInterface->SetSelectionOfNoneIsAllowed( true );

	mSrqChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mSrqChannelInterface->SetTitleAndTooltip( "SRQ", "Service request, active low" );
	mSrqChannelInterface->SetChannel( mSrqChannel );
	mSrqChannelInterface->SetSelectionOfNoneIsAllowed( true );

	AddInterface( mAtnChannelInterface.get() );
	AddInterface( mDavChannelInterface.get() );
	AddInterface( mNrfdChannelInterface.get() );
	AddInterface( mNdacChannelInterface.get() );
	AddInterface( mEoiChannelInterface.get() );
	AddInterface( mIfcChannelInterface.get() );
	AddInterface( mRenChannelInterface.get() );
	AddInterface( mSrqChannelInterface.get() );

	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		AddInterface( mDataChannelInterfaces[ i ] );
	}

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		AddChannel( mDataChannels[ i ], kDataNames[ i ], false );
	}
	AddChannel( mAtnChannel, "ATN", false );
	AddChannel( mDavChannel, "DAV", false );
	AddChannel( mNrfdChannel, "NRFD", false );
	AddChannel( mNdacChannel, "NDAC", false );
	AddChannel( mEoiChannel, "EOI", false );
	AddChannel( mIfcChannel, "IFC", false );
	AddChannel( mRenChannel, "REN", false );
	AddChannel( mSrqChannel, "SRQ", false );
}

GPIBAnalyzerSettings::~GPIBAnalyzerSettings()
{
	for( U32 i = 0; i < mDataChannelInterfaces.size(); i++ )
	{
		delete mDataChannelInterfaces[ i ];
	}
}

bool GPIBAnalyzerSettings::SetSettingsFromInterfaces()
{
	Channel data_channels[ kDataLineCount ];
	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		data_channels[ i ] = mDataChannelInterfaces[ i ]->GetChannel();
		if( data_channels[ i ] == UNDEFINED_CHANNEL )
		{
			SetErrorText( "Please select all 8 DIO channels for the GPIB bus" );
			return false;
		}
	}

	Channel atn_channel = mAtnChannelInterface->GetChannel();
	Channel dav_channel = mDavChannelInterface->GetChannel();
	Channel nrfd_channel = mNrfdChannelInterface->GetChannel();
	Channel ndac_channel = mNdacChannelInterface->GetChannel();
	Channel eoi_channel = mEoiChannelInterface->GetChannel();
	Channel ifc_channel = mIfcChannelInterface->GetChannel();
	Channel ren_channel = mRenChannelInterface->GetChannel();
	Channel srq_channel = mSrqChannelInterface->GetChannel();

	if( dav_channel == UNDEFINED_CHANNEL )
	{
		SetErrorText( "Please select a DAV channel for the GPIB bus" );
		return false;
	}

	std::vector<Channel> used_channels;
	used_channels.push_back( dav_channel );
	used_channels.push_back( atn_channel );
	used_channels.push_back( nrfd_channel );
	used_channels.push_back( ndac_channel );
	used_channels.push_back( eoi_channel );
	used_channels.push_back( ifc_channel );
	used_channels.push_back( ren_channel );
	used_channels.push_back( srq_channel );
	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		used_channels.push_back( data_channels[ i ] );
	}

	for( U32 i = 0; i < used_channels.size(); i++ )
	{
		if( used_channels[ i ] == UNDEFINED_CHANNEL )
		{
			continue;
		}
		for( U32 j = i + 1; j < used_channels.size(); j++ )
		{
			if( used_channels[ i ] == used_channels[ j ] )
			{
				SetErrorText( "GPIB channels must be unique" );
				return false;
			}
		}
	}

	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		mDataChannels[ i ] = data_channels[ i ];
	}
	mAtnChannel = atn_channel;
	mDavChannel = dav_channel;
	mNrfdChannel = nrfd_channel;
	mNdacChannel = ndac_channel;
	mEoiChannel = eoi_channel;
	mIfcChannel = ifc_channel;
	mRenChannel = ren_channel;
	mSrqChannel = srq_channel;

	ClearChannels();
	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		AddChannel( mDataChannels[ i ], kDataNames[ i ], true );
	}
	AddChannel( mAtnChannel, "ATN", mAtnChannel != UNDEFINED_CHANNEL );
	AddChannel( mDavChannel, "DAV", true );
	AddChannel( mNrfdChannel, "NRFD", mNrfdChannel != UNDEFINED_CHANNEL );
	AddChannel( mNdacChannel, "NDAC", mNdacChannel != UNDEFINED_CHANNEL );
	AddChannel( mEoiChannel, "EOI", mEoiChannel != UNDEFINED_CHANNEL );
	AddChannel( mIfcChannel, "IFC", mIfcChannel != UNDEFINED_CHANNEL );
	AddChannel( mRenChannel, "REN", mRenChannel != UNDEFINED_CHANNEL );
	AddChannel( mSrqChannel, "SRQ", mSrqChannel != UNDEFINED_CHANNEL );

	return true;
}

void GPIBAnalyzerSettings::UpdateInterfacesFromSettings()
{
	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		mDataChannelInterfaces[ i ]->SetChannel( mDataChannels[ i ] );
	}

	mAtnChannelInterface->SetChannel( mAtnChannel );
	mDavChannelInterface->SetChannel( mDavChannel );
	mNrfdChannelInterface->SetChannel( mNrfdChannel );
	mNdacChannelInterface->SetChannel( mNdacChannel );
	mEoiChannelInterface->SetChannel( mEoiChannel );
	mIfcChannelInterface->SetChannel( mIfcChannel );
	mRenChannelInterface->SetChannel( mRenChannel );
	mSrqChannelInterface->SetChannel( mSrqChannel );
}

void GPIBAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		text_archive >> mDataChannels[ i ];
	}
	text_archive >> mAtnChannel;
	text_archive >> mDavChannel;
	text_archive >> mNrfdChannel;
	text_archive >> mNdacChannel;
	text_archive >> mEoiChannel;
	text_archive >> mIfcChannel;
	text_archive >> mRenChannel;
	text_archive >> mSrqChannel;

	ClearChannels();
	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		AddChannel( mDataChannels[ i ], kDataNames[ i ], mDataChannels[ i ] != UNDEFINED_CHANNEL );
	}
	AddChannel( mAtnChannel, "ATN", mAtnChannel != UNDEFINED_CHANNEL );
	AddChannel( mDavChannel, "DAV", mDavChannel != UNDEFINED_CHANNEL );
	AddChannel( mNrfdChannel, "NRFD", mNrfdChannel != UNDEFINED_CHANNEL );
	AddChannel( mNdacChannel, "NDAC", mNdacChannel != UNDEFINED_CHANNEL );
	AddChannel( mEoiChannel, "EOI", mEoiChannel != UNDEFINED_CHANNEL );
	AddChannel( mIfcChannel, "IFC", mIfcChannel != UNDEFINED_CHANNEL );
	AddChannel( mRenChannel, "REN", mRenChannel != UNDEFINED_CHANNEL );
	AddChannel( mSrqChannel, "SRQ", mSrqChannel != UNDEFINED_CHANNEL );

	UpdateInterfacesFromSettings();
}

const char* GPIBAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	for( U32 i = 0; i < kDataLineCount; i++ )
	{
		text_archive << mDataChannels[ i ];
	}
	text_archive << mAtnChannel;
	text_archive << mDavChannel;
	text_archive << mNrfdChannel;
	text_archive << mNdacChannel;
	text_archive << mEoiChannel;
	text_archive << mIfcChannel;
	text_archive << mRenChannel;
	text_archive << mSrqChannel;

	return SetReturnString( text_archive.GetString() );
}
