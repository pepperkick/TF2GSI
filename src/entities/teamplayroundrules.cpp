#include "teamplayroundrules.h"

#include "icliententitylist.h"
#include "icvar.h"
#include <convar.h>

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"

TeamPlayRoundRules* TeamPlayRoundRules::instance = nullptr;

TeamPlayRoundRules::TeamPlayRoundRules(IClientEntity* entity) {
	this->entity = entity;
}

float TeamPlayRoundRules::GetMapResetTime() const {
	if (IsValid()) {
		return (float)* Entities::GetEntityProp<float*>(entity, { "m_flMapResetTime" });
	}

	return -1;
}

bool TeamPlayRoundRules::IsValid() const {
	return entity.IsValid() && Entities::CheckEntityBaseclass(entity, "TeamplayRoundBasedRulesProxy");
}

void TeamPlayRoundRules::Set(TeamPlayRoundRules* entity) { instance = entity; }