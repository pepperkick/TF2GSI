#pragma once

#include "ehandle.h"

class C_BaseTeamObjectiveResource;
class IClientEntity;

#include "../tfdefs.h"

class ObjectiveResource {
public:
	ObjectiveResource(IClientEntity* entity);

    int GetNumCP() const;
	bool IsCapLocked(int) const;
	float CapPathDistance(int) const;
	float CapUnlockTime(int) const;
	float CapTimer(int) const;
	int CappingTeam(int) const;
	int CapTeamInZone(int) const;
	bool IsCapBlocked(int) const;
	int CapOwner(int) const;
	float CapTeamCapTime(int) const;
	int GetPlayersOnCap(int) const;
	int CapTimerTimes(int) const;
	float GetCapLazyPerc(int) const;
	bool DoesCPScaleWithPlayers(int) const;
	bool IsValid() const;

	static ObjectiveResource* Get() { return instance; };
	static void Set(ObjectiveResource*);
	static void Find();

private:
	static ObjectiveResource* instance;

	CHandle<IClientEntity> entity;
};