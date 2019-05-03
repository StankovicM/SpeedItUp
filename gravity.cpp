#pragma once
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <thread>
#include <windows.h>

using namespace std;

static const char* WIN_NAME_GTA = "GTA: San Andreas";
static const LPVOID ADDRESS = (LPVOID)0x863984 ;

static HWND		hwnd;
static DWORD	pid;
static HANDLE	pHandle;

static int init = 0;

static float gravity = 0.08;

static int running = 0;

static bool thread_running = true;

void get_input()
{
	float input = 0;
	while (thread_running) {
		printf("Current speed multiplier is %.1f. Type in the new value to change the multiplier.\nMultiplier: ", gravity);
		cin.clear();
		cin >> input;
		if (!cin.fail()) {
			gravity = input;
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
		printf("Press ENTER to exit...\n");
		cin.get();
		return -1;
	}

	pHandle = OpenProcess(0x38, FALSE, pid);
	if (pHandle <= 0) {
		printf("Couldn't access GTA: San Andreas process memory.\n");
		printf("Press ENTER to exit...\n");
		cin.get();
		return -2;
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
			init += 1;
			gravity = input;
			break;
		}
		else {
			printf("The multiplier must be a number. Please, try again.\nMultiplier: ");
			while (getchar() != '\n');
		}
	}
	printf("Speed multiplier set to %.1f.\n", gravity);
	printf("Starting the main loop.\n");
	running = 1;

	float val = 0;
	ReadProcessMemory(pHandle, ADDRESS, &val, sizeof(val), 0);
	cout << "Gravity: " << val << endl;
	thread in_thread(get_input);
	while (running) {
		if (GetLastError())
			break;

		ReadProcessMemory(pHandle, ADDRESS, &val, sizeof(val), 0);

		if (val != gravity)
			WriteProcessMemory(pHandle, ADDRESS, &gravity, sizeof(gravity), 0);

		Sleep(500);
	}

	printf("\nGTA: San Andreas has been closed or an unexpected error has occured.\nPress ENTER to exit...\n");
	cin.get();
	thread_running = false;

	exit(0);
}
