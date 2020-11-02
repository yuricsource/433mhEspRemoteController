#include "LearnerCode.h"



void LearnerCode::Stop()
{
    _timer->Stop();
}

void LearnerCode::Start()
{
    _state = CodeLearnerState::None;
    _gpio->ConfigInput(_pin);
    _bufferIndex = 0;
    memset(_data, 0, sizeof(_data));
    _timer->Start();
    Hal::Hardware::Instance()->GetGpio().SetMode(Hal::Gpio::GpioIndex::Gpio5, Hal::Gpio::Mode::Output);
    Hal::Hardware::Instance()->GetGpio().SetMode(Hal::Gpio::GpioIndex::Gpio18, Hal::Gpio::Mode::Output);
    Hal::Hardware::Instance()->GetGpio().SetMode(Hal::Gpio::GpioIndex::Gpio19, Hal::Gpio::Mode::Output);
    Hal::Hardware::Instance()->GetGpio().SetMode(Hal::Gpio::GpioIndex::Gpio21, Hal::Gpio::Mode::Output);
}

void LearnerCode::PrintResult()
{
    printf("\n");
    printf("_bufferIndex:%d, MinimunBitsAllowed:%d, WaitCount:%d, HighOrLowCount:%d\n",
        _bufferIndex, MinimunBitsAllowed, WaitCount, HighOrLowCount);
    for(uint8_t i = 0; i < _bufferIndex; i++)
        printf("%2X ", _data[i]);
}

void LearnerCode::TimerCallback()
{
    switch(_state)
    {
        case CodeLearnerState::None:
        {
            if (_gpio->Get(_pin))
            {
                Hal::Hardware::Instance()->GetGpio().Set(Hal::Gpio::GpioIndex::Gpio5);
                _waitCounter = 0;
            }
            else
            {
                _waitCounter++;
                if (_waitCounter > WaitCount)
                {
                    _state = CodeLearnerState::Ready;
                }
            }
        }
        break;
        case CodeLearnerState::Ready:
        {
            Hal::Hardware::Instance()->GetGpio().Set(Hal::Gpio::GpioIndex::Gpio19);
            if (_gpio->Get(_pin))
            {
                Hal::Hardware::Instance()->GetGpio().Set(Hal::Gpio::GpioIndex::Gpio5);
                _state = CodeLearnerState::Logging;
                _tOnCounter++;
            }
        }
        break;
        case CodeLearnerState::Logging:
        {
            Hal::Hardware::Instance()->GetGpio().Set(Hal::Gpio::GpioIndex::Gpio21);
            if (_gpio->Get(_pin))
            {
                Hal::Hardware::Instance()->GetGpio().Set(Hal::Gpio::GpioIndex::Gpio5);
                _tOnCounter++;
            }
            else
            {
                _waitCounter++;
                if (_waitCounter > WaitCount)
                {
                    if (_bufferIndex < MinimunBitsAllowed)
                    {
                        _bufferIndex = 0;
                        _waitCounter = 0;
                        _state = CodeLearnerState::Ready;
                        Hal::Hardware::Instance()->GetGpio().Set(Hal::Gpio::GpioIndex::Gpio18);
                    }
                    else
                        _state = CodeLearnerState::Finished;
                }

                if (_tOnCounter != 0)
                {
                    if (_tOnCounter > HighOrLowCount)
                    {
                        _data[_bufferIndex++] = 1;
                        // Hal::Hardware::Instance()->GetGpio().Set(Hal::Gpio::GpioIndex::Gpio18);
                    }
                    else
                    {
                        //Hal::Hardware::Instance()->GetGpio().Set(Hal::Gpio::GpioIndex::Gpio19);
                        _data[_bufferIndex++] = 0;
                    }
                    _tOnCounter = 0;
                    _waitCounter = 0;
                }
            }
        }
        break;
        case CodeLearnerState::Finished:
        {

        }
        break;
        default:
            _state = CodeLearnerState::None;
    }

     Hal::Hardware::Instance()->GetGpio().Reset(Hal::Gpio::GpioIndex::Gpio5);
     Hal::Hardware::Instance()->GetGpio().Reset(Hal::Gpio::GpioIndex::Gpio18);
     Hal::Hardware::Instance()->GetGpio().Reset(Hal::Gpio::GpioIndex::Gpio19);
     Hal::Hardware::Instance()->GetGpio().Reset(Hal::Gpio::GpioIndex::Gpio21);
}