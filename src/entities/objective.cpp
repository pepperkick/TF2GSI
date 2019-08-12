#include "objective.h"

#include "icliententitylist.h"
#include "client_class.h"
#include "shareddefs.h"

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"

#define TEAM_ARRAY( index, team ) (index + (team * MAX_CONTROL_POINTS))

ObjectiveResource* ObjectiveResource::instance = nullptr;
ObjectiveResource::Offsets ObjectiveResource::offsets;

ObjectiveResource::ObjectiveResource(IClientEntity* entity) {
	this->entity = entity;

	const char* classname = Entities::GetEntityClassname(this->entity);

	this->offsets.m_iNumControlPoints 
		= Entities::GetClassPropOffset(classname, { "m_iNumControlPoints" });
	this->offsets.m_bCPLocked 
		= Entities::GetClassPropOffset(classname, { "m_bCPLocked" });
	this->offsets.m_bCPCapRateScalesWithPlayers
		= Entities::GetClassPropOffset(classname, { "m_bCPCapRateScalesWithPlayers" });
	this->offsets.m_flPathDistance
		= Entities::GetClassPropOffset(classname, { "m_flPathDistance" });
	this->offsets.m_flUnlockTimes
		= Entities::GetClassPropOffset(classname, { "m_flUnlockTimes" });
	this->offsets.m_flCPTimerTimes
		= Entities::GetClassPropOffset(classname, { "m_flCPTimerTimes" });
	this->offsets.m_iCappingTeam
		= Entities::GetClassPropOffset(classname, { "m_iCappingTeam" });
	this->offsets.m_iTeamInZone
		= Entities::GetClassPropOffset(classname, { "m_iTeamInZone" });
	this->offsets.m_iNumTeamMembers
		= Entities::GetClassPropOffset(classname, { "m_iNumTeamMembers" });
	this->offsets.m_bBlocked
		= Entities::GetClassPropOffset(classname, { "m_bBlocked" });
	this->offsets.m_flTeamCapTime
		= Entities::GetClassPropOffset(classname, { "m_flTeamCapTime" });
	this->offsets.m_iOwner
		= Entities::GetClassPropOffset(classname, { "m_iOwner" });
	this->offsets.m_flLazyCapPerc
		= Entities::GetClassPropOffset(classname, { "m_flLazyCapPerc" });
}

int ObjectiveResource::GetNumCP() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityValueAtOffset<int*>(this->entity, this->offsets.m_iNumControlPoints);
	}

	return -1;
}

bool ObjectiveResource::IsCapLocked(int i) const {
	if (IsValid()) {
		return (bool)* Entities::GetEntityValueAtOffset<bool*>(this->entity, this->offsets.m_bCPLocked + i);
	}

	return -1;
}

bool ObjectiveResource::DoesCPScaleWithPlayers(int i) const {
	if (IsValid()) {
		return (bool)* Entities::GetEntityValueAtOffset<bool*>(this->entity, this->offsets.m_bCPCapRateScalesWithPlayers + i);
	}

	return -1;
}

float ObjectiveResource::CapPathDistance(int i) const {
	if (IsValid()) {
		return (float)* Entities::GetEntityValueAtOffset<float*>(this->entity, this->offsets.m_flPathDistance + (4 * i));
	}

	return -1;
}

float ObjectiveResource::CapUnlockTime(int i) const {
	if (IsValid()) {
		return (float)* Entities::GetEntityValueAtOffset<float*>(this->entity, this->offsets.m_flUnlockTimes + (4 * i));
	}

	return -1;
}

float ObjectiveResource::CapTimer(int i) const {
	if (IsValid()) {
		return (float)* Entities::GetEntityValueAtOffset<float*>(this->entity, this->offsets.m_flCPTimerTimes + (4 * i));
	}

	return -1;
}

int ObjectiveResource::CappingTeam(int i) const {
	if (IsValid()) {
		return (int)* Entities::GetEntityValueAtOffset<int*>(this->entity, this->offsets.m_iCappingTeam + (4 * i));
	}

	return -1;
}

int ObjectiveResource::CapTeamInZone(int i) const {
	if (IsValid()) {
		return (int)* Entities::GetEntityValueAtOffset<int*>(this->entity, this->offsets.m_iTeamInZone + (4 * i));
	}

	return -1;
}

bool ObjectiveResource::IsCapBlocked(int i) const {
	if (IsValid()) {
		return (bool)* Entities::GetEntityValueAtOffset<bool*>(this->entity, this->offsets.m_bBlocked + i);
	}

	return -1;
}

int ObjectiveResource::GetPlayersOnCap(int i) const {
	if (IsValid()) {
		return (int)* Entities::GetEntityValueAtOffset<int*>(this->entity, this->offsets.m_iNumTeamMembers + (4 * i));
	}

	return -1;
}

float ObjectiveResource::CapTeamCapTime(int i) const {
	if (IsValid()) {
		return (float)* Entities::GetEntityValueAtOffset<float*>(this->entity, this->offsets.m_flTeamCapTime + (4 * i));
	}

	return -1;
}

int ObjectiveResource::CapTimerTimes(int i) const {
	if (IsValid()) {
		return (int)* Entities::GetEntityValueAtOffset<int*>(this->entity, this->offsets.m_flCPTimerTimes + (4 * i));
	}

	return -1;
}

int ObjectiveResource::CapOwner(int i) const {
	if (IsValid()) {
		return (int)* Entities::GetEntityValueAtOffset<int*>(this->entity, this->offsets.m_iOwner + (4 * i));
	}

	return -1;
}

float ObjectiveResource::GetCapLazyPerc(int i) const {
	if (IsValid()) {
		return (float)* Entities::GetEntityValueAtOffset<float*>(this->entity, this->offsets.m_flLazyCapPerc + (4 * i));
	}

	return -1;
}

bool ObjectiveResource::IsValid() const {
	return entity.IsValid() && Entities::CheckEntityBaseclass(this->entity, "BaseTeamObjectiveResource");
}

void ObjectiveResource::Find() {
	int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

	for (int i = 0; i <= maxEntity; i++) {
		IClientEntity* entity = Interfaces::pClientEntityList->GetClientEntity(i);

		if (!entity) {
			continue;
		}

		if (Entities::CheckEntityBaseclass(entity, "BaseTeamObjectiveResource")) {
            instance = new ObjectiveResource(entity);
            break;
		}
	}
}

void ObjectiveResource::Set(ObjectiveResource* entity) { instance = entity; }
