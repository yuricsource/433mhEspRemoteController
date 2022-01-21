
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
#include "RfControl.h"
#include "DeviceInput.h"
#include "CodeReceiver.h"

// #define SPG_GATE 1

namespace Hal
{

class Hardware
{
public:
	Hardware();
	~Hardware();
	Gpio &GetGpio() { return _gpio; }
	Spiffs &GetSpiffs() { return _spiffs; }
	DebugPort &GetDebugPort() { return _debugPort; }
	Rng &GetRng() { return _rng; }
	WifiDriver &GetWifi() { return _wifiDriver; }
	Flash &GetFlash() { return _flash; }
	BankConfiguration &GetBunkConfiguration() { return _bankConfig; }
	Timer &GetTimer0() { return _timer0; }
	Timer &GetTimer1() { return _timer1; }
	Adc &GetAdc() { return _adc; }
	Rmt &GetRmt(){ return _rmtLeds; }
	RfControl& GetRfControl() { return _rfControl; }
	CodeReceiver& GetCodeReceiver() { return _codeReceiver; }
	uint32_t Milliseconds();
	void TimerCallback();
	ResetReason GetResetReason();
	void SoftwareReset();
	int GetRandomNumberRange(int from = 0, int to = 0);
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
	Rng _rng;
	WifiDriver _wifiDriver;
	Flash _flash;
	BankConfiguration _bankConfig;
	TimerInterruptHandler _timerInterruptHandler;
	Timer _timer0;
	Timer _timer1;
	Rmt _rmtLeds;
	Rmt _rmtRemoteControl;
	RfControl _rfControl;
	CodeReceiver _codeReceiver;
};
} // namespace Hal

#endif /* HAL_SYSTEM_H_ */
