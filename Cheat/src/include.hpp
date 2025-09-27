#pragma once

#include "../Libs/ImGui/imgui.h"
#include "../Libs/ImGui/imgui_internal.h"
#include "../Libs/ImGui/imgui_impl_win32.h"
#include "../Libs/ImGui/imgui_impl_dx11.h"

#include "../Libs/MinHook/MinHook.h"

#include <d3d11.h>
#include <dxgi.h>

namespace Cheat
{
	inline int WindowWidth = 1920, WindowHeight = 1080;

	inline bool Running = true;

	inline bool RenderVisuals = false;
}