#pragma once

#include "../SDK/SDK.h"

template <class T>
class CVar
{
public:
	T Value;
};

namespace Vars
{
	namespace Menu
	{
		inline std::string CheatName = "Fedoraware";
		inline std::string CheatPrefix = "[FeD]";

		inline bool ModernDesign = false;
		inline bool ShowPlayerlist = false;
		inline bool ShowKeybinds = false;
		inline CVar<int> MenuKey{ 0 };

//		namespace Colors
//		{
			inline Color_t MenuAccent = { 255, 101, 101, 255 };
//		} why have a namespace for one var?
	}

	namespace CritHack
	{
		inline CVar<bool> Active{ false };
		inline CVar<bool> Indicators{ false };
		inline DragBox_t IndicatorPos{};
		inline CVar<bool> AvoidRandom{ false };
		inline CVar<bool> AlwaysMelee{ false };
		inline CVar<int> CritKey{ VK_SHIFT };
	}

	namespace Backtrack
	{
		inline CVar<bool> Enabled{ false };
		inline CVar<int> Method{ 0 };
		inline CVar<bool> UnchokePrediction{ true };
		inline CVar<int> Latency{ 0 };
		inline CVar<int> Interp{ 0 };
		inline CVar<int> Protect{ 1 };
		inline CVar<int> PassthroughOffset{ 0 }; // debug

		namespace BtChams
		{
			inline CVar<bool> Enabled{ false };
			inline CVar<bool> LastOnly{ false };
			inline CVar<bool> EnemyOnly{ false };
			inline CVar<bool> IgnoreZ{ false };
			inline CVar<int> Material{ 1 };
			inline std::string Custom = "None";
			inline CVar<int> Overlay{ 1 };
			inline Color_t BacktrackColor{ 255, 255, 255, 255 };
		}
	}

	namespace Aimbot
	{
		namespace Global
		{
			inline CVar<bool> Active{ false };
			inline CVar<int> AimKey{ VK_XBUTTON1 };
			inline CVar<bool> AutoShoot{ false };
			inline CVar<int> AimAt{ 0b1000000 }; //players, sentry, dispenser, teleporter, stickies, npcs, bombs
			inline CVar<int> IgnoreOptions{ 0b0000000 }; //disguised, fakelagging players, vaccinator, taunting, friends, deadringer,cloaked, invul
			inline CVar<int> TickTolerance{ 7 };
			inline CVar<int> MaxTargets{ 2 };
			inline CVar<int> IgnoreCloakPercentage{ 100 }; // if player cloak percent > ignore threshold, ignore them 
			inline CVar<bool> BAimLethal{ false }; // This is in global cause i remmebered hunterman exists
			inline CVar<bool> ShowHitboxes{ false };
		}

		namespace Hitscan
		{
			inline CVar<bool> Active{ false };
			inline CVar<float> AimFOV{ 19.f };
			inline CVar<int> SortMethod{ 0 }; //0 - FOV,		1 - Distance
			inline CVar<int> SmoothingAmount{ 25 };
			inline CVar<int> AimMethod{ 2 }; //0 - Normal,	1 - Smooth, 2 - Silent
			inline CVar<int> Hitboxes{ 0b00111 }; // {legs, arms, body, pelvis, head}
			inline CVar<float> PointScale{ .54f };
			inline CVar<int> TapFire{ 0 }; //0 - Off, 1 - Distance, 2 - Always
			inline CVar<float> TapFireDist{ 1000.f };
			inline CVar<bool> ScanBuildings{ false };
			inline CVar<bool> WaitForHeadshot{ false };
			inline CVar<bool> WaitForCharge{ false };
			inline CVar<bool> ScopedOnly{ false };
			inline CVar<bool> AutoScope{ false };
			inline CVar<bool>ExtinguishTeam{ false };
		}

		namespace Projectile
		{
			inline CVar<bool> Active{ false };
			inline CVar<float> AimFOV{ 26.f };
			inline CVar<int> SortMethod{ 0 }; //0 - FOV,		1 - Distance
			inline CVar<int> AimMethod{ 2 }; //0 - Normal,	1 - Smooth, 2 - Silent
			inline CVar<int> SmoothingAmount{ 25 };
			inline CVar<float> ScanScale{ 0.78f }; // how to scale the points. 0.78 is a good number, huntsman bugs at anything below 0.76
			inline CVar<bool> SplashPrediction{ false };
			inline Color_t PredictionColor{ 255, 255, 255, 255 };
			inline Color_t ProjectileColor{ 255, 100, 100, 255 };
			inline CVar<float> PredictionTime{ 2.0f };
			inline CVar<bool> NoSpread{ false };
			inline CVar<bool> ChargeLooseCannon{ false };
			inline CVar<bool> StrafePredictionGround{ false };
			inline CVar<bool> StrafePredictionAir{ false };
			inline CVar<int> StrafePredictionMaxDistance{ 1000 };
			inline CVar<int> StrafePredictionMinDifference{ 10 };
			inline CVar<float> LatOff{ 0.f };
			inline CVar<float> PhyOff{ 0.f };
		}

		namespace Melee
		{
			inline CVar<bool> Active{ false };
			inline CVar<float> AimFOV{ 180.f };
			inline CVar<int> SortMethod{ 1 }; //0 - FOV, 1 - Distance
			inline CVar<int> AimMethod{ 2 }; //0 - Normal,	1 - Smooth, 2 - Silent
			inline CVar<int> SmoothingAmount{ 25 };
			inline CVar<bool> PredictSwing{ false };
			inline CVar<bool> AutoBackstab{ true };
			inline CVar<bool> IgnoreRazorback{ true };
			inline CVar<bool> WhipTeam{ false };
		}
	}

	namespace Triggerbot
	{
		namespace Global
		{
			inline CVar<bool> Active{ false };
			inline CVar<int> TriggerKey{ VK_XBUTTON2 };
			inline CVar<int> IgnoreOptions{ 0b00000 }; //disguised, fakelagging players, taunting, friends, cloaked, invul
		}

		namespace Shoot
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> TriggerPlayers{ false };
			inline CVar<bool> TriggerBuildings{ false };
			inline CVar<bool> HeadOnly{ false };
			inline CVar<bool> WaitForHeadshot{ false };
			inline CVar<bool> ScopeOnly{ false };
			inline CVar<float> HeadScale{ 0.7f };
		}

		namespace Detonate
		{
			inline CVar<bool> Active{ false };
			inline CVar<int> DetonateTargets{ 0b00001 };
			inline CVar<bool> Stickies{ false };
			inline CVar<bool> Flares{ false };
			inline CVar<float> RadiusScale{ 0.9f };
		}

		namespace Blast
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> Rage{ false };
			inline CVar<bool> Silent{ false };
			inline CVar<bool> ExtinguishPlayers{ false };
			inline CVar<bool> DisableOnAttack{ false };
			inline CVar<int> Fov{ 55 };
		}

		namespace Uber
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> OnlyFriends{ false };
			inline CVar<bool> PopLocal{ false };
			inline CVar<bool> AutoVacc{ false };
			inline CVar<float> HealthLeft{ 35.0f };
			inline CVar<bool> VoiceCommand{ false };
			inline CVar<int> ReactFoV{ 25 };
			inline CVar<bool> BulletRes{ true }; inline CVar<bool> BlastRes{ true }; inline CVar<bool> FireRes{ true };
			inline CVar<int> ReactClasses{ 0b000000000 };	//	this is intuitive
		}
	}

	namespace ESP
	{
		namespace Main
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> EnableTeamEnemyColors{ false };
			inline CVar<bool> DormantSoundESP{ false };
			inline CVar<float> DormantTime{ 1.f };
		}

		namespace Players
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> IgnoreLocal{ true };
			inline CVar<bool> IgnoreTeammates{ true };
			inline CVar<bool> IgnoreFriends{ false };
			inline CVar<bool> IgnoreCloaked{ false }; 
			inline CVar<bool> Name{ false };
			inline CVar<bool> NameCustom{ false };
			inline Color_t NameColor{ 255, 255, 255, 255 };
			inline CVar<int> Uber{ 0 }; //0 - Off, 1 - Text, 2 - Bar, 3 - Both
			inline CVar<int> Class{ 0 }; // 0 - Off, 1 - Icon, 2 - Text, 3 - Both
			inline CVar<bool> WeaponIcon{ false };
			inline CVar<bool> WeaponText{ false };
			inline CVar<int> HealthText{ 0 }; // 0 - Off, 1 - Default, 2 - Bar
			inline CVar<bool> Distance{ false };
			inline CVar<bool> HealthBar{ false };
			inline CVar<int> HealthBarStyle{ 0 }; // 0 - gradient, 1 - old
			inline CVar<int> Box{ 0 }; //0 - OFF, 1 - Simple, 2 - Corners
			inline CVar<int> Bones{ 0 };
			inline CVar<float> Alpha{ 1.0f };
			inline CVar<bool> PriorityText{ false };
			inline CVar<bool> SniperSightlines{ false };
			
			namespace Conditions
			{
				inline CVar<bool> Enabled{ false };
				inline CVar<bool> Buffs{ true };
				inline CVar<bool> Debuffs{ true };
				inline CVar<bool> Other{ false };
				inline CVar<bool> LagComp{ false };
				inline CVar<bool> Ping{ true };
				inline CVar<bool> KD{ false };
				inline CVar<bool> Dormant{ true };
			}
		}

		namespace Buildings
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> IgnoreTeammates{ false };
			inline CVar<bool> Name{ false };
			inline CVar<bool> NameCustom{ false };
			inline Color_t NameColor{ 255, 255, 255, 255 };
			inline CVar<bool> Health{ false };
			inline CVar<bool> Owner{ false };
			inline CVar<bool> Level{ false };
			inline CVar<bool> Distance{ false };
			inline CVar<bool> Cond{ false };
			inline CVar<bool> HealthBar{ false };
			inline CVar<int> Box{ 0 }; //0 - OFF, 1 - Simple, 2 - Corners
			inline CVar<float> Alpha{ 1.0f };
		}

		namespace World
		{
			inline CVar<bool> Active{ false };

			inline CVar<bool> HealthName{ false };
			inline CVar<int> HealthBox{ false };
			inline CVar<bool> HealthLine{ false };
			inline CVar<bool> HealthDistance{ false };

			inline CVar<bool> AmmoName{ false };
			inline CVar<int> AmmoBox{ false };
			inline CVar<bool> AmmoLine{ false };
			inline CVar<bool> AmmoDistance{ false };

			inline CVar<bool> NPCName{ false };
			inline CVar<int> NPCBox{ false };
			inline CVar<bool> NPCLine{ false };
			inline CVar<bool> NPCDistance{ false };

			inline CVar<bool> BombName{ false };
			inline CVar<int> BombBox{ false };
			inline CVar<bool> BombLine{ false };
			inline CVar<bool> BombDistance{ false };

			inline CVar<float> Alpha{ 1.0f };
		}
	}

	namespace Chams
	{
		namespace Main
		{
			inline CVar<bool> Active{ false };
		}

		namespace Players
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> Wearables{ false };
			inline CVar<bool> Weapons{ false };
			inline CVar<bool> EnemyOnly{ true };

			// {ignorez, material, overlay type (0 = off), active}
			inline Chams_t Local{};
			inline Chams_t FakeAng{};
			inline Chams_t Friend{};
			inline Chams_t Enemy{};
			inline Chams_t Team{};
			inline Chams_t Target{};
			inline Chams_t Ragdoll{};
		}

		namespace Buildings
		{
			inline Chams_t Local{};
			inline Chams_t Friend{};
			inline Chams_t Enemy{};
			inline Chams_t Team{};
			inline Chams_t Target{};

			inline CVar<bool> Active{ false };
			inline CVar<int> Material{ 3 }; //0 - None, 1 - Shaded, 2 - Shiny, 3 - Flat
			inline CVar<bool> IgnoreZ{ false };
			inline CVar<bool> EnemyOnly{ true };
		}

		namespace World
		{
			inline CVar<bool> Active{ false };
			inline CVar<int> Projectilez{ 2 };
			inline Chams_t Health{};
			inline Chams_t Ammo{};

			namespace Projectiles
			{
				inline Chams_t Enemy{};
				inline Chams_t Team{};
				inline Chams_t Projectiles{};
			}

		}

		namespace DME
		{
			inline CVar<bool> Active{ false };

			inline Chams_t Hands{ 1, 0, 0, true };
			inline Chams_t Weapon{ 1, 0, 0, true };

			inline CVar<int> HandsGlowOverlay{ 0 }; // 0 - Off,  1 - Fresnel Glow, 2 - Wireframe Glow
			inline CVar<int> HandsProxySkin{ 0 };
			inline CVar<float> HandsGlowAmount{ 1 };
			inline CVar<bool> HandsProxyWF{ false };
			inline CVar<bool> HandsRainbow{ false };
			inline CVar<bool> HandsOverlayRainbow{ false };
			inline CVar<bool> HandsOverlayPulse{ false };

			inline CVar<int> WeaponGlowOverlay{ 0 }; // 0 - Off,  1 - Fresnel Glow, 2 - Wireframe Glow
			inline CVar<int> WeaponsProxySkin{ 0 };
			inline CVar<float> WeaponGlowAmount{ 1 };
			inline CVar<bool> WeaponsProxyWF{ false };
			inline CVar<bool> WeaponRainbow{ false };
			inline CVar<bool> WeaponOverlayRainbow{ false };
			inline CVar<bool> WeaponOverlayPulse{ false };
		}
	}

	namespace Glow
	{
		namespace Main
		{
			inline CVar<bool> Active{ false };
			inline CVar<int> Type{ 0 };	//	blur, stencil
			inline CVar<int> Scale{ 1 };
		}

		namespace Players
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> ShowLocal{ false };
			inline CVar<bool> LocalRainbow{ false };
			inline CVar<int> IgnoreTeammates{ 2 }; //0 - OFF, 1 - All, 2 - Keep Friends
			inline CVar<bool> Wearables{ false };
			inline CVar<bool> Weapons{ false };
			inline CVar<float> Alpha{ 1.0f };
			inline CVar<int> Color{ 0 }; //0 - Team, 1 - Health
		}

		namespace Buildings
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> IgnoreTeammates{ false };
			inline CVar<float> Alpha{ 1.0f };
			inline CVar<int> Color{ 0 }; //0 - Team, 1 - Health
		}

		namespace World
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> Health{ false };
			inline CVar<bool> Ammo{ false };
			inline CVar<bool> NPCs{ false };
			inline CVar<bool> Bombs{ false };
			inline CVar<int> Projectiles{ 1 }; //0 - Off, 1 - All, 2 - Enemy Only
			inline CVar<float> Alpha{ 1.0f };
		}
	}

	namespace Radar
	{
		namespace Main
		{
			inline CVar<bool> Active{ false };
			inline CVar<int> BackAlpha{ 128 };
			inline CVar<int> LineAlpha{ 255 };
			inline CVar<int> Range{ 1500 };
			inline CVar<bool> NoTitleGradient{ false };
		}

		namespace Players
		{
			inline CVar<bool> Active{ false };
			inline CVar<int> IconType{ 1 }; //0 - Scoreboard, 1 - Portraits, 2 - Avatars
			inline CVar<int> BackGroundType{ 2 }; //0 - Off, 1 - Rect, 2 - Texture
			inline CVar<bool> Outline{ false };
			inline CVar<int> IgnoreTeam{ 2 }; //0 - Off, 1 - All, 2 - Keep Friends
			inline CVar<int> IgnoreCloaked{ 0 }; //0 - Off, 1 - All, 2 - Enemies Only
			inline CVar<bool> Health{ false };
			inline CVar<int> IconSize{ 24 };
			inline CVar<bool> Height{ false };
		}

		namespace Buildings
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> Outline{ false };
			inline CVar<bool> IgnoreTeam{ false };
			inline CVar<bool> Health{ false };
			inline CVar<int> IconSize{ 18 };
		}

		namespace World
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> Health{ false };
			inline CVar<bool> Ammo{ false };
			inline CVar<int> IconSize{ 14 };
		}
	}

	namespace Visuals
	{
		inline CVar<bool> ScoreboardColours{ false };
		inline CVar<bool> CleanScreenshots{ true };
		inline CVar<bool> RemoveDisguises{ false };
		inline CVar<bool> RemoveTaunts{ false };
		inline CVar<bool> DrawOnScreenConditions{ false };
		inline CVar<bool> DrawOnScreenPing{ false };
		inline DragBox_t OnScreenConditions{ };
		inline DragBox_t OnScreenPing{ };
		inline CVar<int> FieldOfView{ 90 };
		inline CVar<int> ZoomFieldOfView{ 30 };
		inline CVar<bool> RemoveScope{ false };
		inline CVar<bool> RemoveRagdolls{ false };
		inline CVar<bool> RemoveMOTD{ false };
		inline CVar<bool> RemoveScreenEffects{ false };
		inline CVar<bool> RemoveScreenOverlays{ false };
		inline CVar<bool> RemoveConvarQueries{ false };
		inline CVar<bool> RemoveDSP{ false };
		inline CVar<bool> PreventForcedAngles{ false };
		inline CVar<bool> PickupTimers{ false };
		inline CVar<bool> RemoveZoom{ false };
		inline CVar<bool> RemovePunch{ false };
		inline CVar<bool> CrosshairAimPos{ false };
		inline CVar<bool> ChatInfoText{ false };
		inline CVar<bool> ChatInfoChat{ false };
		inline CVar<int> FovArrowsDist{ 25 };
		inline CVar<bool> SpectatorList{ false };
		inline CVar<bool> SpectatorAvatars{ false };
		inline DragBox_t SpectatorPos{};

		inline CVar<bool> ThirdPerson{ false };
		inline CVar<int> ThirdPersonKey{ VK_B };
		inline CVar<bool> ThirdPersonSilentAngles{ false };
		inline CVar<bool> ThirdPersonInstantYaw{ false };
		inline CVar<bool> ThirdPersonServerHitbox{ false };
		inline CVar<bool> ThirdpersonOffset{ false };
		inline CVar<float> ThirdpersonDist{ 200.f };
		inline CVar<float> ThirdpersonRight{ 0.f };
		inline CVar<float> ThirdpersonUp{ 0.f };
		inline CVar<bool> ThirdpersonOffsetWithArrows{ false };
		inline CVar<int> ThirdpersonArrowOffsetKey{ VK_F };
		inline CVar<bool> ThirdpersonCrosshair{ false };

		inline CVar<bool> WorldModulation{ false };
		inline CVar<bool> PropWireframe{ false };
		inline CVar<bool> OverrideWorldTextures{ false };
		inline CVar<bool> SkyboxChanger{ false };
		inline CVar<bool> SkyModulation{ false };
		
		inline CVar<bool> AimbotViewmodel{ false };
		inline CVar<bool> ViewmodelSway{ false };
		inline CVar<float> ViewmodelSwayScale{ 5.f };
		inline CVar<float> ViewmodelSwayInterp{ 0.05f };

		inline CVar<bool> BulletTracer{ false };

		inline CVar<bool> SimLines{ false };
		inline CVar<bool> ClearLines{ false };
		inline CVar<bool> SimSeperators{ false };
		inline CVar<int> SeperatorLength{ 12 };
		inline CVar<int> SeperatorSpacing{ 6 };
		inline CVar<bool> SwingLines{ false };
		inline CVar<bool> ProjectileTrajectory{ false };

		inline CVar<bool> DoPostProcessing{ false };

		namespace Particles
		{
			namespace Colors
			{
				inline CVar<int> Type{ 0 };
				inline CVar<float> RainbowSpeed{ 1.5f };
			}
			namespace Tracers
			{
				inline CVar<int> ParticleTracer{ 2 };
				inline CVar<int> ParticleTracerCrits{ 2 };
				inline std::string ParticleName = "merasmus_zap_beam01"; // dont save these as vars its pointless
				inline std::string ParticleNameCrits = "merasmus_zap_beam01";
			}
			namespace Feet
			{
				inline CVar<bool> Enabled{ false };
				inline CVar<int> ColorType{ 0 }; //0 - color picker, 1 - HHH, 2 - team, 3 - rainbow
				inline CVar<bool> DashOnly{ false };
			}
		}

		namespace Beans
		{
			inline CVar<bool> Active{ false };
			inline CVar<bool> Rainbow{ false };
			inline Color_t BeamColour{ 255, 255, 255, 255 };
			inline CVar<bool> UseCustomModel{ false };
			inline std::string Model = "sprites/physbeam.vmt";
			inline CVar<float> Life{ 2.f };
			inline CVar<float> Width{ 2.f };
			inline CVar<float> EndWidth{ 2.f };
			inline CVar<float> FadeLength{ 10.f };
			inline CVar<float> Amplitude{ 2.f };
			inline CVar<float> Brightness{ 255.f };
			inline CVar<float> Speed{ 0.2f };
			inline CVar<int> Flags{ 65792 };
			inline CVar<int> Segments{ 2 };
		}

		inline CVar<bool> OutOfFOVArrows{ false };
		inline CVar<float> MaxDist{ 1000.f };
		inline CVar<int> VMOffX{ 0 };
		inline CVar<int> VMOffY{ 0 };
		inline CVar<int> VMOffZ{ 0 };
		inline Vec3 VMOffsets{};
		inline CVar<int> VMRoll{ 0 };

		inline CVar<float> NotificationLifetime{ 5.f };
		inline CVar<bool> DamageLoggerText{ false };
		inline CVar<bool> DamageLoggerChat{ false };
		inline CVar<bool> DamageLoggerConsole{ false };

		inline CVar<bool> EquipRegionUnlock{ false };

		inline CVar<bool> NoStaticPropFade{ false };

		namespace RagdollEffects
		{
			inline CVar<bool> NoGib{ false };
			inline CVar<bool> EnemyOnly{ false };
			inline CVar<bool> Burning{ false };
			inline CVar<bool> Electrocuted{ false };
			inline CVar<bool> BecomeAsh{ false };
			inline CVar<bool> Dissolve{ false };
			inline CVar<int> RagdollType{ 0 };

			inline CVar<bool> SeparateVectors{ false };
			inline CVar<float> RagdollForce{ 1.f };
			inline CVar<float> RagdollForceForwards{ 1.f };
			inline CVar<float> RagdollForceSides{ 1.f };
			inline CVar<float> RagdollForceUp{ 1.f };
		}
	}

	namespace Misc
	{
		inline CVar<bool> FastDeltaStrafe{ false };
		inline CVar<int> AccurateMovement{ 0 };
		inline CVar<bool> AutoJump{ false };
		inline CVar<int> AutoStrafe{ 2 };
		inline CVar<bool> DirectionalOnlyOnMove{ false };
		inline CVar<bool> DirectionalOnlyOnSpace{ false };
		inline CVar<bool> Directional{ false };
		inline CVar<bool> TauntSlide{ false };
		inline CVar<bool> BypassPure{ false };
		inline CVar<bool> DisableInterpolation{ false };
		inline CVar<int> NoPush{ 0 };	//	0 off, 1 on, 2 on while not afk, 3 semi while afk
		inline CVar<bool> AntiAFK{ false };
		inline CVar<int> VotingOptions{ 0b000100 }; // 000100 {verbose, autovote, party, chat, console, text}
		inline CVar<bool> CheatsBypass{ false };
		inline CVar<bool> CrouchSpeed{ false };
		inline CVar<bool> FastAccel{ false };
		inline CVar<bool> FakeAccelAngle{ false };
		inline CVar<bool> AntiAutobalance{ false };
		inline CVar<bool> MVMRes{ false };
		inline CVar<bool> PingReducer{ false };
		inline CVar<int> PingTarget{ 0 };
		inline std::string ConvarName{ 0 };
		inline std::string ConvarValue{ 0 };
		inline CVar<int> SoundBlock{ 0 };
		inline CVar<bool> ChatFlags{ false };
		inline CVar<bool> AutoAcceptItemDrops{ false };
		inline CVar<bool> RegionChanger{ false };
		inline CVar<int> RegionsAllowed{ 0 };
		inline CVar<int> AutoCasualQueue{ 0 };
		inline CVar<int> InstantAccept{ false };
		inline CVar<bool> ScoreboardPlayerlist{ false };

		namespace CheaterDetection
		{
			inline CVar<bool> Enabled{ false };
			inline CVar<int> Methods{ 0b111111100 };			//	Duckspeed, Aimbot, OOB pitch, angles, bhop, fakelag, simtime, high score, high accuracy
			inline CVar<int> Protections{ 0b111 };			//	double scans, lagging client, timing out
			inline CVar<int> SuspicionGate{ 10 };				//	the amount of infractions prior to marking someone as a cheater
			inline CVar<int> PacketManipGate{ 14 };			//	the avg choke for someone to receive and infraction for packet choking
			inline CVar<int> BHopMaxDelay{ 1 };				//	max groundticks used when detecting a bunny hop.
			inline CVar<int> BHopDetectionsRequired{ 5 };		//	how many times must some be seen bunny hopping to receive an infraction
			inline CVar<float> ScoreMultiplier{ 2.f };		//	multiply the avg score/s by this to receive the maximum amount
			inline CVar<float> MinimumFlickDistance{ 20.f };	//	min mouse flick size to suspect someone of angle cheats.
			inline CVar<float> MaximumNoise{ 5.f };			//	max mouse noise prior to a flick to mark somebody
			inline CVar<float> MinimumAimbotFoV{ 3.f };		//	scaled with how many ticks a player has choked up to ->
			inline CVar<float> MaxScaledAimbotFoV{ 20.f };	//	self explanatory
		}

		namespace CL_Move
		{
			inline CVar<bool> Enabled{ false };
			inline CVar<bool> Indicator{ false };
			inline DragBox_t DTIndicator{ g_ScreenSize.c, g_ScreenSize.c };
			inline CVar<bool> Doubletap{ false };
			inline CVar<bool> SafeTick{ false };
			inline CVar<bool> SafeTickAirOverride{ false };
			inline CVar<int> PassiveRecharge{ 0 };
			inline CVar<bool> SEnabled{ false };
			inline CVar<int> SFactor{ 1 };
			inline CVar<bool> NotInAir{ false };
			inline CVar<bool> StopMovement{ false };
			inline CVar<int> TeleportKey{ 0x52 }; //R
			inline CVar<int> TeleportMode{ 0 };
			inline CVar<int> TeleportFactor{ 2 };
			inline CVar<int> RechargeKey{ 0x48 }; //H
			inline CVar<int> DoubletapKey{ 0x56 }; //V
			inline CVar<bool> AutoRetain{ true };
			inline CVar<bool> RetainFakelag{ false };
			inline CVar<bool> RetainBlastJump{ false };
			inline CVar<bool> UnchokeOnAttack{ false };
			inline CVar<bool> RechargeWhileDead{ false };
			inline CVar<bool> AutoRecharge{ false }; //H
			inline CVar<bool> AntiWarp{ false }; //H
			inline CVar<int> DTMode{ 0 }; // 0 - On Key, 1 - Always DT, 2 - Disable on key, 3 - Disabled
			inline CVar<int> DTTicks{ 21 };
			inline CVar<bool> WaitForDT{ false };
			inline CVar<bool> Fakelag{ false };
			inline CVar<int> FakelagMode{ 0 }; // 0 - plain, 1 - random
			inline CVar<bool> WhileMoving{ false };
			inline CVar<bool> WhileVisible{ false };
			inline CVar<bool> PredictVisibility{ false };
			inline CVar<bool> WhileUnducking{ false };
			inline CVar<bool> WhileInAir{ false };
			inline CVar<int> FakelagMin{ 1 }; //	only show when FakelagMode=2
			inline CVar<int> FakelagMax{ 22 };
			inline CVar<bool> FakelagOnKey{ false }; // dont show when fakelagmode=2|3
			inline CVar<int> FakelagKey{ 0x54 }; //T
			inline CVar<int> FakelagValue{ 1 }; // dont show when fakelagmode=2
			inline CVar<int> AutoPeekKey{ false };
			inline CVar<float> AutoPeekDistance{ 200.f };
			inline CVar<bool> AutoPeekFree{ false };

			namespace FLGChams
			{
				inline CVar<bool> Enabled{ false };
				inline CVar<int> Material{ 1 };
				inline Color_t FakelagColor{ 255, 255, 255, 255 };
			}
		}

		namespace Steam
		{
			inline CVar<bool> EnableRPC{ false };
			inline CVar<int> MatchGroup{ 0 }; // 0 - Special Event; 1 - MvM Mann Up; 2 - Competitive; 3 - Casual; 4 - MvM Boot Camp;
			inline CVar<bool> OverrideMenu{ false }; // Override matchgroup when in main menu
			inline CVar<int> MapText{ 1 }; // 0 - Fedoraware; 1 - CUM.clab; 2 - Meowhook.club; 3 - rathook.cc; 4 - NNitro.tf; 5 - custom;
			inline CVar<int> GroupSize{ 1337 };
			inline CVar<std::string> CustomText = { "M-FeD is gay" };
		}
	}

	namespace AntiHack
	{
		namespace AntiAim
		{
			inline CVar<bool> Active{ false };
			inline CVar<int> ToggleKey{ 0 };
			inline CVar<int> InvertKey{ 0 };
			inline CVar<int> ManualKey{ 0 };
			inline CVar<int> Pitch{ 0 }; //0 - None, 1 - Zero, 2 - Up, 3 - Down, 4 - Fake Up, 5 - Fake Down
			inline CVar<int> YawReal{ 0 }; //0 - None, 1 - Forward, 2 - Left, 3 - Right, 4 - Backwards
			inline CVar<int> YawFake{ 0 }; //0 - None, 1 - Forward, 2 - Left, 3 - Right, 4 - Backwards
			inline CVar<int> BaseYawMode{ 0 };
			inline CVar<float> BaseYawOffset{ 0.f };
			inline CVar<float> SpinSpeed{ 15.f };
			inline CVar<float> CustomRealPitch{ 0.f };
			inline CVar<float> CustomRealYaw{ 0 };
			inline CVar<float> CustomFakeYaw{ 0 };
			inline CVar<float> FakeJitter{ 0 };
			inline CVar<float> RealJitter{ 0 };
			inline CVar<int> RandInterval{ 25 };
			inline CVar<bool> AntiBackstab{ false };
			inline CVar<bool> LegJitter{ false }; // frick u fourteen
			inline CVar<bool> AntiOverlap{ false };
			inline CVar<bool> InvalidShootPitch{ false }; // i dont know what to name this its TRASH
			// no reason to do this for projectile and melee cause u have psilent lel
		}

		namespace Resolver
		{
			inline CVar<bool> Resolver{ false };
			inline CVar<bool> AutoResolveCheaters{ false };
			inline CVar<bool> IgnoreAirborne{ false };
		}
	}

	namespace Skybox
	{
		inline int SkyboxNum = 0;
		inline std::string SkyboxName = "mr_04";
	}

	namespace Fonts
	{
		namespace FONT_ESP
		{
			inline std::string szName = "Small Fonts";
			inline CVar<int> nTall = { 9 };
			inline CVar<int> nWeight = { 0 };
			inline CVar<int> nFlags = { FONTFLAG_OUTLINE };
		}

		namespace FONT_ESP_NAME
		{
			inline std::string szName = "Small Fonts";
			inline CVar<int> nTall = { 12 };
			inline CVar<int> nWeight = { 0 };
			inline CVar<int> nFlags = { FONTFLAG_OUTLINE };
		}

		namespace FONT_ESP_COND
		{
			inline std::string szName = "Small Fonts";
			inline CVar<int> nTall = { 9 };
			inline CVar<int> nWeight = { 0 };
			inline CVar<int> nFlags = { FONTFLAG_OUTLINE };
		}

		namespace FONT_ESP_PICKUPS
		{
			inline std::string szName = "Small Fonts";
			inline CVar<int> nTall = { 12 };
			inline CVar<int> nWeight = { 0 };
			inline CVar<int> nFlags = { FONTFLAG_OUTLINE };
		}

		namespace FONT_MENU
		{
			inline std::string szName = "Small Fonts";
			inline CVar<int> nTall = { 12 };
			inline CVar<int> nWeight = { 0 };
			inline CVar<int> nFlags = { FONTFLAG_ANTIALIAS };
		}

		namespace FONT_INDICATORS
		{
			inline std::string szName = "Small Fonts";
			inline CVar<int> nTall = { 10 };
			inline CVar<int> nWeight = { 0 };
			inline CVar<int> nFlags = { FONTFLAG_OUTLINE };
		}
	}

	// Debug options
	namespace Debug
	{
		inline CVar<bool> DebugInfo{ false };
	}
}