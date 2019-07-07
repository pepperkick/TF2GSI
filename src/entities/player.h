#pragma once

#include "ehandle.h"

class C_BaseCombatWeapon;
class C_BaseEntity;
class CSteamID;
class IClientEntity;

#include "../tfdefs.h"

class Player {
public:
	Player(int entindex);
	Player(IClientEntity *entity);
	Player() {};

	static Player GetLocalPlayer();
	static Player GetTargetPlayer();

	Player& operator=(int entindex);
	Player& operator=(IClientEntity *entity);
	Player& operator=(const Player &player);

	bool operator==(int entindex) const;
	bool operator==(IClientEntity *entity) const;
	bool operator==(const Player &player) const;

	bool operator!=(int entindex) const;
	bool operator!=(IClientEntity *entity) const;
	bool operator!=(const Player &player) const;

	bool operator<(int entindex) const;
	bool operator<(IClientEntity *entity) const;
	bool operator<(const Player &player) const;

	bool operator<=(int entindex) const;
	bool operator<=(IClientEntity *entity) const;
	bool operator<=(const Player &player) const;

	bool operator>(int entindex) const;
	bool operator>(IClientEntity *entity) const;
	bool operator>(const Player &player) const;

	bool operator>=(int entindex) const;
	bool operator>=(IClientEntity *entity) const;
	bool operator>=(const Player &player) const;

	operator bool() const;
	operator IClientEntity *() const;

	IClientEntity *operator->() const;

	IClientEntity *GetEntity() const;

	Vector GetPosition() const;
	bool FindCondition();
	bool CheckCondition(TFCond condition) const;
	TFClassType GetClass() const;
	int GetHealth() const;
	int GetMaxHealth() const;
	int GetMaxBuffedHealth() const;
	std::string GetName() const;
	int GetObserverMode() const;
	int GetDominations() const;
	int GetDominated() const;
	int GetDominatedBy() const;
	int GetScore() const;
	int GetTotalScore() const;
	int GetDeaths() const;
	int GetDamage() const;
	int GetCaptures() const;
	int GetDefenses() const;
	int GetRevenges() const;
	int GetBuildingsDestroyed() const;
	int GetWeaponAmmo(int i) const;
	int GetHealing() const;
	C_BaseCombatWeapon* GetWeaponIndexByClass(const char* wepclass);
	int GetWeaponDefinationIndex(C_BaseCombatWeapon* weapon) const;
	int IsWearingWeapon() const;
	int GetHeadshots() const;
	int GetBackstabs() const;
	int GetUbers() const;
	int GetTeleports() const;
	int GetKillAssists() const;
	int GetHeals() const;
	int GetKillstreak(int i) const;
	float GetRespawnTime() const;
	C_BaseEntity *GetObserverTarget() const;
	CSteamID GetSteamID() const;
	TFTeam GetTeam() const;
	int GetUserID() const;
	int GetActiveWeapon() const;
	int GetWeaponClip(int i) const;
	C_BaseCombatWeapon *GetWeapon(int i) const;
	C_BaseCombatWeapon *GetMedigun() const;
	bool IsMedigunHealing() const;
	int GetMedigunTarget() const;
	int GetMedigunType() const;
	float GetMedigunCharge() const;
	bool IsAlive() const;
	int GetDisguiseTeam() const;
	int GetDisguiseClass() const;
	int GetTotalDamage() const;

	class Iterator {
		friend class Player;

	public:
		Iterator(const Iterator&);
		~Iterator() = default;
		Iterator& operator=(const Iterator&);
		Iterator& operator++();
		Player operator*() const;
		friend void swap(Iterator& lhs, Iterator& rhs);
		Iterator operator++(int);
		Player *operator->() const;
		friend bool operator==(const Iterator&, const Iterator&);
		friend bool operator!=(const Iterator&, const Iterator&);
		Iterator();
		Iterator& operator--();
		Iterator operator--(int);

	private:
		Iterator(int index);
		int index;
	};

	static Iterator begin();
	static Iterator end();

	class Iterable {
	public:
		Iterator begin();
		Iterator end();
	};

	static bool CheckDependencies();
	static void FindPlayerResource();
	static void SetPlayerResource(CHandle<IClientEntity>);
	static bool classRetrievalAvailable;
	static bool comparisonAvailable;
	static bool conditionsRetrievalAvailable;
	static bool nameRetrievalAvailable;
	static bool steamIDRetrievalAvailable;
	static bool userIDRetrievalAvailable;
	static bool playerResourceAvailable;

private:
	CHandle<IClientEntity> playerEntity;
	unsigned int playerConditions[4];

	bool IsValid() const;
	bool IsEqualTo(const Player &player) const;
	bool IsNotEqualTo(const Player &player) const;
	bool IsLessThan(const Player &player) const;
	bool IsLessThanOrEqualTo(const Player &player) const;
	bool IsGreaterThan(const Player &player) const;
	bool IsGreaterThanOrEqualTo(const Player &player) const;
};