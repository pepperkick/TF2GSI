#include "team.h"

#include "icliententitylist.h"

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"

Team* blueTeam;
Team* redTeam;

Team::Team(IClientEntity* entity) {
	teamEntity = entity;
}

std::string Team::GetName() const {
	if (IsValid()) {
		return (std::string)* Entities::GetEntityProp<std::string*>(teamEntity, { "m_szTeamname" });
	}

	return "";
}

int Team::GetScore() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(teamEntity, { "m_iScore" });
	}

	return -1;
}

int Team::GetRoundsWon() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(teamEntity, { "m_iRoundsWon" });
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
				blueTeam = new Team(entity);
			}
			else if (*team == TFTeam_Red) {
				redTeam = new Team(entity);
			}
		}
	}
}

Team* Team::GetBlueTeam() {
	return blueTeam;
}

Team* Team::GetRedTeam() {
	return redTeam;
}

void Team::SetBlueTeam(Team* team) {
	blueTeam = team;
}

void Team::SetRedTeam(Team* team) {
	redTeam = team;
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
