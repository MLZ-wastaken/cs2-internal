#include "renderer.hpp"
#include "../features/cheat.hpp"

static ID3D11Device*            g_pd3dDevice            = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext     = nullptr;
static IDXGISwapChain*          g_pSwapChain            = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView  = nullptr;
static HWND                     g_hWnd                  = nullptr;
static HMODULE                  g_hModule               = nullptr;

typedef HRESULT(__stdcall* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
WNDPROC oWndProc = nullptr;
Present oPresent = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void InitImGui(IDXGISwapChain* pSwapChain);
void CleanupImGui();

DWORD WINAPI Unload(LPVOID hModule);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
    {
        if (wParam == VK_INSERT)
            GUI::MenuOpen = !GUI::MenuOpen;
        else if (wParam == VK_DELETE)
            CreateThread(nullptr, 0, Unload, reinterpret_cast<LPVOID>(g_hModule), 0, nullptr);
    }
    break;
    }

    if (GUI::MenuOpen && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return TRUE;

    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    static bool imguiInitialized = false;
    if (!imguiInitialized)
    {
        InitImGui(pSwapChain);

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();

        ImFontConfig fontConfig;
        fontConfig.OversampleH = 3;
        fontConfig.OversampleV = 3;
        fontConfig.PixelSnapH = false;
        fontConfig.MergeMode = false;

        const char* mainFontPath = "C:\\Windows\\Fonts\\msyh.ttc";
        ImVector<ImWchar> glyphRanges;
        ImFontGlyphRangesBuilder builder;
        builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
        builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
        builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
        builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
        builder.AddRanges(io.Fonts->GetGlyphRangesKorean());
        builder.AddRanges(io.Fonts->GetGlyphRangesThai());
        builder.BuildRanges(&glyphRanges);

        ImFont* font = io.Fonts->AddFontFromFileTTF(mainFontPath, 18.0f, &fontConfig, glyphRanges.Data);

        io.Fonts->Build();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 5.0f;
        style.ChildRounding = 5.0f;
        style.FrameRounding = 5.0f;
        style.PopupRounding = 5.0f;
        style.ScrollbarRounding = 5.0f;
        style.GrabRounding = 0.0f;
        style.TabRounding = 5.0f;
        style.FrameBorderSize = 1.0f;

        style.AntiAliasedFill = true;
        style.AntiAliasedLines = true;
        style.AntiAliasedLinesUseTex = true;

        imguiInitialized = true;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    if (GUI::MenuOpen)
        GUI::RenderMenu();

    if (Cheat::RenderVisuals)
        Cheat::RenderPlayers();

    ImGui::Render();
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    Cheat::WindowWidth = static_cast<int>(displaySize.x);
    Cheat::WindowHeight = static_cast<int>(displaySize.y);

    return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI Unload(LPVOID hModule)
{
    Cheat::Running = false;

    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    CleanupImGui();
    FreeConsole();

    Sleep(1000);

    FreeLibraryAndExitThread((HMODULE)hModule, 0);
    return 0;

}

void InitImGui(IDXGISwapChain* pSwapChain)
{
    if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice)))
        return;

    g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);

    DXGI_SWAP_CHAIN_DESC desc = {};
    pSwapChain->GetDesc(&desc);
    g_hWnd = desc.OutputWindow;

    ID3D11Texture2D* pBackBuffer = nullptr;
    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();

    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ~ImGuiConfigFlags_NoMouse;

    io.MouseDrawCursor = false;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
}

void CleanupImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (g_mainRenderTargetView)
    {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
    SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
    GUI::MenuOpen = false;
}


bool HookPresent()
{
    if (MH_Initialize() != MH_OK)
        return false;

    uintptr_t* pVTable = *(uintptr_t**)g_pSwapChain;
    void* presentAddr = (void*)pVTable[8];

    if (MH_CreateHook(presentAddr, &hkPresent, reinterpret_cast<void**>(&oPresent)) != MH_OK)
        return false;

    if (MH_EnableHook(presentAddr) != MH_OK)
        return false;

    return true;
}


DWORD WINAPI GUI::MainThread(LPVOID hModule)
{
    g_hModule = (HMODULE)hModule;

        g_hWnd = FindWindow(nullptr, "Counter-Strike 2");
        if (!g_hWnd)
            return FALSE;

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = g_hWnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL featureLevel;
        ID3D11Device* dummyDevice = nullptr;
        ID3D11DeviceContext* dummyContext = nullptr;
        IDXGISwapChain* dummySwapChain = nullptr;

        if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            nullptr, 0, D3D11_SDK_VERSION, &sd, &dummySwapChain, &dummyDevice, &featureLevel, &dummyContext)))
            return FALSE;

        g_pSwapChain = dummySwapChain;

        if (!HookPresent())
            return FALSE;

    return TRUE;
}