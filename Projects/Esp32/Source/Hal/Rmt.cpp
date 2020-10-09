
#include "HalCommon.h"
#include "Rmt.h"
#include "Dwt.h"
#include "DebugAssert.h"

namespace Hal
{
using Utilities::DebugAssert;

Rmt::Rmt(Gpio *IoPins, Gpio::GpioIndex transmitterPin, RmtChannel channel) : _gpio(IoPins), _transmitterPin(transmitterPin)
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

	ESP_ERROR_CHECK(rmt_config(&config));
	ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
	_rmtBuffer.Semaphore = xSemaphoreCreateBinary();
	_rmtBuffer.Channel = channel;
	_rmtBuffer.MaxLeds = Hal::MaxAddressableLeds;
	xSemaphoreGive(_rmtBuffer.Semaphore);
}

Rmt::~Rmt()
{
}

bool Rmt::SetMaxLeds(uint16_t maxLeds)
{
	if (maxLeds > Hal::MaxAddressableLeds)
		return false;
	
	_rmtBuffer.MaxLeds = maxLeds;
	return true;
}

void IRAM_ATTR Rmt::doneOnChannel(rmt_channel_t channel, void * arg)
{
	Hal::Rmt::RmtBufferLed* rmtBuffer = (RmtBufferLed*)arg;
	rmtBuffer->LedIndex++;
	
	if (rmtBuffer->LedIndex < rmtBuffer->MaxLeds)
	{
		ESP_ERROR_CHECK(rmt_write_items(channel, 
						&rmtBuffer->LedBuffer[Hal::BitsPerLed * rmtBuffer->LedIndex],
						Hal::BitsPerLed, false));
	}
	else
		xSemaphoreGive(rmtBuffer->Semaphore);
}

void Rmt::Write()
{
	xSemaphoreTake(_rmtBuffer.Semaphore, portMAX_DELAY);
	// Give a delay of 100 micro seconds to flush the last writing if there was
	Dwt::DelayMicrosecond(100);
	_rmtBuffer.LedIndex = 0;
	ESP_ERROR_CHECK(rmt_write_items(static_cast<rmt_channel_t>(_rmtBuffer.Channel), &_rmtBuffer.LedBuffer[0], Hal::BitsPerLed, false));
	rmt_register_tx_end_callback(doneOnChannel, &_rmtBuffer);
}

void Rmt::UpdateLed(uint16_t ledId, Led color)
{
	uint32_t bits_to_send = color.Value;
	uint32_t mask = 1 << (BITS_PER_LED_CMD - 1);
	for (uint32_t bit = 0; bit < BITS_PER_LED_CMD; bit++)
	{
		uint32_t bit_is_set = bits_to_send & mask;

		if (bit_is_set)
			_rmtBuffer.LedBuffer[ledId * BITS_PER_LED_CMD + bit] = tOn;
		else
			_rmtBuffer.LedBuffer[ledId * BITS_PER_LED_CMD + bit] = tOff;

		mask >>= 1;
	}
}

void Rmt::UpdateAllLeds(LedsArray leds)
{
	for(uint16_t ledIndex = 0; ledIndex < leds.size(); ledIndex++)
	{
		uint32_t bits_to_send = leds.data()[ledIndex].Value;
		uint32_t mask = 1 << (BITS_PER_LED_CMD - 1);
		for (uint32_t bit = 0; bit < BITS_PER_LED_CMD; bit++)
		{
			uint32_t bit_is_set = bits_to_send & mask;

			if (bit_is_set)
				_rmtBuffer.LedBuffer[ledIndex * BITS_PER_LED_CMD + bit] = tOn;
			else
				_rmtBuffer.LedBuffer[ledIndex * BITS_PER_LED_CMD + bit] = tOff;

			mask >>= 1;
		}
	}
}


} // namespace Hal
