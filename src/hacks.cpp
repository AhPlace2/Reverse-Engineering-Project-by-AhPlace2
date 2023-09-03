#include "../header/includes.h"
#include <array>
#include <cmath>
#include <numbers>

constexpr Vector3 CalculateAngle(
	const Vector3& localPosition,
	const Vector3& enemyPosition,
	const Vector3& viewAngles) noexcept
{
	return ((enemyPosition - localPosition).ToAngle() - viewAngles);
}

struct vector
{
	float x, y, z;
};

auto oldPunch = Vector2{};

struct Color
{
	BYTE r{ }, g{ }, b{ }, a{ };
};

struct ViewMatrix {
	ViewMatrix() noexcept
		: data() {}

	float* operator[](int index) noexcept {
		return data[index];
	}

	const float* operator[](int index) const noexcept {
		return data[index];
	}

	float data[4][4];
};

static bool world_to_screen(const Vector3& world, Vector3& screen, const ViewMatrix& vm) noexcept {
	float w = vm[3][0] * world.x + vm[3][1] * world.y + vm[3][2] * world.z + vm[3][3];

	if (w < 0.001f) {
		return false;
	}

	const float x = world.x * vm[0][0] + world.y * vm[0][1] + world.z * vm[0][2] + vm[0][3];
	const float y = world.x * vm[1][0] + world.y * vm[1][1] + world.z * vm[1][2] + vm[1][3];

	w = 1.f / w;
	float nx = x * w;
	float ny = y * w;

	const ImVec2 size = ImGui::GetIO().DisplaySize;

	screen.x = (size.x * 0.5f * nx) + (nx + size.x * 0.5f);
	screen.y = -(size.y * 0.5f * ny) + (ny + size.y * 0.5f);

	return true;
}

constexpr const int GetWeaponPaint(const short& itemDefinition)
{
	switch (itemDefinition)
	{
	case 1: return 711; //deagle
	case 4: return 38; // glock
	case 7: return 490; //ak
	case 9: return 344; // awp
	case 61: return 653; //usp
	case 8: return 73; //aug
	case 10: return 604; //famas
	case 13: return 428; // galil ar
	case 14: return 243; //m249
	case 16: return 588; //m4a1
	case 17: return 433; //mac10
	case 19: return 156; //p90
	case 24: return 688; //ump45
	case 26: return 676; //pp bizon
	case 27: return	703; //mag7
	case 28: return	285; //nagev
	case 29: return 638; //sawed off
	case 30: return	36; //tec 9
	case 32: return 591; //p2000
	case 33: return 481; // mp7
	case 34: return 609; //mp9
	case 35: return 699; //nova
	case 36: return 678; //p250
	case 38: return 165; //scar20
	case 39: return 686; //sg556
	case 40: return 222; //ssg08
	case 60: return 644; //m4a1 silenced
	case 63: return 643; //CZ75A
	case 64: return 37; //revolver
	default: return 0;
	}
}

void hacks::MemoryThread(const Memory& mem) noexcept
{
	while (gui::isRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		memory::EnginePointer = mem.Read<DWORD>(globals::engineAddress + offsets::dwClientState);
		memory::GameState = mem.Read<int>(memory::EnginePointer + offsets::dwClientState_State);
		memory::localPlayer = mem.Read<std::uintptr_t>(globals::clientAddress + offsets::dwLocalPlayer);
		memory::clientState = mem.Read<std::uintptr_t>(globals::engineAddress + offsets::dwClientState);
		memory::localPlayerId = mem.Read<std::int32_t>(memory::clientState + offsets::dwClientState_GetLocalPlayer);
		memory::localTeam = mem.Read<std::uintptr_t>(memory::localPlayer + offsets::m_iTeamNum);

	}
}

void hacks::VisualsThread(const Memory& mem) noexcept
{
	while (gui::isRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (memory::GameState != 6)
			continue;

		if (!memory::localPlayer)
			continue;

		constexpr const auto teamColor = Color{ 0, 0, 255, 255 };
		constexpr const auto enemyColor = Color{ 255, 0, 0, 255 };
		constexpr const auto defuseColor = Color{ 0, 255, 238, 255 };

		Color dColor = Color{ 255, 255, 255, 255 };

		const auto view_matrix = mem.Read<ViewMatrix>(globals::clientAddress + offsets::dwViewMatrix);
		const auto glowObjectManager = mem.Read<std::uintptr_t>(globals::clientAddress + offsets::dwGlowObjectManager);

		for (auto i = 1; i < 32; ++i)
		{
			const auto player = mem.Read<std::uintptr_t>(globals::clientAddress + offsets::dwEntityList + i * 0x10);
			
			if (!player)
				continue;

			const auto team = mem.Read<std::int32_t>(player + offsets::m_iTeamNum);

			const auto lifestate = mem.Read<std::int32_t>(player + offsets::m_lifeState);

			if (lifestate != 0)
				continue;
			
			const auto _this = static_cast<std::uintptr_t>(globals::engineAddress + offsets::model_ambient_min - 0x2c);
			const auto isDefusing = mem.Read<bool>(player + offsets::m_bIsDefusing);

			if (globals::chams)
			{
				globals::brightness = 7.f;

				if (isDefusing && globals::c4_glow)
				{
					mem.Write<Color>(player + offsets::m_clrRender, defuseColor);
				}
				else
				{
					if (team == memory::localTeam && globals::team_check)
					{
						mem.Write<Color>(player + offsets::m_clrRender, teamColor);
					}
					else
					{
						mem.Write<Color>(player + offsets::m_clrRender, enemyColor);
					}
				}

				mem.Write<std::int32_t>(globals::engineAddress + offsets::model_ambient_min, *reinterpret_cast<std::uintptr_t*>(&globals::brightness) ^ _this);
			}
			else
			{
				globals::brightness = 0.f;
				mem.Write<std::int32_t>(globals::engineAddress + offsets::model_ambient_min, *reinterpret_cast<std::uintptr_t*>(&globals::brightness) ^ _this);
				mem.Write<Color>(player + offsets::m_clrRender, dColor);
			}

			if (globals::glow)
			{
				const auto glowIndex = mem.Read<std::int32_t>(player + offsets::m_iGlowIndex);
				const auto spot = mem.Read<std::int32_t>(player + offsets::m_bSpottedByMask) & (1 << memory::localPlayerId);

				if (isDefusing && globals::c4_glow)
				{
					mem.Write(glowObjectManager + (glowIndex * 0x38) + 0x8, globals::c4_glowcol); //color is defusing

					mem.Write(glowObjectManager + (glowIndex * 0x38) + 0x28, true);
					mem.Write(glowObjectManager + (glowIndex * 0x38) + 0x29, false);
				}
				else
				{
					if (team == memory::localTeam && globals::team_check)
						goto skip_glow;

					if (spot)
					{
						mem.Write(glowObjectManager + (glowIndex * 0x38) + 0x8, globals::glowColorVis); //color visible

						mem.Write(glowObjectManager + (glowIndex * 0x38) + 0x28, true);
						mem.Write(glowObjectManager + (glowIndex * 0x38) + 0x29, false);
					}
					else
					{
						mem.Write(glowObjectManager + (glowIndex * 0x38) + 0x8, globals::glowColor); //color behind wall

						mem.Write(glowObjectManager + (glowIndex * 0x38) + 0x28, true);
						mem.Write(glowObjectManager + (glowIndex * 0x38) + 0x29, false);
					}
				}
			}

		skip_glow:
		
			if (team == memory::localTeam && globals::team_check)
				continue;

			if (globals::radar)
				mem.Write(player + offsets::m_bSpotted, true);
			/*
			if (globals::Esp)
			{
				const auto bones = mem.Read<DWORD>(player + offsets::m_dwBoneMatrix);

				if (!bones)
					continue;

				Vector3 head_pos{
				mem.Read<float>(bones + 0x30 * 8 + 0x0C),
				mem.Read<float>(bones + 0x30 * 8 + 0x1C),
				mem.Read<float>(bones + 0x30 * 8 + 0x2C)
				};

				auto feet_pos = mem.Read<Vector3>(player + offsets::m_vecOrigin);

				if (world_to_screen(head_pos + Vector3{ 0, 0, 11.f }, globals::top, view_matrix) && world_to_screen(feet_pos - Vector3{ 0, 0, 7.f }, globals::bottom, view_matrix))
				{
					globals::h = globals::bottom.y - globals::top.y;
					globals::w = globals::h * 0.25f;

					globals::DrawBox = true;
				}
				else
				{
					globals::DrawBox = false;
				}
			}
			else
			{
				globals::DrawBox = false;
			}*/
		}
	}
}

void hacks::HackThread(const Memory& mem) noexcept
{
	while (gui::isRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (memory::GameState == 1 || memory::GameState == 2)
		{
			globals::loading = true;
			globals::showing = false;
			Sleep(10000);
		}
		else
		{
			globals::loading = false;
		}

		if (memory::GameState != 6)
			continue;

		if (!memory::localPlayer)
			continue;

		const auto localHealth = mem.Read<std::int32_t>(memory::localPlayer + offsets::m_iHealth);
		
		const auto scoped = mem.Read<bool>(memory::localPlayer + offsets::m_bIsScoped);
		int weapon1 = mem.Read<int>(memory::localPlayer + offsets::m_hActiveWeapon);
		int weaponEntity = mem.Read<int>((globals::clientAddress + offsets::dwEntityList + (weapon1 & 0xFFF) * 0x10) - 0x10);
		int myWeaponID = mem.Read<int>(weaponEntity + offsets::m_iItemDefinitionIndex);
		const auto crosshairID = mem.Read<int32_t>(memory::localPlayer + offsets::m_iCrosshairId);
		
		int flashDur = 0;

		if (globals::noFlash)
		{
			flashDur = mem.Read<int>(memory::localPlayer + offsets::m_flFlashDuration);

			if (flashDur == 0)
				goto skip_noFlash;

			if (flashDur > 0)
				mem.Write<int>(memory::localPlayer + offsets::m_flFlashDuration, 0);
		}

	skip_noFlash:

		const auto flags = mem.Read<bool>(memory::localPlayer + offsets::m_fFlags);

		if (globals::bhop) {
			
			if (!GetAsyncKeyState(VK_SPACE))
				goto skip_bhop;

			if (GetAsyncKeyState(VK_SPACE) && flags & (1 << 0))
			{
				mem.Write<BYTE>(globals::clientAddress + offsets::dwForceJump, 6);
			}

			//std::cout << offsets::dwForceRight << "\n";

		}

	skip_bhop:

		if (globals::cross)
		{
			if (myWeaponID == 40 || myWeaponID == 9 || myWeaponID == 11 || myWeaponID == 38)
			{
				globals::drawcross = true;
			}
			else
			{
				globals::drawcross = false;
			}
		}
		else
		{
			globals::drawcross = false;
		}

		reniga:

		const auto shotsFired = mem.Read<std::int32_t>(memory::localPlayer + offsets::m_iShotsFired);

		if (globals::trigger) 
		{
			const auto entity = mem.Read<std::uintptr_t>(globals::clientAddress + offsets::dwEntityList + (crosshairID - 1) * 0x10);

			vector myLocation = mem.Read<vector>(memory::localPlayer + offsets::m_vecOrigin);
			vector enemyLocation = mem.Read<vector>(entity + offsets::m_vecOrigin);

			if (weaponEntity == 0)
				goto skip_trigger;

			float distance = sqrt(pow(myLocation.x - enemyLocation.x, 2) + pow(myLocation.y - enemyLocation.y, 2) + pow(myLocation.z - enemyLocation.z, 2)) * 0.0254;

			int weaponDelay = 0;

			switch (myWeaponID)
			{
			case 60: weaponDelay = 3;   break; //m4a1 silent
			case 7:  weaponDelay = 3.6; break; //ak
			case 40: weaponDelay = 0.4; break; //ssg08
			case 9:  weaponDelay = 0.4; break; //awp
			case 41: goto skip_trigger; break; 
			case 42: goto skip_trigger; break;
			case 59: goto skip_trigger; break;
			case 74: goto skip_trigger; break;
			case 43: goto skip_trigger; break;
			case 44: goto skip_trigger; break;
			case 45: goto skip_trigger; break;
			case 46: goto skip_trigger; break;
			case 47: goto skip_trigger; break;
			case 49: goto skip_trigger; break;
			case 57: goto skip_trigger; break;
			case 81: goto skip_trigger; break;
			case 82: goto skip_trigger; break;
			case 83: goto skip_trigger; break; 
			case 1:  weaponDelay = 16;  break; //deagle
			case 8:  weaponDelay = 2.5; break; //aug 2.5s ttk
			case 10: weaponDelay = 3;   break; //famas 3s ttk
			case 13: weaponDelay = 3;   break; //galil ar 3s ttk (5s/2S)
			case 14: weaponDelay = 2.5; break; //m249 2.5s ttk
			case 16: weaponDelay = 1.5; break; //m4a1 1.5s ttk
			case 17: weaponDelay = 3.5; break; //mac10 3.5s ttk
			case 19: weaponDelay = 2.5; break; //p90 2.5s ttk
			case 24: weaponDelay = 3;   break; //ump45 3s ttk (5s/2s)
			case 26: weaponDelay = 2.5; break; //pp bizon 2.5s ttk (1.5s/3s)
			case 28: weaponDelay = 3;   break; //nagev 3s tkk (8s/1.5s)
			case 33: weaponDelay = 2;   break; //mp7 2s ttk (1s/8s)
			case 34: weaponDelay = 2.5; break; //mp9 2.5/3s ttk (1.5s/5s)
			case 38: weaponDelay = 0.3; break; //scar20 0.4s 
			case 39: weaponDelay = 1.5; break; //sg556 1.5s ttk
			case 11: weaponDelay = 0.4; break; //G3SG1
			case 61: weaponDelay = 0.5; break; //usp
			default: weaponDelay = 0;
			}

			if (globals::trigger_only_scope)
			{
				if (myWeaponID == 40 && !scoped || myWeaponID == 9 && !scoped || myWeaponID == 11 && !scoped || myWeaponID == 38 && !scoped)
					goto skip_trigger;
			}
			
			if (!localHealth)
				goto skip_trigger;

			if (!crosshairID || crosshairID > 64)
				goto skip_trigger;

			if (memory::localTeam == mem.Read<std::uintptr_t>(entity + offsets::m_iTeamNum) && globals::team_check)
				goto skip_trigger;

			if (mem.Read<bool>(entity + offsets::m_bGunGameImmunity))
				goto skip_trigger;

			if (!globals::trigger_when_flashed && flashDur > 0)
				goto skip_trigger;

			Sleep(globals::triggerDelay);

			Sleep(distance * weaponDelay);

			mem.Write<std::uintptr_t>(globals::clientAddress + offsets::dwForceAttack, 6);
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			mem.Write<std::uintptr_t>(globals::clientAddress + offsets::dwForceAttack,4);

			if (shotsFired > 10)
			{
				Sleep(5);
			}

		};

		skip_trigger:

		if (shotsFired > 1 && globals::rcs)
		{
			const auto viewAngles = mem.Read<Vector2>(memory::clientState + offsets::dwClientState_ViewAngles);

			const auto aimPunch = mem.Read<Vector2>(memory::localPlayer + offsets::m_aimPunchAngle);

			auto newAngles = Vector2 
			{
				viewAngles.x + oldPunch.x - aimPunch.x * 2.f,
				viewAngles.y + oldPunch.y - aimPunch.y * 2.f,
			};

			if (newAngles.x > 89.f)
				newAngles.x = 89.f;

			if (newAngles.x < -89.f)
				newAngles.x = -89.f;

			while (newAngles.y > 180.f)
				newAngles.y -= 360.f;

			while (newAngles.y < -180.f)
				newAngles.y += 360.f;

			mem.Write<Vector2>(memory::clientState + offsets::dwClientState_ViewAngles, newAngles);

			oldPunch.x = aimPunch.x * 2.f;
			oldPunch.y = aimPunch.y * 2.f;
		}
		else {
			oldPunch.x = oldPunch.y = 0.f;
		}
		
	    if (GetAsyncKeyState(globals::aim_key) && globals::aim || globals::noAim_key)
		{
			const auto localEyePosition = mem.Read<Vector3>(memory::localPlayer + offsets::m_vecOrigin) + mem.Read<Vector3>(memory::localPlayer + offsets::m_vecViewOffset);
			const auto viewAngles = mem.Read<Vector3>(memory::clientState + offsets::dwClientState_ViewAngles);
			const auto aimPunch = mem.Read<Vector3>(memory::localPlayer + offsets::m_aimPunchAngle) * 2;
			
			auto bestFov = globals::aim_fov;
			auto bestAngle = Vector3();
			auto smooth = globals::aim_smooth;

			for (auto i = 1; i < 32; ++i)
			{
				const auto player = mem.Read<std::uintptr_t>(globals::clientAddress + offsets::dwEntityList + i * 0x10);
				const auto spoty = mem.Read<std::int32_t>(player + offsets::m_bSpottedByMask) & (1 << memory::localPlayerId);

				if (mem.Read<std::int32_t>(player + offsets::m_iTeamNum) == memory::localTeam && globals::team_check)
					continue;

				if (mem.Read<bool>(player + offsets::m_bDormant))
					continue;

				if (mem.Read<std::int32_t>(player + offsets::m_lifeState))
					continue;

				if (mem.Read<bool>(player + offsets::m_bGunGameImmunity))
					continue;

				switch (myWeaponID)
				{
				case 41: continue; break;
				case 42: continue; break;
				case 59: continue; break;
				case 74: continue; break;
				case 43: continue; break;
				case 44: continue; break;
				case 45: continue; break;
				case 46: continue; break;
				case 47: continue; break;
				case 49: continue; break;
				case 57: continue; break;
				case 81: continue; break;
				case 82: continue; break;
				case 83: continue; break;
				}
				
				if (!spoty && globals::spotted_check)
					continue;
				/*
				if (spoty && globals::spotted_check && globals::silentus)
				{
					mem.Write<BYTE>(globals::clientAddress + offsets::dwForceBackward + offsets::dwForceForward + offsets::dwForceRight + offsets::dwForceLeft, 1);
				}
				*/
				const auto boneMatrix = mem.Read<std::uintptr_t>(player + offsets::m_dwBoneMatrix);
				const auto cachedMemory = boneMatrix + 0x30 * globals::bone;

				if (!cachedMemory)
					continue;

				const auto BonePosition = Vector3{
					mem.Read<float>(cachedMemory + 0x0C),
					mem.Read<float>(cachedMemory + 0x1C),
					mem.Read<float>(cachedMemory + 0x2C)
				};

				const auto angle = CalculateAngle(
					localEyePosition,
					BonePosition,
					viewAngles + aimPunch
				);

				const auto fov = std::hypot(angle.x, angle.y);

				if (fov < bestFov)
				{

					if (bestAngle.x > 89.f)
						bestAngle.x = 89.f;

					if (bestAngle.x < -89.f)
						bestAngle.x = -89.f;

					while (bestAngle.y > 180.f)
						bestAngle.y -= 360.f;

					while (bestAngle.y < -180.f)
						bestAngle.y += 360.f;

					bestFov = fov;
					bestAngle = angle;
				}
				/*
				switch (player)
				{
				case 0: continue; break;
				case 1750912352: continue; break;
				case 1754540880: continue; break;
				case 1751074576: continue; break;
				case 1751147584: continue; break;
				case 1751329952: continue; break;
				case 1751402960: continue; break;
				case 1754696240: continue; break;
				case 1754769248: continue; break;
				case 1754842256: continue; break;
				case 1754915264: continue; break;
				case 1754988272: continue; break;
				case 1755061280: continue; break;
				}

				std::cout << player << "\n";
				*/
			}

			//std::cout << bestAngle.x << "x" << bestAngle.y << "y" << bestAngle.z << "z" << "\n";

			//Sleep(3);
			if (!bestAngle.IsZero())
			{
				mem.Write<Vector3>(memory::clientState + offsets::dwClientState_ViewAngles, viewAngles + bestAngle / smooth);
			}
				
		}

		skip_aim:
		
		const auto weapons = mem.Read<std::array<unsigned long, 8>>(memory::localPlayer + offsets::m_hMyWeapons);

		if (globals::skins)
		{
			for (const auto handle : weapons)
			{
				const auto weapon = mem.Read<std::uintptr_t>((globals::clientAddress + offsets::dwEntityList + (handle & 0xFFF) * 0x10) - 0x10);

				//make sure weapon is valid
				if (!weapon)
					continue;

				// see if we want skin to apply
				if (const auto paint = GetWeaponPaint(mem.Read<short>(weapon + offsets::m_iItemDefinitionIndex)))
				{
					const bool shouldUpdate = mem.Read<std::int32_t>(weapon + offsets::m_nFallbackPaintKit) != paint;

					//force to use fallback values
					mem.Write<std::int32_t>(weapon + offsets::m_iItemIDHigh, -1);

					mem.Write<std::int32_t>(weapon + offsets::m_nFallbackPaintKit, paint);
					mem.Write<float>(weapon + offsets::m_flFallbackWear, 0.1f);

					if (shouldUpdate)
						mem.Write<std::int32_t>(mem.Read<std::uintptr_t>(globals::engineAddress + offsets::dwClientState) + 0x174, -1);
				}
			}
		}

		if (globals::FOV && !scoped && localHealth != 0)
		{
			mem.Write<int>(memory::localPlayer + offsets::m_iFOV, globals::FOV_int);
		}

		if (!globals::FOV && !scoped || localHealth == 0)
		{
			globals::FOV_int = 90;
			mem.Write<int>(memory::localPlayer + offsets::m_iFOV, 90);
		}
	}
}
