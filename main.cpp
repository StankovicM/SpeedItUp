#pragma once
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <thread>
#include <windows.h>

using namespace std;

static const char* WIN_NAME_GTA = "GTA: San Andreas";
static const LPVOID ADDRESS = (LPVOID)0xB7CB64;

static HWND		hwnd;
static DWORD	pid;
static HANDLE	pHandle;
static DWORD    exitCode;

static int init = 0;

static float speed = 1.0;

static int running = 0;

static bool thread_running = true;

void get_input()
{
	float input = 0;
	while (thread_running) {
		printf("Current speed multiplier is %.1f. Type in the new value to change the multiplier.\nMultiplier: ", speed);
		cin.clear();
		cin >> input;
		if (!cin.fail()) {
			speed = input;
		}
		else {
			printf("The multiplier must be a number. Please, try again.");
			while (getchar() != '\n');
		}
		this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

int main()
{
	printf("Initializing...\n");
	while (init == 0) {
		hwnd = FindWindow(NULL, WIN_NAME_GTA);
		if (hwnd > 0) {
			init += 1;
		}
		else {
			printf("Could not initalize the program, make sure GTA: San Andreas is running.\n");
			printf("Press ENTER to retry...\n");
			cin.get();
		}
	}

	GetWindowThreadProcessId(hwnd, &pid);
	if (pid <= 0) {
		printf("Couldn't find the GTA: San Andreas process id.\n");
		return 0;
	}

	pHandle = OpenProcess(0x38, FALSE, pid);
	if (pHandle <= 0) {
		printf("Couldn't access GTA: San Andreas process memory.\n");
		return 0;
	}

	cout << "Window: " << hwnd << endl;
	cout << "Process ID: " << pid << endl;
	cout << "Process handle: " << pHandle << endl;
	printf("Initialization complete.\n");

	printf("Type in the speed multiplier. Default is 1.0.\nMultiplier: ");
	float input = 1.0;
	while (init == 1) {
		cin.clear();
		cin >> input;
		if (!cin.fail()) {
            GetExitCodeProcess(pHandle, &exitCode);
            if (exitCode != 259) {
                printf("\nGTA: San Andreas has been closed.\n");
                CloseHandle(pHandle);
                return 0;
            }

			init += 1;
			speed = input;
			break;
		}
		else {
			printf("The multiplier must be a number. Please, try again.\nMultiplier: ");
			while (getchar() != '\n');
		}
	}
	printf("Speed multiplier set to %.1f.\n", speed);
	printf("Starting the main loop.\n");
	running = 1;

	float val = 0;
	thread in_thread(get_input);
	while (running) {
        GetExitCodeProcess(pHandle, &exitCode);
        if (exitCode != 259) {
            printf("\nGTA: San Andreas has been closed.\n");
            break;
        }

		if (GetLastError()) {
            printf("\nAn unexpected error has occured.\n");
			break;
		}

		ReadProcessMemory(pHandle, ADDRESS, &val, sizeof(val), 0);
		if (val != speed)
			WriteProcessMemory(pHandle, ADDRESS, &speed, sizeof(speed), 0);

		Sleep(500);
	}

	thread_running = false;
	in_thread.join();
    CloseHandle(pHandle);

	return 0;
}
