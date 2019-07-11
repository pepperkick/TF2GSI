#pragma once

#include "ehandle.h"

class IClientEntity;

#include "../tfdefs.h"

class Team {
public:
	Team(IClientEntity* entity);

	std::string GetName() const;
	int GetScore() const;
	int GetRoundsWon() const;
	bool IsValid() const;

	static Team* GetBlueTeam() { return blueTeam; };
	static Team* GetRedTeam() { return redTeam; };
	static void SetBlueTeam(Team*);
	static void SetRedTeam(Team*);

private:
	static Team* blueTeam;
	static Team* redTeam;

	CHandle<IClientEntity> teamEntity;
};