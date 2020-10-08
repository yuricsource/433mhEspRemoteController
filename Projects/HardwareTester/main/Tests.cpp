
#include "Tests.h"
#include "esp_http_server.h"
#include "CameraStreamTest.h"
#include "ColorConverter.h"

using Hal::Dwt;
using Hal::Hardware;
using Hal::TimeLimit;

using namespace std;

const char *testPhrase = "RTC holds the memory with low power";

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";
httpd_handle_t web_stream_httpd = NULL;

bool startTimer = true;
bool startI2s = true;


void rgb2hsv(const unsigned char &src_r, const unsigned char &src_g, const unsigned char &src_b, unsigned char &dst_h, unsigned char &dst_s, unsigned char &dst_v)
{
    float r = src_r / 255.0f;
    float g = src_g / 255.0f;
    float b = src_b / 255.0f;

    float h = 0;
	float s = 0; 
	float v = 0; // h:0-360.0, s:0.0-1.0, v:0.0-1.0

    float max = std::max(r , std::max(g, b));
    float min = std::max(r , std::max(g, b));

    v = max;

    if (max == 0.0f) {
        s = 0;
        h = 0;
    }
    else if (max - min == 0.0f) {
        s = 0;
        h = 0;
    }
    else {
        s = (max - min) / max;

        if (max == r) {
            h = 60 * ((g - b) / (max - min)) + 0;
        }
        else if (max == g) {
            h = 60 * ((b - r) / (max - min)) + 120;
        }
        else {
            h = 60 * ((r - g) / (max - min)) + 240;
        }
    }

    if (h < 0) h += 360.0f;

    dst_h = (unsigned char)(h / 2);   // dst_h : 0-180
    dst_s = (unsigned char)(s * 255); // dst_s : 0-255
    dst_v = (unsigned char)(v * 255); // dst_v : 0-255
}

void hsv2rgb(const uint16_t &src_h, const unsigned char &src_s, const unsigned char &src_v, unsigned char &dst_r, unsigned char &dst_g, unsigned char &dst_b)
{
    float h = src_h; // 0-360
    float s = src_s / 255.0f; // 0.0-1.0
    float v = src_v / 255.0f; // 0.0-1.0

    float r = 0;
	float g = 0;
	float b = 0; // 0.0-1.0

    int   hi = (int)(h / 60.0f) % 6;
    float f  = (h / 60.0f) - hi;
    float p  = v * (1.0f - s);
    float q  = v * (1.0f - s * f);
    float t  = v * (1.0f - s * (1.0f - f));

    switch(hi) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    dst_r = static_cast<unsigned char>(r * 255); // dst_r : 0-255
    dst_g = (unsigned char)(g * 255); // dst_r : 0-255
    dst_b = (unsigned char)(b * 255); // dst_r : 0-255
}


static esp_err_t stream_handler(httpd_req_t *req)
{
	camera_fb_t *fb = NULL;
	esp_err_t res = ESP_OK;
	size_t _jpg_buf_len = 0;
	uint8_t *_jpg_buf = NULL;
	char *part_buf[64];

	static int64_t last_frame = 0;
	if (!last_frame)
	{
		last_frame = esp_timer_get_time();
	}

	res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
	if (res != ESP_OK)
	{
		return res;
	}

	httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

	while (true)
	{
		fb = esp_camera_fb_get();
		if (!fb)
		{
			printf("Camera capture failed");
			res = ESP_FAIL;
		}
		else
		{
			if (fb->format != PIXFORMAT_JPEG)
			{
				bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
				esp_camera_fb_return(fb);
				fb = NULL;
				if (!jpeg_converted)
				{
					printf("JPEG compression failed");
					res = ESP_FAIL;
				}
			}
			else
			{
				_jpg_buf_len = fb->len;
				_jpg_buf = fb->buf;
			}
		}
		if (res == ESP_OK)
		{
			size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
			res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
		}
		if (res == ESP_OK)
		{
			res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
		}
		if (res == ESP_OK)
		{
			res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
		}
		if (fb)
		{
			esp_camera_fb_return(fb);
			fb = NULL;
			_jpg_buf = NULL;
		}
		else if (_jpg_buf)
		{
			free(_jpg_buf);
			_jpg_buf = NULL;
		}
		if (res != ESP_OK)
		{
			break;
		}
		// int64_t fr_end = esp_timer_get_time();
	}

	last_frame = 0;
	return res;
}

const char *GetTestPhrase()
{
	return testPhrase;
}


void TestSdCard()
{
	if (Hardware::Instance()->GetSdCard().IsMounted() == false)
	{
		if (Hardware::Instance()->GetSdCard().Mount() == false)
		{
			printf("\n\nSD Card Failed failed!\n\n");
			return;
		}
	}

	printf("\n\nOpening file\n");
	FILE *f = fopen("/sdcard/hello.txt", "w");
	if (f == NULL)
	{
		printf("Failed to open file for writing");
		return;
	}
	fprintf(f, "SD Card is Working!!! Hooray!!! :D\n");
	fclose(f);
	printf("File written\n");

	// Check if destination file exists before renaming
	struct stat st;
	if (stat("/sdcard/foo.txt", &st) == 0)
	{
		// Delete it if it exists
		unlink("/sdcard/foo.txt");
	}

	// Rename original file
	printf("Renaming file\n");
	if (rename("/sdcard/hello.txt", "/sdcard/foo.txt") != 0)
	{
		printf("Rename failed\n");
		return;
	}
}


void TestSpiffs()
{
	if (Hardware::Instance()->GetSpiffs().IsMounted() == false)
	{
		if (Hardware::Instance()->GetSpiffs().Mount() == false)
		{
			printf("\n\nSPIFFS failed!\n\n");
			return;
		}
	}

	printf("\n\nOpening file\n");
	FILE *f = fopen("/spiffs/hello.txt", "w");
	if (f == NULL)
	{
		printf("Failed to open file for writing");
		return;
	}
	fprintf(f, "SPIFFS is Working!!! Hooray!!! :D\n");
	fclose(f);
	printf("File written\n");

	// Check if destination file exists before renaming
	struct stat st;
	if (stat("/spiffs/foo.txt", &st) == 0)
	{
		// Delete it if it exists
		unlink("/spiffs/foo.txt");
	}

	// Rename original file
	printf("Renaming file\n");
	if (rename("/spiffs/hello.txt", "/spiffs/foo.txt") != 0)
	{
		printf("Rename failed\n");
		return;
	}
}

void PutCpuToSleep()
{
	printf("\n\nI'm going to bed and I will be back in 5 seconds. BYE :)\n\n");

	Hardware::Instance()->DeepSleep(5 * 1000 * 1000);
}

void SoftwareResetTest()
{
	Hardware::Instance()->SoftwareReset();
}

char ReadKey()
{
	char key = 0;
	while (key == 0)
	{
		scanf("%c", &key);
		vTaskDelay(5);
	}
	return key;
}

void ReadString(char *string, uint8_t size)
{
	uint8_t i = 0;
	char key = 0;
	while (true)
	{
		vTaskDelay(1);
		scanf("%c", &key);
		if (key == 10) // [Enter]
		{
			string[i] = '\0';
			break;
		}
		else if (key == 8) // [Backspace]
		{
			printf("%c %c", 8, 8); // clean the previous char
			i--;
			key = 0;
		}
		else if (key != 0)
		{
			string[i] = key;
			printf("%c", key);
			i++;
			key = 0;
			if (i == size - 1) // if the last key has reached the end of the buffer
			{
				string[i + 1] = '\0';
				break;
			}
		}
	}
	printf("\n");
}

void WifiMenu()
{
	char test = 0;

	while (1)
	{
		switch (test)
		{
		case 's':
		case 'S':
		{
			char mode = 0;
			printf("\n\nSet the Wifi mode:\n\n");
			printf("[1] - Client\n");
			printf("[2] - Hotspot\n");
			printf("[3] - Mesh Network\n");

			mode = ReadKey();
			if (mode == '1')
			{
				Hardware::Instance()->GetWifi().SetMode(Hal::WifiModeConfiguration::Client);
				printf("Configured as Wifi Client.\n");
			}
			else if (mode == '2')
			{
				Hardware::Instance()->GetWifi().SetMode(Hal::WifiModeConfiguration::HotSpot);
				printf("Configured as HotSpot.\n");
			}
			else if (mode == '3')
			{
				Hardware::Instance()->GetWifi().SetMode(Hal::WifiModeConfiguration::Mesh);
				printf("Configured as Mesh network (not supported yet).\n");
			}
			else
				printf("Invalid option.\n");
		}
		break;
		case 'i':
		case 'I':
		{
			char ssid[Hal::WifiSsidMaxLength] = {};
			test = 0;
			printf("Enter the SSID:\n");
			ReadString(ssid, Hal::WifiSsidMaxLength);
			printf("\nSSID set to \"%s\"\n", ssid);
			scanf("%s", ssid);
			Hardware::Instance()->GetWifi().SetSsid(ssid, strlen(ssid));
		}
		break;
		case 'p':
		case 'P':
		{
			char passwd[Hal::WifiPasswordMaxLength] = {};
			printf("Enter the Password:\n");
			ReadString(passwd, Hal::WifiPasswordMaxLength);
			printf("\nPassword set to \"%s\"\n", passwd);
			Hardware::Instance()->GetWifi().SetPassword(passwd, strlen(passwd));
		}
		break;
		case 'c':
		case 'C':
		{
			char channel = 0;
			printf("Enter the channel: \n");
			channel = ReadKey();
			channel -= 48;
			printf("%d", channel);
			Hardware::Instance()->GetWifi().SetChannel(channel);
		}
		break;
		case 't':
		case 'T':
		{
			Hardware::Instance()->GetWifi().Enable();
		}
		break;
		case 'z':
		case 'Z':
		{
			Hardware::Instance()->GetWifi().Disable();
		}
		break;
		case 'W':
		case 'w':
		{
			printf("Testing Websocket.\n");
			websocket_app_start();
			break;;
		}
		case 'x':
		case 'X':
		{
			return;
		}
		break;
		default:
			break;
		}

		printf("\n");
		printf("Wifi menu:\n");
		printf("----------\n");
		printf("[S] - Set WiFi Mode (AP, STA or Mesh)\n");
		printf("[I] - Set WiFi SSID\n");
		printf("[P] - Set WiFi Password\n");
		printf("[C] - Set WiFi Channel\n");
		printf("[T] - Start Wifi\n");
		printf("[Z] - Stop Wifi\n");
		printf("[W] - Test Websocket\n");
		printf("[X] - Return\n");

		test = ReadKey();
	}
}

void LedMenu()
{
	char test = 0;

	while (1)
	{
		switch (test)
		{
		case 'q':
		case 'Q':
		{
			//Hardware::Instance()->GetLeds().SetLed(Hal::Leds::LedIndex::Blue);
		}
		break;
		case 'a':
		case 'A':
		{
			//Hardware::Instance()->GetLeds().ResetLed(Hal::Leds::LedIndex::Blue);
		}
		break;
		case 'x':
		case 'X':
		{
			return;
		}
		break;
		default:
			break;
		}

		printf("\n");
		printf("Led menu:\n");
		printf("----------\n");
		printf("[Q] - Turn Blue On\n");
		printf("[A] - Turn Blue Off\n");
		printf("[X] - Return\n");

		test = ReadKey();
	}
}

void IoExtenderMenu()
{
	char test = 0;

	while (1)
	{
		switch (test)
		{
			case 's':
			case 'S':
			{
				printf("\nAll GPIO on the IO Extender are turned on.\n");
				
				Hardware::Instance()->GetIoExtender().ConfigureOutput(0xFF);

				Hardware::Instance()->GetIoExtender().SetAll();				
			}
			break;
			case 'd':
			case 'D':
			{
				printf("\nAll GPIO on the IO Extender are turned off.\n");
				Hardware::Instance()->GetIoExtender().ConfigureOutput(0xFF);
				Hardware::Instance()->GetIoExtender().ResetAll();

			}
			break;
			case 'f':
			case 'F':
			{
				Hardware::Instance()->GetIoExtender().ConfigureInput(0xFF);
				printf("\nThe Inputs value:%x\n", Hardware::Instance()->GetIoExtender().GetInputs());
				
			}
			break;
			case 'x':
			case 'X':
			{
				return;
			}
			break;
			default:
				break;
			}

		printf("\n");
		printf("IO Extender menu:\n");
		printf("----------\n");
		printf("[S] - Turn on all outputs\n");
		printf("[D] - Turn off all outputs\n");
		printf("[F] - Read Inputs\n");
		printf("[X] - Return\n");

		test = ReadKey();
	}
}

void CameraMenu()
{
	char test = 0;

	while (1)
	{
		switch (test)
		{
		case 's':
		case 'S':
		{
			char mode = 0;
			printf("\n\nSet the camera resolution:\n\n");
			printf("[0] - QQVGA\t(160x120)\n");
			printf("[1] - QVGA \t(320x240)\n");
			printf("[2] - VGA  \t(640x480)\n");
			printf("[3] - SVGA \t(800x600)\n");
			printf("[4] - SXGA \t(1280x1024)\n");
			printf("[5] - UXGA \t(1600x1200)\n");
			mode = ReadKey();
			if (mode == '0')
			{
			}
			else if (mode == '3')
			{
				Hal::Hardware::Instance()->GetCamera().SetResolution(Hal::CameraFrameSize::CameraFrameSizeSVGA);
			}
			else if (mode == '4')
			{
				Hal::Hardware::Instance()->GetCamera().SetResolution(Hal::CameraFrameSize::CameraFrameSizeSXGA);
			}
			else if (mode == '5')
			{
				Hal::Hardware::Instance()->GetCamera().SetResolution(Hal::CameraFrameSize::CameraFrameSizeUXGA);
			}
			else
				printf("Invalid option.\n");
		}
		break;
		case 'i':
		case 'I':
		{
			Hardware *system = Hal::Hardware::Instance();
			system->GetCamera().SetResolution(Hal::CameraFrameSize::CameraFrameSizeSVGA);

		}
		break;
		case 'w':
		case 'W':
		{
			httpd_config_t config = HTTPD_DEFAULT_CONFIG();
			httpd_uri_t stream_uri =
				{
					.uri = "/stream",
					.method = HTTP_GET,
					.handler = stream_handler,
					.user_ctx = NULL};
			if (httpd_start(&web_stream_httpd, &config) == ESP_OK)
			{
				httpd_register_uri_handler(web_stream_httpd, &stream_uri);
			}
		}
		break;
		case 'h':
		case 'H':
		{
			Hardware::Instance()->GetWifi().Disable();
			Hardware::Instance()->GetWifi().SetSsid("Camera Wifi", strlen("Camera Wifi"));
			Hardware::Instance()->GetWifi().SetPassword("123cam123", strlen("123cam123"));
			Hardware::Instance()->GetWifi().SetMode(Hal::WifiModeConfiguration::HotSpot);
			Hardware::Instance()->GetWifi().SetAuthentication(Hal::WifiAuthenticationMode::Wpa2Psk);
			Hardware::Instance()->GetWifi().Enable();
			Hardware::Instance()->GetCamera().Init();
			startCameraServer();
			break;
		}
		case 'x':
		case 'X':
		{
			return;
		}
		break;
		default:
			break;
		}

		printf("\n");
		printf("Camera menu:\n");
		printf("----------\n");
		printf("[S] - Camera Resolution\n");
		//printf("[P] - Capture and Save in the internal Flash\n");
		printf("[I] - Capture 10 images and Save in the SD Card\n");
		printf("[W] - Start Simple Streaming Web\n");
		printf("[H] - Start Demo Streaming Web\n");
		printf("[X] - Return\n");

		test = ReadKey();
	}
}

static void spin_task(void *arg)
{
	printf("\nStarting Timer 1 with 5Hz\n");

    while (1)
	{
		// Hardware::Instance()->GetLeds().Refresh();
        vTaskDelay(100);
    }
}

void TestLed()
{
	Hal::Led led = {};
	led.Color.Red = 0xff /8;
	led.Color.Blue = 0xff /3;
	led.Color.Green = 0xff / 8;
	Hardware::Instance()->GetLeds().SetLedsCount(10);
	
	for(;;)
	{
		uint16_t color = Hardware::Instance()->GetRng().GetNumber() % 361;
		Hal::LedHsv hsv = {color, 255, 255};
		 
		for (uint16_t i = 0; i < 256; i++)
		{
			hsv.Color.Value = i;
			hsv.Color.Saturation = 255;
			Utilities::ColorConverter::HsvToRgb(hsv, led);
			for(uint8_t ledIndex = 0; ledIndex < 10; ledIndex++)
				Hardware::Instance()->GetLeds().SetLedColor(ledIndex, led);
			if (i != 0)
				vTaskDelay((50 / i));
			//printf("i = %d -> R=%d, G=%d, B=%d\n", i, led.Color.Red, led.Color.Blue, led.Color.Blue);
		}
		// printf("-----------------------\n");
		for (uint16_t i = 255; i != 0; i--)
		{
			hsv.Color.Value = i;
			hsv.Color.Saturation = 255;
			Utilities::ColorConverter::HsvToRgb(hsv, led);
			for(uint8_t ledIndex = 0; ledIndex < 10; ledIndex++)
				Hardware::Instance()->GetLeds().SetLedColor(ledIndex, led);
			vTaskDelay((50 / i));
			//printf("i = %d -> R=%d, G=%d, B=%d\n", i, led.Color.Red, led.Color.Blue, led.Color.Blue);
		}

	}

	// if (startTimer)
	// {
	// 	// xTaskCreatePinnedToCore(spin_task, "stats", 4096, NULL, 3, NULL, 1);
	// 	// printf("\nStarting Timer 0 with 5Hz\n");
	// 	// Hardware::Instance()->GetTimer1().SetTimer(16000);
	// 	// Hardware::Instance()->GetTimer1().Start();
	// 	// Hardware::Instance()->GetTimer0().SetTimer(16000);
	// 	// Hardware::Instance()->GetTimer0().Start();
	// 	printf("\nStarting Timer 1 with 5Hz\n");
	// 	for(;;)
	// 	{
	// 		// Hardware::Instance()->GetLeds().Refresh();
	// 		vTaskDelay(100);
	// 	}
	// }
	// else
	// {
	// 	printf("\nStoping Timer 0\n");
	// 	//Hardware::Instance()->GetTimer1().Stop();
	// }
	// startTimer = !startTimer;
}

void TestI2sClock()
{
	// Hal::Rmt::led_state new_state = {};
	// new_state.Red = 0xff;

	// Hardware::Instance()->GetRmt().Write(new_state);
	// if(startI2s)
	// {
	// 	printf("\nStarting I2s at 25MHz\n");
	// 	// Hardware::Instance()->GetI2s().Start(Hal::Gpio::GpioIndex::Gpio26);
	// 	uint8_t buffer[100];
		
	// 	for(uint16_t i = 0; i < 100; i ++)
	// 		buffer[i] = 0xAA;
	// 	// Hardware::Instance()->GetI2s().Send(buffer, 100);
	// }
	// else
	// {
		
	// }
	// startI2s = !startI2s;	
}

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

static void websocket_app_start(void)
{
    esp_websocket_client_config_t websocket_cfg = {};

    shutdown_signal_timer = xTimerCreate("Websocket shutdown timer", NO_DATA_TIMEOUT_SEC * 1000 / portTICK_PERIOD_MS,
                                         pdFALSE, NULL, shutdown_signaler);
    shutdown_sema = xSemaphoreCreateBinary();

#if 0
    char line[128];

    printf("Please enter uri of websocket endpoint");
    ReadString(line, sizeof(line));

    websocket_cfg.uri = line;
    printf( "Endpoint uri: %s\n", line);

#else
    websocket_cfg.uri = "ws://10.1.1.101";
	websocket_cfg.path = "/socket.io/?EIO=3&transport=websocket";
    websocket_cfg.port = 3000;

#endif /* CONFIG_WEBSOCKET_URI_FROM_STDIN */

    printf("Connecting to %s...\n", websocket_cfg.uri);

    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);

    esp_websocket_client_start(client);
    xTimerStart(shutdown_signal_timer, portMAX_DELAY);
    char data[32];
    int i = 0;
    while (i < 10) {
        if (esp_websocket_client_is_connected(client)) {
            int len = sprintf(data, "42[\"chat\",\"Test%d\"]", i++);
            printf("Sending %s\n", data);
            esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    xSemaphoreTake(shutdown_sema, portMAX_DELAY);
    esp_websocket_client_stop(client);
    printf("Websocket Stopped\n");
    esp_websocket_client_destroy(client);
}
