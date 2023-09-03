#pragma once
#include <cstdint>
#include <cstddef>
#include "vector.h"

namespace memory
{
	inline DWORD EnginePointer;
	inline int GameState;
	inline std::uintptr_t localPlayer;
	inline std::int32_t localPlayerId;
	inline std::uintptr_t clientState;
	inline std::uintptr_t localTeam;
}

namespace globals
{
	inline std::uintptr_t processID = 0;
	inline std::uintptr_t clientAddress = 0;
	inline std::uintptr_t engineAddress = 0;

	inline int horizontal;
	inline int vertical;

	inline bool glow = false;
	inline float glowColor[] = { 1.f, 0.f, 0.f, 1.f };
	inline float glowColorVis[] = { 0.f, 1.f, 0.f, 1.f };
	inline float c4_glowcol[] = { 0.f, 252.f, 255.f, 255.f };
	inline bool c4_glow = false;
	inline bool chams = false;
	inline bool Esp = false;
	inline bool DrawBox = false;

	inline bool FOV = false;
	inline int FOV_int = 90;

	inline bool aim = false;
	inline float aim_smooth = 7.f;
	inline int aim_fov = 5;
	inline int btn = 0;
	inline int aim_key = 2;
	inline bool team_check = true;

	inline bool silentus = false;
	inline bool noAim_key = false;
	inline bool spotted_check = true;

	inline bool trigger = false;
	inline bool trigger_key = false;
	inline int triggerDelay = 0;
	inline bool trigger_when_flashed = false;
	inline bool trigger_only_scope = true;

	inline bool rcs = false;
	inline bool noFlash = false;
	inline bool bhop = false;
	inline int selected = 0;
	inline int bone = 8;
	inline float brightness = 25.f;
	inline bool skins = false;
	inline bool radar = false;
	inline bool no_move = false;
	inline bool cross = false;
	inline bool drawcross = false;

	inline bool showing = true;
	inline bool loading = false;
	
	inline Vector3 top;
	inline Vector3 bottom;
	inline float h;
	inline float w;
}
