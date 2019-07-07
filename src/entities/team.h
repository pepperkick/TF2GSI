#pragma once

#include "ehandle.h"

class IClientEntity;

#include "../tfdefs.h"

class Team {
public:
	Team(IClientEntity* entity);

	int GetScore() const;
	int GetRoundsWon() const;
	std::string GetName() const;
	bool IsValid() const;

	static Team* GetBlueTeam();
	static Team* GetRedTeam();
	static void SetBlueTeam(Team*);
	static void SetRedTeam(Team*);
	static bool CheckDependencies();
	static void FindTeams();
private:
	CHandle<IClientEntity> teamEntity;
};