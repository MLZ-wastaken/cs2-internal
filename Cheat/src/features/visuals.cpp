#include "cheat.hpp"
#include "bones.hpp"
#include <chrono>
#include <mmsystem.h>

namespace Cheat
{
	using namespace Scimitar;

    const unsigned char hitWav[]
    {
      
    };

    void SetWaveVolume(float percent)
    {
        DWORD vol = static_cast<DWORD>(0xFFFF * percent);
        DWORD volume = (vol & 0xFFFF) | (vol << 16);
        waveOutSetVolume(NULL, volume);
    }

    void RenderHitsounds(ImDrawList* drawList, uintptr_t LocalPlayer)
    {
        static int previousTotalHits = 0;
        static auto lastHitTime = std::chrono::steady_clock::time_point{};
        static bool showHitmarker = false;

        uintptr_t pBulletServices = GetBulletServices(LocalPlayer);
        if (!pBulletServices)
            return;

        int totalHits = GetTotalHits(pBulletServices);

        if (totalHits < 0 || totalHits > 255)
            return;

        if (totalHits != previousTotalHits)
        {
            if (!(totalHits == 0 && previousTotalHits != 0))
            {
                SetWaveVolume(0.75f);
                PlaySoundA(
                    reinterpret_cast<LPCSTR>(hitWav),
                    NULL,
                    SND_MEMORY | SND_ASYNC
                );

                lastHitTime = std::chrono::steady_clock::now();
                showHitmarker = true;
            }

            previousTotalHits = totalHits;
        }

        if (showHitmarker)
        {
            auto now = std::chrono::steady_clock::now();
            float elapsed = std::chrono::duration<float>(now - lastHitTime).count();

            if (elapsed < 1.0f)
            {
                float alpha = 1.0f - elapsed;

                ImU32 color = IM_COL32(255, 255, 255, static_cast<int>(alpha * 255.0f));

                float cx = Cheat::WindowWidth / 2.0f;
                float cy = Cheat::WindowHeight / 2.0f;
                float size = 7.0f;

                drawList->AddLine(ImVec2(cx - size, cy - size), ImVec2(cx - size / 2, cy - size / 2), color, 1.5f);
                drawList->AddLine(ImVec2(cx + size, cy - size), ImVec2(cx + size / 2, cy - size / 2), color, 1.5f);
                drawList->AddLine(ImVec2(cx - size, cy + size), ImVec2(cx - size / 2, cy + size / 2), color, 1.5f);
                drawList->AddLine(ImVec2(cx + size, cy + size), ImVec2(cx + size / 2, cy + size / 2), color, 1.5f);
            }
            else
            {
                showHitmarker = false;
            }
        }
    }

    void RenderPlayers()
    {
        if (!GetClientBase())
            return;

        auto ViewMatrix = GetViewMatrix();

        uintptr_t LocalPlayer = GetLocalPlayer();
        if (!LocalPlayer)
            return;

        uintptr_t EntityList = GetEntityList();
        if (!EntityList)
            return;

        int LocalTeam = GetTeam(LocalPlayer);

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        RenderHitsounds(drawList, LocalPlayer);
        for (int i = 0; i < 64; ++i)
        {
            uintptr_t ent{}, con{};
            if (!TryGetEntityController(i, EntityList, ent, con))
                continue;

            if (ent == LocalPlayer)
                continue;

            if (GetHealth(ent) <= 0)
                continue;

            int team = GetTeam(ent);
            auto boneData = GetBoneJoints(ent);
            if (boneData.empty())
                continue;

            for (auto [fromIdx, toIdx] : skeleton)
            {
                Vector3 p1 = boneData[fromIdx].Position;
                Vector3 p2 = boneData[toIdx].Position;

                Vector2 s1{}, s2{};
                if (p1.WorldToScreen(s1, ViewMatrix) &&
                    p2.WorldToScreen(s2, ViewMatrix))
                {
                    if ((fromIdx == neck_0 && toIdx == head_0) ||
                        (fromIdx == head_0 && toIdx == neck_0))
                    {
                        float dx = s1.x - s2.x;
                        float dy = s1.y - s2.y;
                        float radius = sqrtf(dx * dx + dy * dy);

                        Vector2 headScreen = (toIdx == head_0) ? s2 : s1;

                        drawList->AddCircle(
                            ImVec2(headScreen.x, headScreen.y),
                            radius,
                            team == LocalTeam ?
                            IM_COL32(0, 255, 0, 255) :
                            IM_COL32(255, 0, 0, 255),
                            32,
                            1.f
                        );
                    }
                    else
                    {
                        drawList->AddLine(
                            ImVec2(s1.x, s1.y),
                            ImVec2(s2.x, s2.y),
                            team == LocalTeam ?
                            IM_COL32(0, 255, 0, 255) :
                            IM_COL32(255, 0, 0, 255),
                            1.f
                        );
                    }
                }
            }
        }
    }

}
