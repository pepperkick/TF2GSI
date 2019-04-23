#include "team.h"

#include "cbase.h"
#include "c_basecombatcharacter.h"
#include "c_baseentity.h"

#include "common.h"
#include "entities.h"
#include "exceptions.h"
#include "ifaces.h"
#include "tfdefs.h"

Team::Team(int entindex) {
	teamEntity = Interfaces::pClientEntityList->GetClientEntity(entindex);
}

Team::Team(IClientEntity* entity) {
	teamEntity = entity;
}

int Team::GetScore() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(teamEntity, { "m_iScore" });
	}

	return -1;
}

bool Team::IsValid() const {
	return teamEntity.IsValid() && Entities::CheckEntityBaseclass(teamEntity, "TFTeam");
}

void Team::FindTeams() {
	int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

	for (int i = 0; i <= maxEntity; i++) {
		IClientEntity* entity = Interfaces::pClientEntityList->GetClientEntity(i);

		if (!entity) {
			continue;
		}

		if (Entities::CheckEntityBaseclass(entity, "TFTeam")) {
			int* team = Entities::GetEntityProp<int*>(entity, { "m_iTeamNum" });

			if (*team == TFTeam_Blue) {
				Team::blueTeam = new Team(entity);
			}
			else if (*team == TFTeam_Red) {
				Team::redTeam = new Team(entity);
			}
		}
	}
}

bool Team::CheckDependencies() {
	bool ready = true;

	if (!Interfaces::pClientEntityList) {
		PRINT_TAG();
		Warning("Required interface IClientEntityList for team helper class not available!\n");

		ready = false;
	}

	if (!Interfaces::pEngineTool) {
		PRINT_TAG();
		Warning("Required interface IEngineTool for team helper class not available!\n");

		ready = false;
	}

	return ready;
}