
#ifndef TESTS_H_
#define TESTS_H_
#include <cstring>
#include <string>
#include "stdio.h"
#include <cstdint>
#include <cstdarg>
#include "stdio.h"
#include "wchar.h"
#include "Hardware.h"
#include "LearnerCode.h"

using Hal::Timer;
using Hal::Hardware;

static void get_string(char *line, size_t size)
{
    int count = 0;
    while (count < size) {
        int c = fgetc(stdin);
        if (c == '\n') {
            line[count] = '\0';
            break;
        } else if (c > 0 && c < 127) {
            line[count] = c;
            ++count;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void SoftwareResetTest();
void PutCpuToSleep();
void TestSpiffs();
void WifiMenu();
char ReadKey();
void ReadString(char * string, uint8_t size);

void TestTransmitter();
void LearnCode(bool infrared = false);


class TestClass : Timer::Callback
{
public:
    TestClass()
    {
        //Hal::Hardware::Instance()->GetTimer0().AddCallback(this);
        Hardware::Instance()->GetGpio().ConfigOutput(Hal::Gpio::GpioIndex::Gpio26,
            Hal::Gpio::OutputType::PullUp);
    }
    void TimerCallback() override
    {
       Hardware::Instance()->GetGpio().Set(Hal::Gpio::GpioIndex::Gpio26);
       Hal::Dwt::DelayMicrosecond(1);
       Hardware::Instance()->GetGpio().Reset(Hal::Gpio::GpioIndex::Gpio26);
    }
};

#endif /* TESTS_H_ */
