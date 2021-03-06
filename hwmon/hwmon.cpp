#include <iostream>
#include "windows.h"
#include "rpiq.h"


int main()
{
	HANDLE hDevice;
	DWORD bytesReturned; bool status;

	try 
	{
		hDevice = CreateFileW(
			RPIQ_USERMODE_PATH, GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0
		);
		if (hDevice == INVALID_HANDLE_VALUE)
			throw std::runtime_error("Failed to open RPIQ device");

		//get temperature
		MAILBOX_GET_TEMPERATURE mailbox_temp;
		INIT_MAILBOX_GET_TEMPERATURE(&mailbox_temp);
		status = DeviceIoControl(hDevice, IOCTL_MAILBOX_PROPERTY,
			(LPVOID)(&mailbox_temp), sizeof(mailbox_temp),
			(LPVOID)(&mailbox_temp), sizeof(mailbox_temp), &bytesReturned, 0);
		if (!status)
			throw std::runtime_error("Failed to read temperature");
		wprintf(L"TEMPERATURE        %8.4g C\n", mailbox_temp.Value / 1000.0);

		//clocks description
		const wchar_t *clockDesc[11] = {
			nullptr,
			L"EMMC", L"UART", L"ARM", L"CORE", L"V3D",
			L"H264", L"ISP", L"SDRAM", L"PIXEL", L"PWM"
		};

		//get clocks
		wprintf(L"\nCLOCKS\n");
		MAILBOX_GET_CLOCK_RATE mailbox_freq; 
		for (ULONG i = 0x1; i <= 0xA; i++) {
			INIT_MAILBOX_GET_CLOCK_RATE(&mailbox_freq, i);
			status = DeviceIoControl(hDevice, IOCTL_MAILBOX_PROPERTY,
				(LPVOID)(&mailbox_freq), sizeof(mailbox_freq),
				(LPVOID)(&mailbox_freq), sizeof(mailbox_freq), &bytesReturned, 0);
			if (!status)
				throw std::runtime_error("Failed to read clocks");
			wprintf(L"\t%-7ws     %5d Mhz\n", clockDesc[i], mailbox_freq.Rate / 1000000);
		}

		//voltages description
		const wchar_t *voltDesc[5] = {
			nullptr,
			L"CORE", L"SDRAM C", L"SDRAM P", L"SDRAM I"
		};

		//get voltages
		wprintf(L"\nVOLTAGES\n");
		MAILBOX_GET_VOLTAGE mailbox_voltage;
		for (ULONG i = 0x1; i <= 0x4; i++) {
			INIT_MAILBOX_GET_VOLTAGE(&mailbox_voltage, i);
			status = DeviceIoControl(hDevice, IOCTL_MAILBOX_PROPERTY, 
				(LPVOID)(&mailbox_voltage), sizeof(mailbox_voltage), 
				(LPVOID)(&mailbox_voltage), sizeof(mailbox_voltage), &bytesReturned, 0);
			if (!status)
				throw std::runtime_error("Failed to read voltages");
			double voltage = ((double)mailbox_voltage.Value / 1000000) * 0.025 + 1.2;
			wprintf(L"\t%-7ws    %8.4lg V\n", voltDesc[i], voltage);
		}
		
		return 0;
	}
	catch (std::exception const &ex)
	{
		puts(ex.what());
		return 1;
	}
}

