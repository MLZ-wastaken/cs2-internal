#pragma once
#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <array>

namespace Scimitar
{
    struct Vector2
    {
        float x{}, y{};
    };

    struct Vector3
    {
        float x{}, y{}, z{};

        constexpr Vector3(float X = 0.f, float Y = 0.f, float Z = 0.f) noexcept
            : x(X), y(Y), z(Z) {
        }

        bool WorldToScreen(Vector2& out, float matrix[4][4]) const
        {
            float clipX = x * matrix[0][0] + y * matrix[0][1] + z * matrix[0][2] + matrix[0][3];
            float clipY = x * matrix[1][0] + y * matrix[1][1] + z * matrix[1][2] + matrix[1][3];
            float clipW = x * matrix[3][0] + y * matrix[3][1] + z * matrix[3][2] + matrix[3][3];

            if (clipW < 0.001f)
                return false;

            float ndcX = clipX / clipW;
            float ndcY = clipY / clipW;

            out.x = (Cheat::WindowWidth * 0.5f) * (1.0f + ndcX);
            out.y = (Cheat::WindowHeight * 0.5f) * (1.0f - ndcY);

            return true;
        }
    };

    struct BoneJointData {
        Vector3 Position;
        char pad[20];
    };

    inline uintptr_t ClientBase = NULL;
    constexpr std::ptrdiff_t dwEntityList = 0x1D042D8;
    constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x1BDFD10;
    constexpr std::ptrdiff_t dwViewMatrix = 0x1E21090;

    constexpr std::ptrdiff_t m_hPlayerPawn = 0x8FC;
    constexpr std::ptrdiff_t m_pGameSceneNode = 0x330;
    constexpr std::ptrdiff_t m_modelState = 0x190;
    constexpr std::ptrdiff_t m_skeletonInstance = 0x80;
    constexpr std::ptrdiff_t m_iTeamNum = 0x3EB;
    constexpr std::ptrdiff_t m_iHealth = 0x34C;

    constexpr std::ptrdiff_t m_pBulletServices = 0x1690;
    constexpr std::ptrdiff_t m_totalHitsOnServer = 0x40;

    inline bool GetClientBase()
    {
        if (!ClientBase)
        {
            ClientBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("client.dll"));
            if (!ClientBase)
                return false;
        }
        return true;
    }

    inline float (*GetViewMatrix())[4]
    {
        return reinterpret_cast<float(*)[4]>(ClientBase + dwViewMatrix);
    }

    inline uintptr_t GetEntityList()
    {
        return *reinterpret_cast<uintptr_t*>(ClientBase + dwEntityList);
    }

    inline uintptr_t GetLocalPlayer()
    {
        return *reinterpret_cast<uintptr_t*>(ClientBase + dwLocalPlayerPawn);
    }

    inline int GetHealth(uintptr_t entity)
    {
        return *reinterpret_cast<int*>(entity + m_iHealth);
    }

    inline int GetTeam(uintptr_t entity)
    {
        return *reinterpret_cast<int*>(entity + m_iTeamNum);
    }

    inline uintptr_t GetBulletServices(uintptr_t entity)
    {
        __try
        {
            return *reinterpret_cast<uintptr_t*>(entity + m_pBulletServices);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return NULL;
        }
    }

    inline int GetTotalHits(uintptr_t m_pBulletServices)
    {
        __try
        {
            return *reinterpret_cast<int*>(m_pBulletServices + m_totalHitsOnServer);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return NULL;
        }
    }

    inline bool TryGetEntityController(int index, uintptr_t entityList, uintptr_t& outEntity, uintptr_t& outController)
    {
        __try {
            const uintptr_t controllerEntry = *reinterpret_cast<uintptr_t*>(
                entityList + 0x8 * ((index & 0x7FFF) >> 9) + 0x10);

            if (!controllerEntry)
                return false;

            uintptr_t controllerAddr = controllerEntry + 0x78 * (index & 0x1FF);
            if (!controllerAddr)
                return false;

            uintptr_t entityController = *reinterpret_cast<uintptr_t*>(controllerAddr);
            if (!entityController)
                return false;

            const uint32_t pawnHandle = *reinterpret_cast<uint32_t*>(entityController + m_hPlayerPawn);
            if (!pawnHandle)
                return false;

            const uintptr_t pawnEntry = *reinterpret_cast<uintptr_t*>(
                entityList + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
            if (!pawnEntry)
                return false;

            const uintptr_t entityPawn = *reinterpret_cast<uintptr_t*>(
                pawnEntry + 0x78 * (pawnHandle & 0x1FF));
            if (!entityPawn)
                return false;

            outEntity = entityPawn;
            outController = entityController;
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    inline std::array<BoneJointData, 125> GetBoneJoints(uintptr_t entity)
    {
        std::array<BoneJointData, 125> boneData{};

        uintptr_t sceneNode = *reinterpret_cast<uintptr_t*>(entity + m_pGameSceneNode);
        if (!sceneNode)
            return boneData;

        uintptr_t modelState = *reinterpret_cast<uintptr_t*>(sceneNode + m_modelState + m_skeletonInstance);
        if (!modelState)
            return boneData;

        memcpy(boneData.data(), reinterpret_cast<void*>(modelState),
            boneData.size() * sizeof(BoneJointData));

        return boneData;
    }
}