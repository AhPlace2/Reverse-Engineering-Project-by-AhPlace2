#include "../header/includes.h"
#include <stdlib.h> 
#include <cstdlib>
#include <tchar.h>
#include <dwmapi.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter); // set click points
	}return 0;
		
	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;
	
	}

	return DefWindowProc(window, message, wideParameter, longParameter);
	/*
	int width;
	int height;

	HWND csgo = FindWindowA("Valve001", ("Counter-Strike: Global Offensive - Direct3D 9"));

	if (csgo)
	{
		RECT rect;
		GetWindowRect(csgo, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
	else
	{
		return FALSE;
	}

	paint = Paint(window, csgo, width, height);

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		RECT rect;
		GetWindowRect(csgo, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;

		MoveWindow(window, rect.left, rect.top, width, height, true);
	}
	*/
}

// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int& horizontal, int& vertical)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;

}

int horizontal = 0;
int vertical = 0;

void gui::CreateHWindow(const char* windowName) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	GetDesktopResolution(horizontal, vertical);

        window = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT,
		"class001",
		"Cheetos",
		WS_POPUP,
		0,
		0,
	    horizontal,
		vertical,
		0,
		0,
		windowClass.hInstance,
		0
	);
		
	  SetLayeredWindowAttributes(window, RGB(0, 0, 0), 0, ULW_COLORKEY);

	  {
		  RECT client_area{};
		  GetClientRect(window, &client_area);

		  RECT window_area{};
		  GetWindowRect(window, &window_area);

		  POINT diff{};
		  ClientToScreen(window, &diff);

		  const MARGINS margins{
			  window_area.left + (diff.x - window_area.left),
			  window_area.top + (diff.y - window_area.top),
			  client_area.right,
			  client_area.bottom
		  };

		  DwmExtendFrameIntoClientArea(window, &margins);
	  }

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	//presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	//presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	presentParameters.hDeviceWindow = window;
	presentParameters.BackBufferFormat = D3DFMT_X8R8G8B8;
	presentParameters.BackBufferWidth = horizontal;
	presentParameters.BackBufferHeight = vertical;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ::ImGui::GetIO();
	io.IniFilename = NULL;
	
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	{
		void Theme(); {

			ImGui::GetStyle().FrameRounding = 4.0f;
			ImGui::GetStyle().GrabRounding = 3.0f;
			ImGui::GetStyle().ChildRounding = 4.0f;

			ImVec4* colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImColor(2, 2, 2, 50);
			colors[ImGuiCol_ChildBg] = ImColor(32, 32, 32, 100);
			colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
			colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
			colors[ImGuiCol_TitleBg] = ImColor(146, 52, 235, 255);
			colors[ImGuiCol_TitleBgActive] = ImColor(146, 52, 235, 255);
			colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 0, 0, 130);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);

			colors[ImGuiCol_ScrollbarBg] = ImColor(31, 30, 31, 255);
			colors[ImGuiCol_ScrollbarGrab] = ImColor(41, 40, 41, 255);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(31, 30, 31, 255);
			colors[ImGuiCol_ScrollbarGrabActive] = ImColor(41, 40, 41, 255);
			colors[ImGuiCol_CheckMark] = ImColor(1, 1, 1, 255);
			colors[ImGuiCol_SliderGrab] = ImColor(149, 36, 220, 255);
			colors[ImGuiCol_SliderGrabActive] = ImColor(217, 39, 245, 255);

			colors[ImGuiCol_Button] = ImColor(79, 33, 122, 255);
			colors[ImGuiCol_ButtonHovered] = ImColor(53, 17, 138, 255);
			colors[ImGuiCol_ButtonActive] = ImColor(31, 30, 31, 255);

			colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

			colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);

			colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

			colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
			colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);

			colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		}
	}

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message;

	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

bool loginC = false;

static int ActiveTab = 1;

void gui::Render() noexcept
{
	/*
	//Login
	static char username[255] = "";
	static char password[255] = "";

	std::string usernameC = "Free";
	std::string passwordC = "Free";
	*/

	GetDesktopResolution(horizontal, vertical);
	int Hhalf = horizontal / 2;
	int Vhalf = vertical / 2;
	
	/*
	ImGui::SetNextWindowPos(ImVec2(Hhalf - 50, Vhalf - 60), ImGuiCond_Once);
	ImGui::SetNextWindowSize({ 150, 150 });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(ImColor(32, 32, 32, 255)));
	//ImGui::SetNextWindowBgAlpha(1.0f);

	if (!loginC)
	{
		ImGui::Begin(
			"Elite Cheats",
			&isRunning,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar
		);

		ImGui::TextColored({ 160, 40, 180, 255 }, "       Login");
		ImGui::Spacing();
		ImGui::InputTextWithHint("##user", "Username", username, sizeof(username));

		ImGui::InputTextWithHint("##pass", "Password", password, sizeof(password));

		ImGui::Spacing();
		if (ImGui::Button("Login", ImVec2(135, 26)))
		{
			if (username == usernameC && password == passwordC)
			{
				loginC = true;
				showing = true;
			}
			else
			{
				MessageBoxA(window, TEXT("Wrong Username or Password"), TEXT("Error"), MB_OK);
			}

		}
		ImGui::PopStyleColor(1);
		ImGui::End();
	}
	*/

	if (globals::showing)
	{
		long extendedStyle = GetWindowLong(window, GWL_EXSTYLE);
		extendedStyle &= ~(WS_EX_TRANSPARENT);
		SetWindowLong(window, GWL_EXSTYLE, extendedStyle);
	}
	else if (!globals::showing)
	{
		long extendedStyle = GetWindowLong(window, GWL_EXSTYLE);
		extendedStyle |= (WS_EX_TRANSPARENT);
		SetWindowLong(window, GWL_EXSTYLE, extendedStyle);
	}

	ImGui::SetNextWindowPos({ 100, 100 }, ImGuiCond_Once);
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::SetNextWindowBgAlpha(1.0f);
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(ImColor(1, 1, 1, 255)));
	if (!globals::loading)
	{
		if (GetAsyncKeyState(VK_INSERT) & 1)
			globals::showing = !globals::showing;
	}
	
	if (globals::showing/* && loginC*/)
	{
		ImGui::Begin(
			"CSGO Cheat by AhPlace#3760             Menu key is INSERT",
			&isRunning,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar
		);

		if (ImGui::Button("Aimbot##1", ImVec2(90, 25)))
		{
			ActiveTab = 1;
		}
		ImGui::SameLine();
		if (ImGui::Button("Triggerbot", ImVec2(90, 25)))
		{
			ActiveTab = 2;
		}
		ImGui::SameLine();
		if (ImGui::Button("Visuals", ImVec2(90, 25)))
		{
			ActiveTab = 3;
		}
		ImGui::SameLine();
		if (ImGui::Button("Misc", ImVec2(90, 25)))
		{
			ActiveTab = 4;
		}
		ImGui::SameLine();
		if (ImGui::Button("Skin Changer", ImVec2(90, 25)))
		{
			ActiveTab = 5;
		}

		const char strin = 0;

		ImGui::BeginColumns(&strin, 2, ImGuiOldColumnFlags_NoBorder);
		ImGui::SetColumnOffset(1, 105);
		ImGui::BeginChild("##Left", ImVec2(95, 235), true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
		ImGui::TextColored({ 255, 0, 0, 255 }, "Developer");
		ImGui::Text("AhPlace#3760");
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::TextColored({ 0, 170, 0, 255 }, "Configs");

		if (ImGui::Button("Rage", ImVec2(50, 25)))
		{
			globals::rcs = true;
			globals::aim = true;
			globals::silentus = true;
			globals::glow = true;
			globals::radar = true;
			globals::noFlash = true;
			globals::spotted_check = true;
			globals::aim_fov = 255;
			globals::aim_smooth = 3.f;
			globals::selected = 0;
			globals::bone = 8;
			globals::team_check = true;
			globals::triggerDelay = 0;
			globals::trigger = false;
			globals::Esp = true;
			globals::cross = true;
		}

		if (ImGui::Button("Legit", ImVec2(50, 25)))
		{
			globals::glow = false;
			globals::noFlash = false;
			globals::aim = true;
			globals::radar = true;
			globals::aim_smooth = 20.f;
			globals::rcs = true;
			globals::selected = 0;
			globals::bone = 8;
			globals::aim_fov = 5;
			globals::chams = true;
			globals::FOV = false;
			globals::noAim_key = true;
			globals::spotted_check = true;
			globals::silentus = true;
			globals::team_check = true;
			globals::trigger_only_scope = true;
			globals::Esp = false;
			globals::cross = true;
		}

		if (ImGui::Button("Clear", ImVec2(50, 25)))
		{
			globals::glow = false;
			globals::aim = false;
			globals::aim_smooth = 7.f;
			globals::radar = false;
			globals::noFlash = false;
			globals::rcs = false;
			globals::bhop = false;
			globals::selected = 0;
			globals::bone = 8;
			globals::chams = false;
			globals::aim_fov = 5;
			globals::btn = 0;
			globals::aim_key = 2;
			globals::silentus = false;
			globals::triggerDelay = 0;
			globals::trigger = false;
			globals::skins = false;
			globals::noAim_key = false;
			globals::spotted_check = true;
			globals::FOV = false;
			globals::team_check = true;
			globals::trigger_only_scope = true;
			globals::c4_glow = false;
			globals::Esp = false;
			globals::cross = false;
		}
		ImGui::Spacing();
		ImGui::Text("Team");
		ImGui::Text("Check");
		ImGui::SameLine();
		ImGui::Checkbox("##checkes", &globals::team_check);

		//ImGui::Dummy(ImVec2(0.0f, 180.0f));
		ImGui::EndChild();
		ImGui::NextColumn();
		ImGui::BeginChild("##Down", ImVec2(377, 235), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);

		if (ActiveTab == 1)
		{
			const char* items[] = { "Head", "Neck", "Chest" };

			const char* aim_btn[] = { "Right Click", "Left Click", "Ctrl" };


			ImGui::TextColored({ 160, 0, 255, 255 }, "                    -=Aimbot=-");
			ImGui::Spacing();
			ImGui::Checkbox("Aimbot##2", &globals::aim);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(22.0f, 0.0f));
			ImGui::SameLine();
			ImGui::Checkbox("No Recoil", &globals::rcs);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(22.0f, 0.0f));
			ImGui::SameLine();
			ImGui::Checkbox("Auto Aim", &globals::silentus);
			//ImGui::Checkbox("Check if Visible", &globals::spotted_check);
			ImGui::Combo("Aimbot Button", &globals::btn, aim_btn, IM_ARRAYSIZE(aim_btn), 0);
			ImGui::SliderFloat("smoothness", &globals::aim_smooth, 1.f, 100.f, "%.0f", ImGuiSliderFlags_NoInput);
			ImGui::Combo("Hitbox", &globals::selected, items, IM_ARRAYSIZE(items), 0);
			ImGui::SliderInt("Aimbot FOV", &globals::aim_fov, 0, 255);
			
			if (globals::silentus)
			{
				globals::noAim_key = true;
			}
			if (!globals::silentus)
			{
				globals::noAim_key = false;
			}

			switch (globals::selected)
			{
			case 0:
				globals::bone = 8;
				break;

			case 1:
				globals::bone = 7;
				break;

			case 2:
				globals::bone = 6;
				break;
			}

			switch (globals::btn)
			{
			case 0:
				globals::aim_key = 2;
				break;

			case 1:
				globals::aim_key = 1;
				break;

			case 2:
				globals::aim_key = 162;
				break;
			}

		}

		if (ActiveTab == 2)
		{
			ImGui::TextColored({ 160, 0, 255, 255 }, "                   -=Triggerbot=-");
			ImGui::Spacing();
			ImGui::Checkbox("Trigger Bot", &globals::trigger);
			ImGui::SameLine();
			ImGui::Checkbox("Scoped only", &globals::trigger_only_scope);
			ImGui::Checkbox("Shoot when Flashed", &globals::trigger_when_flashed);
			ImGui::Spacing();
			ImGui::Text("Trigger Bot Delay:");
			ImGui::SliderInt("", &globals::triggerDelay, 0, 240, "%i ms", ImGuiSliderFlags_NoInput);

		}

		if (ActiveTab == 3)
		{
			ImGui::TextColored({ 160, 0, 255, 255 }, "                    -=Visuals=-");
			ImGui::Spacing();
			ImGui::Checkbox("Glow ESP", &globals::glow);
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(22.0f, 0.0f));
			ImGui::SameLine();
			ImGui::Checkbox("Chams", &globals::chams);
			//ImGui::SameLine();
			//ImGui::Dummy(ImVec2(22.0f, 0.0f));
			//ImGui::SameLine();
			//ImGui::Checkbox("Box Esp", &globals::Esp);
			ImGui::ColorEdit4("Glow Color Not Visible", globals::glowColor, ImGuiColorEditFlags_NoInputs);
			ImGui::ColorEdit4("Glow Color Visible", globals::glowColorVis, ImGuiColorEditFlags_NoInputs);
			ImGui::Checkbox("Defuser Glow", &globals::c4_glow);
			ImGui::ColorEdit4("Glow Color Defusing", globals::c4_glowcol, ImGuiColorEditFlags_NoInputs);
			ImGui::Spacing();
			ImGui::Checkbox("Enable FOV changer", &globals::FOV);
			if (globals::FOV)
			{
				ImGui::SliderInt("FOV", &globals::FOV_int, 10, 170);
			}

		}
		if (ActiveTab == 4)
		{
			ImGui::TextColored({ 160, 0, 255, 255 }, "                      -=Misc=-");
			ImGui::Checkbox("Bhop", &globals::bhop);
			ImGui::Checkbox("No Flash", &globals::noFlash);
			ImGui::Checkbox("Radar", &globals::radar);
			ImGui::Checkbox("Sniper Crosshair", &globals::cross);
		
		}

		if (ActiveTab == 5)
		{
			ImGui::TextColored({ 160, 0, 255, 255 }, "                   -=Skin Changer=-");
			ImGui::Spacing();
			ImGui::Checkbox("Skin Changer", &globals::skins);
			//ImGui::TextColored({ 255, 0, 0, 255 }, " Knives will change after Respawn");

		}

		ImGui::PopStyleColor(1);
		ImGui::End();
	}

	if (globals::drawcross)
	{
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(Hhalf - 10, Vhalf), ImVec2(Hhalf + 10, Vhalf), ImColor(1.f, 0.f, 0.f), 1.f);
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(Hhalf, Vhalf - 10), ImVec2(Hhalf, Vhalf + 10), ImColor(1.f, 0.f, 0.f), 1.f);
	}
	/*
	if (globals::DrawBox)
	{
		ImGui::GetBackgroundDrawList()->AddRect({ globals::top.x - globals::w, globals::top.y }, { globals::top.x + globals::w, globals::bottom.y }, ImColor(1.f, 1.f, 1.f));

	}
	*/
	ImGui::GetBackgroundDrawList()->AddRect(ImVec2(90, 45), ImVec2(190, 70), ImColor(0.000f, 1.000f, 0.000f));

	ImGui::GetBackgroundDrawList()->AddText(ImVec2(100, 50), ImColor(0.000f, 0.000f, 1.000f, 1.000f), "Safe Cheats");
	
}