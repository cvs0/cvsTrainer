/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019
*/

#include "script.h"
#include "scriptmenu.h"
#include "keyboard.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <ctime>

using namespace std;

#include "scriptinfo.h"

void PlayAudio(const char* audioName, Entity entity, const char* audioRef, BOOL isNetwork, Any p4, Any p5)
{
	AUDIO::PLAY_SOUND_FROM_ENTITY(audioName, entity, audioRef, isNetwork, p4, p5);
}

class MenuItemPlayerFastHeal : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (!newState)
			PLAYER::SET_PLAYER_HEALTH_RECHARGE_MULTIPLIER(PLAYER::PLAYER_ID(), 1.0);
		SetState(newState);
	}
	virtual void OnFrame()
	{
		if (GetState())
			PLAYER::SET_PLAYER_HEALTH_RECHARGE_MULTIPLIER(PLAYER::PLAYER_ID(), 1000.0);
	}
public:
	MenuItemPlayerFastHeal(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemPlayerFix : public MenuItemDefault
{
	virtual void OnSelect()
	{
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		ENTITY::SET_ENTITY_HEALTH(playerPed, ENTITY::GET_ENTITY_MAX_HEALTH(playerPed, FALSE), FALSE);
		PED::CLEAR_PED_WETNESS(playerPed);
		PLAYER::RESTORE_PLAYER_STAMINA(PLAYER::PLAYER_ID(), 100.0);
		PLAYER::RESTORE_SPECIAL_ABILITY(PLAYER::PLAYER_ID(), -1, FALSE);

		if (PED::IS_PED_ON_MOUNT(playerPed))
		{
			Ped horse = PED::GET_MOUNT(playerPed);
			ENTITY::SET_ENTITY_HEALTH(horse, ENTITY::GET_ENTITY_MAX_HEALTH(horse, FALSE), FALSE);
			PED::SET_PED_STAMINA(horse, 100.0);
			SetStatusText("player and horse fixed");
		}
		else
			if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, FALSE))
			{
				Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
				ENTITY::SET_ENTITY_HEALTH(veh, ENTITY::GET_ENTITY_MAX_HEALTH(veh, FALSE), FALSE);
				SetStatusText("player and vehicle fixed");
			}
			else
				SetStatusText("player fixed");
	}
public:
	MenuItemPlayerFix(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemHorseRagdoll : public MenuItemDefault
{
	int m_time_min;
	int m_time_max;
	bool m_stop_injured;
	bool m_stop_dead;

	virtual void OnSelect()
	{
		Player player = PLAYER::PLAYER_ID();
		Ped horse = PLAYER::_GET_ACTIVE_HORSE_FOR_PLAYER(player);

		PED::SET_PED_TO_RAGDOLL(horse, m_time_min, m_time_max, 0, m_stop_injured, m_stop_dead, 0);
	}
public:
	MenuItemHorseRagdoll(string caption, int time_min, int time_max, bool stopOnInjury, bool stopOnDeath)
		: MenuItemDefault(caption),
		m_time_min(time_min),
		m_time_max(time_max),
		m_stop_injured(stopOnInjury),
		m_stop_dead(stopOnDeath) {}
};

class MenuItemVehicleBoost : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (newState)
			SetStatusText("PAGEUP / NUM9\nPAGEDOWN / NUM6");
		SetState(newState);
	}

	virtual void OnFrame()
	{
		if (!GetState())
			return;

		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (!PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
			return;

		Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
		if (!ENTITY::DOES_ENTITY_EXIST(veh))
			return;

		DWORD model = ENTITY::GET_ENTITY_MODEL(veh);
		BOOL bTrain = VEHICLE::IS_THIS_MODEL_A_TRAIN(model);

		bool bUp = IsKeyDownLong(VK_NUMPAD9) || IsKeyDownLong(VK_PRIOR);
		bool bDown = IsKeyDown(VK_NUMPAD3) || IsKeyDown(VK_NEXT);

		if (!(bUp || bDown))
			return;

		if (bTrain)
		{
			float speed = bUp ? 30.0f : 0.0f;
			VEHICLE::SET_TRAIN_SPEED(veh, speed);
			VEHICLE::SET_TRAIN_CRUISE_SPEED(veh, speed);
			return;
		}

		float speed = ENTITY::GET_ENTITY_SPEED(veh);
		if (bUp)
		{
			if (speed < 3.0f) speed = 3.0f;
			speed += speed * 0.03f;
			VEHICLE::SET_VEHICLE_FORWARD_SPEED(veh, speed);
		}
		else
		{
			if (ENTITY::IS_ENTITY_IN_AIR(veh, 0) || speed > 5.0)
				VEHICLE::SET_VEHICLE_FORWARD_SPEED(veh, 0.0);
		}
	}

public:
	MenuItemVehicleBoost(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemPlayerAddCash : public MenuItemDefault
{
	int m_value;
	virtual void OnSelect() { CASH::PLAYER_ADD_CASH(m_value, 0x2cd419dc); }
public:
	MenuItemPlayerAddCash(string caption, int value)
		: MenuItemDefault(caption),
		m_value(value) {}
};

class MenuItemPlayerRagdoll : public MenuItemDefault
{
	int m_time_min;
	int m_time_max;
	bool stopIfInjured;
	bool stopIfDead;

	virtual void OnSelect() { PED::SET_PED_TO_RAGDOLL(PLAYER::PLAYER_PED_ID(), m_time_min, m_time_max, 0, stopIfInjured, stopIfDead, 0); }
public:
	MenuItemPlayerRagdoll(string caption, int time_min, int time_max, bool stopOnInjury, bool stopOnDeath)
		: MenuItemDefault(caption),
		m_time_min(time_min),
		m_time_max(time_max),
		stopIfInjured(stopOnInjury),
		stopIfDead(stopOnDeath) {}
};

class MenuItemPlayerInvincible : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (!newState)
			PLAYER::SET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID(), FALSE);
		SetState(newState);
	}
	virtual void OnFrame()
	{
		if (GetState())
			PLAYER::SET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID(), TRUE);
	}
public:
	MenuItemPlayerInvincible(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemPlayerHorseInvincible : public MenuItemSwitchable
{
	void SetPlayerHorseInvincible(bool set)
	{
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (PED::IS_PED_ON_MOUNT(playerPed))
		{
			Ped horse = PED::GET_MOUNT(playerPed);
			ENTITY::SET_ENTITY_INVINCIBLE(horse, set);
		}
	}
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (!newState)
			SetPlayerHorseInvincible(false);
		SetState(newState);
	}
	virtual void OnFrame()
	{
		if (GetState())
			SetPlayerHorseInvincible(true);
	}
public:
	MenuItemPlayerHorseInvincible(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemPlayerUnlimStamina : public MenuItemSwitchable
{
	virtual void OnFrame()
	{
		if (GetState())
			PLAYER::RESTORE_PLAYER_STAMINA(PLAYER::PLAYER_ID(), 100.0);
	}
public:
	MenuItemPlayerUnlimStamina(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemPlayerUnlimAbility : public MenuItemSwitchable
{
	virtual void OnFrame()
	{
		if (GetState())
			PLAYER::RESTORE_SPECIAL_ABILITY(PLAYER::PLAYER_ID(), -1, FALSE);
	}
public:
	MenuItemPlayerUnlimAbility(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemPlayerDrunk : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (!newState) {
			Ped playerPed = PLAYER::PLAYER_PED_ID();

			AUDIO::SET_PED_IS_DRUNK(playerPed, FALSE);
			PED::_SET_PED_DRUNKNESS(playerPed, FALSE, 0.0f);
		}
		SetState(newState);
	}

	virtual void OnFrame()
	{
		if (GetState())
		{
			Ped playerPed = PLAYER::PLAYER_PED_ID();

			AUDIO::SET_PED_IS_DRUNK(playerPed, TRUE);
			PED::_SET_PED_DRUNKNESS(playerPed, TRUE, 1.0f);
		}
	}
public:
	MenuItemPlayerDrunk(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemNoRagdoll : public MenuItemSwitchable
{
	virtual void OnFrame()
	{
		if (GetState())
		{
			if (PED::IS_PED_RAGDOLL(PLAYER::PLAYER_PED_ID())) {
				PED::SET_PED_TO_RAGDOLL(PLAYER::PLAYER_PED_ID(), 0, 0, 0, FALSE, FALSE, FALSE);
			}
		}
	}
public:
	MenuItemNoRagdoll(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemPlayerHorseUnlimStamina : public MenuItemSwitchable
{
	virtual void OnFrame()
	{
		if (GetState())
		{
			Ped playerPed = PLAYER::PLAYER_PED_ID();
			if (PED::IS_PED_ON_MOUNT(playerPed))
			{
				Ped horse = PED::GET_MOUNT(playerPed);
				PED::SET_PED_STAMINA(horse, 100.0);
			}
		}
	}
public:
	MenuItemPlayerHorseUnlimStamina(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemPlayerSuperJump : public MenuItemSwitchable
{
	virtual void OnFrame()
	{
		if (GetState())
			GAMEPLAY::SET_SUPER_JUMP_THIS_FRAME(PLAYER::PLAYER_ID());
	}
public:
	MenuItemPlayerSuperJump(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemPlayerNoiseless : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (!newState)
		{
			PLAYER::SET_PLAYER_NOISE_MULTIPLIER(PLAYER::PLAYER_ID(), 1.0);
			PLAYER::SET_PLAYER_SNEAKING_NOISE_MULTIPLIER(PLAYER::PLAYER_ID(), 1.0);
		}
		SetState(newState);
	}
	virtual void OnFrame()
	{
		if (GetState())
		{
			PLAYER::SET_PLAYER_NOISE_MULTIPLIER(PLAYER::PLAYER_ID(), 0.0);
			PLAYER::SET_PLAYER_SNEAKING_NOISE_MULTIPLIER(PLAYER::PLAYER_ID(), 0.0);
		}
	}
public:
	MenuItemPlayerNoiseless(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemPlayerEveryoneIgnored : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (!newState)
			PLAYER::SET_EVERYONE_IGNORE_PLAYER(PLAYER::PLAYER_ID(), FALSE);
		SetState(newState);
	}
	virtual void OnFrame()
	{
		if (GetState())
			PLAYER::SET_EVERYONE_IGNORE_PLAYER(PLAYER::PLAYER_ID(), TRUE);
	}
public:
	MenuItemPlayerEveryoneIgnored(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemPlayerTeleport : public MenuItemDefault
{
	Vector3 m_pos;
	virtual void OnSelect()
	{
		Entity e = PLAYER::PLAYER_PED_ID();
		if (PED::IS_PED_ON_MOUNT(e))
			e = PED::GET_MOUNT(e);
		else
			if (PED::IS_PED_IN_ANY_VEHICLE(e, FALSE))
				e = PED::GET_VEHICLE_PED_IS_USING(e);
		ENTITY::SET_ENTITY_COORDS(e, m_pos.x, m_pos.y, m_pos.z, 0, 0, 1, FALSE);
	}
public:
	MenuItemPlayerTeleport(string caption, Vector3 pos)
		: MenuItemDefault(caption),
		m_pos(pos) {}
};

class MenuItemPlayerTeleportToMarker : public MenuItemDefault
{
	virtual void OnSelect()
	{
		if (!RADAR::IS_WAYPOINT_ACTIVE())
		{
			SetStatusText("map marker isn't set");
			return;
		}

		Vector3 coords = RADAR::GET_WAYPOINT_COORDS_3D();

		Entity e = PLAYER::PLAYER_PED_ID();
		if (PED::IS_PED_ON_MOUNT(e))
			e = PED::GET_MOUNT(e);
		else
			if (PED::IS_PED_IN_ANY_VEHICLE(e, FALSE))
				e = PED::GET_VEHICLE_PED_IS_USING(e);

		if (!GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(coords.x, coords.y, 100.0, &coords.z, FALSE))
		{
			static const float groundCheckHeight[] = {
				100.0, 150.0, 50.0, 0.0, 200.0, 250.0, 300.0, 350.0, 400.0,
				450.0, 500.0, 550.0, 600.0, 650.0, 700.0, 750.0, 800.0
			};
			for each (float height in groundCheckHeight)
			{
				ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, coords.x, coords.y, height, 0, 0, 1);
				WaitAndDraw(100);
				if (GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(coords.x, coords.y, height, &coords.z, FALSE))
				{
					coords.z += 3.0;
					break;
				}
			}
		}

		ENTITY::SET_ENTITY_COORDS(e, coords.x, coords.y, coords.z, 0, 0, 1, FALSE);
	}
public:
	MenuItemPlayerTeleportToMarker(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemPlayerClearWanted : public MenuItemDefault
{
	bool m_headPrice;
	bool m_pursuit;
	virtual void OnSelect()
	{
		Player player = PLAYER::PLAYER_ID();
		if (ENTITY::DOES_ENTITY_EXIST(player))
		{
			if (m_headPrice)
				PURSUIT::SET_PLAYER_PRICE_ON_A_HEAD(player, 0);
			if (m_pursuit)
			{
				PURSUIT::CLEAR_CURRENT_PURSUIT();
				PURSUIT::SET_PLAYER_WANTED_INTENSITY(player, 0);
			}
			SetStatusText("Player has to be in pursuit\n\n"
				"head price: " + to_string(PURSUIT::GET_PLAYER_PRICE_ON_A_HEAD(player) / 100) + "\n" +
				"wanted intensity: " + to_string(PURSUIT::GET_PLAYER_WANTED_INTENSITY(player) / 100));
		}
		else
		{
			// Player entity is invalid, handle the error accordingly
			SetStatusText("Error: Player entity is invalid");
		}
	}
public:
	MenuItemPlayerClearWanted(string caption, bool headPrice, bool pursuit)
		: MenuItemDefault(caption),
		m_headPrice(headPrice), m_pursuit(pursuit) {}
};

class MenuItemPlayerNeverWanted : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newstate = !GetState();
		if (!newstate)
			PLAYER::SET_WANTED_LEVEL_MULTIPLIER(1.0);
		SetState(newstate);
	}
	virtual void OnFrame()
	{
		if (GetState())
		{
			Player player = PLAYER::PLAYER_ID();
			PURSUIT::CLEAR_CURRENT_PURSUIT();
			PURSUIT::SET_PLAYER_PRICE_ON_A_HEAD(player, 0);
			PURSUIT::SET_PLAYER_WANTED_INTENSITY(player, 0);
			PLAYER::SET_WANTED_LEVEL_MULTIPLIER(0.0);
		}
	}
public:
	MenuItemPlayerNeverWanted(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemChangePlayerModel : public MenuItemDefault
{
	string		m_model;

	virtual void OnSelect()
	{
		DWORD model = GAMEPLAY::GET_HASH_KEY(const_cast<char*>(m_model.c_str()));
		if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
		{
			UINT64* ptr1 = getGlobalPtr(0x28) + 0x27;
			UINT64* ptr2 = getGlobalPtr(((DWORD)7 << 18) | 0x1890C) + 2;
			UINT64 bcp1 = *ptr1;
			UINT64 bcp2 = *ptr2;
			*ptr1 = *ptr2 = model;
			WaitAndDraw(1000);
			Ped playerPed = PLAYER::PLAYER_PED_ID();
			PED::SET_PED_VISIBLE(playerPed, TRUE);
			if (ENTITY::GET_ENTITY_MODEL(playerPed) != model)
			{
				*ptr1 = bcp1;
				*ptr2 = bcp2;
			}
		}
	}
public:
	MenuItemChangePlayerModel(string caption, string model)
		: MenuItemDefault(caption),
		m_model(model) { }
};

class MenuItemSpawnPed : public MenuItemDefault
{
	string		m_model;

	virtual string GetModel() { return m_model; }

	virtual void OnSelect()
	{
		DWORD model = GAMEPLAY::GET_HASH_KEY(const_cast<char*>(GetModel().c_str()));
		if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
		{
			STREAMING::REQUEST_MODEL(model, FALSE);
			while (!STREAMING::HAS_MODEL_LOADED(model))
				WaitAndDraw(0); // WAIT(0);
			Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 3.0, -0.3);
			Ped ped = PED::CREATE_PED(model, coords.x, coords.y, coords.z, static_cast<float>(rand() % 360), 0, 0, 0, 0);
			PED::SET_PED_VISIBLE(ped, TRUE);
			ENTITY::SET_PED_AS_NO_LONGER_NEEDED(&ped);
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
		}
	}
public:
	MenuItemSpawnPed(string caption, string model)
		: MenuItemDefault(caption),
		m_model(model) { }
};

class MenuItemSpawnHorseRandom : public MenuItemSpawnPed
{
	virtual string GetModel()
	{
		while (true)
		{
			int index = rand() % ARRAY_LENGTH(pedModelInfos);
			if (pedModelInfos[index].horse)
			{
				SetStatusText(pedModelInfos[index].name);
				return pedModelInfos[index].model;
			}
		}
	}
public:
	MenuItemSpawnHorseRandom(string caption)
		: MenuItemSpawnPed(caption, "") { }
};

class MenuItemSpawnAnimalRandom : public MenuItemSpawnPed
{
	virtual string GetModel()
	{
		while (true)
		{
			int index = rand() % ARRAY_LENGTH(pedModelInfos);
			if (pedModelInfos[index].animal && !pedModelInfos[index].fish && !pedModelInfos[index].horse)
			{
				SetStatusText(pedModelInfos[index].name);
				return pedModelInfos[index].model;
			}
		}
	}
public:
	MenuItemSpawnAnimalRandom(string caption)
		: MenuItemSpawnPed(caption, "") { }
};

class MenuItemSpawnPedRandom : public MenuItemSpawnPed
{
	virtual string GetModel()
	{
		while (true)
		{
			int index = rand() % ARRAY_LENGTH(pedModelInfos);
			if (!pedModelInfos[index].animal)
			{
				SetStatusText(pedModelInfos[index].name);
				return pedModelInfos[index].model;
			}
		}
	}
public:
	MenuItemSpawnPedRandom(string caption)
		: MenuItemSpawnPed(caption, "") { }
};

class MenuItemSpawnVehicle : public MenuItemDefault
{
	string		m_model;
	Vector3		m_pos;
	float		m_heading;
	bool		m_resetHeading;
	bool		m_noPeds;

	MenuItemSwitchable* m_menuItemWrapIn;
	MenuItemSwitchable* m_menuItemSetProperly;

	virtual void OnSelect()
	{
		DWORD model = GAMEPLAY::GET_HASH_KEY(const_cast<char*>(m_model.c_str()));
		if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
		{
			STREAMING::REQUEST_MODEL(model, FALSE);
			while (!STREAMING::HAS_MODEL_LOADED(model))
				WaitAndDraw(0); // WAIT(0);
			Ped playerPed = PLAYER::PLAYER_PED_ID();
			float playerHeading = ENTITY::GET_ENTITY_HEADING(playerPed) + 5.0f;
			float heading = playerHeading + m_heading;
			Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, m_pos.x, m_pos.y, m_pos.z);
			Vehicle veh = VEHICLE::CREATE_VEHICLE(model, coords.x, coords.y, coords.z, heading, 0, 0, m_noPeds, 0);
			DECORATOR::DECOR_SET_BOOL(veh, "wagon_block_honor", TRUE);
			bool wrapIn = m_menuItemWrapIn && m_menuItemWrapIn->GetState();
			bool setProperly = m_menuItemSetProperly && m_menuItemSetProperly->GetState();
			if (setProperly)
			{
				VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh, 0);
				WaitAndDraw(100);
			}
			if (m_resetHeading || wrapIn)
				ENTITY::SET_ENTITY_HEADING(veh, wrapIn ? playerHeading : heading);
			if (wrapIn)
				PED::SET_PED_INTO_VEHICLE(playerPed, veh, -1);
			ENTITY::SET_VEHICLE_AS_NO_LONGER_NEEDED(&veh);
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
		}
	}
public:
	MenuItemSpawnVehicle(string model,
		Vector3 pos, float heading,
		MenuItemSwitchable* menuItemWrapIn,
		MenuItemSwitchable* menuItemSetProperly,
		bool resetHeading,
		bool noPeds)
		: MenuItemDefault(model),
		m_model(model),
		m_pos(pos), m_heading(heading),
		m_menuItemWrapIn(menuItemWrapIn),
		m_menuItemSetProperly(menuItemSetProperly),
		m_resetHeading(resetHeading),
		m_noPeds(noPeds) { }
};

class MenuItemGiveWeapon : public MenuItemDefault
{
	string m_name;

	virtual void OnSelect()
	{
		Hash hash = GAMEPLAY::GET_HASH_KEY(const_cast<char*>(("WEAPON_" + m_name).c_str()));
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(playerPed, hash, 100, 1, 0x2cd419dc);
		WEAPON::SET_PED_AMMO(playerPed, hash, 100);
		WEAPON::SET_CURRENT_PED_WEAPON(playerPed, hash, 1, 0, 0, 0);
	}
public:
	MenuItemGiveWeapon(string caption, string weaponName)
		: MenuItemDefault(caption),
		m_name(weaponName) {}
};

class MenuItemWeaponPowerfullGuns : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (!newState)
			PLAYER::SET_PLAYER_WEAPON_DAMAGE_MODIFIER(PLAYER::PLAYER_ID(), 1.0);
		SetState(newState);
	}
	virtual void OnFrame()
	{
		if (GetState())
			PLAYER::SET_PLAYER_WEAPON_DAMAGE_MODIFIER(PLAYER::PLAYER_ID(), 100.0);
	}
public:
	MenuItemWeaponPowerfullGuns(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemWeaponPowerfullMelee : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (!newState)
			PLAYER::SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER(PLAYER::PLAYER_ID(), 1.0);
		SetState(newState);
	}
	virtual void OnFrame()
	{
		if (GetState())
			PLAYER::SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER(PLAYER::PLAYER_ID(), 100.0);
	}
public:
	MenuItemWeaponPowerfullMelee(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemWeaponNoReload : public MenuItemSwitchable
{
	virtual void OnFrame()
	{
		if (!GetState())
			return;

		Ped playerPed = PLAYER::PLAYER_PED_ID();

		// Right hand weapon
		Hash curRight;
		if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &curRight, 1, 0, 0) && WEAPON::IS_WEAPON_VALID(curRight))
		{
			int maxAmmoRight;
			if (WEAPON::GET_MAX_AMMO(playerPed, &maxAmmoRight, curRight))
				WEAPON::SET_PED_AMMO(playerPed, curRight, maxAmmoRight);
			maxAmmoRight = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, curRight, 1);
			if (maxAmmoRight > 0)
				WEAPON::SET_AMMO_IN_CLIP(playerPed, curRight, maxAmmoRight);
		}

		// Left hand weapon
		Hash curLeft;
		if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &curLeft, 0, 0, 0) && WEAPON::IS_WEAPON_VALID(curLeft))
		{
			int maxAmmoLeft;
			if (WEAPON::GET_MAX_AMMO(playerPed, &maxAmmoLeft, curLeft))
				WEAPON::SET_PED_AMMO(playerPed, curLeft, maxAmmoLeft);
			maxAmmoLeft = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, curLeft, 1);
			if (maxAmmoLeft > 0)
				WEAPON::SET_AMMO_IN_CLIP(playerPed, curLeft, maxAmmoLeft);
		}
	}

public:
	MenuItemWeaponNoReload(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemWeaponDropCurrent : public MenuItemDefault
{
	virtual void OnSelect()
	{
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		Hash unarmed = GAMEPLAY::GET_HASH_KEY("WEAPON_UNARMED");
		Hash cur;
		if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &cur, 0, 0, 0) && WEAPON::IS_WEAPON_VALID(cur) && cur != unarmed)
			WEAPON::SET_PED_DROPS_INVENTORY_WEAPON(playerPed, cur, 0.0, 0.0, 0.0, 1);
	}
public:
	MenuItemWeaponDropCurrent(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemTimeTitle : public MenuItemTitle
{
	virtual string GetCaption()
	{
		time_t now = time(0);
		tm t;
		localtime_s(&t, &now);
		char str[32];
		sprintf_s(str, "%02d:%02d", TIME::GET_CLOCK_HOURS(), TIME::GET_CLOCK_MINUTES());
		return MenuItemTitle::GetCaption() + "   " + str;
	}

public:
	MenuItemTimeTitle(string caption)
		: MenuItemTitle(caption) {}
};

class MenuItemTimeAdjust : public MenuItemDefault
{
	int m_difHours;
	virtual void OnSelect()
	{
		TIME::ADD_TO_CLOCK_TIME(m_difHours, 0, 0);
	}
public:
	MenuItemTimeAdjust(string caption, int difHours)
		: MenuItemDefault(caption),
		m_difHours(difHours) {}
};

class MenuItemTimePause : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newState = !GetState();
		TIME::PAUSE_CLOCK(newState, 0);
		SetState(newState);
	}
public:
	MenuItemTimePause(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemTimeRealistic : public MenuItemSwitchable
{
	int m_difHour;
	int m_difMin;
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (newState)
		{
			time_t now = time(0);
			tm t;
			localtime_s(&t, &now);
			m_difHour = TIME::GET_CLOCK_HOURS() - t.tm_hour;
			m_difMin = TIME::GET_CLOCK_MINUTES() - t.tm_min;
		}
		SetState(newState);
	}
	virtual void OnFrame()
	{
		if (!GetState())
			return;
		time_t now = time(0);
		tm t;
		localtime_s(&t, &now);
		int hours = t.tm_hour + m_difHour;
		int mins = t.tm_min + m_difMin;
		if (mins >= 60)
		{
			mins -= 60;
			hours++;
		}
		else
			if (mins < 0)
			{
				mins += 60;
				hours--;
			}
		if (hours >= 24)
			hours -= 24;
		else
			if (hours < 0)
				hours += 24;
		TIME::SET_CLOCK_TIME(hours, mins, t.tm_sec);
	}
public:
	MenuItemTimeRealistic(string caption)
		: MenuItemSwitchable(caption),
		m_difHour(0), m_difMin(0) {}
};

class MenuItemTimeSystemSynced : public MenuItemSwitchable
{
	virtual void OnFrame()
	{
		if (GetState())
		{
			time_t now = time(0);
			tm t;
			localtime_s(&t, &now);
			TIME::SET_CLOCK_TIME(t.tm_hour, t.tm_min, t.tm_sec);
		}
	}
public:
	MenuItemTimeSystemSynced(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemWeatherFreeze : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newstate = !GetState();
		if (!newstate)
			GAMEPLAY::FREEZE_WEATHER(false);
		SetState(newstate);
	}
	virtual void OnFrame()
	{
		if (GetState())
			GAMEPLAY::FREEZE_WEATHER(true);
	}
public:
	MenuItemWeatherFreeze(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemWeatherWind : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newstate = !GetState();
		if (newstate)
		{
			GAMEPLAY::SET_WIND_SPEED(50.0);
			GAMEPLAY::SET_WIND_DIRECTION(ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));
		}
		else
		{
			GAMEPLAY::SET_WIND_SPEED(0.0);
		}
		SetState(newstate);
	}
public:
	MenuItemWeatherWind(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemWeatherSelect : public MenuItemDefault
{
	virtual void OnSelect()
	{
		GAMEPLAY::CLEAR_OVERRIDE_WEATHER();
		Hash weather = GAMEPLAY::GET_HASH_KEY(const_cast<char*>(GetCaption().c_str()));
		GAMEPLAY::SET_WEATHER_TYPE(weather, TRUE, TRUE, FALSE, 0.0, FALSE);
		GAMEPLAY::CLEAR_WEATHER_TYPE_PERSIST();
	}
public:
	MenuItemWeatherSelect(string caption)
		: MenuItemDefault(caption) { }
};

class MenuItemMiscHideHud : public MenuItemSwitchable
{
	virtual void OnFrame()
	{
		if (GetState())
			UI::HIDE_HUD_AND_RADAR_THIS_FRAME();
	}
public:
	MenuItemMiscHideHud(string caption)
		: MenuItemSwitchable(caption) {}
};

class MenuItemMiscAddHonor : public MenuItemDefault
{
	virtual void OnSelect()
	{
		Hash unarmed = GAMEPLAY::GET_HASH_KEY("WEAPON_UNARMED");
		WEAPON::SET_CURRENT_PED_WEAPON(PLAYER::PLAYER_PED_ID(), unarmed, 1, 0, 0, 0);
		DWORD model = GAMEPLAY::GET_HASH_KEY("U_M_O_VHTEXOTICSHOPKEEPER_01");
		STREAMING::REQUEST_MODEL(model, FALSE);
		while (!STREAMING::HAS_MODEL_LOADED(model))
			WaitAndDraw(0);
		Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 3.0, -0.3);
		Ped ped = PED::CREATE_PED(model, coords.x, coords.y, coords.z, 0.0, 0, 0, 0, 0);
		PED::SET_PED_VISIBLE(ped, TRUE);
		DECORATOR::DECOR_SET_INT(ped, "honor_override", -9999);
		AI::TASK_COMBAT_PED(ped, PLAYER::PLAYER_PED_ID(), 0, 0);
		//ENTITY::SET_PED_AS_NO_LONGER_NEEDED(&ped);
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
	}
public:
	MenuItemMiscAddHonor(string caption)
		: MenuItemDefault(caption) {}
};

class MenuItemMiscRevealMap : public MenuItemDefault
{
	virtual void OnSelect()
	{
		RADAR::_SET_MINIMAP_REVEALED(TRUE);
		RADAR::REVEAL_MAP(0);
		SetStatusText("map revealed");
	}
public:
	MenuItemMiscRevealMap(string caption) :
		MenuItemDefault(caption) {}
};

class MenuItemNuclearBomb : public MenuItemDefault
{
	float m_radius;
	virtual void OnSelect()
	{
		Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true, true);
		FIRE::ADD_EXPLOSION(playerPos.x, playerPos.y, playerPos.z, 7, m_radius, true, false, 1.0f);
	}
public:
	MenuItemNuclearBomb(string caption, float radius) :
		MenuItemDefault(caption),
		m_radius(radius) {}
};

class MenuItemMiscTransportGuns : public MenuItemSwitchable
{
	bool m_isHorse;
	bool m_isBullet;

	DWORD m_lastShootTime;

	virtual void OnSelect()
	{
		bool newstate = !GetState();
		if (newstate)
			SetStatusText("WARN: may cause ERR_GFX_STATE while breaking certain objects\n\nINSERT / NUM+", 5000);
		SetState(newstate);
	}

	virtual void OnFrame()
	{
		if (!GetState())
			return;

		if (!IsKeyDownLong(VK_ADD) && !IsKeyDownLong(VK_INSERT)) // num plus / insert
			return;

		if (m_lastShootTime + (m_isBullet ? 50 : 250) > GetTickCount64())
			return;

		Player player = PLAYER::PLAYER_ID();
		Ped playerPed = PLAYER::PLAYER_PED_ID();

		if (!PLAYER::IS_PLAYER_CONTROL_ON(player))
			return;

		Entity transport;
		if (m_isHorse)
			if (PED::IS_PED_ON_MOUNT(playerPed))
				transport = PED::GET_MOUNT(playerPed);
			else
				return;
		else
			if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, 0))
				transport = PED::GET_VEHICLE_PED_IS_USING(playerPed);
			else
				return;

		Vector3 v0, v1;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(transport), &v0, &v1);

		Hash modelHash = m_isBullet ? 0 : GAMEPLAY::GET_HASH_KEY("S_CANNONBALL");
		Hash weaponHash = GAMEPLAY::GET_HASH_KEY(m_isBullet ? "WEAPON_TURRET_GATLING" : "WEAPON_TURRET_REVOLVING_CANNON");

		if (modelHash && !STREAMING::HAS_MODEL_LOADED(modelHash))
		{
			STREAMING::REQUEST_MODEL(modelHash, FALSE);
			while (!STREAMING::HAS_MODEL_LOADED(modelHash))
				WAIT(0);
		}

		Vector3 coords0from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(transport, -(v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords1from = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(transport, (v1.x + 0.25f), v1.y + 1.25f, 0.1);
		Vector3 coords0to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(transport, -v1.x, v1.y + 100.0f, 0.1f);
		Vector3 coords1to = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(transport, v1.x, v1.y + 100.0f, 0.1f);

		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
			coords0to.x, coords0to.y, coords0to.z,
			250, 1, weaponHash, playerPed, 1, 1, -1.0, 0);
		GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
			coords1to.x, coords1to.y, coords1to.z,
			250, 1, weaponHash, playerPed, 1, 1, -1.0, 0);

		if (m_isBullet)
		{
			weaponHash = GAMEPLAY::GET_HASH_KEY("WEAPON_SNIPERRIFLE_CARCANO");
			GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords0from.x, coords0from.y, coords0from.z,
				coords0to.x, coords0to.y, coords0to.z,
				250, 1, weaponHash, playerPed, 1, 0, -1.0, 0);
			GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords1from.x, coords1from.y, coords1from.z,
				coords1to.x, coords1to.y, coords1to.z,
				250, 1, weaponHash, playerPed, 1, 0, -1.0, 0);
		}

		m_lastShootTime = GetTickCount64();
	}
public:
	MenuItemMiscTransportGuns(string caption, bool isHorse, bool isBullet)
		: MenuItemSwitchable(caption),
		m_isHorse(isHorse), m_isBullet(isBullet),
		m_lastShootTime(0) {}
};

class MenuItemDeathNoise : public MenuItemSwitchable
{
	bool isPlayerDead = false;

	virtual void OnSelect()
	{
		bool newstate = !GetState();
		SetState(newstate);
	}

	virtual void OnFrame()
	{
		if (!GetState())
			return;

		Ped playerPed = PLAYER::PLAYER_PED_ID();

		if (!isPlayerDead && ENTITY::IS_ENTITY_DEAD(playerPed))
		{
			PlayAudio("broke_alert.mp3", playerPed, "SOUND_REFERENCE", FALSE, 0, 0);
			isPlayerDead = true;
		}
		else if (isPlayerDead && !ENTITY::IS_ENTITY_DEAD(playerPed))
		{
			isPlayerDead = false;
		}
	}
public:
	MenuItemDeathNoise(string caption)
		: MenuItemSwitchable(caption) {}
};

MenuBase* CreatePlayerTeleportMenu(MenuController* controller)
{
	MenuBase* menu = new MenuBase(new MenuItemListTitle("TELEPORT"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemPlayerTeleportToMarker("MARKER"));

	menu->AddItem(new MenuItemPlayerTeleport("SOUTH MAP", { -5311.2583,  -4612.00,   -10.63389 }));
	menu->AddItem(new MenuItemPlayerTeleport("SOUTH GUAMA", { 1315.66381,  -6815.48,   42.377101 }));
	menu->AddItem(new MenuItemPlayerTeleport("ANNESBURG", { 2898.593994, 1239.85253, 44.073299 }));
	menu->AddItem(new MenuItemPlayerTeleport("STRAWBERRY", { -1725.22143, -418.11560, 153.55740 }));
	menu->AddItem(new MenuItemPlayerTeleport("VALENTINE", { -213.152496, 691.802979, 112.37100 }));
	menu->AddItem(new MenuItemPlayerTeleport("RHODES", { 1282.707520, -1275.7485, 74.945099 }));
	menu->AddItem(new MenuItemPlayerTeleport("SAINT DENIS", { 2336.584961, -1106.2358, 44.737598 }));
	menu->AddItem(new MenuItemPlayerTeleport("WAPITI", { 538.738525,  2217.46557, 240.23280 }));
	menu->AddItem(new MenuItemPlayerTeleport("BUTCHERCREEK", { 2552.203613, 835.510010, 81.183098 }));
	menu->AddItem(new MenuItemPlayerTeleport("BLACKWATER", { -798.338379, -1238.9395, 43.537899 }));
	menu->AddItem(new MenuItemPlayerTeleport("BEECHERS", { -1653.19738, -1448.8156, 82.503502 }));
	menu->AddItem(new MenuItemPlayerTeleport("CALIGA HALL", { 1705.509888, -1386.3237, 42.884998 }));
	menu->AddItem(new MenuItemPlayerTeleport("BRAITHWAITE", { 1011.190674, -1661.6768, 45.918301 }));
	menu->AddItem(new MenuItemPlayerTeleport("VANHORN", { 2982.234863, 445.724915, 51.491501 }));
	menu->AddItem(new MenuItemPlayerTeleport("CORNWALL", { 437.7247920, 494.582092, 107.67649 }));
	menu->AddItem(new MenuItemPlayerTeleport("COLTER", { -1371.6590,  2388.5073,  307.7218 }));
	menu->AddItem(new MenuItemPlayerTeleport("EMERALD RANCH", { 1332.332642, 300.425110, 86.306297 }));
	menu->AddItem(new MenuItemPlayerTeleport("PRONGHORN", { -2616.57714, 519.256775, 144.10809 }));
	menu->AddItem(new MenuItemPlayerTeleport("MANZANITA POST", { -1977.98754, -1545.6749, 112.87020 }));
	menu->AddItem(new MenuItemPlayerTeleport("LAGRAS", { 2111.099121, -662.25317, 41.259899 }));
	menu->AddItem(new MenuItemPlayerTeleport("ARMADILLO", { -3622.65527, -2586.5795, -15.36900 }));
	menu->AddItem(new MenuItemPlayerTeleport("TUMBLEWEED", { -5382.39453, -2940.1596, 1.582700 }));
	menu->AddItem(new MenuItemPlayerTeleport("MACFARLANES RANCH", { -2296.26318, -2454.4101, 60.969898 }));
	menu->AddItem(new MenuItemPlayerTeleport("BENEDICT POINT", { -5269.60400, -3411.0588, -23.15930 }));

	return menu;
}

MenuBase* CreatePlayerChangeModelHorseMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemListTitle("HORSE  MODELS"));
	controller->RegisterMenu(menu);

	unordered_map<string, vector<pair<string, string>>> breeds;
	for each (auto & modelInfo in pedModelInfos)
		if (modelInfo.horse)
		{
			size_t pos = modelInfo.name.find_first_of(' ');
			string breed = modelInfo.name.substr(0, pos);
			string kind = modelInfo.name.substr(pos + 1, modelInfo.name.size() - pos - 1);
			breeds[breed].push_back({ kind, modelInfo.model });
		}

	for each (auto & breed in breeds)
	{
		auto breedMenu = new MenuBase(new MenuItemListTitle(breed.first));
		controller->RegisterMenu(breedMenu);
		menu->AddItem(new MenuItemMenu(breed.first, breedMenu));
		for each (auto & kindAndModel in breed.second)
			breedMenu->AddItem(new MenuItemChangePlayerModel(kindAndModel.first, kindAndModel.second));
	}

	return menu;
}

MenuBase* CreatePlayerChangeModelAnimalMenuExactFilter(MenuController* controller, bool horse, bool dog, bool fish)
{
	auto menu = new MenuBase(new MenuItemListTitle("ANIMAL  MODELS"));
	controller->RegisterMenu(menu);

	for each (auto & modelInfo in pedModelInfos)
		if (modelInfo.animal &&
			modelInfo.horse == horse && modelInfo.dog == dog && modelInfo.fish == fish)
			menu->AddItem(new MenuItemChangePlayerModel(modelInfo.name, modelInfo.model));

	return menu;
}

MenuBase* CreatePlayerChangeModelAnimalMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTitle("ANIMAL  MODELS"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemMenu("HORSES", CreatePlayerChangeModelHorseMenu(controller)));
	menu->AddItem(new MenuItemMenu("DOGS", CreatePlayerChangeModelAnimalMenuExactFilter(controller, false, true, false)));
	menu->AddItem(new MenuItemMenu("FISH", CreatePlayerChangeModelAnimalMenuExactFilter(controller, false, false, true)));
	menu->AddItem(new MenuItemMenu("OTHER", CreatePlayerChangeModelAnimalMenuExactFilter(controller, false, false, false)));

	return menu;
}

MenuBase* CreatePlayerChangeModelHumanMenuExactFilter(MenuController* controller, bool cutscene, bool male, bool female, bool young, bool middleaged, bool old)
{
	auto menu = new MenuBase(new MenuItemListTitle("PED  MODELS"));
	controller->RegisterMenu(menu);

	for each (auto & modelInfo in pedModelInfos)
		if (!modelInfo.animal &&
			modelInfo.cutscene == cutscene && modelInfo.male == male && modelInfo.female == female &&
			modelInfo.young == young && modelInfo.middleaged == middleaged && modelInfo.old == old)
			menu->AddItem(new MenuItemChangePlayerModel(modelInfo.name, modelInfo.model));

	return menu;
}

MenuBase* CreatePlayerChangeModelHumanMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTitle("PED  MODELS"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemMenu("CUTSCENE", CreatePlayerChangeModelHumanMenuExactFilter(controller, true, false, false, false, false, false)));
	menu->AddItem(new MenuItemMenu("MALE YOUNG", CreatePlayerChangeModelHumanMenuExactFilter(controller, false, true, false, true, false, false)));
	menu->AddItem(new MenuItemMenu("MALE MIDDLE", CreatePlayerChangeModelHumanMenuExactFilter(controller, false, true, false, false, true, false)));
	menu->AddItem(new MenuItemMenu("MALE OLD", CreatePlayerChangeModelHumanMenuExactFilter(controller, false, true, false, false, false, true)));
	menu->AddItem(new MenuItemMenu("FEMALE YOUNG", CreatePlayerChangeModelHumanMenuExactFilter(controller, false, false, true, true, false, false)));
	menu->AddItem(new MenuItemMenu("FEMALE MIDDLE", CreatePlayerChangeModelHumanMenuExactFilter(controller, false, false, true, false, true, false)));
	menu->AddItem(new MenuItemMenu("FEMALE OLD", CreatePlayerChangeModelHumanMenuExactFilter(controller, false, false, true, false, false, true)));
	menu->AddItem(new MenuItemMenu("MISC", CreatePlayerChangeModelHumanMenuExactFilter(controller, false, false, false, false, false, false)));

	return menu;
}

MenuBase* CreatePlayerChangeModelMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTitle("SKIN  CHANGER"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemMenu("ANIMALS", CreatePlayerChangeModelAnimalMenu(controller)));
	menu->AddItem(new MenuItemMenu("PEDS", CreatePlayerChangeModelHumanMenu(controller)));

	return menu;
}

MenuBase* CreatePlayerWantedMenu(MenuController* controller)
{
	MenuBase* menu = new MenuBase(new MenuItemTitle("WANTED  OPTIONS"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemPlayerClearWanted("CLEAR BOUNTY", true, false));
	menu->AddItem(new MenuItemPlayerClearWanted("CLEAR WANTED", true, true));
	menu->AddItem(new MenuItemPlayerNeverWanted("NEVER WANTED"));

	return menu;
}

MenuBase* CreatePlayerTransportMenu(MenuController* controller)
{
	MenuBase* menu = new MenuBase(new MenuItemTitle("TRANSPORT  OPTIONS"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemPlayerHorseInvincible("INVINCIBLE HORSE"));
	menu->AddItem(new MenuItemPlayerHorseUnlimStamina("UNLIM HORSE STAMINA"));
	menu->AddItem(new MenuItemVehicleBoost("VEHICLE BOOST"));
	menu->AddItem(new MenuItemHorseRagdoll("HORSE RAGDOLL", 5000, 5000, false, false));

	return menu;
}

MenuBase* CreatePlayerMiscMenu(MenuController* controller)
{
	MenuBase* menu = new MenuBase(new MenuItemTitle("PLAYER  MISC"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemPlayerEveryoneIgnored("EVERYONE IGNORED"));
	menu->AddItem(new MenuItemPlayerNoiseless("NOISELESS"));
	menu->AddItem(new MenuItemPlayerSuperJump("SUPER JUMP"));

	return menu;
}

MenuBase* CreatePlayerMenu(MenuController* controller)
{
	MenuBase* menu = new MenuBase(new MenuItemTitle("PLAYER  OPTIONS"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemMenu("TRANSPORT", CreatePlayerTransportMenu(controller)));
	menu->AddItem(new MenuItemMenu("TELEPORT", CreatePlayerTeleportMenu(controller)));
	menu->AddItem(new MenuItemMenu("SKINS", CreatePlayerChangeModelMenu(controller)));
	menu->AddItem(new MenuItemMenu("WANTED", CreatePlayerWantedMenu(controller)));
	menu->AddItem(new MenuItemPlayerFix("FIX PLAYER"));
	menu->AddItem(new MenuItemPlayerAddCash("ADD CASH", 1000 * 100));
	menu->AddItem(new MenuItemPlayerRagdoll("RAGDOLL", 5000, 5000, false, false));
	menu->AddItem(new MenuItemPlayerFastHeal("FAST HEAL"));
	menu->AddItem(new MenuItemPlayerInvincible("INVINCIBLE"));
	menu->AddItem(new MenuItemPlayerUnlimStamina("UNLIM STAMINA"));
	menu->AddItem(new MenuItemPlayerUnlimAbility("UNLIM ABILITY"));
	menu->AddItem(new MenuItemPlayerDrunk("DRUNK"));
	menu->AddItem(new MenuItemNoRagdoll("NO RAGDOLL"));
	menu->AddItem(new MenuItemMenu("MISC", CreatePlayerMiscMenu(controller)));

	return menu;
}

MenuBase* CreateHorseSpawnerMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemListTitle("HORSE  SPAWNER"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemSpawnHorseRandom("random horse"));

	unordered_map<string, vector<pair<string, string>>> breeds;
	for each (auto & modelInfo in pedModelInfos)
		if (modelInfo.horse)
		{
			size_t pos = modelInfo.name.find_first_of(' ');
			string breed = modelInfo.name.substr(0, pos);
			string kind = modelInfo.name.substr(pos + 1, modelInfo.name.size() - pos - 1);
			breeds[breed].push_back({ kind, modelInfo.model });
		}

	for each (auto & breed in breeds)
	{
		auto breedMenu = new MenuBase(new MenuItemListTitle(breed.first));
		controller->RegisterMenu(breedMenu);
		menu->AddItem(new MenuItemMenu(breed.first, breedMenu));
		for each (auto & kindAndModel in breed.second)
			breedMenu->AddItem(new MenuItemSpawnPed(kindAndModel.first, kindAndModel.second));
	}

	return menu;
}

MenuBase* CreateAnimalSpawnerMenuExactFilter(MenuController* controller, bool horse, bool dog, bool fish)
{
	auto menu = new MenuBase(new MenuItemListTitle("ANIMAL  SPAWNER"));
	controller->RegisterMenu(menu);

	for each (auto & modelInfo in pedModelInfos)
		if (modelInfo.animal &&
			modelInfo.horse == horse && modelInfo.dog == dog && modelInfo.fish == fish)
			menu->AddItem(new MenuItemSpawnPed(modelInfo.name, modelInfo.model));

	return menu;
}

MenuBase* CreateAnimalSpawnerMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTitle("ANIMAL  SPAWNER"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemSpawnAnimalRandom("RANDOM"));
	menu->AddItem(new MenuItemMenu("HORSES", CreateHorseSpawnerMenu(controller)));
	menu->AddItem(new MenuItemMenu("DOGS", CreateAnimalSpawnerMenuExactFilter(controller, false, true, false)));
	menu->AddItem(new MenuItemMenu("FISH", CreateAnimalSpawnerMenuExactFilter(controller, false, false, true)));
	menu->AddItem(new MenuItemMenu("OTHER", CreateAnimalSpawnerMenuExactFilter(controller, false, false, false)));

	return menu;
}

MenuBase* CreateHumanSpawnerMenuExactFilter(MenuController* controller, bool cutscene, bool male, bool female, bool young, bool middleaged, bool old)
{
	auto menu = new MenuBase(new MenuItemListTitle("PED  SPAWNER"));
	controller->RegisterMenu(menu);

	for each (auto & modelInfo in pedModelInfos)
		if (!modelInfo.animal &&
			modelInfo.cutscene == cutscene && modelInfo.male == male && modelInfo.female == female &&
			modelInfo.young == young && modelInfo.middleaged == middleaged && modelInfo.old == old)
			menu->AddItem(new MenuItemSpawnPed(modelInfo.name, modelInfo.model));

	return menu;
}

MenuBase* CreateHumanSpawnerMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTitle("PED  SPAWNER"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemSpawnPedRandom("RANDOM PED"));
	menu->AddItem(new MenuItemMenu("CUTSCENE", CreateHumanSpawnerMenuExactFilter(controller, true, false, false, false, false, false)));
	menu->AddItem(new MenuItemMenu("MALE YOUNG", CreateHumanSpawnerMenuExactFilter(controller, false, true, false, true, false, false)));
	menu->AddItem(new MenuItemMenu("MALE MIDDLE", CreateHumanSpawnerMenuExactFilter(controller, false, true, false, false, true, false)));
	menu->AddItem(new MenuItemMenu("MALE OLD", CreateHumanSpawnerMenuExactFilter(controller, false, true, false, false, false, true)));
	menu->AddItem(new MenuItemMenu("FEMALE YOUNG", CreateHumanSpawnerMenuExactFilter(controller, false, false, true, true, false, false)));
	menu->AddItem(new MenuItemMenu("FEMALE MIDDLE", CreateHumanSpawnerMenuExactFilter(controller, false, false, true, false, true, false)));
	menu->AddItem(new MenuItemMenu("FEMALE OLD", CreateHumanSpawnerMenuExactFilter(controller, false, false, true, false, false, true)));
	menu->AddItem(new MenuItemMenu("MISC", CreateHumanSpawnerMenuExactFilter(controller, false, false, false, false, false, false)));

	return menu;
}

enum eVehicleType
{
	vtAirbaloon,
	vtBoat,
	vtCannon,
	vtTrain,
	vtWagon
};

eVehicleType GetVehicleTypeUsingModel(string model)
{
	Hash hash = GAMEPLAY::GET_HASH_KEY(const_cast<char*>(model.c_str()));
	if (VEHICLE::IS_THIS_MODEL_A_BOAT(hash))
		return vtBoat;
	if (VEHICLE::IS_THIS_MODEL_A_TRAIN(hash))
		return vtTrain;
	if (model == "gatling_gun" || model == "gatlingMaxim02" || model == "hotchkiss_cannon" || model == "breach_cannon")
		return vtCannon;
	if (model == "hotAirBalloon01")
		return vtAirbaloon;
	return vtWagon;
}

MenuBase* CreateCannonSpawnerMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTitle("CANNON  SPAWNER"));
	controller->RegisterMenu(menu);

	auto menuItemWrapIn = new MenuItemSwitchable("WRAP IN SPAWNED");
	menu->AddItem(menuItemWrapIn);

	auto menuItemSetProperly = new MenuItemSwitchable("SET PROPERLY");
	menu->AddItem(menuItemSetProperly);

	for each (auto & model in vehicleModels)
		if (GetVehicleTypeUsingModel(model) == vtCannon)
			menu->AddItem(new MenuItemSpawnVehicle(model, { 0.0, 3.0, 0.0 }, 0.0, menuItemWrapIn, menuItemSetProperly, true, false));

	return menu;
}

MenuBase* CreateBoatSpawnerMenu(MenuController* controller,
	MenuItemSwitchable* menuItemWrapIn, MenuItemSwitchable* menuItemSetProperly)
{
	auto menu = new MenuBase(new MenuItemListTitle("BOAT  SPAWNER"));
	controller->RegisterMenu(menu);

	for each (auto & model in vehicleModels)
		if (GetVehicleTypeUsingModel(model) == vtBoat)
			menu->AddItem(new MenuItemSpawnVehicle(model, { 0.0, 10.0, 0.0 }, 90.0, menuItemWrapIn, menuItemSetProperly, false, false));

	return menu;
}

MenuBase* CreateTrainSpawnerMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemListTitle("TRAIN  SPAWNER"));
	controller->RegisterMenu(menu);

	for each (auto & model in vehicleModels)
		if (GetVehicleTypeUsingModel(model) == vtTrain)
			menu->AddItem(new MenuItemSpawnVehicle(model, { 0.0, 5.0, -1.0 }, 90.0, NULL, NULL, false, false));

	return menu;
}

MenuBase* CreateWagonSpawnerMenu(MenuController* controller,
	MenuItemSwitchable* menuItemWrapIn, MenuItemSwitchable* menuItemSetProperly, bool noPeds)
{
	auto menu = new MenuBase(new MenuItemListTitle("WAGON  SPAWNER"));
	controller->RegisterMenu(menu);

	for each (auto & model in vehicleModels)
		if (GetVehicleTypeUsingModel(model) == vtWagon)
			menu->AddItem(new MenuItemSpawnVehicle(model, { 1.0, 5.0, 0.0 }, 90.0, menuItemWrapIn, menuItemSetProperly, true, noPeds));

	return menu;
}

MenuBase* CreateVehicleMiscSpawnerMenu(MenuController* controller, MenuItemSwitchable* menuItemWrapIn)
{
	auto menu = new MenuBase(new MenuItemListTitle("MISC  SPAWNER"));
	controller->RegisterMenu(menu);

	for each (auto & model in vehicleModels)
		if (GetVehicleTypeUsingModel(model) == vtAirbaloon)
			menu->AddItem(new MenuItemSpawnVehicle(model, { 0.0, 5.0, 0.0 }, 0.0, menuItemWrapIn, NULL, false, false));

	return menu;
}

MenuBase* CreateVehicleSpawnerMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTitle("VEHICLE  SPAWNER"));
	controller->RegisterMenu(menu);

	auto menuItemWrapIn = new MenuItemSwitchable("WRAP IN SPAWNED");
	menu->AddItem(menuItemWrapIn);

	auto menuItemSetProperly = new MenuItemSwitchable("SET PROPERLY");
	menu->AddItem(menuItemSetProperly);

	menu->AddItem(new MenuItemMenu("BOATS", CreateBoatSpawnerMenu(controller, menuItemWrapIn, menuItemSetProperly)));
	menu->AddItem(new MenuItemMenu("TRAINS", CreateTrainSpawnerMenu(controller)));
	menu->AddItem(new MenuItemMenu("WAGONS", CreateWagonSpawnerMenu(controller, menuItemWrapIn, menuItemSetProperly, false)));
	menu->AddItem(new MenuItemMenu("JUST WAGONS", CreateWagonSpawnerMenu(controller, menuItemWrapIn, menuItemSetProperly, true)));
	menu->AddItem(new MenuItemMenu("MISC", CreateVehicleMiscSpawnerMenu(controller, menuItemWrapIn)));

	return menu;
}

MenuBase* CreateWeaponSelectMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemListTitle("GET WEAPON"));
	controller->RegisterMenu(menu);

	for each (auto info in weaponInfos)
		menu->AddItem(new MenuItemGiveWeapon(info.uiname, info.name));

	return menu;
}

MenuBase* CreateWeaponMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTitle("WEAPON  OPTIONS"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemMenu("GET ALL WEAPON", CreateWeaponSelectMenu(controller)));
	menu->AddItem(new MenuItemWeaponPowerfullGuns("POWERFULL GUNS"));
	menu->AddItem(new MenuItemWeaponPowerfullMelee("POWERFULL MELEE"));
	menu->AddItem(new MenuItemWeaponNoReload("NO RELOAD"));
	menu->AddItem(new MenuItemWeaponDropCurrent("DROP CURRENT"));

	return menu;
}

MenuBase* CreateTimeMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTimeTitle("TIME"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemTimeAdjust("HOUR FORWARD", 1));
	menu->AddItem(new MenuItemTimeAdjust("HOUR BACKWARD", -1));
	menu->AddItem(new MenuItemTimePause("CLOCK PAUSED"));
	menu->AddItem(new MenuItemTimeRealistic("CLOCK REAL"));
	menu->AddItem(new MenuItemTimeSystemSynced("SYNC WITH SYSTEM"));

	return menu;
}

MenuBase* CreateWeatherMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemListTitle("WEATHER"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemWeatherFreeze("FREEZE CURRENT"));
	menu->AddItem(new MenuItemWeatherWind("FAST WIND"));

	for each (string weather in weatherTypes)
		menu->AddItem(new MenuItemWeatherSelect(weather));

	return menu;
}

MenuBase* CreateMiscMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTitle("MISC  OPTIONS"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemMiscRevealMap("REVEAL MAP"));
	menu->AddItem(new MenuItemMiscAddHonor("ADD HONOR"));
	menu->AddItem(new MenuItemMiscHideHud("HIDE HUD"));

	menu->AddItem(new MenuItemNuclearBomb("EXPLODE NUKE", 200000.0f));

	menu->AddItem(new MenuItemMiscTransportGuns("HORSE TURRETS", true, true));
	menu->AddItem(new MenuItemMiscTransportGuns("HORSE CANNONS", true, false));
	menu->AddItem(new MenuItemMiscTransportGuns("VEHICLE TURRETS", false, true));
	menu->AddItem(new MenuItemMiscTransportGuns("VEHICLE CANNONS", false, false));
	menu->AddItem(new MenuItemDeathNoise("DEATH NOISE"));

	return menu;
}

MenuBase* CreateMainMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTitle("cvsTrainer 1.0.4"));
	controller->RegisterMenu(menu);

	menu->AddItem(new MenuItemMenu("PLAYER", CreatePlayerMenu(controller)));
	menu->AddItem(new MenuItemMenu("ANIMALS", CreateAnimalSpawnerMenu(controller)));
	menu->AddItem(new MenuItemMenu("PEDS", CreateHumanSpawnerMenu(controller)));
	menu->AddItem(new MenuItemMenu("CANNONS", CreateCannonSpawnerMenu(controller)));
	menu->AddItem(new MenuItemMenu("VEHICLES", CreateVehicleSpawnerMenu(controller)));
	menu->AddItem(new MenuItemMenu("WEAPON", CreateWeaponMenu(controller)));
	menu->AddItem(new MenuItemMenu("TIME", CreateTimeMenu(controller)));
	menu->AddItem(new MenuItemMenu("WEATHER", CreateWeatherMenu(controller)));
	menu->AddItem(new MenuItemMenu("MISC", CreateMiscMenu(controller)));

	return menu;
}

void main()
{
	auto menuController = new MenuController();
	auto mainMenu = CreateMainMenu(menuController);

	while (true)
	{
		if (!menuController->HasActiveMenu() && MenuInput::MenuSwitchPressed())
		{
			MenuInput::MenuInputBeep();
			menuController->PushMenu(mainMenu);
		}
		menuController->Update();
		WAIT(0);
	}
}

void ScriptMain()
{
	srand(GetTickCount64());
	main();
}