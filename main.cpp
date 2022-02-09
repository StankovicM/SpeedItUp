#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <thread>
#include <windows.h>

using namespace std;

static const char* SA_WIN_NAME = "GTA: San Andreas";
static const LPVOID SA_ADDRESS = (LPVOID)0xB7CB64;

static const char* VC_WIN_NAME = "GTA: Vice City";
static const LPVOID VC_ADDRESS = (LPVOID)0x97F264;

static char WIN_NAME[20];
static LPVOID ADDRESS;

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
    char buffer[3];
    float buffer_val = 1.0;
    char* endptr;
	while (thread_running) {
		printf("Current speed multiplier is %.1f. Type in the new value to change it.\nMultiplier: ", speed);
        fflush(stdin);
        fgets(buffer, 3, stdin);

        if (thread_running == 0) {
            break;
        }

        buffer_val = strtof(buffer, &endptr);
        if (buffer_val == 0 && buffer == endptr) {
            printf("The multiplier must be a number. Please, try again.\n");
        } else {
            speed = buffer_val;
        }
	}

	return;
}

int main()
{
    char buffer[3];
    while (init == 0) {
        printf("Select a game (sa, vc): ");
        fflush(stdin);
        fgets(buffer, 3, stdin);
        char c1 = buffer[0];
        char c2 = buffer[1];
        if ((c1 == 's' || c1 == 'S') && (c2 == 'a' || c2 == 'A')) {
            strcpy(WIN_NAME, SA_WIN_NAME);
            WIN_NAME[strlen(SA_WIN_NAME)] = '\0';
            ADDRESS = SA_ADDRESS;
            init += 1;
        }

        if ((c1 == 'v' || c1 == 'V') && (c2 == 'c' || c2 == 'C')) {
            strcpy(WIN_NAME, VC_WIN_NAME);
            WIN_NAME[strlen(VC_WIN_NAME)] = '\0';
            ADDRESS = VC_ADDRESS;
            init += 1;
        }

        if (init == 0) {
            printf("Error, could not recognize %s. Please try again.\n", buffer);
        }

        while (getchar() != '\n');
    }
    printf("Selected game: %s\n", WIN_NAME);

	printf("Initializing...\n");
	while (init == 1) {
		hwnd = FindWindow(NULL, WIN_NAME);
		if (hwnd > 0) {
			init += 1;
		}
		else {
			printf("Could not initalize the program, make sure %s is running.\n", WIN_NAME);
			printf("Press ENTER to retry...\n");
			cin.get();
		}
	}

	GetWindowThreadProcessId(hwnd, &pid);
	if (pid <= 0) {
		printf("Couldn't find the %s process id.\n", WIN_NAME);
		return 0;
	}

	pHandle = OpenProcess(0x38, FALSE, pid);
	if (pHandle <= 0) {
		printf("Couldn't access %s process memory.\n", WIN_NAME);
		return 0;
	}

	cout << "Window: " << hwnd << endl;
	cout << "Process ID: " << pid << endl;
	cout << "Process handle: " << pHandle << endl;
	printf("Initialization complete.\n");

	printf("Type in the speed multiplier. Default is 1.0.\nMultiplier: ");
	float buffer_val = 1.0;
	char* endptr;
	while (init == 2) {
		fflush(stdin);
		fgets(buffer, 3, stdin);

        GetExitCodeProcess(pHandle, &exitCode);
        if (exitCode != 259) {
            printf("\n%s has been closed.\n", WIN_NAME);
            CloseHandle(pHandle);
            return 0;
        }

        buffer_val = strtof(buffer, &endptr);
        if (buffer_val == 0 && buffer == endptr) {
            printf("The multiplier must be a number. Please, try again.\nMultiplier: ");
        } else {
            speed = buffer_val;
            init += 1;
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
            printf("\n%s has been closed.\n", WIN_NAME);
            running = false;
            break;
        }

		if (GetLastError()) {
            printf("\nAn unexpected error has occured.\n");
			running = false;
			break;
		}

		ReadProcessMemory(pHandle, ADDRESS, &val, sizeof(val), 0);
		if (val != speed)
			WriteProcessMemory(pHandle, ADDRESS, &speed, sizeof(speed), 0);

		Sleep(500);
	}

	thread_running = false;
	printf("Press ENTER to exit...\n");
	in_thread.join();
    CloseHandle(pHandle);

	return 0;
}
