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
	float TFGameRules::GetMapResetTime() const;
    int TFGameRules::GetRedKOTHTimer() const;
    int TFGameRules::GetBlueKOTHTimer() const;
	bool IsValid() const;

	static TFGameRules* Get() { return instance; };
	static void Set(TFGameRules*);
	static void Find();

private:
	static TFGameRules* instance;
	static struct Offsets {
		int m_nRoundsPlayed;
		int m_nGameType;
		int m_bPlayingKoth;
		int m_bPlayingHybrid_CTF_CP;
		int m_hRedKothTimer;
		int m_hBlueKothTimer;
		int m_flMapResetTime;
	} offsets;

	CHandle<IClientEntity> entity;
};