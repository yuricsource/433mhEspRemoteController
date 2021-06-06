#include "RemoteReceiverService.h"
#include "ConfigurationAgent.h"
#include "Hardware.h"
#include "BankConfiguration.h"
#include "Md5Hash.h"
#include "HalCommon.h"
#include "Flash.h"
#include "Timer.h"

namespace Applications
{
using Hal::Gpio;

RemoteReceiverService::RemoteReceiverService() : cpp_freertos::Thread("REMSVC", configAUDPLAYERSVC_STACK_DEPTH, 3)
{
	_hardware = Hardware::Instance();
}

RemoteReceiverService::~RemoteReceiverService()
{
}

void RemoteReceiverService::Run()
{
	Logger::LogInfo(Utilities::Logger::LogSource::AudioPlayer, "Remote Receiver Service Initialized.");

	for(;;)
	{
		Delay(100);
	}
}

} // namespace Applications
