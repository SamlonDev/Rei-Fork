#include "Misc.h"

#include "../Vars.h"
#include "../../Utils/Timer/Timer.hpp"
#include "../Aimbot/AimbotGlobal/AimbotGlobal.h"
#include "../Backtrack/Backtrack.h"
#include "../AntiHack/CheaterDetection.h"
#include "../PacketManip/AntiAim/AntiAim.h"

extern int attackStringW;
extern int attackStringH;

void CMisc::RunPre(CUserCmd* pCmd, bool* pSendPacket)
{
	bMovementStopped = false; bMovementScuffed = false;
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		AutoJump(pCmd, pLocal);
		AutoStrafe(pCmd, pLocal);
		AntiBackstab(pLocal, pCmd);
		AutoPeek(pCmd, pLocal);
	}

	AntiAFK(pCmd);
	CheatsBypass();
	PingReducer();
	FakeInterp();
	DetectChoke();
	WeaponSway();
}

void CMisc::RunPost(CUserCmd* pCmd, bool* pSendPacket)
{
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		FastStop(pCmd, pLocal);
		FastAccel(pCmd, pLocal, pSendPacket);
		FastStrafe(pCmd, pSendPacket);
		InstaStop(pCmd, pSendPacket);
		StopMovement(pCmd, pLocal);
		LegJitter(pCmd, pLocal);
		DoubletapPacket(pCmd, pSendPacket);
	}
}



void CMisc::AutoJump(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!Vars::Misc::AutoJump.Value
		|| !pLocal->IsAlive()
		|| pLocal->IsSwimming()
		|| pLocal->IsInBumperKart()
		|| pLocal->IsAGhost())
	{
		return;
	}

	if (pLocal->GetMoveType() == MOVETYPE_NOCLIP
		|| pLocal->GetMoveType() == MOVETYPE_LADDER
		|| pLocal->GetMoveType() == MOVETYPE_OBSERVER)
	{
		return;
	}

	const bool bJumpHeld = pCmd->buttons & IN_JUMP;
	const bool bCurHop = bJumpHeld && pLocal->OnSolid();
	static bool bHopping = bCurHop;
	static bool bTried = false;

	if (bCurHop && !bTried)
	{	//	this is our initial jump
		bTried = true;
		bHopping = true; return;
	}
	else if (bCurHop && bTried) 
	{	//	we tried and failed to bunnyhop, let go of the key and try again the next tick
		bTried = false;
		pCmd->buttons &= ~IN_JUMP; return;
	}
	else if (bHopping && bJumpHeld && (!pLocal->OnSolid() || pLocal->IsDucking()))
	{	//	 we are not on the ground and the key is in the same hold cycle
		bTried = false;
		pCmd->buttons &= ~IN_JUMP; return;
	}
	else if (bHopping && !bJumpHeld)
	{	//	we are no longer in the jump key cycle
		bTried = false;
		bHopping = false; return;
	}
	else if (!bHopping && bJumpHeld)
	{	//	we exited the cycle but now we want back in, don't mess with keys for doublejump, enter us back into the cycle for next tick
		bTried = false;
		bHopping = true; return;
	}

	return;
}

void CMisc::AutoStrafe(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!Vars::Misc::AutoStrafe.Value)
		return;

	if (!pLocal->IsAlive()
		|| pLocal->IsSwimming()
		|| pLocal->IsInBumperKart()
		|| pLocal->IsAGhost()
		|| pLocal->OnSolid())
	{
		return;
	}

	if (pLocal->GetMoveType() == MOVETYPE_NOCLIP
		|| pLocal->GetMoveType() == MOVETYPE_LADDER
		|| pLocal->GetMoveType() == MOVETYPE_OBSERVER)
	{
		return;
	}

	static auto cl_sidespeed = g_ConVars.FindVar("cl_sidespeed");
	if (!cl_sidespeed || !cl_sidespeed->GetFloat())
		return;

	static bool wasJumping = false;
	const bool isJumping = pCmd->buttons & IN_JUMP;

	switch (Vars::Misc::AutoStrafe.Value)
	{
	case 1:
	{
		if (pCmd->mousedx && (!isJumping || wasJumping))
			pCmd->sidemove = pCmd->mousedx > 0 ? cl_sidespeed->GetFloat() : -cl_sidespeed->GetFloat();
		wasJumping = isJumping;
		break;
	}
	case 2:
	{
		if (Vars::Misc::DirectionalOnlyOnMove.Value && !(pCmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK)))
			break;
		if (Vars::Misc::DirectionalOnlyOnSpace.Value && !(GetAsyncKeyState(VK_SPACE) & 0x8000))
			break;

		const auto vel = pLocal->GetVelocity();
		const float speed = vel.Length2D();
		if (speed < 2.0f)
			break;

		constexpr auto perfectDelta = [](float speed, CBaseEntity* pLocal) noexcept
			{
				auto speedVar = pLocal->TeamFortress_CalculateMaxSpeed();
				static auto airVar = g_ConVars.FindVar("sv_airaccelerate");
				static auto wishSpeed = 90.0f;

				const auto term = wishSpeed / airVar->GetFloat() / speedVar * 100.f / speed;

				if (term < 1.0f && term > -1.0f)
				{
					return acosf(term);
				}
				return 0.0f;
			};

		const float pDelta = perfectDelta(speed, pLocal);
		if (!isJumping || wasJumping) //credits to fourteen
		{
			static auto old_yaw = 0.0f;

			auto get_velocity_degree = [](float velocity)
				{
					auto tmp = RAD2DEG(atan(30.0f / velocity));

#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)
					if (CheckIfNonValidNumber(tmp) || tmp > 90.0f)
						return 90.0f;

					else if (tmp < 0.0f)
						return 0.0f;
					else
						return tmp;
				};

			if (pLocal->GetMoveType() != MOVETYPE_WALK)
				return;

			auto velocity = pLocal->m_vecVelocity();
			velocity.z = 0.0f;

			auto forwardmove = pCmd->forwardmove;
			auto sidemove = pCmd->sidemove;

			if (velocity.Length2D() < 5.0f && !forwardmove && !sidemove)
				return;

			static auto flip = false;
			flip = !flip;

			auto turn_direction_modifier = flip ? 1.0f : -1.0f;
			auto viewangles = pCmd->viewangles;

			if (forwardmove || sidemove)
			{
				pCmd->forwardmove = 0.0f;
				pCmd->sidemove = 0.0f;

				auto turn_angle = atan2(-sidemove, forwardmove);
				viewangles.y += turn_angle * M_RADPI;
			}
			else if (forwardmove) //-V550
				pCmd->forwardmove = 0.0f;

			auto strafe_angle = RAD2DEG(atan(15.0f / velocity.Length2D()));

			if (strafe_angle > 90.0f)
				strafe_angle = 90.0f;
			else if (strafe_angle < 0.0f)
				strafe_angle = 0.0f;

			auto temp = Vector(0.0f, viewangles.y - old_yaw, 0.0f);
			temp.y = Math::NormalizeYaw(temp.y);

			auto yaw_delta = temp.y;
			old_yaw = viewangles.y;

			auto abs_yaw_delta = fabs(yaw_delta);

			Vector velocity_angles;
			Math::VectorAngles(velocity, velocity_angles);

			temp = Vector(0.0f, viewangles.y - velocity_angles.y, 0.0f);
			temp.y = Math::NormalizeYaw(temp.y);

			auto velocityangle_yawdelta = temp.y;
			auto velocity_degree = get_velocity_degree(velocity.Length2D()) * 0.3f;

			if (velocityangle_yawdelta <= velocity_degree || velocity.Length2D() <= 15.0f)
			{
				if (-velocity_degree <= velocityangle_yawdelta || velocity.Length2D() <= 15.0f)
				{
					viewangles.y += strafe_angle * turn_direction_modifier;
					pCmd->sidemove = cl_sidespeed->GetFloat() * turn_direction_modifier;
				}
				else
				{
					viewangles.y = velocity_angles.y - velocity_degree;
					pCmd->sidemove = cl_sidespeed->GetFloat();
				}
			}
			else
			{
				viewangles.y = velocity_angles.y + velocity_degree;
				pCmd->sidemove = -cl_sidespeed->GetFloat();
			}

			Vector angles_move;
			auto move = Vector(pCmd->forwardmove, pCmd->sidemove, 0.0f);
			auto speed = move.Length();
			Math::VectorAngles(move, angles_move);

			auto normalized_y = fmod(pCmd->viewangles.y + 180.0f, 360.0f) - 180.0f;
			auto yaw = DEG2RAD(normalized_y - viewangles.y + angles_move.y);

			pCmd->forwardmove = cos(yaw) * speed;
			pCmd->sidemove = sin(yaw) * speed;
		}
		wasJumping = isJumping;
		break;
	}
	}
}

void CMisc::AntiBackstab(CBaseEntity* pLocal, CUserCmd* pCmd)
{
	G::AvoidingBackstab = false;
	Vec3 vTargetPos;

	if (!pLocal->IsAlive() || pLocal->IsStunned() || pLocal->IsInBumperKart() || pLocal->IsAGhost() || !Vars::AntiHack::AntiAim::AntiBackstab.Value)
		return;

	if (G::IsAttacking)
		return;

	const Vec3 vLocalPos = pLocal->GetWorldSpaceCenter();
	CBaseEntity* target = nullptr;

	for (const auto& pEnemy : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
	{
		if (!pEnemy || !pEnemy->IsAlive() || pEnemy->GetClassNum() != CLASS_SPY || pEnemy->IsCloaked() || pEnemy->IsAGhost() || pEnemy->GetFeignDeathReady())
			continue;

		if (CBaseCombatWeapon* pWeapon = pEnemy->GetActiveWeapon())
		{
			if (pWeapon->GetWeaponID() != TF_WEAPON_KNIFE)
				continue;
		}

		PlayerInfo_t pInfo{};
		if (!I::EngineClient->GetPlayerInfo(pEnemy->GetIndex(), &pInfo))
		{
			if (G::IsIgnored(pInfo.friendsID))
				continue;
		}

		Vec3 vEnemyPos = pEnemy->GetWorldSpaceCenter();
		if (!Utils::VisPos(pLocal, pEnemy, vLocalPos, vEnemyPos))
			continue;
		if (!target && vLocalPos.DistTo(vEnemyPos) < 150.f)
		{
			target = pEnemy;
			vTargetPos = target->GetWorldSpaceCenter();
		}
		else if (vLocalPos.DistTo(vEnemyPos) < vLocalPos.DistTo(vTargetPos) && vLocalPos.DistTo(vEnemyPos) < 150.f)
		{
			target = pEnemy;
			vTargetPos = target->GetWorldSpaceCenter();
		}
	}

	if (target)
	{
		vTargetPos = target->GetWorldSpaceCenter();
		const Vec3 vAngleToSpy = Math::CalcAngle(vLocalPos, vTargetPos);
		G::AvoidingBackstab = true;
		Utils::FixMovement(pCmd, vAngleToSpy);
		pCmd->viewangles = vAngleToSpy;
	}
}

bool CanAttack(CBaseEntity* pLocal, const Vec3& pPos)
{
	if (const auto pWeapon = pLocal->GetActiveWeapon())
	{
		if (!G::WeaponCanHeadShot && pLocal->IsScoped())
			return false;
		if (!pWeapon->CanShoot(pLocal))
			return false;

		for (const auto& target : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			if (!target->IsAlive() || target->IsAGhost())
				continue;
			if (F::AimbotGlobal.ShouldIgnore(target))
				continue;

			// Get the hitbox position (Backtrack if possible)
			Vec3 targetPos = target->GetHitboxPos(HITBOX_HEAD);
			//if (Vars::Backtrack::Enabled.Value)
			//{
			//	const auto& btRecord = F::Backtrack.GetRecord(target->GetIndex(), BacktrackMode::Last);
			//	if (btRecord) { targetPos = btRecord->HeadPosition; }
			//}

			// Is the player visible?
			if (Utils::VisPos(pLocal, target, pPos, targetPos))
				return true;
		}
	}

	return false;
}
void CMisc::AutoPeek(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	static bool posPlaced = false;
	static bool isReturning = false;
	static bool hasDirection = false;
	static Vec3 peekStart;
	static Vec3 peekVector;

	if (pLocal->IsAlive() && !pLocal->IsAGhost() && Vars::CL_Move::AutoPeekKey.Value && F::KeyHandler.Down(Vars::CL_Move::AutoPeekKey.Value))
	{
		const Vec3 localPos = pLocal->GetAbsOrigin();

		// We just started peeking. Save the return position!
		if (!posPlaced)
		{
			if (pLocal->OnSolid())
			{
				PeekReturnPos = localPos;
				posPlaced = true;
			}
		}
		else
		{
			static Timer particleTimer{};
			if (particleTimer.Run(700))
				Particles::DispatchParticleEffect("ping_circle", PeekReturnPos, {});
		}

		// We need a peek direction (A / D)
		if (!Vars::CL_Move::AutoPeekFree.Value && !hasDirection && pLocal->OnSolid())
		{
			const Vec3 viewAngles = I::EngineClient->GetViewAngles();
			Vec3 vForward, vRight, vUp, vDirection;
			Math::AngleVectors(viewAngles, &vForward, &vRight, &vUp);

			if (GetAsyncKeyState(VK_A) & 0x8000 || GetAsyncKeyState(VK_W) & 0x8000 || GetAsyncKeyState(VK_D) & 0x8000 || GetAsyncKeyState(VK_S) & 0x8000)
			{
				CGameTrace trace;
				CTraceFilterWorldAndPropsOnly traceFilter;
				Ray_t traceRay;

				if (GetAsyncKeyState(VK_A) & 0x8000 || GetAsyncKeyState(VK_W) & 0x8000)
					vDirection = pLocal->GetEyePosition() - vRight * Vars::CL_Move::AutoPeekDistance.Value; // Left
				else if (GetAsyncKeyState(VK_D) & 0x8000 || GetAsyncKeyState(VK_S) & 0x8000)
					vDirection = pLocal->GetEyePosition() + vRight * Vars::CL_Move::AutoPeekDistance.Value; // Right

				traceRay.Init(pLocal->GetEyePosition(), vDirection);
				I::EngineTrace->TraceRay(traceRay, MASK_SOLID, &traceFilter, &trace);
				peekStart = trace.vStartPos;
				peekVector = trace.vEndPos - trace.vStartPos;
				hasDirection = true;
			}
		}

		// Should we peek?
		if (!Vars::CL_Move::AutoPeekFree.Value && hasDirection)
		{
			bool targetFound = false;
			for (int i = 10; i < 100; i += 10)
			{
				const float step = i / 100.f;
				Vec3 currentPos = peekStart + (peekVector * step);
				if (CanAttack(pLocal, currentPos))
				{
					Utils::WalkTo(pCmd, pLocal, currentPos);
					targetFound = true;
				}

				if (targetFound)
				{
					I::DebugOverlay->AddLineOverlayAlpha(PeekReturnPos, currentPos, 68, 189, 50, 100, false, 0.04f);
					break;
				}

				I::DebugOverlay->AddLineOverlayAlpha(PeekReturnPos, currentPos, 235, 59, 90, 100, false, 0.04f);
			}

			if (!targetFound) { isReturning = true; }
		}

		// We've just attacked. Let's return!
		if (G::LastUserCmd->buttons & IN_ATTACK || G::IsAttacking)
			isReturning = true;

		if (isReturning)
		{
			if (localPos.DistTo(PeekReturnPos) < 7.f)
			{
				// We reached our destination. Recharge DT if wanted
				if (Vars::CL_Move::DoubleTap::AutoRecharge.Value && isReturning && !G::DoubleTap && !G::ShiftedTicks)
					G::Recharge = true;
				isReturning = false;
				return;
			}

			Utils::WalkTo(pCmd, pLocal, PeekReturnPos);
		}
	}
	else
	{
		posPlaced = isReturning = hasDirection = false;
		PeekReturnPos = Vec3();
	}
}

void CMisc::AntiAFK(CUserCmd* pCmd)
{
	if (Vars::Misc::AntiAFK.Value && g_ConVars.afkTimer->GetInt() != 0)
	{
		if (pCmd->command_number % 2)
			pCmd->buttons |= (1 << 27);
	}
}

void CMisc::CheatsBypass()
{
	static bool cheatset = false;
	if (ConVar* sv_cheats = g_ConVars.FindVar("sv_cheats"))
	{
		if (Vars::Misc::CheatsBypass.Value && sv_cheats)
		{
			sv_cheats->m_Value.m_nValue = 1;
			cheatset = true;
		}
		else
		{
			if (cheatset)
			{
				sv_cheats->m_Value.m_nValue = 0;
				cheatset = false;
			}
		}
	}
}

void CMisc::Event(CGameEvent* pEvent, FNV1A_t uNameHash)
{
	if (uNameHash == FNV1A::HashConst("teamplay_round_start") || uNameHash == FNV1A::HashConst("client_disconnect") ||
		uNameHash == FNV1A::HashConst("client_beginconnect") || uNameHash == FNV1A::HashConst("game_newmap"))
	{
		iLastCmdrate = -1;
		F::Backtrack.flWishInterp = 0.f;

		G::BulletsStorage.clear();
		G::BoxesStorage.clear();
		G::LinesStorage.clear();
	}
}

void CMisc::PingReducer()
{
	const ConVar* cl_cmdrate = g_ConVars.FindVar("cl_cmdrate");
	CNetChannel* netChannel = I::EngineClient->GetNetChannelInfo();
	if (!cl_cmdrate || !netChannel)
		return;

	static Timer updateRateTimer{};
	if (updateRateTimer.Run(500))
	{
		const int iTarget = Vars::Misc::PingReducer.Value ? Vars::Misc::PingTarget.Value : cl_cmdrate->GetInt();
		if (iTarget == iLastCmdrate)
			return;
		iLastCmdrate = iTarget;

		Utils::ConLog("SendNetMsg", std::format("cl_cmdrate: {}", iTarget).c_str(), { 224, 255, 131, 255 }, Vars::Debug::Logging.Value);

		NET_SetConVar cmd("cl_cmdrate", std::to_string(iTarget).c_str());
		netChannel->SendNetMsg(cmd);
	}
}

void CMisc::FakeInterp()
{
	CNetChannel* netChannel = I::EngineClient->GetNetChannelInfo();
	if (!netChannel) return;

	static Timer interpTimer{};
	if (interpTimer.Run(500))
	{
		float flTarget = F::Backtrack.GetLerp();
		if (flTarget == F::Backtrack.flWishInterp) return;
		F::Backtrack.flWishInterp = flTarget;

		Utils::ConLog("SendNetMsg", std::format("cl_interp: {}", flTarget).c_str(), { 224, 255, 131, 255 }, Vars::Debug::Logging.Value);

		{
			NET_SetConVar cmd("cl_interp", std::to_string(flTarget).c_str());
			netChannel->SendNetMsg(cmd);
		}
		{
			NET_SetConVar cmd("cl_interp_ratio", "1.0");
			netChannel->SendNetMsg(cmd);
		}
		{
			NET_SetConVar cmd("cl_interpolate", "1");
			netChannel->SendNetMsg(cmd);
		}
	}
}

void CMisc::DetectChoke()
{
	static int iOldTick = I::GlobalVars->tickcount;
	if (I::GlobalVars->tickcount == iOldTick) { return; }
	iOldTick = I::GlobalVars->tickcount;
	for (const auto& pEntity : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
	{
		if (!pEntity->IsAlive() || pEntity->IsAGhost() || pEntity->GetDormant())
		{
			G::ChokeMap[pEntity->GetIndex()] = 0;
			continue;
		}

		if (pEntity->GetSimulationTime() == pEntity->GetOldSimulationTime())
			G::ChokeMap[pEntity->GetIndex()]++;
		else
		{
			F::BadActors.ReportTickCount({ pEntity, G::ChokeMap[pEntity->GetIndex()] });
			G::ChokeMap[pEntity->GetIndex()] = 0;
		}
	}
}

void CMisc::WeaponSway()
{
	if (ConVar* cl_wpn_sway_interp = g_ConVars.FindVar("cl_wpn_sway_interp"))
		cl_wpn_sway_interp->SetValue(Vars::Visuals::ViewmodelSway.Value ? Vars::Visuals::ViewmodelSwayInterp.Value : 0.f);
	if (ConVar* cl_wpn_sway_scale = g_ConVars.FindVar("cl_wpn_sway_scale"))
		cl_wpn_sway_scale->SetValue(Vars::Visuals::ViewmodelSway.Value ? Vars::Visuals::ViewmodelSwayScale.Value : 0.f);
}



void CMisc::FastStop(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!Vars::Misc::FastStop.Value || (bMovementScuffed || bMovementStopped))
		return;

	const int iStopMode = G::ShiftedTicks == G::MaxShift ? 1 : 2;

	if (!pLocal->IsAlive()
		|| pLocal->IsSwimming()
		|| (pLocal->IsInBumperKart() && iStopMode != 2)
		|| pLocal->IsAGhost()
		|| pLocal->IsCharging()
		|| !pLocal->OnSolid())
	{
		return;
	}

	if (pLocal->GetMoveType() == MOVETYPE_NOCLIP
		|| pLocal->GetMoveType() == MOVETYPE_LADDER
		|| pLocal->GetMoveType() == MOVETYPE_OBSERVER)
	{
		return;
	}

	if (pCmd->buttons & (IN_JUMP | IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK))
		return;

	const float speed = pLocal->GetVecVelocity().Length2D();
	const float speedLimit = 10.f;

	if (speed > speedLimit)
	{
		switch (iStopMode)
		{
		case 1:
		{
			Vec3 direction = pLocal->GetVecVelocity().toAngle();
			direction.y = pCmd->viewangles.y - direction.y;
			const Vec3 negatedDirection = direction.fromAngle() * -speed;
			pCmd->forwardmove = negatedDirection.x;
			pCmd->sidemove = negatedDirection.y;
			break;
		}
		case 2:
			G::ShouldStop = true;
			break;
		}
	}
	else
	{
		pCmd->forwardmove = 0.0f;
		pCmd->sidemove = 0.0f;
	}
}

void CMisc::FastAccel(CUserCmd* pCmd, CBaseEntity* pLocal, bool* pSendPacket)
{
	bFastAccel = false;

	const bool bShouldAccel = pLocal->IsDucking() ? Vars::Misc::CrouchSpeed.Value : !G::DoubleTap && Vars::Misc::FastAccel.Value;
	if (!bShouldAccel)
		return;

	static bool flipVar = false;
	flipVar = !flipVar;
	if (G::AntiAim || bMovementScuffed || bMovementStopped || !flipVar)
		return;

	if (!pLocal->IsAlive() || pLocal->IsSwimming() || pLocal->IsAGhost() || !pLocal->OnSolid() || pLocal->IsCharging() ||
		G::Recharge || G::Frozen || G::IsAttacking ||
		pLocal->GetMoveType() == MOVETYPE_NOCLIP || pLocal->GetMoveType() == MOVETYPE_LADDER || pLocal->GetMoveType() == MOVETYPE_OBSERVER)
		return;

	const int maxSpeed = std::min(pLocal->GetMaxSpeed() * (pCmd->forwardmove < 0 && !pCmd->sidemove ? 0.9f : 1.f), 520.f) - 10.f;
	const float curSpeed = pLocal->GetVecVelocity().Length2D();
	if (curSpeed > maxSpeed)
		return;

	if (pLocal->GetClassNum() == ETFClass::CLASS_HEAVY && pCmd->buttons & IN_ATTACK2 && pLocal->IsDucking())
		return;

	if (pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))
	{
		const Vec3 vecMove(pCmd->forwardmove, pCmd->sidemove, 0.0f);
		const float flLength = vecMove.Length();
		Vec3 angMoveReverse;
		Math::VectorAngles(vecMove * -1.f, angMoveReverse);
		pCmd->forwardmove = -flLength;
		pCmd->sidemove = 0.0f;
		pCmd->viewangles.y = fmodf(pCmd->viewangles.y - angMoveReverse.y, 360.0f);
		pCmd->viewangles.z = 270.f;
		G::UpdateView = false; bFastAccel = true;
		*pSendPacket = false;
	}
}

void CMisc::FastStrafe(CUserCmd* pCmd, bool* pSendPacket) {
	if (!Vars::Misc::FastStrafe.Value || bFastAccel)
		return;

	bool bChanged = false;

	static bool bFwd = pCmd->forwardmove > 0;
	static bool bSde = pCmd->sidemove > 0;
	const bool bCurFwd = pCmd->forwardmove > 0;
	const bool bCurSde = pCmd->sidemove > 0;

	if (fabsf(pCmd->sidemove) > 400)
	{
		if (bSde != bCurSde)
		{
			pCmd->viewangles.x = 90.f;	//	look down
			pCmd->viewangles.y += bSde ? -90.f : 90.f;	//	face left or right depending
			pCmd->sidemove = bSde ? -pCmd->forwardmove : pCmd->forwardmove;	//	set our forward move to our sidemove
			bChanged = true;

			bMovementScuffed = true;
			G::UpdateView = false;
			*pSendPacket = false;
		}

		// "why is dis one in anoda place doe" because if you're moving forward and you stop pressing the button foe 1 tick it no work :D
		bSde = bCurSde;

		if (bChanged)
			return;
	}
	if (fabsf(pCmd->forwardmove) > 400)
	{
		if (bFwd != bCurFwd)
		{
			pCmd->viewangles.x = 90.f;	//	look down
			pCmd->viewangles.y += bFwd ? 0.f : 180.f;
			pCmd->sidemove *= bFwd ? 1 : -1;
			bChanged = true;

			bMovementScuffed = true;
			G::UpdateView = false;
			*pSendPacket = false;
		}

		// "why dont u weset it outside of dis doe" because if the user stop press buton foe 1 tick it no work :D
		bFwd = bCurFwd;

		if (bChanged)
			return;
	}
}

void CMisc::InstaStop(CUserCmd* pCmd, bool* pSendPacket)
{
	if (!G::ShouldStop)
		return;
	Utils::StopMovement(pCmd);
	if (G::ShouldStop)
		return;

	G::UpdateView = false; bMovementStopped = true; bMovementScuffed = true;
	if (G::Recharge || G::DoubleTap)
		return;
	*pSendPacket = false;
}

//	Accelerate ( wishdir, wishspeed, sv_accelerate.GetFloat() );
//	accelspeed = accel * gpGlobals->frametime * wishspeed * player->m_surfaceFriction;
//	wishspeed = side/forwardmove from pCmd
//	accel = sv_accelerate value
//	10 * .015 * 450 * surfaceFriction	=	acceleration
//	67.5(surfaceFriction)				=	acceleration
//	acceleration = 60
//	surfaceFriction = 1.125	// this doesn't account for ice, etc. (it is also possible that the reason our accel is lower is because we are locked below 450 with our actual acceleration)
//	if our forward velocity is 400, to get it to 0, we would need to spend ~7 ticks of time decelerating.
void CMisc::StopMovement(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	// 1<<17 = TFCond_Charging

	if (pLocal && pLocal->IsAlive() && !pLocal->IsAGhost() && !pLocal->IsCharging() && !pLocal->IsTaunting() && !pLocal->IsStunned() && pLocal->GetVelocity().Length2D() > 5.f)
	{
		static Vec3 prediction = {};
		static Vec3 origin = {};
		static Vec3 angles = {};
		static int nShiftTickG = 0;
		static int nShiftTickA = 0;

		if (G::AntiWarp && G::ShiftedTicks && pLocal->OnSolid())
		{
			/*
			pCmd->forwardmove = 0.f; pCmd->sidemove = 0.f;

			Vec3 origin = pLocal->GetAbsOrigin();
			Vec3 velocity; pLocal->EstimateAbsVelocity(velocity);
			Vec3 predicted = origin + (velocity * TICKS_TO_TIME(G::ShiftedTicks));
			Vec3 predicted_max = origin + (velocity * TICKS_TO_TIME(22 - G::ChokeAmount));

			float scale = Math::RemapValClamped(predicted.DistTo(origin), 0.0f, predicted_max.DistTo(origin) * 1.27f, 1.0f, 0.0f);
			float scale_ = Math::RemapValClamped(velocity.Length2D(), 0.0f, 520.0f, 0.0f, 1.0f);

			if (pLocal->IsClass(CLASS_SCOUT))
			{
				Utils::WalkTo(pCmd, pLocal, predicted, origin, TICKS_TO_TIME(22 - G::ChokeAmount));
			}
			else
			{
				Utils::WalkTo(pCmd, pLocal, predicted_max, origin, scale * scale_);
			}
			*/

			switch (nShiftTickG)
			{
			case 0:
				G::ShouldStop = true;
				prediction = pLocal->GetVecOrigin() + pLocal->GetVecVelocity();
				origin = pLocal->GetVecOrigin();
				angles = I::EngineClient->GetViewAngles();

				nShiftTickG++;
				break;
			default:
				nShiftTickG++;
				break;
			}

			Utils::WalkTo(pCmd, pLocal, prediction, origin, (1.f / origin.Dist2D(prediction)));
			pCmd->viewangles = angles;
			//	the "slight stop" that u can see when we do this is due to (i believe) the player reaching the desired point, and then constantly accelerating backwards, meaning their velocity-
			//	when they finish shifting ticks, is lower than when they started.
			//	alot of things worked better than (1/dist) as the scale, but caused issues on different classes, for now this is the best I can get it to.
			return;
		}
		else
		{
			nShiftTickG = 0;
			nShiftTickA = 0;
			return;
		}
	}
}

void CMisc::LegJitter(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!Vars::AntiHack::AntiAim::LegJitter.Value || !pLocal->OnSolid() || pLocal->IsInBumperKart() || pLocal->IsAGhost() || !pLocal->IsAlive())
		return;

	if (G::IsAttacking || G::DoubleTap || !F::AntiAim.bSendingReal)
		return;

	static bool pos = true;
	const float scale = pLocal->IsDucking() ? 14.f : 1.0f;
	if (pCmd->forwardmove == 0.f && pCmd->sidemove == 0.f && pLocal->GetVecVelocity().Length2D() < 10.f && (Vars::AntiHack::AntiAim::LegJitter.Value || F::AntiAim.bSendingReal)) // force leg jitter if we are sending our real.
	{
		pos ? pCmd->forwardmove = scale : pCmd->forwardmove = -scale;
		pos ? pCmd->sidemove = scale : pCmd->sidemove = -scale;
		pos = !pos;
	}
}

void CMisc::DoubletapPacket(CUserCmd* pCmd, bool* pSendPacket)
{
	INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!iNetChan)
		return;

	if (G::DoubleTap || G::Teleport)
	{
		*pSendPacket = G::ShiftedGoal == G::ShiftedTicks;
		if ((G::DoubleTap || pCmd->buttons & IN_ATTACK) && iNetChan->m_nChokedPackets >= 21)
			*pSendPacket = true;
	}
}



bool CMisc::TauntControl(CUserCmd* pCmd, bool bInDuck)
{
	bool bReturn = true;
	// Handle Taunt Slide
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		if (Vars::Misc::TauntSlide.Value && pLocal->IsTaunting())
		{
			if (pCmd->buttons & IN_FORWARD)
			{
				pCmd->forwardmove = 450.f;
				pCmd->viewangles.x = 0.0f;
			}
			if (pCmd->buttons & IN_BACK)
			{
				pCmd->forwardmove = 450.f;
				pCmd->viewangles.x = 91.0f;
			}
			if (pCmd->buttons & IN_MOVELEFT)
				pCmd->sidemove = -450.f;
			if (pCmd->buttons & IN_MOVERIGHT)
				pCmd->sidemove = 450.f;

			if (!(pCmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK)))
				pCmd->viewangles.x = 90.0f;
				
			if (bInDuck)
				pCmd->buttons |= IN_DUCK;

			Vec3 vAngle = I::EngineClient->GetViewAngles();
			pCmd->viewangles.y = vAngle.y;

			bReturn = false;
		}
	}

	return bReturn;
}

#ifdef DEBUG
void CMisc::DumpClassIDS() {
	std::ofstream fDump("CLASSIDDUMP.txt");
	fDump << "enum struct ETFClassID\n{\n";
	CClientClass* ClientClass = I::BaseClientDLL->GetAllClasses();
	while (ClientClass) {
		fDump << "	" << ClientClass->GetName() << " = " << ClientClass->m_ClassID << ",\n";
		ClientClass = ClientClass->m_pNext;
	}
	fDump << "}";
	fDump.close();
}
#endif

void CMisc::UnlockAchievements()
{
	using FN = IAchievementMgr * (*)(void);
	const auto achievementmgr = GetVFunc<FN>(I::EngineClient, 114)();
	if (achievementmgr)
	{
		g_SteamInterfaces.UserStats->RequestCurrentStats();
		for (int i = 0; i < achievementmgr->GetAchievementCount(); i++)
			achievementmgr->AwardAchievement(achievementmgr->GetAchievementByIndex(i)->GetAchievementID());
		g_SteamInterfaces.UserStats->StoreStats();
		g_SteamInterfaces.UserStats->RequestCurrentStats();
	}
}

void CMisc::LockAchievements()
{
	using FN = IAchievementMgr * (*)(void);
	const auto achievementmgr = GetVFunc<FN>(I::EngineClient, 114)();
	if (achievementmgr)
	{
		g_SteamInterfaces.UserStats->RequestCurrentStats();
		for (int i = 0; i < achievementmgr->GetAchievementCount(); i++)
			g_SteamInterfaces.UserStats->ClearAchievement(achievementmgr->GetAchievementByIndex(i)->GetName());
		g_SteamInterfaces.UserStats->StoreStats();
		g_SteamInterfaces.UserStats->RequestCurrentStats();
	}
}

void CMisc::SteamRPC()
{
	if (!Vars::Misc::Steam::EnableRPC.Value)
	{
		if (SteamCleared == false) //stupid way to return back to normal rpc
		{
			g_SteamInterfaces.Friends->SetRichPresence("steam_display", "");
			//this will only make it say "Team Fortress 2" until the player leaves/joins some server. its bad but its better than making 1000 checks to recreate the original
			SteamCleared = true;
		}
		return;
	}

	SteamCleared = false;
	g_SteamInterfaces.Friends->SetRichPresence("steam_display", "#TF_RichPresence_Display");

	/*
	"TF_RichPresence_State_MainMenu"              "Main Menu"
	"TF_RichPresence_State_SearchingGeneric"      "Searching for a Match"
	"TF_RichPresence_State_SearchingMatchGroup"   "Searching - %matchgrouploc_token%"
	"TF_RichPresence_State_PlayingGeneric"        "In Match - %currentmap%"
	"TF_RichPresence_State_LoadingGeneric"        "Joining Match"
	"TF_RichPresence_State_PlayingMatchGroup"     "%matchgrouploc_token% - %currentmap%" <--!!!! used
	"TF_RichPresence_State_LoadingMatchGroup"     "Joining %matchgrouploc_token%"
	"TF_RichPresence_State_PlayingCommunity"      "Community - %currentmap%"
	"TF_RichPresence_State_LoadingCommunity"      "Joining Community Server"
	*/
	if (!I::EngineClient->IsInGame() && Vars::Misc::Steam::OverrideMenu.Value)
		g_SteamInterfaces.Friends->SetRichPresence("state", "MainMenu");
	else
	{
		g_SteamInterfaces.Friends->SetRichPresence("state", "PlayingMatchGroup");

		switch (Vars::Misc::Steam::MatchGroup.Value)
		{
		case 0: g_SteamInterfaces.Friends->SetRichPresence("matchgrouploc", "SpecialEvent"); break;
		case 1: g_SteamInterfaces.Friends->SetRichPresence("matchgrouploc", "MannUp"); break;
		case 2: g_SteamInterfaces.Friends->SetRichPresence("matchgrouploc", "Competitive6v6"); break;
		case 3: g_SteamInterfaces.Friends->SetRichPresence("matchgrouploc", "Casual"); break;
		case 4: g_SteamInterfaces.Friends->SetRichPresence("matchgrouploc", "BootCamp"); break;
		default: g_SteamInterfaces.Friends->SetRichPresence("matchgrouploc", "SpecialEvent"); break;
		}
	}

	/*
	"TF_RichPresence_MatchGroup_Competitive6v6"   "Competitive"
	"TF_RichPresence_MatchGroup_Casual"           "Casual"
	"TF_RichPresence_MatchGroup_SpecialEvent"     "Special Event"
	"TF_RichPresence_MatchGroup_MannUp"           "MvM Mann Up"
	"TF_RichPresence_MatchGroup_BootCamp"         "MvM Boot Camp"
	*/
	switch (Vars::Misc::Steam::MapText.Value)
	{
	case 0: g_SteamInterfaces.Friends->SetRichPresence("currentmap", Vars::Misc::Steam::CustomText.Value.empty() ? "Fedoraware" : Vars::Misc::Steam::CustomText.Value.c_str()); break;
	case 1: g_SteamInterfaces.Friends->SetRichPresence("currentmap", "Fedoraware"); break;
	case 2: g_SteamInterfaces.Friends->SetRichPresence("currentmap", "Figoraware"); break;
	case 3: g_SteamInterfaces.Friends->SetRichPresence("currentmap", "Meowhook.club"); break;
	case 4: g_SteamInterfaces.Friends->SetRichPresence("currentmap", "Rathook.cc"); break;
	case 5: g_SteamInterfaces.Friends->SetRichPresence("currentmap", "Nitro.tf"); break;
	default: g_SteamInterfaces.Friends->SetRichPresence("currentmap", "Fedoraware"); break;
	}

	g_SteamInterfaces.Friends->SetRichPresence("steam_player_group_size", std::to_string(Vars::Misc::Steam::GroupSize.Value).c_str());
}

/*
void CMisc::InstantRespawnMVM() {
	if (I::Engine->IsInGame() && I::Engine->GetLocalPlayer() && !g_EntityCache.GetLocal()->IsAlive() && Vars::Misc::MVMRes.m_Var) {
		auto kv = new KeyValues("MVM_Revive_Response");
		kv->SetInt("accepted", 1);
		I::Engine->ServerCmdKeyValues(kv);
	}
}
*/

void CMisc::PrintProjAngles(CBaseEntity* pLocal)
{
	if (!Vars::Debug::Logging.Value) { return; }
	if (!pLocal->IsAlive() || pLocal->IsAGhost()) { return; }
	static float flNextPrint = 0.f; if (flNextPrint > I::GlobalVars->curtime) { return; }
	const Vec3 vLocalEyeAngles = pLocal->GetEyeAngles();
	const Vec3 vLocalEyePosition = pLocal->GetEyePosition();
	for (CBaseEntity* pEntity : g_EntityCache.GetGroup(EGroupType::WORLD_PROJECTILES))
	{
		if (I::ClientEntityList->GetClientEntityFromHandle(pEntity->GethOwner()) != pLocal) { continue; }
		const Vec3 vProjAngles = pEntity->GetAbsAngles();
		const Vec3 vProjPosition = pEntity->GetAbsOrigin();

		const Vec3 vDeltaAng = vLocalEyeAngles - vProjAngles;
		const Vec3 vDeltaPos = vLocalEyePosition - vProjPosition;
		Utils::ConLog("ProjDebug", std::format("dAngles [{}, {}, {}] : dPosition [{}, {}, {}]", vDeltaAng.x, vDeltaAng.y, vDeltaAng.z, vDeltaPos.x, vDeltaPos.y, vDeltaPos.z).c_str(), { 255, 180, 0, 255 });
		flNextPrint = I::GlobalVars->curtime + 1.f;
	}
}