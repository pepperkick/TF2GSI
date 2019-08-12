#include "player.h"

#include <cstdint>

#include "cbase.h"
#include "c_basecombatcharacter.h"
#include "c_baseentity.h"
#include "cdll_int.h"
#include "globalvars_base.h"
#include "icliententity.h"
#include "icliententitylist.h"
#include "steam/steam_api.h"
#include "toolframework/ienginetool.h"
#include "Color.h"
#include <client_class.h>
#include "hltvcamera.h"

#include "../common.h"
#include "../entities.h"
#include "../exceptions.h"
#include "../ifaces.h"
#include "../camerastate.h"

using namespace tao;

CHandle<IClientEntity> playerResource;
Player::Offsets Player::offsets;
Player::PlayerResourceOffsets Player::playerResourceOffsets;

Player::Player(IClientEntity *entity) {
	this->playerEntity = entity;
}

Player::Player(int entindex) {
	this->playerEntity = Interfaces::pClientEntityList->GetClientEntity(entindex);
}

Player& Player::operator=(int entindex) {
	playerEntity = Interfaces::pClientEntityList->GetClientEntity(entindex);

	return *this;
}

Player& Player::operator=(IClientEntity *entity) {
	playerEntity = entity;

	return *this;
}

Player& Player::operator=(const Player &player) {
	if (this == &player) {
		return *this;
	}

	playerEntity = player.playerEntity;

	return *this;
}

bool Player::operator==(int entindex) const {
	return IsEqualTo(Player(entindex));
}

bool Player::operator==(IClientEntity *entity) const {
	return IsEqualTo(Player(entity));
}

bool Player::operator==(const Player &player) const {
	return IsEqualTo(player);
}

bool Player::operator!=(int entindex) const {
	return IsNotEqualTo(Player(entindex));
}

bool Player::operator!=(IClientEntity *entity) const {
	return IsNotEqualTo(Player(entity));
}

bool Player::operator!=(const Player &player) const {
	return IsNotEqualTo(player);
}

bool Player::operator<(int entindex) const {
	return IsLessThan(Player(entindex));
}

bool Player::operator<(IClientEntity *entity) const {
	return IsLessThan(Player(entity));
}

bool Player::operator<(const Player &player) const {
	return IsLessThan(player);
}

bool Player::operator<=(int entindex) const {
	return IsLessThanOrEqualTo(Player(entindex));
}

bool Player::operator<=(IClientEntity *entity) const {
	return IsLessThanOrEqualTo(Player(entity));
}

bool Player::operator<=(const Player &player) const {
	return IsLessThanOrEqualTo(player);
}

bool Player::operator>(int entindex) const {
	return IsGreaterThan(Player(entindex));
}

bool Player::operator>(IClientEntity *entity) const {
	return IsGreaterThan(Player(entity));
}

bool Player::operator>(const Player &player) const {
	return IsGreaterThan(player);
}

bool Player::operator>=(int entindex) const {
	return IsGreaterThanOrEqualTo(Player(entindex));
}

bool Player::operator>=(IClientEntity *entity) const {
	return IsGreaterThanOrEqualTo(Player(entity));
}

bool Player::operator>=(const Player &player) const {
	return IsGreaterThanOrEqualTo(player);
}

bool Player::IsEqualTo(const Player &player) const {
	if (IsValid() && player.IsValid()) {
		return playerEntity == player.playerEntity;
	}

	return false;
}

bool Player::IsNotEqualTo(const Player &player) const {
	return !IsEqualTo(player);
}

bool Player::IsLessThan(const Player &player) const {
	if (!IsValid()) {
		return true;
	}

	if (!player.IsValid()) {
		return false;
	}

	if (GetTeam() < player.GetTeam()) {
		return true;
	}
	else if (GetTeam() > player.GetTeam()) {
		return false;
	}

	if (TFDefinitions::normalClassOrdinal.find(GetClass())->second < TFDefinitions::normalClassOrdinal.find(player.GetClass())->second) {
		return true;
	}
	else if (TFDefinitions::normalClassOrdinal.find(GetClass())->second > TFDefinitions::normalClassOrdinal.find(player.GetClass())->second) {
		return false;
	}

	if (this->GetEntity()->entindex() < player.GetEntity()->entindex()) {
		return true;
	}
	else if (this->GetEntity()->entindex() > player.GetEntity()->entindex()) {
		return false;
	}

	return false;
}

bool Player::IsLessThanOrEqualTo(const Player &player) const {
	return IsEqualTo(player) || IsLessThan(player);
}

bool Player::IsGreaterThan(const Player &player) const {
	if (!IsValid()) {
		return false;
	}

	if (!player.IsValid()) {
		return true;
	}

	if (GetTeam() > player.GetTeam()) {
		return true;
	}
	else if (GetTeam() < player.GetTeam()) {
		return false;
	}

	if (TFDefinitions::normalClassOrdinal.find(GetClass())->second > TFDefinitions::normalClassOrdinal.find(player.GetClass())->second) {
		return true;
	}
	else if (TFDefinitions::normalClassOrdinal.find(GetClass())->second < TFDefinitions::normalClassOrdinal.find(player.GetClass())->second) {
		return false;
	}

	if (this->GetEntity()->entindex() > player.GetEntity()->entindex()) {
		return true;
	}
	else if (this->GetEntity()->entindex() < player.GetEntity()->entindex()) {
		return false;
	}

	return false;
}

bool Player::IsGreaterThanOrEqualTo(const Player &player) const {
	return IsEqualTo(player) || IsGreaterThan(player);
}

Player::operator bool() const {
	return IsValid();
}

bool Player::IsValid() const {
	return playerEntity.IsValid() && playerEntity.Get() && playerEntity->entindex() >= 1 && playerEntity->entindex() <= Interfaces::pEngineTool->GetMaxClients() && Entities::CheckEntityBaseclass(playerEntity, "TFPlayer");
}

Player::operator IClientEntity *() const {
	return playerEntity;
}

IClientEntity *Player::operator->() const {
	return playerEntity;
}

IClientEntity *Player::GetEntity() const {
	return playerEntity;
}

json::value Player::GetData() {
	std::string steamid = std::to_string(this->GetSteamID().ConvertToUint64());
	Vector position = dynamic_cast<C_BaseEntity*>(playerEntity.Get())->GetAbsOrigin();
	bool isAlive = dynamic_cast<C_BaseEntity*>(playerEntity.Get())->IsAlive();
	int tfCLass = (TFClassType)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.m_iClass);

	json::value data = {
		{ "name",		this->GetName().c_str() },
		{ "steamid",	steamid },
		{ "alive",		isAlive },
		{ "maxHealth",	(int)* Entities::GetEntityValueAtOffset<int*>(playerResource.Get(), this->playerResourceOffsets.m_iMaxHealth + (4 * playerEntity.GetEntryIndex())) },
		{ "team",		(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity.Get(), this->offsets.m_iTeamNum) },
		{ "health",		(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity.Get(), this->offsets.m_iHealth) },
		{ "class",		(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity.Get(), this->offsets.m_iClass) },
		{ "totalStats", {
			{ "score",		(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.totalStats.m_iPoints) },
			{ "kills",		(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.totalStats.m_iKills) },
			{ "assists",	(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.totalStats.m_iKillAssists) },
			{ "deaths",		(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.totalStats.m_iDeaths) },
			{ "damage",		(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.totalStats.m_iDamageDone) },
			{ "headshots",	(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.totalStats.m_iHeadshots) },
			{ "healing",	(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.totalStats.m_iHealPoints) },
		}},
		{ "rountStats", {
			{ "score",		(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.roundStats.m_iPoints) },
			{ "kills",		(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.roundStats.m_iKills) },
			{ "assists",	(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.roundStats.m_iKillAssists) },
			{ "deaths",		(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.roundStats.m_iDeaths) },
			{ "damage",		(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.roundStats.m_iDamageDone) },
			{ "headshots",	(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.roundStats.m_iHeadshots) },
			{ "healing",	(int)* Entities::GetEntityValueAtOffset<int*>(this->playerEntity, this->offsets.roundStats.m_iHealPoints) },
		}},
		{ "respawnTime", (float)* Entities::GetEntityValueAtOffset<float*>(playerResource, this->playerResourceOffsets.m_flNextRespawnTime + (4 * playerEntity.GetEntryIndex())) - Interfaces::GetEngineTools()->ClientTime() },
		{ "position", { { "x", position.x }, { "y", position.y }, { "z", position.z } } }
	};

	if (this->FindCondition()) {
		if (this->CheckCondition(TFCond::TFCond_Slowed)) {
			data["isSlowed"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_Ubercharged)) {
			data["isUbered"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_Bleeding)) {
			data["isBleeding"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_Bonked)) {
			data["isBonked"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_MarkedForDeath)) {
			data["isMarkedForDeath"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_MarkedForDeathSilent)) {
			data["isMarkedForDeath"] = true;
			data["isMarkedForDeathSilent"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_SpeedBuffAlly)) {
			data["isAllySpeedBuffed"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_Cloaked)) {
			data["isCloaked"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_CloakFlicker)) {
			data["isCloakFlickering"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_Disguised)) {
			data["isDisguised"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_Zoomed)) {
			data["isZoomed"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_OnFire)) {
			data["isOnFire"] = true;
		}

		if (this->CheckCondition(TFCond::TFCond_Kritzkrieged)) {
			data["isKritzkrieged"] = true;
		}

		if (isAlive) {
			CHandle<IClientEntity> weapon = CHandle<IClientEntity>(this->GetActiveWeapon());

			if (weapon) {
				int type = *Entities::GetEntityProp<int*>(weapon, { "m_iPrimaryAmmoType" });
				int ammo = -1;

				if (type != -1)
					ammo = this->GetWeaponAmmo(type);

				data["weapon"] = {
					{ "class", Entities::GetEntityClassname(weapon) },
					{ "clip1", *Entities::GetEntityProp<int*>(weapon.Get(), { "m_iClip1" }) },
					{ "clip2", *Entities::GetEntityProp<int*>(weapon.Get(), { "m_iClip2" }) },
					{ "index", *Entities::GetEntityProp<int*>(weapon.Get(), { "m_iItemDefinitionIndex" }) },
					{ "reserve", ammo },
				};
			}
		}


		if (tfCLass == TFClassType::TFClass_Medic) {
			int type = this->GetMedigunType();
			float charge = this->GetMedigunCharge();
			bool isHealing = this->IsMedigunHealing();

			std::string name;
			name = "Unknown";

			std::ostringstream targetId;
			targetId.str("");
			targetId.clear();

			// TODO: Fix this
			if (isHealing) {
				CHandle<IClientEntity> target = this->GetMedigunTarget();
				if (target) {
					int index = target.GetEntryIndex();
					Player healing = Player(index);

					targetId << healing.GetSteamID().ConvertToUint64();
				}
			}

			switch (type) {
			case TFMedigun_Unknown:
				name = "Unknown";
				break;
			case TFMedigun_MediGun:
				name = "MediGun";
				break;
			case TFMedigun_Kritzkrieg:
				name = "Kritzkrieg";
				break;
			case TFMedigun_QuickFix:
				name = "QuickFix";
				break;
			case TFMedigun_Vaccinator:
				name = "Vaccinator";
				break;
			default:
				name = "Unknown";
			}

			data["medigun"] = {
				{ "type", name },
				{ "charge", charge },
				{ "target",  targetId.str() },
			};
		}
		else if (tfCLass == TFClassType::TFClass_Spy) {
			data["disguise"] = {
				{ "class", this->GetDisguiseClass() },
				{ "team", this->GetDisguiseTeam() },
			};
		}
	}
	return data;
}

bool Player::FindCondition() {
	if (IsValid()) {
		uint32_t condBits = *Entities::GetEntityProp<uint32_t*>(playerEntity.Get(), { "_condition_bits" });
		uint32_t playerCond = *Entities::GetEntityProp<uint32_t*>(playerEntity.Get(), { "m_nPlayerCond" });
		uint32_t playerCondEx = *Entities::GetEntityProp<uint32_t*>(playerEntity.Get(), { "m_nPlayerCondEx" });
		uint32_t playerCondEx2 = *Entities::GetEntityProp<uint32_t*>(playerEntity.Get(), { "m_nPlayerCondEx2" });
		uint32_t playerCondEx3 = *Entities::GetEntityProp<uint32_t*>(playerEntity.Get(), { "m_nPlayerCondEx3" });

		playerConditions[0] = (condBits | playerCond);
		playerConditions[1] = playerCondEx;
		playerConditions[2] = playerCondEx2;
		playerConditions[3] = playerCondEx3;

		return true;
	}

	return false;
}

bool Player::CheckCondition(TFCond condition) const {
	if (condition < 32) {
		if (playerConditions[0] & (1 << condition)) {
			return true;
		}
	}
	else if (condition < 64) {
		if (playerConditions[1] & (1 << (condition - 32))) {
			return true;
		}
	}
	else if (condition < 96) {
		if (playerConditions[2] & (1 << (condition - 64))) {
			return true;
		}
	}
	else if (condition < 128) {
		if (playerConditions[3] & (1 << (condition - 96))) {
			return true;
		}
	}

	return false;
}

Vector Player::GetPosition() const {
	if (IsValid()) {
		return dynamic_cast<C_BaseEntity*>(playerEntity.Get())->GetAbsOrigin();
	}

	return {};
}

TFClassType Player::GetClass() const {
	if (IsValid()) {
		return (TFClassType)*Entities::GetEntityProp<int *>(playerEntity.Get(), { "m_iClass" });
	}

	return TFClass_Unknown;
}

int Player::GetHealth() const {
	if (IsValid()) {
		return dynamic_cast<C_BaseEntity *>(playerEntity.Get())->GetHealth();
	}

	return 0;
}

int Player::GetMaxHealth() const {
	if (IsValid()) {
		if (!playerResource.Get()) return -1;

		char index[4];
		GetPropIndexString(playerEntity->entindex(), index);
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iMaxHealth", index });
	}

	return -1;
}

int Player::GetMaxBuffedHealth() const {
	if (IsValid()) {
		if (!playerResource.Get()) return -1;

		char index[4];
		GetPropIndexString(playerEntity->entindex(), index);
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iMaxBuffedHealth", index });
	}

	return -1;
}

std::string Player::GetName() const {
	if (IsValid()) {
		player_info_t playerInfo;

		if (Interfaces::pEngineClient->GetPlayerInfo(playerEntity->entindex(), &playerInfo)) {
			return playerInfo.name;
		}
	}
	
	return "";
}

int Player::GetObserverMode() const {
	if (IsValid()) {
		return (int)*Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_iObserverMode" });
	}

	return OBS_MODE_NONE;
}

C_BaseEntity *Player::GetObserverTarget() const {
	if (IsValid()) {
		return (EHANDLE)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_hObserverTarget" });
	}

	return playerEntity->GetBaseEntity();
}

int Player::GetDominations() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_iDominations" });
	}

	return 0;
}

int Player::GetDominated() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_bPlayerDominated" });
	}

	return 0;
}

int Player::GetDominatedBy() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_bPlayerDominatingMe" });
	}

	return 0;
}

int Player::GetScore() const {
	if (IsValid()) {
		if (!playerResource.Get()) return -1;

		char index[4];
		GetPropIndexString(playerEntity->entindex(), index);
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iScore", index });
	}

	return -1;
}

int Player::GetTotalScore() const {
	if (IsValid()) {
		if (!playerResource.Get()) return -1;

		char index[4];
		GetPropIndexString(playerEntity->entindex(), index);
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iTotalScore", index });
	}

	return -1;
}

int Player::GetDeaths() const {
	if (IsValid()) {
		if (!playerResource.Get()) return -1;

		char index[4];
		GetPropIndexString(playerEntity->entindex(), index);
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iDeaths", index });
	}

	return -1;
}

int Player::GetHealing() const {
	if (IsValid()) {
		if (!playerResource.Get()) return -1;

		char index[4];
		GetPropIndexString(playerEntity->entindex(), index);
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iHealing", index });
	}

	return -1;
}

int Player::GetDamage() const {
	if (IsValid()) {
		if (!playerResource.Get()) return -1;

		char index[4];
		GetPropIndexString(playerEntity->entindex(), index);
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iDamage", index });
	}

	return -1;
}

int Player::GetTotalHeadshots() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityValueAtOffset<int*>(playerEntity.Get(), 6200);		// CTFPlayer > m_ScoreData > m_iHeadshots
	}

	return -1;
}

int Player::GetTotalDamage() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityValueAtOffset<int*>(playerEntity.Get(), 6220);		// CTFPlayer > m_ScoreData > m_iDamageDone
	}

	return -1;
}

int Player::GetCaptures() const {
	if (IsValid()) {
		if (!playerResource.Get()) return -1;

		char index[4];
		GetPropIndexString(playerEntity->entindex(), index);
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iCaptures", index });
	}

	return -1;
}

int Player::GetDefenses() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_iDefenses" });
	}

	return 0;
}

int Player::GetRevenges() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_iRevenge" });
	}

	return 0;
}

int Player::GetBuildingsDestroyed() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_iBuildingsDestroyed" });
	}

	return 0;
}

int Player::GetHeadshots() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_iHeadshots" });
	}

	return 0;
}

int Player::GetBackstabs() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_iBackstabs" });
	}

	return 0;
}

int Player::GetHeals() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_iHealPoints" });
	}

	return 0;
}

int Player::GetUbers() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_iInvulns" });
	}

	return 0;
}

int Player::GetTeleports() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_iTeleports" });
	}

	return 0;
}

int Player::GetKillAssists() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_ScoreData", "m_iKillAssists" });
	}

	return 0;
}

int Player::GetKillstreak(int i) const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_nStreaks", std::to_string(i) });
	}

	return -1;
}

float Player::GetRespawnTime() const {
	if (IsValid()) {
		if (!playerResource.Get()) return -1;

		char index[4];
		GetPropIndexString(playerEntity->entindex(), index);
		return (float)* Entities::GetEntityProp<float*>(playerResource.Get(), { "m_flNextRespawnTime", index });
	}

	return -1;
}

CSteamID Player::GetSteamID() const {
	if (IsValid()) {
		player_info_t playerInfo;

		if (Interfaces::pEngineClient->GetPlayerInfo(playerEntity->entindex(), &playerInfo)) {
			if (playerInfo.friendsID) {
				static EUniverse universe = k_EUniverseInvalid;

				if (universe == k_EUniverseInvalid) {
					if (Interfaces::pSteamAPIContext->SteamUtils()) {
						universe = Interfaces::pSteamAPIContext->SteamUtils()->GetConnectedUniverse();
					}
					else {
						PRINT_TAG();
						Warning("Steam libraries not available - assuming public universe for user Steam IDs!\n");

						universe = k_EUniversePublic;
					}
				}

				return CSteamID(playerInfo.friendsID, 1, universe, k_EAccountTypeIndividual);
			}
		}
	}

	return CSteamID();
}

int Player::GetDisguiseTeam() const {
	if (IsValid()) {
		return *Entities::GetEntityValueAtOffset<int*>(playerEntity, offsets.m_nDisguiseTeam);
	}

	return 0;
}

int Player::GetDisguiseClass() const {
	if (IsValid()) {
		return *Entities::GetEntityValueAtOffset<int*>(playerEntity, offsets.m_nDisguiseClass);
	}

	return 0;
}

TFTeam Player::GetTeam() const {
	if (IsValid()) {
		TFTeam team = (TFTeam)dynamic_cast<C_BaseEntity *>(playerEntity.Get())->GetTeamNumber();

		return team;
	}

	return TFTeam_Unassigned;
}

int Player::GetUserID() const {
	if (IsValid()) {
		player_info_t playerInfo;

		if (Interfaces::pEngineClient->GetPlayerInfo(playerEntity->entindex(), &playerInfo)) {
			return playerInfo.userID;
		}
	}

	return 0;
}

int Player::GetActiveWeapon() const {
	if (IsValid()) {
		return *Entities::GetEntityValueAtOffset<int*>(playerEntity.Get(), offsets.m_hActiveWeapon);
	}

	return nullptr;
}

int Player::GetWeaponAmmo(int i) const {
	if (IsValid()) {
		return *Entities::GetEntityValueAtOffset<int*>(playerEntity.Get(), offsets.m_iAmmo + (4 * i));
	}

	return nullptr;
}

C_BaseCombatWeapon* Player::GetWeapon(int i) const {
	if (IsValid()) {
		return dynamic_cast<C_BaseCombatCharacter*>(playerEntity.Get())->GetWeapon(i);
	}

	return nullptr;
}

int Player::GetWeaponDefinationIndex(C_BaseCombatWeapon* weapon) const {
	if (IsValid()) {
		return *Entities::GetEntityProp<int*>(weapon, { "m_iItemDefinitionIndex" });
	}

	return nullptr;
}

int Player::IsWearingWeapon() const {
	int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

	for (int i = 0; i <= maxEntity; i++) {
		IClientEntity* entity = Interfaces::pClientEntityList->GetClientEntity(i);

		if (!entity) {
			continue;
		}

		if (Entities::CheckEntityBaseclass(entity, "CTFWearableItem")) {
			int* owner = (int*)Entities::GetEntityProp<int*>(entity, { "m_hOwnerEntity" });
			int* index = (int*)Entities::GetEntityProp<int*>(entity, { "m_iItemDefinitionIndex" });

			if (*owner == playerEntity.GetEntryIndex()) {
				return *index;
			}

			break;
		}
	}

	return -1;
}

C_BaseCombatWeapon* Player::GetWeaponIndexByClass(const char* wepclass) {
	std::string s = wepclass;
	s.erase(0, 1);
	
	for (int i = 0; i < MAX_WEAPONS; i++) {
		C_BaseCombatWeapon* weapon = GetWeapon(i);

		if (weapon) {
			const char* wep = Entities::GetEntityClassname(weapon);

			if (Entities::CheckEntityBaseclass(weapon, s)) {
				return weapon;
			}
		}
	}

	return nullptr;
}

C_BaseCombatWeapon* Player::GetMedigun() const {
	if (GetClass() == TFClassType::TFClass_Medic) {
		for (int i = 0; i < MAX_WEAPONS; i++) {
			C_BaseCombatWeapon* weapon = GetWeapon(i);

			if (weapon && Entities::CheckEntityBaseclass(weapon, "WeaponMedigun")) {
				return weapon;
			}
		}
	}

	return nullptr;
}

bool Player::IsMedigunHealing() const {
	C_BaseCombatWeapon* weapon = GetMedigun();

	if (weapon) {
		return *Entities::GetEntityProp<bool *>(weapon, { "m_bHealing" });
	}

	return false;
}

int Player::GetMedigunTarget() const {
	C_BaseCombatWeapon* weapon = GetMedigun();
	
	if (weapon) {
		return *Entities::GetEntityProp<int *>(weapon, { "m_hHealingTarget" });
	}

	return nullptr;
}

int Player::GetMedigunType() const {
	C_BaseCombatWeapon* weapon = GetMedigun();

	if (weapon) {
		int itemDefinitionIndex = *Entities::GetEntityProp<int*>(weapon, { "m_iItemDefinitionIndex" });
		TFMedigun type = TFMedigun_Unknown;
	
		if (itemDefinitionIndex == 29 || itemDefinitionIndex == 211 || itemDefinitionIndex == 663 || itemDefinitionIndex == 796 || itemDefinitionIndex == 805 || itemDefinitionIndex == 885 || itemDefinitionIndex == 894 || itemDefinitionIndex == 903 || itemDefinitionIndex == 912 || itemDefinitionIndex == 961 || itemDefinitionIndex == 970 || itemDefinitionIndex == 15008 || itemDefinitionIndex == 15010 || itemDefinitionIndex == 15025 || itemDefinitionIndex == 15039 || itemDefinitionIndex == 15050) {
			type = TFMedigun_MediGun;
		} 
		else if (itemDefinitionIndex == 35) {
			type = TFMedigun_Kritzkrieg;
		}
		else if (itemDefinitionIndex == 411) {
			type = TFMedigun_QuickFix;
		}
		else if (itemDefinitionIndex == 998) {
			type = TFMedigun_Vaccinator;
		}

		return type;	
	}

	return TFMedigun_Unknown;
}

float Player::GetMedigunCharge() const {
	C_BaseCombatWeapon* weapon = GetMedigun();

	if (weapon) {
		return *Entities::GetEntityProp<float*>(weapon, { "m_flChargeLevel" });
	}

	return -1;
}

bool Player::IsAlive() const {
	if (IsValid()) {
		return dynamic_cast<C_BaseEntity *>(playerEntity.Get())->IsAlive();
	}

	return false;
}

Player::Iterator::Iterator(const Player::Iterator& old) {
	index = old.index;
}

Player::Iterator& Player::Iterator::operator=(const Player::Iterator& old) {
	index = old.index;

	return  *this;
};

Player::Iterator& Player::Iterator::operator++() {
	for (int i = index + 1; i <= Interfaces::pEngineTool->GetMaxClients(); i++) {
		if (Player(i)) {
			index = i;

			return *this;
		}
	}

	index = Interfaces::pEngineTool->GetMaxClients() + 1;

	return *this;
};

Player Player::Iterator::operator*() const {
	return Player(index);
}

void swap(Player::Iterator& lhs, Player::Iterator& rhs) {
	using std::swap;
	swap(lhs.index, rhs.index);
}

Player::Iterator Player::Iterator::operator++(int) {
	Player::Iterator current(*this);

	for (int i = index + 1; i <= Interfaces::pEngineTool->GetMaxClients(); i++) {
		if (Player(i)) {
			index = i;

			return current;
		}
	}

	index = Interfaces::pEngineTool->GetMaxClients() + 1;

	return current;
}

Player *Player::Iterator::operator->() const {
	return new Player(index);
}

bool operator==(const Player::Iterator& lhs, const Player::Iterator& rhs) {
	return lhs.index == rhs.index;
}

bool operator!=(const Player::Iterator& lhs, const Player::Iterator& rhs) {
	return lhs.index != rhs.index;
}

Player::Iterator::Iterator() {
	for (int i = 1; i <= Interfaces::pEngineTool->GetMaxClients(); i++) {
		if (Player(i)) {
			index = i;

			return;
		}
	}

	index = Interfaces::pEngineTool->GetMaxClients() + 1;

	return;
}

Player::Iterator& Player::Iterator::operator--() {
	for (int i = index - 1; i >= 1; i++) {
		if (Player(i)) {
			index = i;

			return *this;
		}
	}

	index = 0;

	return *this;
}

Player::Iterator Player::Iterator::operator--(int) {
	Player::Iterator current(*this);

	for (int i = index - 1; i >= 1; i++) {
		if (Player(i)) {
			index = i;

			return current;
		}
	}

	index = 0;

	return current;
}

Player::Iterator::Iterator(int startIndex) {
	index = startIndex;
}

Player::Iterator Player::begin() {
	return Player::Iterator();
}

Player::Iterator Player::end() {
	return Player::Iterator(Interfaces::pEngineTool->GetMaxClients() + 1);
}

Player::Iterator Player::Iterable::begin() {
	return Player::begin();
}

Player::Iterator Player::Iterable::end() {
	return Player::end();
}

Player Player::GetLocalPlayer() {
	return Interfaces::GetEngineClient()->GetLocalPlayer();
}

int Player::GetTargetObserverMode() {
	HLTVCameraOverride* hltvcamera = (HLTVCameraOverride*)Interfaces::GetHLTVCamera();
	return hltvcamera->m_nCameraMode;
}

Player Player::GetTargetPlayer() {
	if (Interfaces::GetEngineClient()->IsHLTV()) {
		HLTVCameraOverride* hltvcamera = (HLTVCameraOverride*)Interfaces::GetHLTVCamera();
		int mode = hltvcamera->m_nCameraMode;

		if (mode == OBS_MODE_CHASE || mode == OBS_MODE_IN_EYE) {
			Player targetPlayer = hltvcamera->m_iTraget1;

			if (targetPlayer) return targetPlayer;
		}
	}
	else {
		Player localPlayer = Player::GetLocalPlayer();
		if (localPlayer) {
			int mode = localPlayer.GetObserverMode();

			if (mode == OBS_MODE_CHASE || mode == OBS_MODE_IN_EYE) {
				Player targetPlayer = localPlayer.GetObserverTarget();

				if (targetPlayer) return targetPlayer;
			}
		}
	}

	return nullptr;
}

bool Player::classRetrievalAvailable = false;
bool Player::comparisonAvailable = false;
bool Player::conditionsRetrievalAvailable = false;
bool Player::nameRetrievalAvailable = false;
bool Player::steamIDRetrievalAvailable = false;
bool Player::userIDRetrievalAvailable = false;

bool Player::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pClientEntityList) {
		PRINT_TAG();
		Warning("Required interface IClientEntityList for player helper class not available!\n");

		ready = false;
	}

	if (!Interfaces::pEngineTool) {
		PRINT_TAG();
		Warning("Required interface IEngineTool for player helper class not available!\n");

		ready = false;
	}

	classRetrievalAvailable = true;
	comparisonAvailable = true;
	conditionsRetrievalAvailable = true;
	nameRetrievalAvailable = true;
	steamIDRetrievalAvailable = true;
	userIDRetrievalAvailable = true;

	if (!Interfaces::pEngineClient) {
		PRINT_TAG();
		Warning("Interface IVEngineClient for player helper class not available (required for retrieving certain info)!\n");

		nameRetrievalAvailable = false;
		steamIDRetrievalAvailable = false;
		userIDRetrievalAvailable = false;
	}

	if (!Interfaces::steamLibrariesAvailable) {
		PRINT_TAG();
		Warning("Steam libraries for player helper class not available (required for accuracy in retrieving Steam IDs)!\n");
	}

	if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_nPlayerCond" })) {
		PRINT_TAG();
		Warning("Required property m_nPlayerCond for CTFPlayer for player helper class not available!\n");

		conditionsRetrievalAvailable = false;
	}
		
	if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "_condition_bits" })) {
		PRINT_TAG();
		Warning("Required property _condition_bits for CTFPlayer for player helper class not available!\n");

		conditionsRetrievalAvailable = false;
	}
		
	if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_nPlayerCondEx" })) {
		PRINT_TAG();
		Warning("Required property m_nPlayerCondEx for CTFPlayer for player helper class not available!\n");

		conditionsRetrievalAvailable = false;
	}
			
	if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_nPlayerCondEx2" })) {
		PRINT_TAG();
		Warning("Required property m_nPlayerCondEx2 for CTFPlayer for player helper class not available!\n");

		conditionsRetrievalAvailable = false;
	}

	if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_nPlayerCondEx3" })) {
		PRINT_TAG();
		Warning("Required property m_nPlayerCondEx3 for CTFPlayer for player helper class not available!\n");

		conditionsRetrievalAvailable = false;
	}

	if (!Entities::RetrieveClassPropOffset("CTFPlayer", { "m_iClass" })) {
		PRINT_TAG();
		Warning("Required property m_iClass for CTFPlayer for player helper class not available!\n");

		classRetrievalAvailable = false;
		comparisonAvailable = false;
	}

	return ready;
}

void Player::FindPlayerResource() {
	int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

	for (int i = 0; i <= maxEntity; i++) {
		IClientEntity* entity = Interfaces::pClientEntityList->GetClientEntity(i);

		if (!entity) {
			continue;
		}

		if (Entities::CheckEntityBaseclass(entity, "TFPlayerResource")) {
			playerResource = dynamic_cast<C_BaseEntity*>(entity);
			break;
		}
	}
}

void Player::InitOffsets() {
	const char* classname = "CTFPlayer";

	offsets.m_iClass =
		Entities::GetClassPropOffset(classname, { "m_iClass" });
	offsets.m_iHealth =
		Entities::GetClassPropOffset(classname, { "m_iHealth" });
	offsets.m_iTeamNum =
		Entities::GetClassPropOffset(classname, { "m_iTeamNum" });
	offsets.m_lifeState =
		Entities::GetClassPropOffset(classname, { "m_lifeState" });
	offsets.m_hActiveWeapon =
		Entities::GetClassPropOffset(classname, { "m_hActiveWeapon" });
	offsets.m_iAmmo =
		Entities::GetClassPropOffset(classname, { "m_iAmmo" });
	offsets.m_nDisguiseTeam =
		Entities::GetClassPropOffset(classname, { "m_nDisguiseTeam" });
	offsets.m_nDisguiseClass =
		Entities::GetClassPropOffset(classname, { "m_nDisguiseClass" });

	offsets.totalStats.m_iCaptures =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iCaptures" });
	offsets.totalStats.m_iDefenses =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iDefenses" });
	offsets.totalStats.m_iKills =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iKills" });
	offsets.totalStats.m_iBuildingsBuilt =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iBuildingsBuilt" });
	offsets.totalStats.m_iBuildingsDestroyed =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iBuildingsDestroyed" });
	offsets.totalStats.m_iHeadshots =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iHeadshots" });
	offsets.totalStats.m_iBackstabs =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iBackstabs" });
	offsets.totalStats.m_iHealPoints =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iHealPoints" });
	offsets.totalStats.m_iInvulns =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iInvulns" });
	offsets.totalStats.m_iTeleports =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iTeleports" });
	offsets.totalStats.m_iDamageDone =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iDamageDone" });
	offsets.totalStats.m_iCrits =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iCrits" });
	offsets.totalStats.m_iResupplyPoints =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iResupplyPoints" });
	offsets.totalStats.m_iKillAssists =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iKillAssists" });
	offsets.totalStats.m_iBonusPoints =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iBonusPoints" });
	offsets.totalStats.m_iPoints =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iPoints" });
	offsets.totalStats.m_iDeaths =
		Entities::GetClassPropOffset(classname, { "m_ScoreData", "m_iDeaths" });

	offsets.roundStats.m_iCaptures =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iCaptures" });
	offsets.roundStats.m_iDefenses =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iDefenses" });
	offsets.roundStats.m_iKills =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iKills" });
	offsets.roundStats.m_iBuildingsBuilt =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iBuildingsBuilt" });
	offsets.roundStats.m_iBuildingsDestroyed =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iBuildingsDestroyed" });
	offsets.roundStats.m_iHeadshots =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iHeadshots" });
	offsets.roundStats.m_iBackstabs =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iBackstabs" });
	offsets.roundStats.m_iHealPoints =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iHealPoints" });
	offsets.roundStats.m_iInvulns =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iInvulns" });
	offsets.roundStats.m_iTeleports =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iTeleports" });
	offsets.roundStats.m_iDamageDone =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iDamageDone" });
	offsets.roundStats.m_iCrits =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iCrits" });
	offsets.roundStats.m_iResupplyPoints =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iResupplyPoints" });
	offsets.roundStats.m_iKillAssists =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iKillAssists" });
	offsets.roundStats.m_iBonusPoints =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iBonusPoints" });
	offsets.roundStats.m_iPoints =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iPoints" });
	offsets.roundStats.m_iDeaths =
		Entities::GetClassPropOffset(classname, { "m_RoundScoreData", "m_iDeaths" });
}

void Player::SetPlayerResource(CHandle<IClientEntity> entity) {
	playerResource = entity;

	const char* classname = Entities::GetEntityClassname(playerResource);

	playerResourceOffsets.m_flNextRespawnTime = 
		Entities::GetClassPropOffset(classname, { "m_flNextRespawnTime" });
	playerResourceOffsets.m_iMaxHealth =
		Entities::GetClassPropOffset(classname, { "m_iMaxHealth" });
}
