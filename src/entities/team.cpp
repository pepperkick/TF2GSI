#include "team.h"

#include "icliententitylist.h"

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"

Team* Team::blueTeam = nullptr;
Team* Team::redTeam = nullptr;
Team::Offsets Team::offsets;

Team::Team(IClientEntity* entity) {
	this->entity = entity;

	const char* classname = Entities::GetEntityClassname(this->entity);

	this->offsets.m_szTeamname
		= Entities::GetClassPropOffset(classname, { "m_szTeamname" });
	this->offsets.m_iScore
		= Entities::GetClassPropOffset(classname, { "m_iScore" });
	this->offsets.m_iRoundsWon
		= Entities::GetClassPropOffset(classname, { "m_iRoundsWon" });
}

std::string Team::GetName() const {
	if (IsValid()) {
		return (std::string)* Entities::GetEntityValueAtOffset<std::string*>(this->entity, this->offsets.m_szTeamname);
	}

	return "";
}

int Team::GetScore() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityValueAtOffset<int*>(this->entity, this->offsets.m_iScore);
	}

	return -1;
}

int Team::GetRoundsWon() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityValueAtOffset<int*>(this->entity, this->offsets.m_iRoundsWon);
	}

	return -1;
}

bool Team::IsValid() const {
	return entity.IsValid() && Entities::CheckEntityBaseclass(entity, "TFTeam");
}

void Team::SetBlueTeam(Team* team) { blueTeam = team; }
void Team::SetRedTeam(Team* team) {	redTeam = team; }