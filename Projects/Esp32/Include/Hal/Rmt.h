
#ifndef INCLUDE_HAL_RMT_H_
#define INCLUDE_HAL_RMT_H_

//#include "Hardware.h"
#include "HalCommon.h"
#include "Gpio.h"
#include "Timer.h"
#include "Rmt.h"
#include "driver/rmt.h"
//#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Configure these based on your project needs ********
#define LED_RMT_TX_GPIO 18
// ****************************************************

#define BITS_PER_LED_CMD 24 
#define NUM_LEDS 1
#define LED_BUFFER_ITEMS (BITS_PER_LED_CMD)

// These values are determined by measuring pulse timing with logic 
// analyzer and adjusting to match datasheet. 
#define T0H  15  // 0 bit high time
#define T0L  35  // 1 bit high time
#define T1H  35  // low time for either bit
#define T1L  15  // low time for either bit

	/*
		In order to send bits through the leds,
		they need to respect the timing table below
		_____________________________________
		|	Bit		|	T On	|	T Off	|
		_____________________________________
		|	Bit 1	|	800ns	|	450ns	|
		|	Bit 0	|	400ns	|	850ns	|
		-------------------------------------

		The full pixel colour needs to receive 24 bits.
		If the next 24 bits comes withing the reset code timing, the first 24 bit
		will be passed to the next led, otherwise, the new 24 bits
		colours will overwrite the first led colour.
		_________________________________________________________________
		|				   pixel LedColor				|	Reset Code	|
		_________________________________________________________________
		| 8 bits Green	|	8 bits Red	|	8 bits Blue	|	  >=50us	|
		-----------------------------------------------------------------
	*/

namespace Hal
{

class Rmt // : public Timer::Callback
{

public:

	Rmt(Gpio *IoPins, Gpio::GpioIndex transmitterPin, RmtChannel channel);
	~Rmt();
	void Write();
	void UpdateLed(uint16_t ledId, Led color);
	void UpdateAllLeds(LedsArray leds);
	bool SetMaxLeds(uint16_t maxLeds);
	struct RmtBufferLed
	{
		uint16_t LedIndex = 0;
		uint16_t MaxLeds = 0;
		xSemaphoreHandle Semaphore;
		rmt_item32_t LedBuffer[Hal::BitsPerLed * Hal::MaxAddressableLeds] = {};
		RmtChannel Channel = RmtChannel::RmtChannel0;
	};
private:
	RmtBufferLed _rmtBuffer = {};
	static void IRAM_ATTR doneOnChannel(rmt_channel_t channel, void * arg);
	Gpio *_gpio;
	Gpio::GpioIndex _transmitterPin;
	rmt_item32_t tOn = {{{T1H, 1, T1L, 0}}};
	rmt_item32_t tOff = {{{T0H, 1, T0L, 0}}};
	// This is the buffer which the hw peripheral will access while pulsing the output pin
	rmt_item32_t led_data_buffer[LED_BUFFER_ITEMS];
};
} // namespace Hal

#endif /* INCLUDE_HAL_RMT_H_ */
