#pragma once

#include "ehandle.h"

class IClientEntity;

#include "../tfdefs.h"

class TFGameRules {
public:
	TFGameRules(IClientEntity* entity);

    int TFGameRules::GetRoundsPlayed() const;
    int TFGameRules::GetGameType() const;
    bool TFGameRules::IsKoth() const;
    bool TFGameRules::IsHybridCTFCP() const;
    int TFGameRules::GetRedKOTHTimer() const;
    int TFGameRules::GetBlueKOTHTimer() const;
	bool IsValid() const;

	static TFGameRules* Get();
	static void Find();

private:
	CHandle<IClientEntity> entity;
};