#pragma once

#include "ehandle.h"

class IClientEntity;

#include "../tfdefs.h"

class RoundTimer {
public:
	RoundTimer(IClientEntity* entity);

	bool IsPaused() const;
	int GetState() const;
	float GetTimeRemaining() const;
	float GetEndTime() const;
	int GetMaxLength() const;
	bool IsValid() const;

	static RoundTimer* Get(int);
	static void Find(int);

private:
	CHandle<IClientEntity> entity;
};