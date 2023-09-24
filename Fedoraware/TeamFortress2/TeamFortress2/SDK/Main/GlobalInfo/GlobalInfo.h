#pragma once
#include "../BaseEntity/BaseEntity.h"

constexpr auto DT_WAIT_CALLS = 26;

struct VelFixRecord {
	Vec3 m_vecOrigin;
	int m_nFlags;
	float m_flSimulationTime;
};

struct PlayerCache {
	Vec3 m_vecOrigin;
	Vec3 m_vecVelocity;
	Vec3 eyePosition;
	int playersPredictedTick;
	bool didDamage = false;
};

struct DormantData {
	Vec3 Location;
	float LastUpdate = 0.f;
};

struct Priority {
	int Mode = 2; // 0 - Friend, 1 - Ignore, 2 - Default, 3 - Rage, 4 - Cheater
};

struct DrawBullet {
	std::pair<Vec3, Vec3> m_line;
	float m_flTime;
	Color_t m_color;
};

struct DrawLine {
	std::deque<std::pair<Vec3, Vec3>> m_line;
	float m_flTime;
	Color_t m_color;
};

struct DrawBox {
	Vec3 m_vecPos;
	//Vec3 m_vecSize;
	Vec3 m_vecMins;
	Vec3 m_vecMaxs;
	Vec3 m_vecOrientation;
	float m_flTime;
	Color_t m_colorEdge;
	Color_t m_colorFace;
};

namespace G
{
	inline int CurrentTargetIdx = 0; // Index of the current aimbot target
	inline int CurItemDefIndex = 0; // DefIndex of the current weapon
	inline int NotifyCounter = 0;
	inline int EyeAngDelay = 25;
	inline int NextSafeTick = 0;	//	I::GlobalVars->tickcount + sv_maxusrcmdprocessticks_holdaim + 1 (when attacking)
	inline float LerpTime = 0.f;	//	current lerp time
	inline bool WeaponCanHeadShot = false; // Can the current weapon headshot?
	inline bool WeaponCanAttack = false; // Can the current weapon attack?
	inline bool WeaponCanSecondaryAttack = false;
	inline bool AAActive = false; // Is the Anti-Aim active?
	inline bool HitscanSilentActive = false;
	inline bool AvoidingBackstab = false; // Are we currently avoiding a backstab? (Overwrites AA)
	inline bool AutoBackstabRunning = false;
	inline bool LocalSpectated = false; // Is the local player being spectated?
	inline bool ShouldStop = false; // Stops our players movement, takes 1 tick.
	inline bool UnloadWndProcHook = false;
	inline bool Frozen = false;	//	angles & movement are frozen.

	/* Double tap / Tick shift */
	inline int WaitForShift = 0;
	inline int ShiftedTicks = 0; // Amount of ticks that are shifted
	inline bool ShouldShift = false; // Should we shift now?
	inline bool Teleporting = false;
	inline bool Recharging = false; // Are we currently recharging?
	inline bool RechargeQueued = false; // Queues a recharge

	/* Choking / Packets */
	inline int ChokedTicks = 0; // Amount of ticks that are choked
	inline bool ForceSendPacket = false; // might not actually be useful 
	inline bool ForceChokePacket = false; // might not actually be useful 
	inline bool IsChoking = false; // might not actually be useful 

	/* Aimbot */
	inline bool IsAttacking = false; // this is only used by aimbot, and is also set to false at the start of a lot of functions, this is not reliable
	inline bool HitscanRunning = false;
	inline bool SilentTime = false;
	inline Vec3 AimPos = {};
	inline VMatrix WorldToProjection = {};

	/* Angles */
	inline Vec3 ViewAngles = {};
	inline Vec3 RealViewAngles = {}; // Real view angles (AA)
	inline Vec3 FakeViewAngles = {}; // Fake view angles (AA)
	inline std::pair<bool, bool> AntiAim = {};	//	fakeset, realset
	inline Vec3 PunchAngles = {};

	/* Bullets */
	inline std::vector<DrawBullet> BulletsStorage;

	/* Prediction */
	inline Vec3 PredictedPos = {};
	inline Vec3 LinearPredLine = {};
	inline std::deque<std::pair<Vec3, Vec3>> MoveLines;
	inline std::deque<std::pair<Vec3, Vec3>> ProjLines;
	inline std::vector<DrawLine> LinesStorage;

	/* Boxes */
	inline std::vector<DrawBox> BoxesStorage;

	inline CUserCmd* CurrentUserCmd{nullptr}; // Unreliable! Only use this if you really have to.
	inline CUserCmd* LastUserCmd{nullptr};
	
	inline EWeaponType CurWeaponType = {};
	inline std::unordered_map<CBaseEntity*, VelFixRecord> VelFixRecords;
	inline bool FreecamActive = false;
	inline Vec3 FreecamPos = {};
	inline std::unordered_map<int, DormantData> DormantPlayerESP; // <Index, DormantData>
	inline std::unordered_map<int, int> ChokeMap; // Choked packets of players <Index, Amount>
	inline bool DrawingStaticProps = false;
	inline std::unordered_map<uint32_t, Priority> PlayerPriority; // Playerlist priorities <FriendsID, Priority>
	inline bool BulletTracerFix = false;

	inline bool UpdateView = false;

	inline DWORD CalcIsAttackCriticalHelperOffset = 0;

	inline bool ShouldAutoQueue = false;

	inline int BackpackQuality = 1;

	inline std::vector<int> MedicCallers;

	inline bool ShouldUpdateMaterialCache = false;

	inline bool IsIgnored(uint32_t friendsID)
	{
		return PlayerPriority[friendsID].Mode < 2;
	}

	inline bool IsRage(uint32_t friendsID)
	{
		return PlayerPriority[friendsID].Mode > 2;
	}

	inline void SwitchIgnore(uint32_t playerID) //this code is so fucking shitty
	{
		bool ignore = true;
		bool unignore = false;

		if (PlayerPriority[playerID].Mode > 1)
		{
			ignore = true;
			unignore = false;
		}
		else if (PlayerPriority[playerID].Mode == 1)
		{
			ignore = false;
			unignore = true;
		}

		if (ignore == true)
		{
			PlayerPriority[playerID].Mode = 1;
		}
		else if (unignore == true)
		{
			PlayerPriority[playerID].Mode = 2;
		}
	}

	inline void SwitchMark(uint32_t playerID)
	{
		bool Mark = true;
		bool Unmark = false;

		if (PlayerPriority[playerID].Mode != 4)
		{
			Mark = true;
			Unmark = false;
		}
		else if (PlayerPriority[playerID].Mode == 4)
		{
			Mark = false;
			Unmark = true;
		}

		if (Mark == true)
		{
			PlayerPriority[playerID].Mode = 4;
			Mark = false;
			Unmark = true;
		}
		else if (Unmark == true)
		{
			PlayerPriority[playerID].Mode = 2;
			Mark = true;
			Unmark = false;
		}
	}
};