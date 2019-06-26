#pragma once

#include "ehandle.h"

class IClientEntity;

#include "../tfdefs.h"

class ObjectiveResource {
public:
	ObjectiveResource(IClientEntity* entity);

    int ObjectiveResource::GetNumCP() const;
	bool ObjectiveResource::IsCapLocked(int) const;
	float ObjectiveResource::CapPathDistance(int) const;
	float ObjectiveResource::CapUnlockTime(int) const;
	float ObjectiveResource::CapTimer(int) const;
	int ObjectiveResource::CappingTeam(int) const;
	int ObjectiveResource::CapTeamInZone(int) const;
	bool ObjectiveResource::IsCapBlocked(int) const;
	int ObjectiveResource::CapOwner(int) const;
	float ObjectiveResource::CapTeamCapTime(int i) const;
	int ObjectiveResource::GetPlayersOnCap(int i) const;
	int ObjectiveResource::CapTimerTimes(int i) const;
	bool IsValid() const;

	static ObjectiveResource* Get();
	static void Find();

private:
	CHandle<IClientEntity> entity;
};