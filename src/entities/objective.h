#pragma once

#include "ehandle.h"

class C_BaseTeamObjectiveResource;
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
	float ObjectiveResource::CapTeamCapTime(int) const;
	int ObjectiveResource::GetPlayersOnCap(int) const;
	int ObjectiveResource::CapTimerTimes(int) const;
	float ObjectiveResource::GetCapLazyPerc(int) const;
	bool ObjectiveResource::DoesCPScaleWithPlayers(int i) const;
	float ObjectiveResource::GetCapTimeLeft(int) const;
	float ObjectiveResource::GetTimeLeft(int) const;
	bool IsValid() const;

	static ObjectiveResource* Get();
	static void Set(ObjectiveResource*);
	static void Find();

private:
	CHandle<IClientEntity> entity;
};