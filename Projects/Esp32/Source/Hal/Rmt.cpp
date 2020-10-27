
#include "HalCommon.h"
#include "Rmt.h"
#include "Dwt.h"

namespace Hal
{
using Utilities::DebugAssert;

Rmt::Rmt(Gpio *IoPins, Gpio::GpioIndex transmitterPin, RmtChannel channel, uint16_t bufferSize, uint16_t unitSize) : _gpio(IoPins), _transmitterPin(transmitterPin)
{
	rmt_config_t config;
	config.rmt_mode = RMT_MODE_TX;
	config.channel = static_cast<rmt_channel_t>(channel);
	config.gpio_num = static_cast<gpio_num_t>(_transmitterPin);
	config.mem_block_num = 3;
	config.tx_config.loop_en = false;
	config.tx_config.carrier_en = false;
	config.tx_config.idle_output_en = true;
	config.tx_config.idle_level = static_cast<rmt_idle_level_t>(0);
	config.clk_div = 2;

	_rmtBuffer.UnitSize = unitSize;
	_rmtBuffer.BufferSize = bufferSize;
	_rmtBuffer.Buffer = new rmt_item32_t[bufferSize];

	ESP_ERROR_CHECK(rmt_config(&config));
	ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
	_rmtBuffer.Semaphore = xSemaphoreCreateBinary();
	_rmtBuffer.Channel = channel;
	_rmtBuffer.MaxUnitsToSend = bufferSize / unitSize;
	xSemaphoreGive(_rmtBuffer.Semaphore);
}

Rmt::~Rmt()
{
}

bool Rmt::SetMaxUnitsToSend(uint16_t maxUnits)
{
	if (maxUnits > Hal::MaxAddressableLeds)
		return false;
	
	_rmtBuffer.MaxUnitsToSend = maxUnits;
	return true;
}


bool Rmt::SetBitsPerUnit(uint16_t unitSize)
{
	if (unitSize == 0)
		return false;

	DebugAssertMessage(unitSize <= _rmtBuffer.BufferSize,
		"Unit Size is bigger than RMT BufferSize: %d > %d", unitSize, _rmtBuffer.BufferSize);
	
	_rmtBuffer.UnitSize = unitSize;
	return true;
}

void IRAM_ATTR Rmt::doneOnChannel(rmt_channel_t channel, void * arg)
{
	Hal::Rmt::RmtBufferLed* rmtBuffer = (RmtBufferLed*)arg;
	rmtBuffer->Index++;
	
	if (rmtBuffer->Index < rmtBuffer->MaxUnitsToSend)
	{
		ESP_ERROR_CHECK(rmt_write_items(channel, 
						&rmtBuffer->Buffer[Hal::BitsPerLed * rmtBuffer->Index],
						Hal::BitsPerLed, false));
	}
	else
		xSemaphoreGive(rmtBuffer->Semaphore);
}

void Rmt::Write(bool wait)
{
	xSemaphoreTake(_rmtBuffer.Semaphore, portMAX_DELAY);
	// Give a delay of 50 micro seconds to flush the last writing if there was
	Dwt::DelayMicrosecond(50);
	_rmtBuffer.Index = 0;
	ESP_ERROR_CHECK(rmt_write_items(static_cast<rmt_channel_t>(_rmtBuffer.Channel), &_rmtBuffer.Buffer[0], _rmtBuffer.UnitSize, false));
	rmt_register_tx_end_callback(doneOnChannel, &_rmtBuffer);

	if (wait)
	{
		xSemaphoreTake(_rmtBuffer.Semaphore, portMAX_DELAY);
		xSemaphoreGive(_rmtBuffer.Semaphore);
	}
}

bool Rmt::UpdateBuffer(uint32_t *buffer, uint16_t length)
{
	if (length > _rmtBuffer.BufferSize)
		return false;

	for(uint16_t i = 0; i < length; i++)
	{
		uint32_t bits_to_send = buffer[i];
		uint32_t mask = 1 << (_rmtBuffer.UnitSize - 1);
		for (uint32_t bit = 0; bit < _rmtBuffer.UnitSize; bit++)
		{
			uint32_t bit_is_set = bits_to_send & mask;

			if (bit_is_set)
				_rmtBuffer.Buffer[i * _rmtBuffer.UnitSize + bit] = _timeOn;
			else
				_rmtBuffer.Buffer[i * _rmtBuffer.UnitSize + bit] = _timeOff;

			mask >>= 1;
		}
	}
	
	return true;
}


} // namespace Hal
