#ifndef GPIB_ANALYZER_SETTINGS
#define GPIB_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

#include <vector>

class GPIBAnalyzerSettings : public AnalyzerSettings
{
public:
	static const U32 kDataLineCount = 8;
	static const U32 kControlLineCount = 8;

	GPIBAnalyzerSettings();
	virtual ~GPIBAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	std::vector<Channel> mDataChannels;
	Channel mAtnChannel;
	Channel mDavChannel;
	Channel mNrfdChannel;
	Channel mNdacChannel;
	Channel mEoiChannel;
	Channel mIfcChannel;
	Channel mRenChannel;
	Channel mSrqChannel;

protected:
	std::vector<AnalyzerSettingInterfaceChannel*> mDataChannelInterfaces;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mAtnChannelInterface;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mDavChannelInterface;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mNrfdChannelInterface;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mNdacChannelInterface;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mEoiChannelInterface;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mIfcChannelInterface;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mRenChannelInterface;
	std::unique_ptr<AnalyzerSettingInterfaceChannel> mSrqChannelInterface;
};

#endif //GPIB_ANALYZER_SETTINGS
