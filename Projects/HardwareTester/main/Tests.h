
#ifndef TESTS_H_
#define TESTS_H_







#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"
#include "esp_event_loop.h"

#define NO_DATA_TIMEOUT_SEC 10

static const char *TAG = "WEBSOCKET";

static TimerHandle_t shutdown_signal_timer;
static SemaphoreHandle_t shutdown_sema;

static void shutdown_signaler(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG, "No data received for %d seconds, signaling shutdown", NO_DATA_TIMEOUT_SEC);
    xSemaphoreGive(shutdown_sema);
}

#if CONFIG_WEBSOCKET_URI_FROM_STDIN
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

#endif /* CONFIG_WEBSOCKET_URI_FROM_STDIN */

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
        ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

        xTimerReset(shutdown_signal_timer, portMAX_DELAY);
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}







#include <cstring>
#include <string>
#include "stdio.h"
#include <cstdint>
#include <cstdarg>
#include "stdio.h"
#include "wchar.h"
#include "Hardware.h"

using Hal::Timer;
using Hal::Hardware;

void SoftwareResetTest();
void PutCpuToSleep();
void TestSpiffs();
void WifiMenu();
void LedMenu();
char ReadKey();
void ReadString(char * string, uint8_t size);
void CameraMenu();
void TestLed();
void TestI2sClock();
void TestSdCard();
void IoExtenderMenu();

const char *GetTestPhrase();

class TestClass : Timer::Callback
{
public:
    TestClass()
    {
        // Hal::Hardware::Instance()->GetTimer0().AddCallback(this);
        Hardware::Instance()->GetGpio().ConfigOutput(Hal::Gpio::GpioIndex::Gpio26,
            Hal::Gpio::OutputType::PullUp);
    }
    void TimerCallback() override
    {
       Hardware::Instance()->GetGpio().Set(Hal::Gpio::GpioIndex::Gpio26);
       Hal::Dwt::DelayMilliseconds(40);
       Hardware::Instance()->GetGpio().Reset(Hal::Gpio::GpioIndex::Gpio26);
    }
};



static void websocket_app_start(void);
#endif /* TESTS_H_ */
