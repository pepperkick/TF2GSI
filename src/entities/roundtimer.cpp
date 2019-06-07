#include "roundtimer.h"

#include "icliententitylist.h"

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"

RoundTimer* timer1;
RoundTimer* timer2;
RoundTimer* timer3;
RoundTimer* timer4;

RoundTimer::RoundTimer(IClientEntity* entity) {
	this->entity = entity;
}

bool RoundTimer::IsPaused() const {
	if (IsValid()) {
		return (bool)* Entities::GetEntityProp<bool*>(entity, { "m_bTimerPaused" });
	}

	return -1;
}

float RoundTimer::GetTimeRemaining() const {
	if (IsValid()) {
		return (float)* Entities::GetEntityProp<float*>(entity, { "m_flTimeRemaining" });
	}

	return -1;
}

float RoundTimer::GetEndTime() const {
	if (IsValid()) {
		return (float)* Entities::GetEntityProp<float*>(entity, { "m_flTimerEndTime" });
	}

	return -1;
}

int RoundTimer::GetMaxLength() const {
	if (IsValid()) {
		return (float)* Entities::GetEntityProp<float*>(entity, { "m_nTimerMaxLength" });
	}

	return -1;
}

bool RoundTimer::IsValid() const {
	return entity.IsValid() && Entities::CheckEntityBaseclass(entity, "TeamRoundTimer");
}

void RoundTimer::Find(int i) {
	int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();
	int n = 0, c = 0;
	
	for (int m = 0; m < i; m++) {
		for (int i = n; i <= maxEntity; i++) {
			IClientEntity* entity = Interfaces::pClientEntityList->GetClientEntity(i);

			if (!entity) {
				continue;
			}

			if (Entities::CheckEntityBaseclass(entity, "TeamRoundTimer")) {
				if (m == 0) timer1 = new RoundTimer(entity);
				if (m == 1) timer2 = new RoundTimer(entity);
				if (m == 2) timer3 = new RoundTimer(entity);
				if (m == 3) timer4 = new RoundTimer(entity);

				n = i + 1;

				break;
			}
		}
	}
}

RoundTimer* RoundTimer::Get(int m) {
	if (m == 0) return timer1;
	if (m == 1) return timer2;
	if (m == 2) return timer3;
	if (m == 3) return timer4;
}