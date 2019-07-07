#pragma once

#include "ehandle.h"

class IClientEntity;

#include "../tfdefs.h"

class TeamPlayRoundRules {
public:
	TeamPlayRoundRules(IClientEntity* entity);

	float GetMapResetTime() const;
	bool IsValid() const;

	static TeamPlayRoundRules* Get();
	static void Set(TeamPlayRoundRules*);
	static void Find();
private:
	CHandle<IClientEntity> entity;
};