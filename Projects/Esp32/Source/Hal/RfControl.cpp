
#include "RfControl.h"
#include <string>
#include <cstring>

namespace Hal
{

RfControl::RfControl(Gpio *IoPins, Rmt* rmt) : _gpio(IoPins), _rmt(rmt)
{
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
	uint32_t* commandBufferPointer = reinterpret_cast<uint32_t*>(_commands[commandId].data()); 
	_rmt->UpdateBuffer(commandBufferPointer, _commands[commandId].size());
	_rmt->Write();
	return true;
}

RfControl::~RfControl()
{
}

} // namespace Hal
