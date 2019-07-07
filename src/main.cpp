// Fix INVALID_HANDLE_VALUE redefinition warning
#ifndef _LINUX
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifndef NULL
#define NULL nullptr
#endif

#include "interface.h"
#include "filesystem.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "toolframework/ienginetool.h"
#include "eiface.h"
#include "igameevents.h"
#include "convar.h"
#include "Color.h"
#include "vstdlib/random.h"
#include "engine/IEngineTrace.h"
#include "tier2/tier2.h"
#include "ihltv.h"
#include "ihltvdirector.h"
#include "KeyValues.h"
#include "dt_send.h"
#include "server_class.h"
#include "cdll_int.h"
#include "shareddefs.h"
#include "icliententity.h"
#include "icliententitylist.h"
#include "ehandle.h"

#include "common.h"
#include "entities/player.h"
#include "entities/team.h"
#include "entities/roundtimer.h"
#include "entities/tfgamerules.h"
#include "entities/objective.h"
#include "entities/teamplayroundrules.h"
#include "ifaces.h"
#include "entities.h"

#include <vector>
#include <set>
#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TEAM_ARRAY( index, team ) (index + (team * MAX_CONTROL_POINTS))

IServerGameDLL* g_pGameDLL;
IFileSystem* g_pFileSystem;
IHLTVDirector* g_pHLTVDirector;
IVEngineServer* engine;

bool poolReady = false;
bool breakPool = false;
bool inGameFlag = false;
bool inOvertime = false;

float mapStartTime = 0;
int gAppId; 
const char* gVersion;

float m_flCapTimeLeft[MAX_CONTROL_POINTS] = { 0.0f };
float m_flCapLastThinkTime[MAX_CONTROL_POINTS] = { 0.0f };
int m_nPlayersOnCap[MAX_CONTROL_POINTS] = { 0 };
CHandle<IClientEntity> m_hActiveWeapon[MAX_PLAYERS] = { nullptr };

HWND gTimers;

std::ostringstream extraData;

void CALLBACK LoopTimer(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime);
void SendData();

class Plugin : public IServerPluginCallbacks, IGameEventListener2 {
public:
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) override;
    virtual void Unload() override;
    virtual void Pause() override { }
    virtual void UnPause() override { }
    virtual const char* GetPluginDescription() override { return "GSI"; }
    virtual void LevelInit(const char* mapName) override { }
    virtual void ServerActivate(edict_t* pEdictList, int edictCount, int clientMax) override { }
    virtual void GameFrame(bool simulating) override { };
    virtual void LevelShutdown() override { }
    virtual void ClientActive(edict_t* pEntity) override { }
    virtual void ClientDisconnect(edict_t* pEntity) override { }
    virtual void ClientPutInServer(edict_t* pEntity, const char* playername) override;
    virtual void SetCommandClient(int index) override { }
    virtual void ClientSettingsChanged(edict_t* pEdict) override { }
    virtual PLUGIN_RESULT ClientConnect(bool* bAllowConnect, edict_t* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen) override { return PLUGIN_CONTINUE; }
    virtual PLUGIN_RESULT ClientCommand(edict_t* pEntity, const CCommand& args) override { return PLUGIN_CONTINUE; }
    virtual PLUGIN_RESULT NetworkIDValidated(const char* pszUserName, const char* pszNetworkID) override { return PLUGIN_CONTINUE; }
    virtual void OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t* pPlayerEntity, EQueryCvarValueStatus eStatus, const char* pCvarName, const char* pCvarValue) override { }
    virtual void OnEdictAllocated(edict_t* edict) { }
    virtual void OnEdictFreed(const edict_t* edict) { }

    void FireGameEvent(IGameEvent* pEvent) override;

    void Transmit(const char* msg);
};

Plugin g_Plugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(Plugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_Plugin);

bool Plugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) {
    PRINT_TAG();
    ConColorMsg(Color(255, 255, 0, 255), "Loading plugin, Version: %s\n", PLUGIN_VERSION);

    Interfaces::Load(interfaceFactory, gameServerFactory);
    Interfaces::pGameEventManager->AddListener(this, "player_death", false);
    Interfaces::pGameEventManager->AddListener(this, "tf_game_over", false);
    Interfaces::pGameEventManager->AddListener(this, "teamplay_round_start", false);
    Interfaces::pGameEventManager->AddListener(this, "teamplay_round_active", false);
    Interfaces::pGameEventManager->AddListener(this, "teamplay_round_win", false);
	Interfaces::pGameEventManager->AddListener(this, "teamplay_round_stalemate", false);
	Interfaces::pGameEventManager->AddListener(this, "teamplay_point_captured", false);
	Interfaces::pGameEventManager->AddListener(this, "teamplay_overtime_begin", false);
	Interfaces::pGameEventManager->AddListener(this, "teamplay_overtime_end", false);
    Interfaces::pGameEventManager->AddListener(this, "teamplay_game_over", false);
	Interfaces::pGameEventManager->AddListener(this, "teamplay_win_panel", false);
	Interfaces::pGameEventManager->AddListener(this, "controlpoint_updatecapping", false);

	gAppId = Interfaces::GetEngineClient()->GetAppID();
	gVersion = Interfaces::GetEngineClient()->GetProductVersionString();

	if (!Interfaces::GetClientDLL()) {
        PRINT_TAG();
        ConColorMsg(Color(255, 0, 0, 255), "Could not find game DLL interface, aborting load\n");
        return false;
    }

	if (!Interfaces::GetClientEngineTools()) {
        PRINT_TAG();
        ConColorMsg(Color(255, 0, 0, 255), "Could not find engine tools, aborting load\n");
        return false;
    }

	if (!Interfaces::GetEngineClient()) {
        PRINT_TAG();
        ConColorMsg(Color(255, 0, 0, 255), "Could not find engine client, aborting load\n");
        return false;
    }

	if (!Player::CheckDependencies()) {
		PRINT_TAG();
		ConColorMsg(Color(255, 0, 0, 255), "Required player helper class!\n");
		return false;
	}

	if (!Team::CheckDependencies()) {
		PRINT_TAG();
		ConColorMsg(Color(255, 0, 0, 255), "Required team helper class!\n");
		return false;
	}

    PRINT_TAG();
    ConColorMsg(Color(255, 255, 0, 255), "Successfully Started!\n");

    SetTimer(gTimers, 0, 125, &LoopTimer);

    return true;
}

void Plugin::Unload() {
    Interfaces::Unload();
    KillTimer(gTimers, 0);
}

void Plugin::ClientPutInServer(edict_t *pEntity, char const *playername) {}

void Plugin::FireGameEvent(IGameEvent* event) {
	if (!strcmp(event->GetName(), "player_death")) {
		int victimUserID = event->GetInt("userid", -1);
		int attackerUserID = event->GetInt("attacker", -1);
		int assisterUserID = event->GetInt("assister", -1);
		std::string weapon = event->GetString("weapon");
		
		Player victim = Interfaces::pEngineClient->GetPlayerForUserID(victimUserID);
		Player attacker = Interfaces::pEngineClient->GetPlayerForUserID(attackerUserID);
		Player assister = Interfaces::pEngineClient->GetPlayerForUserID(assisterUserID);

		extraData 
			<< "\"event\": {"
			<< "\"name\": \"" << event->GetName() << "\", "
			<< "\"victim\": \"" << victim.GetSteamID().ConvertToUint64() << "\", "
			<< "\"attacker\": \"" << attacker.GetSteamID().ConvertToUint64() << "\", "
			<< "\"assister\": \"" << assister.GetSteamID().ConvertToUint64() << "\", "
			<< "\"weapon\": \"" << weapon << "\", "
			<< "}, ";

		SendData();
	}
	else if (!strcmp(event->GetName(), "teamplay_win_panel")) {
		extraData
			<< "\"event\": {"
			<< "\"name\": \"" << event->GetName() << "\", "
			<< "\"team\": \"" << event->GetInt("winning_team") << "\", "
			<< "}, ";

		SendData();
	}
	else if (!strcmp(event->GetName(), "teamplay_point_captured")) {
		int index = event->GetInt("cp", -1);

		if (index == -1) return;

		m_flCapTimeLeft[index] = 0.0f;
		m_flCapLastThinkTime[index] = 0;
	}
	else if (!strcmp(event->GetName(), "teamplay_overtime_begin")) {
		inOvertime = true;
	}
	else if (!strcmp(event->GetName(), "teamplay_overtime_end")) {
		inOvertime = false;
	}
	else if (!strcmp(event->GetName(), "controlpoint_updatecapping")) {
		int index = event->GetInt("index", -1);

		if (index == -1) return;

		if (!ObjectiveResource::Get())
			ObjectiveResource::Find();

		int team = ObjectiveResource::Get()->CappingTeam(index);

		m_flCapTimeLeft[index] = ObjectiveResource::Get()->CapTeamCapTime(TEAM_ARRAY(index, team));
		m_flCapLastThinkTime[index] = Interfaces::GetEngineTools()->ClientTime();
	}
}

void Plugin::Transmit(const char* msg) {
    HANDLE hPipe;
    LPTSTR lptPipeName = TEXT("\\\\.\\pipe\\tf2-gsi");

    hPipe = CreateFile(lptPipeName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	
    DWORD cbWritten;
    WriteFile(hPipe, msg, strlen(msg), &cbWritten, NULL);
}

void CALLBACK LoopTimer(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime) {
	SendData();
}

void SendData() {
	Player localPlayer = Player::GetLocalPlayer();
	Player targetPlayer = Player::GetTargetPlayer();

	bool isInGame = Interfaces::GetEngineClient()->IsInGame();

	const char* mapName = Interfaces::GetEngineClient()->GetLevelName();
	
	std::string extra = extraData.str();
	std::ostringstream os;

	os << "{";
	os << "\"provider\": { "
		<< "\"name\": \"Team Fortress 2\", "
		<< "\"appid\": \"" << gAppId << "\", "
		<< "\"version\": \"" << gVersion << "\""
		<< " }, ";

	if (targetPlayer) {
		os << "\"player\": { "
			<< "\"name\": \"" << targetPlayer.GetName().c_str() << "\", "
			<< "\"steamid\": \"" << targetPlayer.GetSteamID().ConvertToUint64() << "\" "
			<< " }, ";
	}

	if (extra.length() > 3) {
		os << extra;
		extra = "";
		extraData.str("");
		extraData.clear();
	}
	
	bool tvFlag = false;

	if (isInGame) {
		if (!inGameFlag) {
			int maxEntity = Interfaces::pClientEntityList->GetHighestEntityIndex();

			for (int i = 0; i <= maxEntity; i++) {
				IClientEntity* entity = Interfaces::pClientEntityList->GetClientEntity(i);

				if (!entity) {
					continue;
				}

				if (Entities::CheckEntityBaseclass(entity, "TFPlayerResource")) {
					Player::SetPlayerResource(dynamic_cast<IClientEntity*>(entity));
				}

				if (Entities::CheckEntityBaseclass(entity, "TFTeam")) {
					int* team = Entities::GetEntityProp<int*>(entity, { "m_iTeamNum" });

					if (*team == TFTeam_Blue) {
						Team::SetBlueTeam(new Team(entity));
					}
					else if (*team == TFTeam_Red) {
						Team::SetRedTeam(new Team(entity));
					}
				}

				if (Entities::CheckEntityBaseclass(entity, "TFGameRulesProxy")) {
					TFGameRules::Set(new TFGameRules(entity));
				}

				if (Entities::CheckEntityBaseclass(entity, "BaseTeamObjectiveResource")) {
					ObjectiveResource::Set(new ObjectiveResource(entity));
				}

				if (Entities::CheckEntityBaseclass(entity, "TeamplayRoundBasedRulesProxy")) {
					TeamPlayRoundRules::Set(new TeamPlayRoundRules(entity));
				}
			}

			for (int i = 0; i < MAX_CONTROL_POINTS; i++) {
				m_flCapTimeLeft[i] = 0.0f;
				m_flCapLastThinkTime[i] = 0.0f;
				m_nPlayersOnCap[i] = 0;
			}

			inOvertime = false;
			inGameFlag = true;

			mapStartTime = Interfaces::GetEngineTools()->ClientTime();
		}

		os << "\"map\": { "
			<< "\"name\": \"" << mapName << "\", "
			<< " },";

		if (ObjectiveResource::Get()->IsValid()) {
			ObjectiveResource* objective = ObjectiveResource::Get();
			bool isKoth = false, is5cp = false;
			char type[24];
			int numOfCaps = -1;

			sprintf(type, "Unknown");

			if (objective->IsValid()) {
				numOfCaps = objective->GetNumCP();

				if (numOfCaps == 1) {
					sprintf(type, "KOTH");
					isKoth = true;
				}
				else if (numOfCaps == 5) {
					sprintf(type, "5CP");
					is5cp = true;
				}
			}
			
			for (int index = 0; index < numOfCaps; index++) {
				int team = ObjectiveResource::Get()->CappingTeam(index);
				int cappers = ObjectiveResource::Get()->GetPlayersOnCap(TEAM_ARRAY(index, team));
				float curtime = Interfaces::GetEngineTools()->ClientTime();
				ConVar* cavr_detoriatetime = Interfaces::GetCvar()->FindVar("mp_capdeteriorate_time");

				if (m_flCapTimeLeft[index]) {
					bool bDeteriorateNormally = true;

					if (cappers > 0) {
						float flReduction = curtime - m_flCapLastThinkTime[index];

						if (ObjectiveResource::Get()->DoesCPScaleWithPlayers(index)) {
							for (int iPlayer = 1; iPlayer < cappers; iPlayer++) {
								flReduction += ((curtime - m_flCapLastThinkTime[index]) / (float)(iPlayer + 1));
							}
						}

						if (ObjectiveResource::Get()->CapTeamInZone(index) == team) {
							bDeteriorateNormally = false;
							m_flCapTimeLeft[index] -= flReduction;
						}
						else if (ObjectiveResource::Get()->CapOwner(index) == TEAM_UNASSIGNED && ObjectiveResource::Get()->CapTeamInZone(index) != TEAM_UNASSIGNED) {
							bDeteriorateNormally = false;
							m_flCapTimeLeft[index] += flReduction;
						}
					}

					if (bDeteriorateNormally) {
						float flCapLength = ObjectiveResource::Get()->CapTeamCapTime(TEAM_ARRAY(index, team));
						float flDecreaseScale = ObjectiveResource::Get()->DoesCPScaleWithPlayers(index) ? cavr_detoriatetime->GetFloat() : flCapLength;
						float flDecrease = (flCapLength / flDecreaseScale) * (curtime - m_flCapLastThinkTime[index]);

						if (inOvertime) {
							flDecrease *= 6;
						}

						m_flCapTimeLeft[index] += flDecrease;
					}
				}

				m_flCapLastThinkTime[index] = curtime;
			}

			if (isKoth) {
				RoundTimer::Find(2);
				RoundTimer* timer1 = RoundTimer::Get(0);
				RoundTimer* timer2 = RoundTimer::Get(1);

				float blueTime, redTime;

				if (objective->CapOwner(0) == 2) {
					redTime = timer2->GetEndTime() - Interfaces::GetEngineTools()->ClientTime();
					blueTime = timer1->GetTimeRemaining();
				}
				else if (objective->CapOwner(0) == 3) {
					blueTime = timer1->GetEndTime() - Interfaces::GetEngineTools()->ClientTime();
					redTime = timer2->GetTimeRemaining();
				}
				else {
					redTime = timer1->GetTimeRemaining();
					blueTime = timer1->GetTimeRemaining();
				}

				int cappingTeam = objective->CappingTeam(0);
				int playersOnCap = objective->GetPlayersOnCap(TEAM_ARRAY(0, cappingTeam));
				float teamTime = objective->CapTeamCapTime(TEAM_ARRAY(0, cappingTeam));
				float time = objective->GetCapLazyPerc(0);
				float percentage = 0.0f;

				if (m_flCapTimeLeft[0] > 0) {
					if (teamTime <= 0)
						percentage = 0.0f;

					float flElapsedTime = teamTime - m_flCapTimeLeft[0];

					if (flElapsedTime > teamTime)
						percentage = 1.0f;

					percentage = (flElapsedTime / teamTime);
				}

				os << "\"round\": {"
					<< "\"gameType\": \"" << type << "\", "
					<< "\"redTimeLeft\": \"" << redTime << "\", "	
					<< "\"blueTimeLeft\": \"" << blueTime << "\", "
					<< "\"cap0\": {"
					<< "\"locked\": \"" << objective->IsCapLocked(0) << "\", "
					<< "\"blocked\": \"" << objective->IsCapBlocked(0) << "\", "
					<< "\"cappedTeam\": \"" << objective->CapOwner(0) << "\", "
					<< "\"cappingTeam\": \"" << cappingTeam << "\", "
					<< "\"unlockTime\": \"" << objective->CapUnlockTime(0) - Interfaces::GetEngineTools()->ClientTime() << "\", "
					<< "\"playersOnCap\": " << playersOnCap << ", "
					<< "\"percentage\": " << percentage << ", "
					<< " }, "
					<< " }, ";
			}
			else if (is5cp) {
				RoundTimer::Find(1);
				RoundTimer* timer = RoundTimer::Get(0);

				ConVar *cvar_timelimit = Interfaces::GetCvar()->FindVar("mp_timelimit");
				int timelimit = cvar_timelimit->GetInt() * 60;
				float timepast = Interfaces::GetEngineTools()->ClientTime() - mapStartTime;
				float timeleft = timelimit - timepast;

				os << "\"round\": {"
					<< "\"gameType\": \"" << type << "\", "
					<< "\"isPaused\": \"" << Interfaces::GetEngineClient()->IsPaused() << "\", "
					<< "\"maxLength\": \"" << timer->GetMaxLength() << "\", "
					<< "\"matchTimeLeft\": \"" << timeleft << "\", "
					<< "\"roundTimeLeft\": \"" << timer->GetEndTime() - Interfaces::GetEngineTools()->ClientTime() << "\", "
					<< "\"noOfCaps\": \"" << numOfCaps << "\", ";
				
				for (int i = 0; i < numOfCaps; i++) {
					int cappingTeam = objective->CappingTeam(i);
					int playersOnCap = objective->GetPlayersOnCap(TEAM_ARRAY(i, cappingTeam));
					float teamTime = objective->CapTeamCapTime(TEAM_ARRAY(i, cappingTeam));
					float time = objective->GetCapLazyPerc(i);
					float percentage = 0.0f;

					if (m_flCapTimeLeft[i] > 0) {
						if (teamTime <= 0)
							percentage = 0.0f;

						float flElapsedTime = teamTime - m_flCapTimeLeft[i];

						if (flElapsedTime > teamTime)
							percentage = 1.0f;

						percentage = (flElapsedTime / teamTime);
					}

					os << "\"cap" << i << "\": {"
						<< "\"locked\": \"" << objective->IsCapLocked(i) << "\", "
						<< "\"blocked\": \"" << objective->IsCapBlocked(i) << "\", "
						<< "\"cappedTeam\": \"" << objective->CapOwner(i) << "\", "
						<< "\"cappingTeam\": \"" << cappingTeam << "\", "
						<< "\"unlockTime\": \"" << objective->CapUnlockTime(i) - Interfaces::GetEngineTools()->ClientTime() << "\", "
						<< "\"playersOnCap\": " << playersOnCap << ", "
						<< "\"percentage\": " << percentage << ", "
						<< "\"teamTime\": " << teamTime << ", "
						<< " }, ";
				}

				os << " }, ";

			}
			else {
				RoundTimer::Find(1);
				RoundTimer* timer = RoundTimer::Get(0);

				os << "\"round\": {"
					<< "\"isPaused\": \"" << timer->IsPaused() << "\", "
					<< "\"timeRemaining\": \"" << timer->GetTimeRemaining() << "\", "
					<< "\"maxLength\": \"" << timer->GetMaxLength() << "\", "
					<< "\"endTime\": \"" << timer->GetEndTime() - Interfaces::GetEngineTools()->ClientTime() << "\", "
					<< "\"noOfCaps\": \"" << numOfCaps << "\", "
					<< " }, ";
			}
		}

		if (Team::GetRedTeam()->IsValid() && Team::GetBlueTeam()->IsValid()) {
			os << "\"teams\": {"
				<< "\"team_blue\": {"
				<< "\"name\": \"" << Team::GetBlueTeam()->GetName().c_str() << "\", "
				<< "\"score\": \"" << Team::GetBlueTeam()->GetScore() << "\", "
				<< "\"rounds\": \"" << Team::GetBlueTeam()->GetRoundsWon() << "\", "
				<< " }, "
				<< "\"team_red\": {"
				<< "\"name\": \"" << Team::GetRedTeam()->GetName().c_str() << "\", "
				<< "\"score\": \"" << Team::GetRedTeam()->GetScore() << "\", "
				<< "\"rounds\": \"" << Team::GetRedTeam()->GetRoundsWon() << "\", "
				<< " }, "
				<< " }, ";
		}

		os << "\"allplayers\": { ";
		for (Player player : Player::Iterable()) {
			Vector position = player.GetPosition();
			const char* name = player.GetName().c_str();

			float timepast = Interfaces::GetEngineTools()->ClientTime() - mapStartTime;
			int minspast = floor(timepast / 60);
			int dpm = 0;

			if (minspast > 0) {
				dpm = player.GetDamage() / minspast;
			}

			if (!tvFlag) {
				tvFlag = true;
				continue;
			}

			os << "\"" << player.GetSteamID().ConvertToUint64() << "\": {"
				<< "\"name\": \"" << player.GetName().c_str()
				<< "\", \"steamid\": \"" << player.GetSteamID().ConvertToUint64()
				<< "\", \"team\": \"" << player.GetTeam()
				<< "\", \"health\": \"" << player.GetHealth()
				<< "\", \"maxHealth\": \"" << player.GetMaxHealth()
				<< "\", \"maxBuffedHealth\": \"" << player.GetMaxBuffedHealth()
				<< "\", \"class\": \"" << player.GetClass()
				<< "\", \"alive\": \"" << player.IsAlive()
				<< "\", \"score\": \"" << player.GetTotalScore()
				<< "\", \"kills\": \"" << player.GetScore()
				<< "\", \"deaths\": \"" << player.GetDeaths()
				<< "\", \"assists\": \"" << player.GetKillAssists()
				<< "\", \"damage\": \"" << player.GetDamage()
				<< "\", \"totalDamage\": \"" << player.GetTotalDamage()
				<< "\", \"healing\": \"" << player.GetHealing()
				<< "\", \"dpm\": \"" << dpm
				<< "\", \"respawnTime\": \"" << player.GetRespawnTime() - Interfaces::GetEngineTools()->ClientTime()
				<< "\", \"position\": \"" << position.x << ", " << position.y << ", " << position.z << "\", ";
			
			if (player.FindCondition()) {
				if (player.CheckCondition(TFCond::TFCond_Slowed)) {
					os << "\"isSlowed\": true, ";
				}

				if (player.CheckCondition(TFCond::TFCond_Ubercharged)) {
					os << "\"isUbered\": true, ";
				}

				if (player.CheckCondition(TFCond::TFCond_Bleeding)) {
					os << "\"isBleeding\": true, ";
				}

				if (player.CheckCondition(TFCond::TFCond_Bonked)) {
					os << "\"isBonked\": true, ";
				}

				if (player.CheckCondition(TFCond::TFCond_MarkedForDeath)) {
					os << "\"isMarkedForDeath\": true, ";
				}

				if (player.CheckCondition(TFCond::TFCond_MarkedForDeathSilent)) {
					os << "\"isMarkedForDeath\": true, ";
					os << "\"isMarkedForDeathSilent\": true, ";
				}

				if (player.CheckCondition(TFCond::TFCond_SpeedBuffAlly)) {
					os << "\"isAllySpeedBuffed\": true, ";
				}

				if (player.CheckCondition(TFCond::TFCond_Cloaked)) {
					os << "\"isCloaked\": true, ";
				}

				if (player.CheckCondition(TFCond::TFCond_CloakFlicker)) {
					os << "\"isCloakFlickering\": true, ";
				}

				if (player.CheckCondition(TFCond::TFCond_Disguised)) {
					os << "\"isDisguised\": true, ";
				}
			}

			if (player.IsAlive()) {
				CHandle<IClientEntity> weapon = CHandle<IClientEntity>(player.GetActiveWeapon());

				if (weapon) {
					int type = *Entities::GetEntityProp<int*>(weapon, { "m_iPrimaryAmmoType" });
					int ammo = -1;

					if (type != -1)
						ammo = player.GetWeaponAmmo(type);

					os << "\"weapon\": {"
						<< "\"class\": \"" << Entities::GetEntityClassname(weapon) << "\", "
						<< "\"clip1\": " << *Entities::GetEntityProp<int*>(weapon.Get(), { "m_iClip1" }) << ", "
						<< "\"clip2\": " << *Entities::GetEntityProp<int*>(weapon.Get(), { "m_iClip2" }) << ", "
						<< "\"reserve\": " << ammo << ", "
						<< "\"index\": " << *Entities::GetEntityProp<int*>(weapon.Get(), { "m_iItemDefinitionIndex" }) << ", "
						<< "}, ";
				}
			}

			if (player.GetClass() == TFClassType::TFClass_Medic) {
				int type = player.GetMedigunType();
				float charge = player.GetMedigunCharge();
				bool isHealing = player.IsMedigunHealing();

				char name[64];
				sprintf(name, "Unknown");

				std::ostringstream targetId;
				targetId.str("");
				targetId.clear();

				// TODO: Fix this
				if (isHealing) {
					CHandle<IClientEntity> target = player.GetMedigunTarget();
					if (target) {
						int index = target.GetEntryIndex();
						Player healing = Player(index);
						
						targetId << healing.GetSteamID().ConvertToUint64();
					}
				}

				switch (type) {
				case TFMedigun_Unknown:
					sprintf(name, "Unknown");
					break;
				case TFMedigun_MediGun:
					sprintf(name, "MediGun");
					break;
				case TFMedigun_Kritzkrieg:
					sprintf(name, "Kritzkrieg");
					break;
				case TFMedigun_QuickFix:
					sprintf(name, "QuickFix");
					break;
				case TFMedigun_Vaccinator:
					sprintf(name, "Vaccinator");
					break;
				default:
					sprintf(name, "Unknown");
				}

				os << "\"medigun\": {"
					<< "\"type\": \"" << name << "\" , "
					<< "\"charge\": \"" << charge << "\", "
					<< "\"target\": \"" << targetId.str() << "\""
					<< "}, ";
			}
			else if (player.GetClass() == TFClassType::TFClass_Spy) {
				os << "\"disguise\": {"
					<< "\"class\": \"" << player.GetDisguiseClass() << "\" , "
					<< "\"team\": \"" << player.GetDisguiseTeam() << "\" , "
					<< "}, ";
			}

			os << "}, ";
		}
		os << " }";
	}
	else {
		for (int i = 0; i < MAX_CONTROL_POINTS; i++) {
			m_flCapTimeLeft[i] = 0.0f;
			m_flCapLastThinkTime[i] = 0.0f;
			m_nPlayersOnCap[i] = 0;
		}

		inOvertime = false;
		inGameFlag = false;
	}
	os << " }";

	g_Plugin.Transmit(os.str().c_str());
}