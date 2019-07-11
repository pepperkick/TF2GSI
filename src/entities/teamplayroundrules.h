#pragma once

#include "ehandle.h"

class IClientEntity;

#include "../tfdefs.h"

class TeamPlayRoundRules {
public:
	TeamPlayRoundRules(IClientEntity* entity);

	float GetMapResetTime() const;
	bool IsValid() const;

	static TeamPlayRoundRules* Get() { return instance; };
	static void Set(TeamPlayRoundRules*);

private:
	static TeamPlayRoundRules* instance;

	CHandle<IClientEntity> entity;
};