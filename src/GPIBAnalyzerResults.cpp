#include "GPIBAnalyzerResults.h"

#include <AnalyzerHelpers.h>

#include "GPIBAnalyzer.h"
#include "GPIBAnalyzerSettings.h"

#include <fstream>
#include <string>

namespace
{
const char* GetFrameType( const Frame& frame )
{
	return ( frame.mData2 & 1 ) != 0 ? "CMD" : "DATA";
}

bool HasEoi( const Frame& frame )
{
	return ( frame.mData2 & 2 ) != 0;
}

const char* GetGPIBCommandName( U8 command_byte )
{
	switch( command_byte )
	{
		case 0x01: return "GTL";	// Go To Local
		case 0x04: return "SDC";	// Selected Device Clear
		case 0x05: return "PPC";	// Parallel Poll Configure
		case 0x08: return "GET";	// Group Execute Trigger
		case 0x09: return "TCT";	// Take Control
		case 0x10: return "LLO";	// Local Lockout
		case 0x11: return "DCL";	// Device Clear
		case 0x14: return "PPE";	// Parallel Poll Enable
		case 0x15: return "PPD";	// Parallel Poll Disable
		case 0x16: return "MTA";	// My Talk Address
		case 0x17: return "MSA";	// My Secondary Address
		case 0x20: return "MLA";	// My Listen Address
		case 0x3F: return "UNT";	// Untalk
		case 0x5F: return "UNL";	// Unlisten
		default: return NULL;
	}
}

void BuildFrameText( std::string& text, const Frame& frame, DisplayBase display_base )
{
	char value_str[ 128 ];
	AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, value_str, 128 );

	text = GetFrameType( frame );
	text += " ";
	
	// If it's a command, try to add the command name
	if( ( frame.mData2 & 1 ) != 0 )
	{
		const char* cmd_name = GetGPIBCommandName( frame.mData1 );
		if( cmd_name != NULL )
		{
			text += cmd_name;
			text += " (";
			text += value_str;
			text += ")";
		}
		else
		{
			text += value_str;
		}
	}
	else
	{
		text += value_str;
	}
	
	if( HasEoi( frame ) )
	{
		text += " EOI";
	}
}
}

GPIBAnalyzerResults::GPIBAnalyzerResults( GPIBAnalyzer* analyzer, GPIBAnalyzerSettings* settings )
: 	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

GPIBAnalyzerResults::~GPIBAnalyzerResults()
{
}

void GPIBAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	Frame frame = GetFrame( frame_index );
	std::string text;
	BuildFrameText( text, frame, display_base );
	AddResultString( text.c_str() );
}

void GPIBAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time [s],Type,Value,EOI" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i = 0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );

		char time_str[ 128 ];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char value_str[ 128 ];
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, value_str, 128 );

		file_stream << time_str << "," << GetFrameType( frame ) << "," << value_str << "," << ( HasEoi( frame ) ? "yes" : "no" ) << std::endl;

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void GPIBAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
	ClearTabularText();
	Frame frame = GetFrame( frame_index );
	std::string text;
	BuildFrameText( text, frame, display_base );
	AddTabularText( text.c_str() );
}

void GPIBAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}

void GPIBAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}