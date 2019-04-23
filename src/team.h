#pragma once

#include "ehandle.h"

class IClientEntity;

#include "tfdefs.h"

class Team {
public:
	Team(int entindex);
	Team(IClientEntity* entity);

	int GetScore() const;
	bool IsValid() const;

	static Team *blueTeam;
	static Team *redTeam;

	static bool CheckDependencies();
	static void FindTeams();

private:
	CHandle<IClientEntity> teamEntity;
};