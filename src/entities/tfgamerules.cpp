#include "tfgamerules.h"

#include "icliententitylist.h"

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"

TFGameRules* TFGameRules::instance = nullptr;

TFGameRules::TFGameRules(IClientEntity* entity) {
	this->entity = entity;

	const char* classname = Entities::GetEntityClassname(this->entity);


}

int TFGameRules::GetRoundsPlayed() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(this->entity, { "m_nRoundsPlayed" });
	}

	return -1;
}

int TFGameRules::GetGameType() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(this->entity, { "m_nGameType" });
	}

	return -1;
}

bool TFGameRules::IsKoth() const {
	if (IsValid()) {
		return (bool)* Entities::GetEntityProp<bool*>(this->entity, { "m_bPlayingKoth" });
	}

	return -1;
}

bool TFGameRules::IsHybridCTFCP() const {
	if (IsValid()) {
		return (bool)* Entities::GetEntityProp<bool*>(this->entity, { "m_bPlayingHybrid_CTF_CP" });
	}

	return -1;
}

int TFGameRules::GetRedKOTHTimer() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(this->entity, { "m_hRedKothTimer" });
	}

	return -1;
}

int TFGameRules::GetBlueKOTHTimer() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(this->entity, { "m_hBlueKothTimer" });
	}

	return -1;
}

float TFGameRules::GetMapResetTime() const {
	if (IsValid()) {
		return (float)* Entities::GetEntityProp<float*>(this->entity, { "m_flMapResetTime" });
	}

	return -1;
}

bool TFGameRules::IsValid() const {
	return entity.IsValid() && Entities::CheckEntityBaseclass(this->entity, "TFGameRulesProxy");
}

void TFGameRules::Set(TFGameRules* entity) { instance = entity; }