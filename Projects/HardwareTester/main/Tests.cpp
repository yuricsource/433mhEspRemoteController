
#include "Tests.h"
#include "ColorConverter.h"
#include "LearnerCode.h"
#include "HalCommon.h"
#include "CodeReceiver.h"

Hal::CodeReceiver* learnerTest = nullptr;

using Hal::Dwt;
using Hal::Hardware;
using Hal::TimeLimit;

using namespace std;

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

void LearnCode(bool infrared)
{
	printf("LearnCode 1\n");
	char test = 0;
	if (learnerTest == nullptr)
	{
		learnerTest = &Hal::Hardware::Instance()->GetCodeReceiver();
	}

	if (infrared)
		learnerTest->Configure(Hal::Gpio::GpioIndex::Gpio4);
	else
		learnerTest->Configure(Hal::Gpio::GpioIndex::Gpio14);

	printf("LearnCode 2\n");

	learnerTest->Configure(infrared);

	printf("LearnCode 3\n");
	while (1)
	{
		switch (test)
		{
			case 'l':
			case 'L':
			{
				learnerTest->Stop();
				Hardware::Instance()->GetTimer0().SetTimer(16000);
				learnerTest->Start();
			}
			break;
			case 's':
			case 'S':
			{
				learnerTest->Stop();
				Hardware::Instance()->GetTimer0().Stop();
			}
			break;
			case 'p':
			case 'P':
			{
				learnerTest->PrintResult();
			}
			break;
			case 'k':
			case 'K':
			{
				printf("Printing code:\n");
				while(true)
				{
					if (learnerTest->CodeReceived())
					{
						learnerTest->PrintResult();
						learnerTest->Stop();
						learnerTest->Start();
					}
					vTaskDelay(1000);
				}
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
		printf("Code Learner menu:\n");
		printf("----------\n");
		printf("[L] - Start Code Learner\n");
		printf("[S] - Stop Code Learner\n");
		printf("[P] - Print Code\n");
		printf("[K] - Keep Printing Code\n");
		printf("[X] - Return\n");

		test = ReadKey();
	}	
}

void TestTransmitter()
{
	//for(uint8_t i = 0; i < 10; i++)
		Hardware::Instance()->GetRfControl().RunCommand(0);
}

