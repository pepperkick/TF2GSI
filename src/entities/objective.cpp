#include "objective.h"

#include "icliententitylist.h"
#include "client_class.h"

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"

ObjectiveResource* instance;

ObjectiveResource::ObjectiveResource(IClientEntity* entity) {
	this->entity = entity;
}

int ObjectiveResource::GetNumCP() const {
	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(this->entity, { "m_iNumControlPoints" });
	}

	return -1;
}

bool ObjectiveResource::IsCapLocked(int i) const {
	char index[4];
	sprintf(index, "%03d", i);

	if (IsValid()) {
		return (bool)* Entities::GetEntityProp<bool*>(this->entity, { "m_bCPLocked", index });
	}

	return -1;
}

float ObjectiveResource::CapPathDistance(int i) const {
	char index[4];
	sprintf(index, "%03d", i);

	if (IsValid()) {
		return (float)* Entities::GetEntityProp<float*>(this->entity, { "m_flPathDistance", index });
	}

	return -1;
}

float ObjectiveResource::CapUnlockTime(int i) const {
	char index[4];
	sprintf(index, "%03d", i);

	if (IsValid()) {
		return (float)* Entities::GetEntityProp<float*>(this->entity, { "m_flUnlockTimes", index });
	}

	return -1;
}

float ObjectiveResource::CapTimer(int i) const {
	char index[4];
	sprintf(index, "%03d", i);

	if (IsValid()) {
		return (float)* Entities::GetEntityProp<float*>(this->entity, { "m_flCPTimerTimes", index });
	}

	return -1;
}

int ObjectiveResource::CappingTeam(int i) const {
	char index[4];
	sprintf(index, "%03d", i);

	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(this->entity, { "m_iCappingTeam", index });
	}

	return -1;
}

int ObjectiveResource::CapTeamInZone(int i) const {
	char index[4];
	sprintf(index, "%03d", i);

	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(this->entity, { "m_iTeamInZone", index });
	}

	return -1;
}

bool ObjectiveResource::IsCapBlocked(int i) const {
	char index[4];
	sprintf(index, "%03d", i);

	if (IsValid()) {
		return (bool)* Entities::GetEntityProp<bool*>(this->entity, { "m_bBlocked", index });
	}

	return -1;
}

int ObjectiveResource::GetPlayersOnCap(int i) const {
	char index[4];
	sprintf(index, "%03d", i);

	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(this->entity, { "m_iNumTeamMembers", index });
	}

	return -1;
}

float ObjectiveResource::CapTeamCapTime(int i) const {
	char index[4];
	sprintf(index, "%03d", i);

	if (IsValid()) {
		return (int)* Entities::GetEntityProp<float *>(this->entity, { "m_flTeamCapTime", index });
	}

	return -1;
}

int ObjectiveResource::CapTimerTimes(int i) const {
	char index[4];
	sprintf(index, "%03d", i);

	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(this->entity, { "m_flCPTimerTimes", index });
	}

	return -1;
}

int ObjectiveResource::CapOwner(int i) const {
	char index[4];
	sprintf(index, "%03d", i);

	if (IsValid()) {
		return (int)* Entities::GetEntityProp<int*>(this->entity, { "m_iOwner", index });
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

ObjectiveResource* ObjectiveResource::Get() {
	return instance;
}