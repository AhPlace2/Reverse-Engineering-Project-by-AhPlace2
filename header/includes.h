#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <cstdint>
#include <string_view>
#include <cstddef>
#include <d3d9.h>
#include "memory.h"
#include <numbers>
#include <cmath>
#include <vector>
#include <string>
#include <string.h>
#include <fstream>
#include <winbase.h>
#include <tchar.h>
#include <WinInet.h>
#include <thread>
#include "globals.h"
#include "gui.h"
#include "hacks.h"
#include "offsets.h"
#include "vector.h"

#pragma comment(lib,"Wininet.lib")
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib, "winmm.lib")

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include "../Imgui/imgui_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>