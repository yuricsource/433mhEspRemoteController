
#ifndef HAL_SYSTEM_H_
#define HAL_SYSTEM_H_

#include "HalCommon.h"
#include <stdlib.h>
#include <stdio.h>
#include <Spiffs.h>
#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "TimerInterruptHandler.h"
#include "Gpio.h"
#include "Dwt.h"
#include "DebugPort.h"
#include "Leds.h"
#include "TimeLimit.h"
#include "Camera.h"
#include "Rng.h"
#include "Adc.h"
#include "WifiDriver.h"
#include "Flash.h"
#include "BankConfiguration.h"
#include "Spi.h"
#include "Timer.h"
#include "TimerInterruptHandler.h"
#include "Dac.h"
#include "SdCard.h"
#include "Rmt.h"
#include "I2c.h"
#include "IoExtender.h"

namespace Hal
{

class Hardware
{
public:
	Hardware();
	~Hardware();
	Gpio &GetGpio() { return _gpio; }
	Leds &GetLeds() { return _leds; }
	Spiffs &GetSpiffs() { return _spiffs; }
	Camera &GetCamera() { return _camera; }
	DebugPort &GetDebugPort() { return _debugPort; }
	Rng &GetRng() { return _rng; }
	WifiDriver &GetWifi() { return _wifiDriver; }
	Flash &GetFlash() { return _flash; }
	BankConfiguration &GetBunkConfiguration() { return _bankConfig; }
	Spi &GetSpi() { return _spi; }
	Timer &GetTimer0() { return _timer0; }
	Timer &GetTimer1() { return _timer1; }
	Adc &GetAdc() { return _adc; }
	Dac &GetDac() { return _dac; }
	SdCard &GetSdCard() { return _sdCard; }
	Rmt &GetRmt(){ return _rmt; }
	I2c &GetI2c() { return _i2c; }
	IoExtender& GetIoExtender() { return _ioExtender; }

	uint32_t Milliseconds();
	void TimerCallback();
	ResetReason GetResetReason();
	void SoftwareReset();
	uint32_t GetRandomNumber();
	uint32_t GetSystemClockBase();
	uint32_t GetHeapSize();

	uint32_t GetMinFreeHeap(void);
	uint32_t GetMaxAllocHeap(void);
	uint32_t GetPsramSize(void);
	uint32_t GetFreePsram(void);
	uint32_t GetMinFreePsram(void);
	uint32_t GetMaxAllocPsram(void);
	
	void DeepSleep(uint32_t uSeconds);
	char *GetResetReasonAsString(ResetReason reason);
	
	static inline Hardware *Instance()
	{
		if (_pHardware == nullptr)
		{
			_pHardware = new Hardware();
		}
		return _pHardware;
	}

private:
	static Hardware *_pHardware;
	Gpio _gpio;
	Adc _adc;
	DebugPort _debugPort;
	Spiffs _spiffs;
	esp_chip_info_t _mcuInfo;
	MacAddress _macAdrress;
	Camera _camera;
	Rng _rng;
	WifiDriver _wifiDriver;
	Flash _flash;
	BankConfiguration _bankConfig;
	Spi _spi;
	TimerInterruptHandler _timerInterruptHandler;
	Timer _timer0;
	Timer _timer1;
	Dac _dac;
	SdCard _sdCard;
	Rmt _rmt;
	Leds _leds;
	I2c _i2c;
	IoExtender _ioExtender;
};
} // namespace Hal

#endif /* HAL_SYSTEM_H_ */
