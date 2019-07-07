#include "teamplayroundrules.h"

#include "icliententitylist.h"
#include "icvar.h"
#include <convar.h>

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"

TeamPlayRoundRules* instance;

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

void TeamPlayRoundRules::Find() {
	int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

	for (int i = 0; i <= maxEntity; i++) {
		IClientEntity* entity = Interfaces::pClientEntityList->GetClientEntity(i);

		if (!entity) {
			continue;
		}

		if (Entities::CheckEntityBaseclass(entity, "TeamplayRoundBasedRulesProxy")) {
			instance = new TeamPlayRoundRules(entity);
			break;
		}
	}
}

TeamPlayRoundRules* TeamPlayRoundRules::Get() {
	return instance;
}

void TeamPlayRoundRules::Set(TeamPlayRoundRules* entity) {
	instance = entity;
}