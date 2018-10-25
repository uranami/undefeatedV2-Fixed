#pragma once
#include "stdafx.h"
#include "../Utils/Hitbox.h"
#include "Misc.h"
#include "../Utils/LocalInfo.h"
#include "../Utils/Playerlist.h"

int realAimSpot[] = { 8, 7, 6, 0 };
#define keystate(i) GetAsyncKeyState(i & 0x8000)
class Legit
{
private:
	float deltaTime;
	float curAimTime;
	int besttarget = -1;
	int besthitbox = -1;


	bool LineGoesThroughSmoke(Vector loc, Vector rem)
	{
		typedef bool(*LineGoesThroughSmokeFn)(Vector, Vector);

		LineGoesThroughSmokeFn LineGoesThroughSmoke = (LineGoesThroughSmokeFn)(Utils.PFindPattern("client.dll", "55 8B EC 83 EC 08 8B 15 ?? ?? ?? ?? 0F 57 C0"));

		return LineGoesThroughSmoke(loc, rem);
	}

	/*bool isBehindSmoke(Vector loc, Vector rem)
	{
		static bool initLGTS = false;
		if (!initLGTS)
		{
			Smoke = Utils.PFindPattern("client.dll", "\x55\x8B\xEC\x83\xEC\x08\x8B\x15\x00\x00\x00\x00\x0F\x57\xC0");
			initLGTS = true;
		}

		LineThroughSmoke = (LineThroughSmokeFn)(offs.lgtSmoke);

		return LineThroughSmoke(loc, rem, true);
	}*/

	bool EntityIsValid(int i, int hitbox)
	{
		auto pEntity = static_cast<CBaseEntity*> (Interfaces.pEntList->GetClientEntity(i));

		if (!pEntity)
			return false;
		if (pEntity == Hacks.LocalPlayer)
			return false;
		if (pEntity->GetHealth() <= 0)
			return false;
		if (pEntity->HasGunGameImmunity())
			return false;
		if (pEntity->IsDormant())
			return false;
		if (pEntity->GetTeam() == Hacks.LocalPlayer->GetTeam())
			return false;

		/*Vector VecAngle = pEntity->GetVecViewOffset();
		Vector VecOrigin = pEntity->GetVecOrigin();
		Vector b1g;
		b1g = VecAngle + VecOrigin;

		Vector omg = pEntity->GetBonePos(hitbox);

		if (LineGoesThroughSmoke(b1g, omg))
			return false;*/

		return true;
	}

	void GetBestTarget()
	{
		float bestfov = 9999999.f;

		for (int i = 0; i < Interfaces.pEntList->GetHighestEntityIndex(); i++)
		{
			if (!EntityIsValid(i, Settings.GetSetting(Tab_LegitBot, Legitbot_hitbox) + 1))
				continue;

			auto pEntity = static_cast<CBaseEntity*> (Interfaces.pEntList->GetClientEntity(i));
			if (!pEntity)
				continue;
			if (Settings.GetSetting(Tab_LegitBot, Legitbot_multihitbox))
			{
				for (int j = 0; j <= 27; j++)
				{
					if (!Misc::isVisible(pEntity, j))
						continue;
					//if (!RS::LineGoesThroughSmoke(Hacks.LocalPlayer, pEntity))
						//continue;
					float fov = Misc::GetFov(Hacks.CurrentCmd->viewangles + LocalInfo.PunchAns, Misc::CalcAngle(Hacks.LocalPlayer->GetEyePosition(), pEntity->GetBonePos(j)));
					if (fov > Settings.GetSetting(Tab_LegitBot, Legitbot_fov))
						continue;
					if (fov < bestfov)
					{
						bestfov = fov;
						besttarget = i;
						besthitbox = j;
					}
				}
			}
			else
			{
				if (!Misc::isVisible(pEntity, Settings.GetSetting(Tab_LegitBot, Legitbot_hitbox) + 1))
					continue;
				//if (!RS::LineGoesThroughSmoke(Hacks.LocalPlayer, pEntity))
					//continue;
				float fov = Misc::GetFov(Hacks.CurrentCmd->viewangles + LocalInfo.PunchAns, Misc::CalcAngle(Hacks.LocalPlayer->GetEyePosition(), pEntity->GetBonePos(Settings.GetSetting(Tab_LegitBot, Legitbot_hitbox) + 1)));
				if (fov > Settings.GetSetting(Tab_LegitBot, Legitbot_fov))
					continue;
				if (fov < bestfov)
				{
					bestfov = fov;
					besttarget = i;
					besthitbox = Settings.GetSetting(Tab_LegitBot, Legitbot_hitbox) + 1;
				}
			}

		}
		if (bestfov == 9999999.f)
		{
			besttarget = -1;
			besthitbox = -1;
		}
		return;

	}

	void GoToTarget(int target, int hitbox)
	{
		auto pEntity = static_cast<CBaseEntity*> (Interfaces.pEntList->GetClientEntity(target));
		if (!pEntity)
			return;
		Vector dst = Misc::CalcAngle(Hacks.LocalPlayer->GetEyePosition(), pEntity->GetBonePos(hitbox));
		Vector src = Hacks.CurrentCmd->viewangles;

		dst.x -= LocalInfo.PunchAns.x;
		dst.y -= LocalInfo.PunchAns.y;

		Vector delta = dst - src;

		delta.Normalize();

		if (!delta.IsZero())
		{
			float smoothX = Settings.GetSetting(Tab_LegitBot, Legitbot_smoothX);
			float finalTimeX = delta.Length();
			float smoothY = Settings.GetSetting(Tab_LegitBot, Legitbot_smoothY);
			float finalTimeY = delta.Length();

			if (smoothX != 0)
			{
				finalTimeX = delta.Length() / smoothX;
			}

			if (smoothY != 0)
			{
				finalTimeY = delta.Length() / smoothY;
			}

			float curAimTimeY = curAimTime;
			curAimTimeY += deltaTime;
			curAimTime += deltaTime;

			if (curAimTime > finalTimeX)
				curAimTime = finalTimeX;

			if (curAimTimeY > finalTimeY)
				curAimTimeY = finalTimeY;

			float percentX = curAimTime / finalTimeX;
			float percentY = curAimTimeY / finalTimeY;

			delta.x *= percentX;
			delta.y *= percentY;
			dst = src + delta;
		}

		Hacks.CurrentCmd->viewangles = dst.Normalize();
		Interfaces.pEngine->SetViewAngles(Hacks.CurrentCmd->viewangles);
	}

	Vector LastPuch;

	void RCS(Vector angles)
	{
		if (Settings.GetMenuSetting(Tab_LegitBot, Legitbot_enable))
		{
			CBaseEntity* LocalPlayer = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());

			Vector CurrtentPuch = LocalPlayer->GetPunchAngle();

			if (LocalPlayer->GetiShots() > 1)
			{
				Vector NewPuch = { CurrtentPuch.x - LastPuch.x, CurrtentPuch.y - LastPuch.y, CurrtentPuch.z - LastPuch.z };
				angles.x -= NewPuch.x * Settings.GetMenuSetting(Tab_LegitBot, Legitbot_rcsforceX);
				angles.y -= NewPuch.y * Settings.GetMenuSetting(Tab_LegitBot, Legitbot_rcsforceY);
			}

			LastPuch = CurrtentPuch;
		}
	}

public:
	void Run()
	{
		if (Hacks.LocalWeapon->IsMiscWeapon() || Hacks.LocalWeapon->IsKnife())
			return;
		if (Hacks.LocalWeapon->IsReloading())
			return;
		if (Menu.Opened)
			return;
		if ((!Settings.GetSetting(Tab_LegitBot, Legitbot_enable)) || (Settings.GetSetting(Tab_LegitBot, Legitbot_Aimonkey) && !(GetAsyncKeyState(Settings.GetSetting(Tab_LegitBot, Legitbot_Aimbotkey)) & 0x8000)))
			return;

		double cur_time = clock();
		static double time = clock();
		int fire_delay = Settings.GetSetting(Tab_LegitBot, Legitbot_Firedelay);
		if (fire_delay > 0)
		{
			if (Hacks.CurrentCmd->buttons & IN_ATTACK)
			{
				if (cur_time - time < fire_delay)
				{
					Hacks.CurrentCmd->buttons &= ~IN_ATTACK;
				}
			}
			else
			{
				time = clock();
			}
		}

		CBaseEntity* Target = Interfaces.pEntList->GetClientEntity(besttarget);

	

	

		curAimTime = 0.f;

		static float oldServerTime = Hacks.LocalPlayer->GetTickBase() * Interfaces.pGlobalVars->interval_per_tick;
		float serverTime = Hacks.LocalPlayer->GetTickBase() * Interfaces.pGlobalVars->interval_per_tick;
		deltaTime = serverTime - oldServerTime;
		oldServerTime = serverTime;

		if (Settings.GetMenuSetting(Tab_LegitBot, Legitbot_hitbox) == 0)
		{
			Settings.SetSetting(Tab_LegitBot, Legitbot_hitbox, 1);
		}

		static int do_or_not = 0;
		if (do_or_not == 0)
		{
			GetBestTarget();
		}
		else
		{
			do_or_not++;
			if (do_or_not > 15)
			{
				do_or_not = 0;
			}
		}

		if (besttarget == -1 || besthitbox == -1)
			return;
		GoToTarget(besttarget, besthitbox);
	}
}Legitbot;
