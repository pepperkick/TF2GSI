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
#include "ehandle.h"

#include "common.h"
#include "entities/player.h"
#include "entities/team.h"
#include "entities/roundtimer.h"
#include "entities/tfgamerules.h"
#include "entities/objective.h"
#include "ifaces.h"
#include "entities.h"

#include <vector>
#include <set>
#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IServerGameDLL* g_pGameDLL;
IFileSystem* g_pFileSystem;
IHLTVDirector* g_pHLTVDirector;
IVEngineServer* engine;

bool poolReady = false;
bool breakPool = false;

int gAppId; 
const char* gVersion;

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
    Interfaces::pGameEventManager->AddListener(this, "teamplay_game_over", false);
    Interfaces::pGameEventManager->AddListener(this, "teamplay_win_panel", false);

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

    SetTimer(gTimers, 0, 25, &LoopTimer);

    return true;
}

void Plugin::Unload() {
    Interfaces::Unload();
    KillTimer(gTimers, 0);
}

void Plugin::ClientPutInServer(edict_t *pEntity, char const *playername) {}

void Plugin::FireGameEvent(IGameEvent* event) {
	if (strcmp(event->GetName(), "player_death") == 0) {
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
	} else if (strcmp(event->GetName(), "teamplay_win_panel") == 0) {
		extraData 
			<< "\"event\": {"
			<< "\"name\": \"" << event->GetName() << "\", "
			<< "\"team\": \"" <<event->GetInt("winning_team") << "\", "
			<< "}, ";
	}

	SendData();
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
	bool inGameFlag = false;

	if (isInGame) {
		if (!inGameFlag) {
			Player::FindPlayerResource();
			Team::FindTeams();
			TFGameRules::Find();
			ObjectiveResource::Find();

			inGameFlag = true;
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

				os << "\"round\": {"
					<< "\"redTimeLeft\": \"" << redTime << "\", "
					<< "\"blueTimeLeft\": \"" << blueTime << "\", "
					<< "\"cap0\": {"
					<< "\"locked\": \"" << objective->IsCapLocked(0) << "\", "
					<< "\"blocked\": \"" << objective->IsCapBlocked(0) << "\", "
					<< "\"cappedTeam\": \"" << objective->CapOwner(0) << "\", "
					<< "\"cappingTeam\": \"" << objective->CappingTeam(0) << "\", "
					<< "\"unlockTime\": \"" << objective->CapUnlockTime(0) - Interfaces::GetEngineTools()->ClientTime() << "\", "
					<< " }, "
					<< " }, ";
			}
			else if (is5cp) {
				RoundTimer::Find(1);
				RoundTimer* timer = RoundTimer::Get(0);

				os << "\"round\": {"
					<< "\"isPaused\": \"" << timer->IsPaused() << "\", "
					<< "\"timeRemaining\": \"" << timer->GetTimeRemaining() << "\", "
					<< "\"maxLength\": \"" << timer->GetMaxLength() << "\", "
					<< "\"endTime\": \"" << timer->GetEndTime() - Interfaces::GetEngineTools()->ClientTime() << "\", "
					<< "\"noOfCaps\": \"" << numOfCaps << "\", "
					<< "\"cap0\": {"
					<< "\"locked\": \"" << objective->IsCapLocked(0) << "\", "
					<< "\"blocked\": \"" << objective->IsCapBlocked(0) << "\", "
					<< "\"cappedTeam\": \"" << objective->CapOwner(0) << "\", "
					<< "\"cappingTeam\": \"" << objective->CappingTeam(0) << "\", "
					<< "\"unlockTime\": \"" << objective->CapUnlockTime(0) - Interfaces::GetEngineTools()->ClientTime() << "\", "
					<< " }, "
					<< "\"cap1\": {"
					<< "\"locked\": \"" << objective->IsCapLocked(1) << "\", "
					<< "\"blocked\": \"" << objective->IsCapBlocked(1) << "\", "
					<< "\"cappedTeam\": \"" << objective->CapOwner(1) << "\", "
					<< "\"cappingTeam\": \"" << objective->CappingTeam(1) << "\", "
					<< "\"unlockTime\": \"" << objective->CapUnlockTime(1) - Interfaces::GetEngineTools()->ClientTime() << "\", "
					<< " }, "
					<< "\"cap2\": {"
					<< "\"locked\": \"" << objective->IsCapLocked(2) << "\", "
					<< "\"blocked\": \"" << objective->IsCapBlocked(2) << "\", "
					<< "\"cappedTeam\": \"" << objective->CapOwner(2) << "\", "
					<< "\"cappingTeam\": \"" << objective->CappingTeam(2) << "\", "
					<< "\"unlockTime\": \"" << objective->CapUnlockTime(2) - Interfaces::GetEngineTools()->ClientTime() << "\", "
					<< " }, "
					<< "\"cap3\": {"
					<< "\"locked\": \"" << objective->IsCapLocked(3) << "\", "
					<< "\"blocked\": \"" << objective->IsCapBlocked(3) << "\", "
					<< "\"cappedTeam\": \"" << objective->CapOwner(3) << "\", "
					<< "\"cappingTeam\": \"" << objective->CappingTeam(3) << "\", "
					<< "\"unlockTime\": \"" << objective->CapUnlockTime(3) - Interfaces::GetEngineTools()->ClientTime() << "\", "
					<< " }, "
					<< "\"cap4\": {"
					<< "\"locked\": \"" << objective->IsCapLocked(4) << "\", "
					<< "\"blocked\": \"" << objective->IsCapBlocked(4) << "\", "
					<< "\"cappedTeam\": \"" << objective->CapOwner(4) << "\", "
					<< "\"cappingTeam\": \"" << objective->CappingTeam(4) << "\", "
					<< "\"unlockTime\": \"" << objective->CapUnlockTime(4) - Interfaces::GetEngineTools()->ClientTime() << "\", "
					<< " }, "
					<< " }, ";

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

			if (!tvFlag) {
				tvFlag = true;
				continue;
			}

			os << "\"" << player.GetSteamID().ConvertToUint64() << "\": {"
				<< "\"name\": \"" << player.GetName().c_str()
				<< "\", \"steamid\": \"" << player.GetSteamID().ConvertToUint64()
				<< "\", \"team\": \"" << player.GetTeam()
				<< "\", \"health\": \"" << player.GetHealth()
				<< "\", \"class\": \"" << player.GetClass()
				<< "\", \"maxHealth\": \"" << player.GetMaxHealth()
				<< "\", \"alive\": \"" << player.IsAlive()
				<< "\", \"score\": \"" << player.GetTotalScore()
				<< "\", \"kills\": \"" << player.GetScore()
				<< "\", \"deaths\": \"" << player.GetDeaths()
				<< "\", \"damage\": \"" << player.GetDamage()
				<< "\", \"respawnTime\": \"" << player.GetRespawnTime() - Interfaces::GetEngineTools()->ClientTime()
				<< "\", \"position\": \"" << position.x << ", " << position.y << ", " << position.z << "\", ";

			if (player.CheckCondition(TFCond::TFCond_Ubercharged)) {
				os << "\"isUbered\": true";
			}

			/*if (player.CheckCondition(TFCond::TFCond_Bleeding)) {
				os << "\"isBleeding\": true";
			}

			if (player.CheckCondition(TFCond::TFCond_Bonked)) {
				os << "\"isBonked\": true";
			}

			if (player.CheckCondition(TFCond::TFCond_MarkedForDeath)) {
				os << "\"isMarkedForDeath\": true";
			}

			if (player.CheckCondition(TFCond::TFCond_MarkedForDeath)) {
				os << "\"isMarkedForDeath\": true";
			}

			if (player.CheckCondition(TFCond::TFCond_Cloaked)) {
				os << "\"isCloaked\": true";
			}

			if (player.CheckCondition(TFCond::TFCond_CloakFlicker)) {
				os << "\"isCloakFlickering\": true";
			}

			if (player.CheckCondition(TFCond::TFCond_Disguised)) {
				os << "\"isDisguised\": true";
			}*/

			CHandle<IClientEntity> weapon = CHandle<IClientEntity>(player.GetActiveWeapon());

			if (weapon) {
				os << "\"weapon\": {"
					<< "\"class\": \"" << Entities::GetEntityClassname(weapon) << "\", "
					<< "\"clip1\": " << *Entities::GetEntityProp<int*>(weapon, { "m_iClip1" }) << ", "
					<< "\"clip2\": " << *Entities::GetEntityProp<int*>(weapon, { "m_iClip2" }) << ", "
					<< "}, ";
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

			os << "}, ";
		}
		os << " }";
	}
	else {
		inGameFlag = false;
	}
	os << " }";

	g_Plugin.Transmit(os.str().c_str());
}