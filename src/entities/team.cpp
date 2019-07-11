#include "team.h"

#include "icliententitylist.h"

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"

Team* Team::blueTeam = nullptr;
Team* Team::redTeam = nullptr;

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

void Team::SetBlueTeam(Team* team) { blueTeam = team; }
void Team::SetRedTeam(Team* team) {	redTeam = team; }