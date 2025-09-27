#pragma once
#include "../include.hpp"
#include <Windows.h>
#include <vector>
#include <string>

namespace GUI
{
	inline bool MenuOpen = true;

	void RenderMenu();
	DWORD WINAPI MainThread(LPVOID);
}
