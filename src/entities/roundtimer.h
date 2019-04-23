#pragma once

#include "ehandle.h"

class IClientEntity;

#include "../tfdefs.h"

class RoundTimer {
public:
	RoundTimer(IClientEntity* entity);

	bool IsPaused() const;
	float GetTimeRemaining() const;
	float GetEndTime() const;
	int GetMaxLength() const;
	bool IsValid() const;

	static RoundTimer* GetRoundTimer();
	static void FindRoundTimer();

private:
	CHandle<IClientEntity> roundEntity;
};