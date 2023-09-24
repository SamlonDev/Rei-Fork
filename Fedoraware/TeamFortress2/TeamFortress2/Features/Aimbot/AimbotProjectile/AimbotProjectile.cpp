#include "AimbotProjectile.h"
#include "../../Vars.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"
#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Visuals/Visuals.h"

/* Returns all valid targets */
std::vector<Target_t> CAimbotProjectile::GetTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	std::vector<Target_t> validTargets;
	const auto sortMethod = static_cast<ESortMethod>(Vars::Aimbot::Projectile::SortMethod.Value);

	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	// Players
	if (Vars::Aimbot::Global::AimAt.Value & (ToAimAt::PLAYER))
	{
		EGroupType groupType = EGroupType::PLAYERS_ENEMIES;
		if (pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW || SandvichAimbot::bIsSandvich)
			groupType = EGroupType::PLAYERS_ALL;
		else if (SandvichAimbot::bIsSandvich)
			groupType = EGroupType::PLAYERS_TEAMMATES;

		for (const auto& pTarget : g_EntityCache.GetGroup(groupType))
		{
			if (!pTarget->IsAlive() || pTarget->IsAGhost() || pTarget == pLocal)
			{
				continue;
			}

			// Check if weapon should shoot at friendly players
			if ((groupType == EGroupType::PLAYERS_ALL || groupType == EGroupType::PLAYERS_TEAMMATES) &&
				pTarget->GetTeamNum() == pLocal->GetTeamNum())
			{
				if (pTarget->GetHealth() >= pTarget->GetMaxHealth())
				{
					continue;
				}
			}

			if (pTarget->GetTeamNum() != pLocal->GetTeamNum())
			{
				if (F::AimbotGlobal.ShouldIgnore(pTarget)) { continue; }
			}

			Vec3 vPos = pTarget->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

			if (flFOVTo > Vars::Aimbot::Projectile::AimFOV.Value)
			{
				continue;
			}

			const float flDistTo = (sortMethod == ESortMethod::DISTANCE) ? vLocalPos.DistTo(vPos) : 0.0f;
			const auto& priority = F::AimbotGlobal.GetPriority(pTarget->GetIndex());
			validTargets.push_back({ pTarget, ETargetType::PLAYER, vPos, vAngleTo, flFOVTo, flDistTo, -1, false, priority });
		}
	}

	// Buildings
	const bool bIsRescueRanger = pWeapon->GetWeaponID() == TF_WEAPON_SHOTGUN_BUILDING_RESCUE;

	for (const auto& pBuilding : g_EntityCache.GetGroup(bIsRescueRanger ? EGroupType::BUILDINGS_ALL : EGroupType::BUILDINGS_ENEMIES))
	{
		bool isSentry = pBuilding->GetClassID() == ETFClassID::CObjectSentrygun;
		bool isDispenser = pBuilding->GetClassID() == ETFClassID::CObjectDispenser;
		bool isTeleporter = pBuilding->GetClassID() == ETFClassID::CObjectTeleporter;

		if (!(Vars::Aimbot::Global::AimAt.Value & (ToAimAt::SENTRY)) && isSentry) { continue; }
		if (!(Vars::Aimbot::Global::AimAt.Value & (ToAimAt::DISPENSER)) && isDispenser) { continue; }
		if (!(Vars::Aimbot::Global::AimAt.Value & (ToAimAt::TELEPORTER)) && isTeleporter) { continue; }

		const auto& Building = reinterpret_cast<CBaseObject*>(pBuilding);

		if (!pBuilding->IsAlive()) { continue; }

		// Check if the Rescue Ranger should shoot at friendly buildings
		if (bIsRescueRanger && (pBuilding->GetTeamNum() == pLocal->GetTeamNum()))
		{
			if (Building->GetHealth() >= Building->GetMaxHealth()) { continue; }
		}

		Vec3 vPos = pBuilding->GetWorldSpaceCenter();
		Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
		const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

		if (flFOVTo > Vars::Aimbot::Projectile::AimFOV.Value)
		{
			continue;
		}
		const float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.0f;
		validTargets.push_back({ pBuilding, isSentry ? ETargetType::SENTRY : (isDispenser ? ETargetType::DISPENSER : ETargetType::TELEPORTER), vPos, vAngleTo, flFOVTo, flDistTo });
	}

	// NPCs
	if (Vars::Aimbot::Global::AimAt.Value & (ToAimAt::NPC))
	{
		for (const auto& NPC : g_EntityCache.GetGroup(EGroupType::WORLD_NPC))
		{
			Vec3 vPos = NPC->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);

			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			const float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.0f;

			if (flFOVTo > Vars::Aimbot::Projectile::AimFOV.Value)
			{
				continue;
			}

			validTargets.push_back({ NPC, ETargetType::NPC, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	//Bombs
	if (Vars::Aimbot::Global::AimAt.Value & (ToAimAt::BOMB))
	{
		//This is pretty bad with projectiles
		for (const auto& Bombs : g_EntityCache.GetGroup(EGroupType::WORLD_BOMBS))
		{
			Vec3 vPos = Bombs->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);

			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			const float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.0f;

			if (flFOVTo > Vars::Aimbot::Projectile::AimFOV.Value)
			{
				continue;
			}

			validTargets.push_back({ Bombs, ETargetType::BOMBS, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	return validTargets;
}

std::vector<Target_t> CAimbotProjectile::SortTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	auto validTargets = GetTargets(pLocal, pWeapon);

	const auto& sortMethod = static_cast<ESortMethod>(Vars::Aimbot::Projectile::SortMethod.Value);
	F::AimbotGlobal.SortTargets(&validTargets, sortMethod);
	F::AimbotGlobal.SortPriority(&validTargets, sortMethod);

	std::vector<Target_t> sortedTargets = {};
	int i = 0; for (auto& target : validTargets)
	{
		i++; if (i > Vars::Aimbot::Global::MaxTargets.Value) break;

		sortedTargets.push_back(target);
	}

	return sortedTargets;
}



bool CanSplash(CBaseCombatWeapon* pWeapon)
{
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_PARTICLE_CANNON:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_PIPEBOMBLAUNCHER: return true;
	}

	return false;
}

int CAimbotProjectile::GetHitboxPriority(int nHitbox, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& target)
{
	bool bHeadshot = target.m_TargetType == ETargetType::PLAYER && pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW;
	if (Vars::Aimbot::Global::BAimLethal.Value && bHeadshot)
	{
		float charge = I::GlobalVars->curtime - pWeapon->GetChargeBeginTime();
		float damage = Math::RemapValClamped(charge, 0.f, 1.f, 50.f, 120.f);
		if (pLocal->IsMiniCritBoosted())
			damage *= 1.36f;
		if (damage >= target.m_pEntity->GetHealth())
			bHeadshot = false;

		if (pLocal->IsCritBoosted()) // for reliability
			bHeadshot = false;
	}
	const bool bLower = target.m_pEntity->m_fFlags() & FL_ONGROUND && CanSplash(pWeapon);

	if (bHeadshot)
		target.m_nAimedHitbox = HITBOX_HEAD;

	switch (nHitbox)
	{
	case 0: return bHeadshot ? 0 : 2; // head
	case 1: return bHeadshot ? 3 : (bLower ? 1 : 0); // body
	case 2: return bHeadshot ? 3 : (bLower ? 0 : 1); // feet
	}

	return 3;
};

bool CAimbotProjectile::CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, const float& flVelocity, const float& flGravity, Solution_t& out)
{
	const Vec3 vDelta = vTargetPos - vLocalPos;
	const float flDist = vDelta.Length2D();
	const float flHeight = vDelta.z;
	const float flGrav = g_ConVars.sv_gravity->GetFloat() * flGravity;

	if (!flGrav)
	{
		const Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vTargetPos);
		out.m_flPitch = -DEG2RAD(vAngleTo.x);
		out.m_flYaw = DEG2RAD(vAngleTo.y);
	}
	else
	{ // arch
		const float fRoot = pow(flVelocity, 4) - flGrav * (flGrav * pow(flDist, 2) + 2.f * flHeight * pow(flVelocity, 2));
		if (fRoot < 0.f)
			return false;
		out.m_flPitch = atan((pow(flVelocity, 2) - sqrt(fRoot)) / (flGrav * flDist));
		out.m_flYaw = atan2(vDelta.y, vDelta.x);
	}
	out.m_flTime = flDist / (cos(out.m_flPitch) * flVelocity);

	return true;
}

bool CAimbotProjectile::TestAngle(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& target, const Vec3& vOriginal, const Vec3& vPredict, const Vec3& vAngles, const int& iSimTime)
{
	target.m_pEntity->SetAbsOrigin(vPredict);

	ProjectileInfo projInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, vAngles, projInfo))
		return false;

	if (!F::ProjSim.Initialize(projInfo))
		return false;

	for (int n = 0; n < iSimTime; n++) {
		Vec3 Old = F::ProjSim.GetOrigin();
		F::ProjSim.RunTick(projInfo);
		Vec3 New = F::ProjSim.GetOrigin();

		CGameTrace trace = {};
		CTraceFilterHitscan filter = {};
		filter.pSkip = pLocal;
		Utils::TraceHull(Old, New, projInfo.m_hull * -1.f, projInfo.m_hull, MASK_SOLID, &filter, &trace);

		if (trace.DidHit())
		{
			target.m_pEntity->SetAbsOrigin(vOriginal);

			if (trace.entity && trace.entity == target.m_pEntity)
			{
				G::ProjLines = projInfo.PredictionLines;
				G::ProjLines.push_back({ trace.vEndPos, Math::GetRotatedPosition(trace.vEndPos, Math::VelocityToAngles(F::ProjSim.GetVelocity() * Vec3(1, 1, 0)).Length2D() + 90, Vars::Visuals::SeperatorLength.Value) });

				if (target.m_nAimedHitbox == HITBOX_HEAD)
				{
					const Vec3 vOffset = (trace.vEndPos - New) + (vOriginal - vPredict);

					Old = F::ProjSim.GetOrigin() + vOffset;
					F::ProjSim.RunTick(projInfo);
					New = F::ProjSim.GetOrigin() + vOffset;

					Utils::Trace(Old, New, MASK_SHOT, &filter, &trace);
					trace.vEndPos -= vOffset;

					if (trace.DidHit() && !(trace.entity && trace.entity == target.m_pEntity && trace.hitbox == HITBOX_HEAD))
						return false;
					
					// possibly add loop for closest hitbox
				}

				return true;
			}
			return false;
		}
	}

	target.m_pEntity->SetAbsOrigin(vOriginal);

	return false;
}

bool CAimbotProjectile::CanHit(Target_t& target, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	if (Vars::Aimbot::Global::IgnoreOptions.Value & (1 << 6) && G::ChokeMap[target.m_pEntity->GetIndex()] > Vars::Aimbot::Global::TickTolerance.Value)
		return false;

	const INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!iNetChan)
		return false;

	ProjectileInfo projInfo = {};

	const bool bPlayer = target.m_TargetType == ETargetType::PLAYER;
	const Vec3 vEyePos = pLocal->GetShootPos();
	const Vec3 vOriginalPos = target.m_pEntity->GetAbsOrigin();
	Vec3 vTargetPos = vOriginalPos, vOffset = {};
	float flMaxTime = Vars::Aimbot::Projectile::PredictionTime.Value;
	{
		if (!F::ProjSim.GetInfo(pLocal, pWeapon, {}, projInfo))
			return false;

		if (flMaxTime > projInfo.m_lifetime)
			flMaxTime = projInfo.m_lifetime;
	}
	//const int iLatency = TIME_TO_TICKS(F::Backtrack.GetReal());
	//const float flLatency = iNetChan->GetLatency(FLOW_OUTGOING) + I::GlobalVars->interval_per_tick;
	const float flLatency = iNetChan->GetLatency(FLOW_OUTGOING);
	const float latOff = I::GlobalVars->interval_per_tick * Vars::Aimbot::Projectile::LatOff.Value;
	const float phyOff = I::GlobalVars->interval_per_tick * Vars::Aimbot::Projectile::PhyOff.Value;

	switch (pWeapon->GetWeaponID()) // fix for this use
	{
	case TF_WEAPON_PARTICLE_CANNON: // temp
		Utils::ConLog("Particle cannon", " ", {255, 255, 255, 255}); break;
	case TF_WEAPON_CANNON:
	case TF_WEAPON_GRENADELAUNCHER: projInfo.m_gravity = 0.75f; break;
	case TF_WEAPON_PIPEBOMBLAUNCHER: {
		float charge = (I::GlobalVars->curtime - pWeapon->GetChargeBeginTime());
		projInfo.m_gravity = Math::RemapValClamped(charge, 0.0f, Utils::ATTRIB_HOOK_FLOAT(4.0f, "stickybomb_charge_rate", pWeapon), 0.5f, 0.1f);
		break;
	}
	}

	std::vector<Vec3> vPoints = {};
	{
		const bool bIsDucking = (target.m_pEntity->m_fFlags() & FL_DUCKING);
		const Vec3 vMins = I::GameMovement->GetPlayerMins(bIsDucking), vMaxs = I::GameMovement->GetPlayerMaxs(bIsDucking);

		for (int i = 0; i < 3; i++) {
			for (int n = 0; n < 3; n++) {
				if (GetHitboxPriority(n, pLocal, pWeapon, target) == i)
				{
					switch (n)
					{
					case 0: vPoints.push_back(bPlayer && target.m_nAimedHitbox == HITBOX_HEAD ?
											  Utils::GetHeadOffset(target.m_pEntity, { 0, -6.f, 0 }) : // i think this is the right axis
											  Vec3(0, 0, vMaxs.z - 10.f)); break;
					case 1: vPoints.push_back(Vec3(0, 0, (vMins.z + vMaxs.z) / 2)); break;
					case 2: vPoints.push_back(Vec3(0, 0, vMins.z + 10.f)); break;
					}
				}
			}
		}
	}

	bool bSimulate = true;

	PlayerStorage storage;
	if (!F::MoveSim.Initialize(target.m_pEntity, storage))
		bSimulate = false;

	Vec3 vAngleTo;
	int i = 0, iLowestPriority = 3, iEndTick = 0; // time to point valid, end in some ticks
	for (;i < TIME_TO_TICKS(flMaxTime); i++)
	{
		if (bSimulate)
		{
			F::MoveSim.RunTick(storage);
			vTargetPos = storage.m_MoveData.m_vecAbsOrigin;
		}
		
		// demoman angle offset
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_GRENADELAUNCHER:
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_STICKBOMB:
		case TF_WEAPON_STICKY_BALL_LAUNCHER:
		if (bSimulate || !bSimulate && !i)
		{
			Vec3 vDelta = (vTargetPos - vEyePos);
			const float fRange = Math::VectorNormalize(vDelta);
			const float fElevationAngle = std::min(fRange * (G::CurItemDefIndex == Demoman_m_TheLochnLoad ? 0.0075f : 0.013f), 45.f);
			// if our angle is above 45 degree will we even hit them? shouldn't we just return???

			float sin = 0.0f, cos = 0.0f;
			Math::SinCos((fElevationAngle * PI / 180.0f), &sin, &cos);

			const float fElevation = (fRange * (sin / cos));
			vOffset.z = (cos > 0.0f ? fElevation : 0.0f);
		}
		break;
		}

		// actually test points
		Solution_t solution;

		int iPriority = -1;
		for (const auto& vPoint : vPoints) // get most ideal point
		{
			iPriority++;

			if (iPriority == iLowestPriority || !iLowestPriority)
				break;

			if (!CalculateAngle(vEyePos, vTargetPos + vOffset + vPoint, projInfo.m_velocity, projInfo.m_gravity, solution))
				continue;

			if (!iEndTick)
			{
				if (solution.m_flTime + flLatency + latOff > TICKS_TO_TIME(i)) // TICKS_TO_TIME(flLatency)
					continue;
				iEndTick = i + vPoints.size() - 1;
			}

			const Vec3 vAngles = Aim(G::CurrentUserCmd->viewangles, { -RAD2DEG(solution.m_flPitch), RAD2DEG(solution.m_flYaw), 0.0f });
			if (TestAngle(pLocal, pWeapon, target, vOriginalPos, vTargetPos, vAngles, i - TIME_TO_TICKS(flLatency + phyOff)))
			{
				iLowestPriority = iPriority;
				vAngleTo = vAngles;
			}
		}

		if (iEndTick && iEndTick == i || !iLowestPriority)
			break;
	}

	G::MoveLines = storage.PredictionLines;

	if (bSimulate)
		F::MoveSim.Restore(storage);

	if (iLowestPriority != 3 &&
		(target.m_TargetType != ETargetType::PLAYER ||
		target.m_TargetType == ETargetType::PLAYER && bSimulate)) // don't attempt to aim at players when movesim fails
	{
		target.m_vAngleTo = vAngleTo;
		if (Vars::Aimbot::Global::ShowHitboxes.Value)
		{
			F::Visuals.DrawHitbox(target.m_pEntity, vTargetPos, I::GlobalVars->curtime + TICKS_TO_TIME(i));

			if (target.m_nAimedHitbox == HITBOX_HEAD) // huntsman head
			{
				const Vec3 vOriginOffset = target.m_pEntity->GetAbsOrigin() - vTargetPos;

				const model_t* pModel = target.m_pEntity->GetModel();
				if (!pModel) return true;
				const studiohdr_t* pHDR = I::ModelInfoClient->GetStudioModel(pModel);
				if (!pHDR) return true;
				const mstudiohitboxset_t* pSet = pHDR->GetHitboxSet(target.m_pEntity->GetHitboxSet());
				if (!pSet) return true;

				matrix3x4 BoneMatrix[128];
				if (!target.m_pEntity->SetupBones(BoneMatrix, 128, BONE_USED_BY_ANYTHING, target.m_pEntity->GetSimulationTime()))
					return true;

				const mstudiobbox_t* bbox = pSet->hitbox(HITBOX_HEAD);
				if (!bbox)
					return true;

				matrix3x4 rotMatrix;
				Math::AngleMatrix(bbox->angle, rotMatrix);

				matrix3x4 matrix;
				Math::ConcatTransforms(BoneMatrix[bbox->bone], rotMatrix, matrix);

				Vec3 bboxAngle;
				Math::MatrixAngles(matrix, bboxAngle);

				Vec3 matrixOrigin;
				Math::GetMatrixOrigin(matrix, matrixOrigin);

				G::BoxesStorage.push_back({ matrixOrigin - vOriginOffset, bbox->bbmin, bbox->bbmax, bboxAngle, I::GlobalVars->curtime + TICKS_TO_TIME(i), Colors::HitboxEdge, Colors::HitboxFace });
			}
		}
		return true;
	}

	return false;
}



bool CAimbotProjectile::IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon)
{
	if (G::CurItemDefIndex == Soldier_m_TheBeggarsBazooka)
	{
		static bool bLoading = false, bFiring = false;

		if (pWeapon->GetClip1() == 0)
			bLoading = false,
			bFiring = false;
		else if (!bFiring)
			bLoading = true;

		if ((bFiring || bLoading && !(pCmd->buttons & IN_ATTACK)) && G::WeaponCanAttack)
		{
			bFiring = true;
			bLoading = false;
			return true;
		}
	}
	else
	{
		if (pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW || pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
		{
			static bool bCharging = false;

			if (pWeapon->GetChargeBeginTime() > 0.0f)
			{
				bCharging = true;
			}

			if (!(pCmd->buttons & IN_ATTACK) && bCharging)
			{
				bCharging = false;
				return true;
			}
		}
		else if (pWeapon->GetWeaponID() == TF_WEAPON_CANNON)
		{
			static bool Charging = false;

			if (pWeapon->GetDetonateTime() > 0.0f)
			{
				Charging = true;
			}

			if (!(pCmd->buttons & IN_ATTACK) && Charging)
			{
				Charging = false;
				return true;
			}
		}

		//pssst..
		//Dragon's Fury has a gauge (seen on the weapon model) maybe it would help for pSilent hmm..
		/*
		if (pWeapon->GetWeaponID() == 109) {
		}*/

		else
		{
			if ((pCmd->buttons & IN_ATTACK) && G::WeaponCanAttack)
			{
				return true;
			}
		}
	}

	return false;
}

// assume angle calculated outside with other overload
void CAimbotProjectile::Aim(CUserCmd* pCmd, Vec3& vAngle)
{
	if (Vars::Aimbot::Projectile::AimMethod.Value != 2)
	{
		pCmd->viewangles = vAngle;
		I::EngineClient->SetViewAngles(pCmd->viewangles);
	}
	else if (G::IsAttacking)
	{
		pCmd->viewangles = vAngle;
		Utils::FixMovement(pCmd, pCmd->viewangles);
		//if (bFlameThrower)
		//	G::UpdateView = false;
		//else
			G::SilentTime = true;
	}
}

Vec3 CAimbotProjectile::Aim(Vec3 vCurAngle, Vec3 vToAngle)
{
	Vec3 vReturn = {};

	vToAngle -= G::PunchAngles;
	Math::ClampAngles(vToAngle);

	switch (Vars::Aimbot::Projectile::AimMethod.Value)
	{
	case 0: // Plain
		vReturn = vToAngle;
		break;

	case 1: //Smooth
		if (Vars::Aimbot::Projectile::SmoothingAmount.Value == 0)
		{ // plain aim at 0 smoothing factor
			vReturn = vToAngle;
			break;
		}
		//a + (b - a) * t [lerp]
		vReturn = vCurAngle + (vToAngle - vCurAngle) * (1.f - (float)Vars::Aimbot::Projectile::SmoothingAmount.Value / 100.f);
		break;

	case 2: // Silent
		vReturn = vToAngle;
		break;

	default: break;
	}

	return vReturn;
}

void CAimbotProjectile::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	const bool bAutomatic = pWeapon->IsStreamingWeapon(), bKeepFiring = bAutomatic && bLastTickAttack;
	if (!Vars::Aimbot::Global::Active.Value || !Vars::Aimbot::Projectile::Active.Value || !G::WeaponCanAttack && Vars::Aimbot::Projectile::AimMethod.Value == 2 && !G::ShouldShift && !bKeepFiring)
	{
		Exit(pLocal, pWeapon, pCmd); return;
	}

	const bool bShouldAim = (Vars::Aimbot::Global::AimKey.Value == VK_LBUTTON ? (pCmd->buttons & IN_ATTACK) : F::AimbotGlobal.IsKeyDown());
	if (!bShouldAim &&
		(Vars::Aimbot::Global::AutoShoot.Value ||
		!Vars::Aimbot::Global::AutoShoot.Value && !bLastTickAttack))
	{
		Exit(pLocal, pWeapon, pCmd); return;
	}

	const int nWeaponID = pWeapon->GetWeaponID();

	auto targets = SortTargets(pLocal, pWeapon);
	if (targets.empty())
	{
		Exit(pLocal, pWeapon, pCmd); return;
	}

	if (bShouldAim && (nWeaponID == TF_WEAPON_COMPOUND_BOW ||
		nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER || nWeaponID == TF_WEAPON_CANNON))
		pCmd->buttons |= IN_ATTACK;

	for (auto& target : targets)
	{
		if (SandvichAimbot::bIsSandvich)
		{
			SandvichAimbot::RunSandvichAimbot(pLocal, pWeapon, pCmd, target.m_pEntity);
			return;
		}

		if (!CanHit(target, pLocal, pWeapon)) continue;

		G::CurrentTargetIdx = target.m_pEntity->GetIndex();
		if (Vars::Aimbot::Projectile::AimMethod.Value == 1)
		{
			G::AimPos = target.m_vPos;
		}

		if (Vars::Aimbot::Global::AutoShoot.Value)
		{
			pCmd->buttons |= IN_ATTACK;

			if (G::CurItemDefIndex == Soldier_m_TheBeggarsBazooka)
			{
				if (pWeapon->GetClip1() > 0)
				{
					pCmd->buttons &= ~IN_ATTACK;
				}
			}
			else
			{
				if ((nWeaponID == TF_WEAPON_COMPOUND_BOW || nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER)
					&& pWeapon->GetChargeBeginTime() > 0.0f)
				{
					pCmd->buttons &= ~IN_ATTACK;
				}
				else if (nWeaponID == TF_WEAPON_CANNON && pWeapon->GetDetonateTime() > 0.0f)
				{
					const Vec3 vEyePos = pLocal->GetShootPos();
					const float bestCharge = vEyePos.DistTo(G::PredictedPos) / 1454.0f;

					if (Vars::Aimbot::Projectile::ChargeLooseCannon.Value)
					{
						if (target.m_TargetType == ETargetType::SENTRY ||
							target.m_TargetType == ETargetType::DISPENSER ||
							target.m_TargetType == ETargetType::TELEPORTER ||
							target.m_TargetType == ETargetType::BOMBS) // please DONT double donk buildings or bombs
						{
							pCmd->buttons &= ~IN_ATTACK;
						}

						if (target.m_pEntity->GetHealth() > 50) // check if we even need to double donk to kill first
						{
							if (pWeapon->GetDetonateTime() - I::GlobalVars->curtime <= bestCharge)
							{
								pCmd->buttons &= ~IN_ATTACK;
							}
						}
						else
						{
							pCmd->buttons &= ~IN_ATTACK;
						}
					}
					else
					{
						pCmd->buttons &= ~IN_ATTACK;
					}
				}
			}
		}

		G::IsAttacking = IsAttacking(pCmd, pWeapon);

		if ((G::IsAttacking || !Vars::Aimbot::Global::AutoShoot.Value) && Vars::Visuals::SimLines.Value)
		{
			F::Visuals.ClearBulletLines();
			G::LinesStorage.clear();

			G::LinesStorage.push_back({ G::MoveLines, Vars::Visuals::ClearLines.Value ? -1.f : I::GlobalVars->curtime + 5.f, Vars::Aimbot::Projectile::PredictionColor});
			if (G::IsAttacking)
				G::LinesStorage.push_back({ G::ProjLines, Vars::Visuals::ClearLines.Value ? -1.f - F::Backtrack.GetReal() : I::GlobalVars->curtime + 5.f, Vars::Aimbot::Projectile::ProjectileColor});
		}

		Aim(pCmd, target.m_vAngleTo);

		break;
	}

	Exit(pLocal, pWeapon, pCmd, false);
}

void CAimbotProjectile::Exit(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, bool bEarly)
{
	const float charge = I::GlobalVars->curtime - pWeapon->GetChargeBeginTime();
	const float amount = Math::RemapValClamped(charge, 0.f, Utils::ATTRIB_HOOK_FLOAT(4.0f, "stickybomb_charge_rate", pWeapon), 0.f, 1.f);
	const bool bCancel = amount > 0.95f && pWeapon->GetWeaponID() != TF_WEAPON_COMPOUND_BOW;

	if ((bCancel || bEarly) && bLastTickAttack && !(pCmd->buttons & IN_ATTACK) && Vars::Aimbot::Global::AutoShoot.Value) // add user toggle to control whether to cancel or not
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_COMPOUND_BOW:
			pCmd->buttons |= IN_ATTACK2;
			pCmd->buttons &= ~IN_ATTACK;
			break;
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_CANNON:
			pCmd->weaponselect = pLocal->GetWeaponFromSlot(SLOT_MELEE)->GetIndex();
			bLastTickCancel = true;
			pCmd->buttons &= ~IN_ATTACK;
		}
	}

	bLastTickAttack = false;
	if (bEarly)
		return;
	bLastTickAttack = pCmd->buttons & IN_ATTACK;
}