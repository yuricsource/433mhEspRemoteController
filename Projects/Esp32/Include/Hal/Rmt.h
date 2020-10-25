
#ifndef INCLUDE_HAL_RMT_H_
#define INCLUDE_HAL_RMT_H_

//#include "Hardware.h"
#include "HalCommon.h"
#include "Gpio.h"
#include "Timer.h"
#include "Rmt.h"
#include "driver/rmt.h"
#include <array>
#include "freertos/semphr.h"

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
using std::array;

class Rmt // : public Timer::Callback
{

public:
	// @brief Gpio flass pointer, Gpio pin, RMT Channel, Buffer Size (uint32_t)
	Rmt(Gpio *IoPins, Gpio::GpioIndex transmitterPin, RmtChannel channel, uint16_t bufferSize, uint16_t unitSize = 1);
	~Rmt();
	void Write();
	bool UpdateBuffer(uint32_t *buffer, uint16_t length);
	bool SetMaxUnitsToSend(uint16_t maxUnits);
	bool SetUnitSize(uint16_t unitSize);

	// @brief Set the period of bit 1. Ex.: |	Bit 1	|	timeHigh = 15 -> 800ns	|	timeLow = 35-> 450ns	|
	inline bool SetTimeBitOn(const uint16_t timeHigh, const uint16_t timeLow)
	{
		if (timeHigh == 0 || timeLow == 0)
			return false;

		tOn.duration0 = timeHigh;
		tOn.duration1 = timeLow;

		return true;
	}
	
	// @brief Set the period of bit 0. Ex.: |	Bit 1	|	timeHigh = 35 -> 400ns	|	timeLow = 15-> 850ns	|
	inline bool SetTimeBitOff(const uint16_t timeHigh, const uint16_t timeLow)
	{
		if (timeHigh == 0 || timeLow == 0)
			return false;

		tOff.duration0 = timeHigh;
		tOff.duration1 = timeLow;
		
		return true;
	}

	struct RmtBufferLed
	{
		uint16_t Index = 0;
		uint16_t MaxUnitsToSend = 0;
		uint16_t UnitSize = 0;
		uint16_t BufferSize = 0;
		xSemaphoreHandle Semaphore;
		rmt_item32_t* Buffer = nullptr;
		RmtChannel Channel = RmtChannel::RmtChannel0;
	};
private:
	RmtBufferLed _rmtBuffer = {};
	static void IRAM_ATTR doneOnChannel(rmt_channel_t channel, void * arg);
	Gpio *_gpio;
	Gpio::GpioIndex _transmitterPin;
	rmt_item32_t tOn = {{{T1H, 1, T1L, 0}}};
	rmt_item32_t tOff = {{{T0H, 1, T0L, 0}}};
};
} // namespace Hal

#endif /* INCLUDE_HAL_RMT_H_ */
