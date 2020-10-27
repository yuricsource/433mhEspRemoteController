
#include "RfControl.h"
#include <string>
#include <cstring>
#include "Dwt.h"
namespace Hal
{

RfControl::RfControl(Gpio *IoPins, Rmt* rmt) : _gpio(IoPins), _rmt(rmt)
{
	_commands[0] = {0x22437856, 0xF3BA1FFB, 0x80000000};
}

bool RfControl::SetCommand(RfCommandArray& command, const uint8_t commandId)
{
	if (commandId > MaxCommandSupported)
		return false;

	memcpy(_commands[commandId].data(), command.data(), _commands[commandId].size());
	return true;
}

bool RfControl::RunCommand(uint8_t commandId)
{
	_rmt->SetProtocol(Rmt::ProtocolSupported::herculiftRemoteControl);
	// first send the 12 pulses
	_rmt->SetTimeBitOn(440, 500);
	_rmt->SetTimeBitOff(440, 500);
	uint32_t pulses[3];

	memset(pulses, 0xFF, sizeof(pulses));
	
	_rmt->SetBitsPerUnit(12);
	_rmt->SetMaxUnitsToSend(1);
	_rmt->UpdateBuffer(pulses, sizeof(pulses));
	_rmt->Write(true);
	Dwt::DelayMilliseconds(4);
	_rmt->SetProtocol(Rmt::ProtocolSupported::herculiftRemoteControl);
	
	_rmt->SetBitsPerUnit(22);
	_rmt->SetMaxUnitsToSend(3);
	uint32_t* commandBufferPointer = reinterpret_cast<uint32_t*>(_commands[commandId].data()); 
	_rmt->UpdateBuffer(commandBufferPointer, 17);
	_rmt->Write();
	return true;
}

RfControl::~RfControl()
{
}

} // namespace Hal
