#include "../header/includes.h"
#include <crtdbg.h>
#include <conio.h>
#include "../header/skStr.h"
#include <string>
#include <stdlib.h> 
#include <cstdlib>
#include <shellapi.h>

using namespace std;

int main(HINSTANCE instance, HINSTANCE previousInstance, PWSTR arguments, int commandShow)
{

	HWND consoleWindow = GetConsoleWindow();
	//MoveWindow(consoleWindow, 600, 350, 400, 300, TRUE);
	SetWindowPos(consoleWindow, 0, 500, 250, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	SetConsoleTitleA(skCrypt("AhPlace2"));
	system("taskkill /IM x64dbg.exe /F");
	system("taskkill /IM x32dbg.exe /F");
	system("taskkill /IM MSIAfterburner.exe /F");
	system("cls");

	Beep(500, 500);

	//Memory mem{ "csgo.exe" };

	auto mem = Memory{ "csgo.exe" };

	if (!mem.GetProcessId())
	{
		int play_sound = 0;
		system("color 5");
		std::cout << "\n";
		std::cout << "\n";
		std::cout << "\n";
		std::cout << "\n";
		std::cout << "\n";
		std::cout << "\n";
		std::cout << "\n";
		std::cout << "\n";
		std::cout << "\n                                                  Close this window\n";
		std::cout << "\n                                                   and open CS:GO\n";
		std::cout << "\n";
		std::cin.ignore(9999);
	}  

	if (mem.GetProcessId())
	{
		system("cls");
		system("color 3");
		std::cout << "\n                                                 AhPlace2 Loading...\n";
		Sleep(1500);
		system("cls");
		ShowWindow(consoleWindow, SW_HIDE);

		globals::clientAddress = mem.GetModuleAddress("client.dll");
		globals::engineAddress = mem.GetModuleAddress("engine.dll");
		
		std::thread(hacks::MemoryThread, mem).detach();
		std::thread(hacks::HackThread, mem).detach();
		std::thread(hacks::VisualsThread, mem).detach();
		
		// create gui
		gui::CreateHWindow("AhPlace22");
		gui::CreateDevice();
		gui::CreateImGui();

		while (gui::isRunning)
		{
			gui::BeginRender();
			gui::Render();
			gui::EndRender();

			mem = Memory{ "csgo.exe" };
			if (!mem.GetProcessId())
				break;

			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		// destroy gui
		gui::DestroyImGui();
		gui::DestroyDevice();
		gui::DestroyHWindow();
	}
}