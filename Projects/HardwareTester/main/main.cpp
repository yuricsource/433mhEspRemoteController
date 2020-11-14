#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <string>
#include <sstream>
#include <esp_attr.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <chrono>
#include <memory>
#include <string>
#include <sstream>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "Hardware.h"
#include "Tests.h"
#include "Rng.h"
#include "esp_http_server.h"
#include <cstring>

void executetMenu(char Test)
{
	switch (Test)
	{

	case 't':
	case 'T':
		TestLed();
		break;
	case 'r':
	case 'R':
		SoftwareResetTest();
		break;
	case 'f':
	case 'F':
		PutCpuToSleep();
		break;
	case 's':
	case 'S':
		TestSpiffs();
		break;
	case 'v':
	case 'V':
		TestSdCard();
		break;
	case 'w':
	case 'W':
		WifiMenu();
		break;
	case 'd':
	case 'D':
		TestTransmitter();
		break;
	case 'l':
	case 'L':
		LearnCode();
		break;
	case 'i':
	case 'I':
		IoExtenderMenu();
		break;
	case 'b':
	case 'B':
		ReadButtonAndAnalog();
		break;
	default:
		break;
	}

	printf("\n");
	printf("Main menu:\n");
	printf("----------\n");
	printf("[T] - Led Test (on pin 32)\n");
	printf("[R] - Software Reset Test\n");
	printf("[F] - Deep Sleep for 5 Seconds.\n");
	printf("[S] - Test SPIFFS\n");
	printf("[V] - Test SD Card\n");
	printf("[W] - WiFi Menu\n");
	printf("[B] - Input Menu\n");
	printf("[D] - Test controller transmitter\n");
	printf("[L] - Learn RF remote control code\n");
	printf("[I] - IO Extender menu\n");
}

extern "C" void app_main(void)
{
	Hal::Hardware::Instance();
 	printf("Hardware Tester for ESP32\n");

	
	TestClass testClass;
	char test = 0;
	
	/*
	Hal::Hardware::Instance()->GetWifi().Disable();
	Hal::Hardware::Instance()->GetWifi().SetSsid("Yuri_Duda", strlen("Yuri_Duda"));
	Hal::Hardware::Instance()->GetWifi().SetPassword("Australia2us", strlen("Australia2us"));
	Hal::Hardware::Instance()->GetWifi().SetMode(Hal::WifiModeConfiguration::Client);
	Hal::Hardware::Instance()->GetWifi().SetAuthentication(Hal::WifiAuthenticationMode::Wpa2Psk);
	Hal::Hardware::Instance()->GetWifi().Enable();
	*/
	while (1)
	{
		executetMenu(test);
		test = ReadKey();
	}
}
