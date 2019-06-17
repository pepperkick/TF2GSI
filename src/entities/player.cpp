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

CHandle<C_BaseEntity> playerResource;
Player::Player(int entindex) {
	playerEntity = Interfaces::pClientEntityList->GetClientEntity(entindex);
}

Player::Player(IClientEntity *entity) {
	playerEntity = entity;
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

bool Player::CheckCondition(TFCond condition) const {
	if (IsValid()) {
		uint32_t playerCond = *Entities::GetEntityProp<uint32_t *>(playerEntity.Get(), { "m_nPlayerCond" });
		uint32_t condBits = *Entities::GetEntityProp<uint32_t *>(playerEntity.Get(), { "_condition_bits" });
		uint32_t playerCondEx = *Entities::GetEntityProp<uint32_t *>(playerEntity.Get(), { "m_nPlayerCondEx" });
		uint32_t playerCondEx2 = *Entities::GetEntityProp<uint32_t *>(playerEntity.Get(), { "m_nPlayerCondEx2" });
		uint32_t playerCondEx3 = *Entities::GetEntityProp<uint32_t *>(playerEntity.Get(), { "m_nPlayerCondEx3" });

		uint32_t conditions[4];
		conditions[0] = playerCond | condBits;
		conditions[1] = playerCondEx;
		conditions[2] = playerCondEx2;
		conditions[3] = playerCondEx3;

		if (condition < 32) {
			if (conditions[0] & (1 << condition)) {
				return true;
			}
		}
		else if (condition < 64) {
			if (conditions[1] & (1 << (condition - 32))) {
				return true;
			}
		}
		else if (condition < 96) {
			if (conditions[2] & (1 << (condition - 64))) {
				return true;
			}
		}
		else if (condition < 128) {
			if (conditions[3] & (1 << (condition - 96))) {
				return true;
			}
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
	if (!playerResource.Get()) return -1;

	char index[4];
	GetPropIndexString(playerEntity->entindex(), index);

	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iMaxHealth", index });
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
	if (!playerResource.Get()) return -1;

	char index[4];
	GetPropIndexString(playerEntity->entindex(), index);

	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iScore", index });
	}

	return -1;
}

int Player::GetTotalScore() const {	
	if (!playerResource.Get()) return -1;

	char index[4];
	GetPropIndexString(playerEntity->entindex(), index);

	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iTotalScore", index });
	}

	return -1;
}

int Player::GetDeaths() const {
	if (!playerResource.Get()) return -1;

	char index[4];
	GetPropIndexString(playerEntity->entindex(), index);

	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iDeaths", index });
	}

	return -1;
}

int Player::GetDamage() const {
	if (!playerResource.Get()) return -1;

	char index[4];
	GetPropIndexString(playerEntity->entindex(), index);

	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(playerResource.Get(), { "m_iDamage", index });
	}

	return -1;
}

int Player::GetCaptures() const {
	if (!playerResource.Get()) return -1;

	char index[4];
	GetPropIndexString(playerEntity->entindex(), index);

	if (IsValid()) {
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
		return (int)* Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_iKillAssists" });
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
	if (!playerResource.Get()) return -1;

	char index[4];
	GetPropIndexString(playerEntity->entindex(), index);

	if (IsValid()) {
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
		return *Entities::GetEntityProp<int*>(playerEntity.Get(), { "m_hActiveWeapon" });
	}

	return nullptr;
}


C_BaseCombatWeapon *Player::GetWeapon(int i) const {
	if (IsValid()) {
		return dynamic_cast<C_BaseCombatCharacter *>(playerEntity.Get())->GetWeapon(i);
	}

	return nullptr;
}

int Player::GetWeaponClip(int i) const {
	C_BaseCombatWeapon* weapon = GetWeapon(i);

	if (weapon) {
		return *Entities::GetEntityProp<int*>(weapon, { "m_iClip1" });
	}

	return -1;
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
			ConColorMsg(Color(255, 255, 0, 255), "Mode: %d\n", mode);

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