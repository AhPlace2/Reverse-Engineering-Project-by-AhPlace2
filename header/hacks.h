#pragma once
#include "memory.h"

namespace hacks
{
	void HackThread(const Memory& mem) noexcept;
	void VisualsThread(const Memory& mem) noexcept;
	void MemoryThread(const Memory& mem) noexcept;
}