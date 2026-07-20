#pragma once

#include "../../../output/offsets.hpp"
#include "../../../output/client_dll.hpp"
#include "../../../output/buttons.hpp"

// Pawn offsets (inheritance: C_BaseEntity -> ... -> C_BasePlayerPawn -> C_CSPlayerPawnBase -> C_CSPlayerPawn)
namespace pawn_offsets {
    // C_BaseEntity
    constexpr std::ptrdiff_t m_pGameSceneNode = 0x330;
    constexpr std::ptrdiff_t m_iHealth = 0x34C;
    constexpr std::ptrdiff_t m_lifeState = 0x354;
    constexpr std::ptrdiff_t m_iTeamNum = 0x3E7;
    constexpr std::ptrdiff_t m_fFlags = 0x3F4;

    // C_BasePlayerPawn
    constexpr std::ptrdiff_t m_pWeaponServices = 0x1208;
    constexpr std::ptrdiff_t m_pCameraServices = 0x1240;
    constexpr std::ptrdiff_t m_vOldOrigin = 0x13B8;
    constexpr std::ptrdiff_t m_hController = 0x13D0;

    // C_CSPlayerPawnBase
    constexpr std::ptrdiff_t m_flFlashMaxAlpha = 0x1424;
    constexpr std::ptrdiff_t m_flFlashDuration = 0x1428;

    // C_CSPlayerPawn
    constexpr std::ptrdiff_t m_iShotsFired = 0x1C84;
    constexpr std::ptrdiff_t m_bIsScoped = 0x1C70;
    constexpr std::ptrdiff_t m_entitySpottedState = 0x1C58;
    constexpr std::ptrdiff_t m_angEyeAngles = 0x3340;
    constexpr std::ptrdiff_t m_iIDEntIndex = 0x341C;
}

namespace controller_offsets {
    constexpr std::ptrdiff_t m_hPlayerPawn = 0x914;
    constexpr std::ptrdiff_t m_sSanitizedPlayerName = 0x868;
}

namespace weapon_offsets {
    // CPlayer_WeaponServices
    constexpr std::ptrdiff_t m_hActiveWeapon = 0x60;

    // C_EconEntity
    constexpr std::ptrdiff_t m_AttributeManager = 0x11A8;

    // C_AttributeContainer
    constexpr std::ptrdiff_t m_Item = 0x50;

    // C_EconItemView
    constexpr std::ptrdiff_t m_iItemDefinitionIndex = 0x1BA;

    // C_CSWeaponBase
    constexpr std::ptrdiff_t m_iClip1 = 0x1700;
}

namespace scene_offsets {
    // CGameSceneNode
    constexpr std::ptrdiff_t m_vecAbsOrigin = 0xC8;

    // CSkeletonInstance (inherits CGameSceneNode)
    constexpr std::ptrdiff_t m_modelState = 0x140;
}
