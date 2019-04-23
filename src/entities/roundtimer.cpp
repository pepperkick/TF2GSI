#include "roundtimer.h"

#include "icliententitylist.h"

#include "../common.h"
#include "../entities.h"
#include "../ifaces.h"

RoundTimer* roundTimer;

RoundTimer::RoundTimer(IClientEntity* entity) {
	roundEntity = entity;
}

bool RoundTimer::IsPaused() const {
	if (IsValid()) {
		return (bool)* Entities::GetEntityProp<bool*>(roundEntity, { "m_bTimerPaused" });
	}

	return -1;
}

float RoundTimer::GetTimeRemaining() const {
	if (IsValid()) {
		return (float)* Entities::GetEntityProp<float*>(roundEntity, { "m_flTimeRemaining" });
	}

	return -1;
}

float RoundTimer::GetEndTime() const {
	if (IsValid()) {
		return (float)* Entities::GetEntityProp<float*>(roundEntity, { "m_flTimerEndTime" });
	}

	return -1;
}

int RoundTimer::GetMaxLength() const {
	if (IsValid()) {
		return (float)* Entities::GetEntityProp<float*>(roundEntity, { "m_nTimerMaxLength" });
	}

	return -1;
}

bool RoundTimer::IsValid() const {
	return roundEntity.IsValid() && Entities::CheckEntityBaseclass(roundEntity, "TeamRoundTimer");
}

void RoundTimer::FindRoundTimer() {
	int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

	for (int i = 0; i <= maxEntity; i++) {
		IClientEntity* entity = Interfaces::pClientEntityList->GetClientEntity(i);

		if (!entity) {
			continue;
		}

		if (Entities::CheckEntityBaseclass(entity, "TeamRoundTimer")) {
			bool inHud = (bool)* Entities::GetEntityProp<bool*>(entity, { "m_bShowInHUD" });

			if (inHud) {
				roundTimer = new RoundTimer(entity);
				break;
			}
		}
	}
}

RoundTimer* RoundTimer::GetRoundTimer() {
	return roundTimer;
}