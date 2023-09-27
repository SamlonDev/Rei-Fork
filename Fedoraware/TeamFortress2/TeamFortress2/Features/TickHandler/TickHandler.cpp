#include "TickHandler.h"
#include "../../Hooks/HookManager.h"
#include "../../Hooks/Hooks.h"

void CTickshiftHandler::Speedhack(CUserCmd* pCmd)
{
	bSpeedhack = Vars::CL_Move::SpeedEnabled.Value;
	if (!bSpeedhack)
		return;

	G::Teleporting = false;
	G::Recharging = false;
	G::ShouldShift = false;
}

void CTickshiftHandler::Recharge(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	/*
	static KeyHelper kRecharge{ &Vars::CL_Move::RechargeKey.Value };
	G::Recharging = (((!G::Teleporting && !G::ShouldShift && !bSpeedhack) && (kRecharge.Down()) || G::RechargeQueued) || G::Recharging) && G::ShiftedTicks < Vars::CL_Move::DTTicks.Value;
	G::ShouldStop = (G::Recharging && Vars::CL_Move::StopMovement.Value) || G::ShouldStop;
	*/
	G::Recharge = false;

	if (Vars::CL_Move::DoubleTap::RechargeWhileDead.Value && !pLocal->IsAlive())
		G::Recharging = true;

	if (Vars::CL_Move::DoubleTap::AutoRecharge.Value && !G::ShouldShift && !G::Recharging && G::ShiftedTicks > Vars::CL_Move::DoubleTap::TickLimit.Value && pLocal->GetVecVelocity().Length2D() < 5.0f && !(pCmd->buttons))
		G::Recharging = true;


	bool bPassive = false;
	if (I::GlobalVars->tickcount >= iNextPassiveTick && Vars::CL_Move::DoubleTap::PassiveRecharge.Value)
	{
		bPassive = true;
		iNextPassiveTick = I::GlobalVars->tickcount + (iTickRate / Vars::CL_Move::DoubleTap::PassiveRecharge.Value);
	}

	if (iDeficit && G::ShiftedTicks < g_ConVars.sv_maxusrcmdprocessticks->GetInt() && Vars::CL_Move::DoubleTap::AutoRetain.Value)
	{
		bPassive = true;
		iDeficit--;
	}
	else if (iDeficit)
		iDeficit = 0;

	static KeyHelper kRecharge{ &Vars::CL_Move::DoubleTap::RechargeKey.Value };
	G::Recharge = (kRecharge.Down() || G::Recharging || bPassive) && !G::Teleporting && !G::ShouldShift && G::ShiftedTicks < Vars::CL_Move::DoubleTap::TickLimit.Value && !bSpeedhack;

	if (!G::Recharge)
		G::Recharging = false;
}

void CTickshiftHandler::Teleport(CUserCmd* pCmd)
{
	static KeyHelper kTeleport{ &Vars::CL_Move::DoubleTap::TeleportKey.Value };
	G::Teleporting = kTeleport.Down() && !G::Recharging && !G::ShouldShift && G::ShiftedTicks && !bSpeedhack;
}

void CTickshiftHandler::Doubletap(const CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (G::Teleporting || G::Recharging || bSpeedhack/*|| (G::ShiftedTicks < Vars::CL_Move::DTTicks.Value)*/)
		return;
	if (G::WaitForShift && Vars::CL_Move::DoubleTap::WaitReady.Value)
		return;
	if (G::ShouldShift || !pCmd || !G::ShiftedTicks)
		return;

	static KeyHelper kDoubletap{ &Vars::CL_Move::DoubleTap::DoubletapKey.Value };
	if (!kDoubletap.Down() && Vars::CL_Move::DoubleTap::Mode.Value == 1)
		return;

	if (G::WeaponCanAttack && (G::IsAttacking || G::CurWeaponType == EWeaponType::MELEE && pCmd->buttons & IN_ATTACK))
	{
		G::ShouldShift = Vars::CL_Move::DoubleTap::NotInAir.Value ? pLocal->OnSolid() : true;
	}
}

bool CTickshiftHandler::MeleeDoubletapCheck(CBaseEntity* pLocal) // this is dumb
{
	if (!Vars::CL_Move::DoubleTap::Enabled.Value)
		return false;

	if (G::Teleporting || G::Recharging || bSpeedhack/*|| (G::ShiftedTicks < Vars::CL_Move::DTTicks.Value)*/)
		return false;
	if (G::WaitForShift && Vars::CL_Move::DoubleTap::WaitReady.Value)
		return false;
	if (G::ShouldShift)
		return false;

	static KeyHelper kDoubletap{ &Vars::CL_Move::DoubleTap::DoubletapKey.Value };
	if (!kDoubletap.Down() && Vars::CL_Move::DoubleTap::Mode.Value == 1)
		return false;

	return Vars::CL_Move::DoubleTap::NotInAir.Value ? pLocal->OnSolid() : true;
}

void CTickshiftHandler::CLMoveFunc(float accumulated_extra_samples, bool bFinalTick)
{
	static auto CL_Move = g_HookManager.GetMapHooks()["CL_Move"];
	if (!CL_Move)
	{
		CL_Move = g_HookManager.GetMapHooks()["CL_Move"];
		return;
	}

	G::ShiftedTicks--;
	if (G::ShiftedTicks < 0)
		return;
	if (G::WaitForShift > 0)
		G::WaitForShift--;

	return CL_Move->Original<void(__cdecl*)(float, bool)>()(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::CLMove(float accumulated_extra_samples, bool bFinalTick)
{
	while (G::ShiftedTicks > Vars::CL_Move::DoubleTap::TickLimit.Value)
		CLMoveFunc(accumulated_extra_samples, false); // skim any excess ticks

	G::ShiftedTicks++; // since we now have full control over CL_Move, increment.
	if (G::ShiftedTicks <= 0)
	{
		G::ShiftedTicks = 0;
		return;
	} // ruhroh

	if (!Vars::CL_Move::DoubleTap::Enabled.Value || I::EngineClient->IsPlayingTimeDemo())
	{
		/* disabling as users can now toggle at will
		while (G::ShiftedTicks > 1)
			CLMoveFunc(accumulated_extra_samples, false);
		*/
		return CLMoveFunc(accumulated_extra_samples, true);
	}

	if (G::Recharge)
	{
		if (G::ShiftedTicks <= Vars::CL_Move::DoubleTap::TickLimit.Value)
		{
			static KeyHelper kRecharge{ &Vars::CL_Move::DoubleTap::RechargeKey.Value };
			if (!kRecharge.Down() && !G::Recharging)
				G::Recharge = false;

			G::WaitForShift = iTickRate - G::ShiftedTicks;
			return;
		}
		G::Recharging = G::Recharge = false;
	}

	if (G::Teleporting)
	{
		for (int i = 0; i < Vars::CL_Move::DoubleTap::WarpRate.Value; i++)
			CLMoveFunc(accumulated_extra_samples, i == Vars::CL_Move::DoubleTap::WarpRate.Value);
		return;
	}

	if (G::ShouldShift)
	{
		const int iStart = G::ShiftedTicks;
		while (G::ShiftedTicks && iStart - G::ShiftedTicks < 22)
			CLMoveFunc(accumulated_extra_samples, G::ShiftedTicks == 1);
		G::ShouldShift = false;
		return;
	}

	if (bSpeedhack)
	{
		G::ShiftedTicks = 0;
		for (int i = 0; i < Vars::CL_Move::SpeedFactor.Value; i++)
			CLMoveFunc(accumulated_extra_samples, i == Vars::CL_Move::SpeedFactor.Value);
		return;
	}

	return CLMoveFunc(accumulated_extra_samples, true);
}

void CTickshiftHandler::CreateMove(CUserCmd* pCmd)
{
	static KeyHelper kDoubletap{ &Vars::CL_Move::DoubleTap::DoubletapKey.Value };
	if (kDoubletap.Pressed() && Vars::CL_Move::DoubleTap::Mode.Value == 2)
		Vars::CL_Move::DoubleTap::Enabled.Value = !Vars::CL_Move::DoubleTap::Enabled.Value;

	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!Vars::CL_Move::DoubleTap::Enabled.Value || !pLocal)
		return;

	Recharge(pCmd, pLocal);
	Teleport(pCmd);
	Doubletap(pCmd, pLocal);
	Speedhack(pCmd);
}

void CTickshiftHandler::Reset()
{
	bSpeedhack = G::ShouldShift = G::Recharging = G::Teleporting = false;
	G::ShiftedTicks = 0;
	iNextPassiveTick = 0;
	iTickRate = round(1.f / I::GlobalVars->interval_per_tick);
}