#pragma once

#include "ehandle.h"

class IClientEntity;

#include "../tfdefs.h"

#define MAX_TIMERS 8

class RoundTimer {
public:
	RoundTimer(IClientEntity* entity);

	int GetMaxLength() const;
	int GetState() const;
	float GetEndTime() const;
	float GetTimeRemaining() const;
	bool IsPaused() const;
	bool IsValid() const;

	static RoundTimer* Get(int i) { return timers[i]; };
	static void Find(int);

private:
	static RoundTimer* timers[MAX_TIMERS];

	CHandle<IClientEntity> entity;
};