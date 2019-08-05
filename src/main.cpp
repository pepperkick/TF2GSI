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

#include "Common.h"
#include "Entities/Player.h"
#include "Entities/Team.h"
#include "Entities/RoundTimer.h"
#include "Entities/TFGameRules.h"
#include "Entities/Objective.h"
#include "Entities/TeamplayRoundRules.h"
#include "Entities.h"
#include "ifaces.h"
#include "gamedata.h"

#include "socketserver.h"

#include <vector>
#include <set>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>

#include <tao/json.hpp>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TEAM_ARRAY( index, team ) (index + (team * MAX_CONTROL_POINTS))

using namespace std;
using namespace tao;

#include "exceptions.h"
#include "Utils/Hook.h"

IServerGameDLL* g_pGameDLL;
IFileSystem* g_pFileSystem;
IHLTVDirector* g_pHLTVDirector;
IVEngineServer* engine;

bool poolReady = false;
bool breakPool = false;
bool inGameFlag = false;
bool inOvertime = false;

float g_LastUpdate = 0.0f;
int g_AppId, g_EventId = 0;
const char* g_Version;

float m_flCapTimeLeft[MAX_CONTROL_POINTS] = { 0.0f };
float m_flCapLastThinkTime[MAX_CONTROL_POINTS] = { 0.0f };
int m_nPlayersOnCap[MAX_CONTROL_POINTS] = { 0 };
CHandle<IClientEntity> m_hActiveWeapon[MAX_PLAYERS] = { nullptr };

json::value g_EventData = tao::json::empty_object;

void LoopTimer();
void SendData();
string GetEventKey();

const int VTableElements = 76;
std::vector<size_t> vtable() {
	size_t vtbl[VTableElements];
	memcpy(vtbl, *reinterpret_cast<size_t * *>(Interfaces::GetClientDLL()), VTableElements * sizeof(size_t));
	return std::vector<size_t>(vtbl, vtbl + sizeof vtbl / sizeof vtbl[0]);
}

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

    void Transmit(const char*);
	void Transmit(std::string);
	void Transmit(json::value);
};

Plugin g_Plugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(Plugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_Plugin);

bool Plugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) {
    LogInfo("Loading plugin, Version: %s\n", PLUGIN_VERSION);

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

	g_AppId = Interfaces::GetEngineClient()->GetAppID();
	g_Version = Interfaces::GetEngineClient()->GetProductVersionString();

	if (!Interfaces::GetClientDLL()) {
		LogError("Could not find game DLL interface, aborting load\n");
        return false;
    }

	if (!Interfaces::GetClientEngineTools()) {
		LogError("Could not find engine tools, aborting load\n");
        return false;
    }

	if (!Interfaces::GetEngineClient()) {
		LogError("Could not find engine client, aborting load\n");
        return false;
    }

	if (!Player::CheckDependencies()) {
		LogError("Required player helper class\n");
		return false;
	}

	static Hook<CallConvention::stdcall_t, HRESULT, ClientFrameStage_t> FrameStageNotify;
	FrameStageNotify.apply(vtable()[35], [](
		ClientFrameStage_t a1) -> HRESULT	{

		if (a1 == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_END) {
			g_LastUpdate = Interfaces::GetEngineTools()->ClientTime();
		}

		auto ret = FrameStageNotify.call_orig(a1);

		return ret;
	});

	poolReady = true;

	thread websocketThread(SocketServer::Start);
	thread datapoolThread(LoopTimer);

	websocketThread.detach();
	datapoolThread.detach();

	LogSuccess("Plugin Started\n");

    return true;
}

void Plugin::Unload() {
    Interfaces::Unload();
	SocketServer::Stop();

	LogInfo("Plugin Stopped\n");

	poolReady = false;
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

		string key = "event" + to_string(g_EventId);
		g_EventData[GetEventKey()] = {
			{ "name", event->GetName() },
			{ "victim", victim.GetSteamID().ConvertToUint64() },
			{ "attacker", attacker.GetSteamID().ConvertToUint64() },
			{ "assister", assister.GetSteamID().ConvertToUint64() },
			{ "weapon", weapon },
		};
	}
	else if (!strcmp(event->GetName(), "teamplay_win_panel")) {
		string key = "event" + to_string(g_EventId);
		g_EventData[GetEventKey()] = {
			{ "name", event->GetName() },
			{ "team", event->GetInt("winning_team") },
		};
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
	SocketServer::SetMessage(msg);
}

void Plugin::Transmit(std::string msg) {
	Plugin::Transmit(msg.c_str());
}

void Plugin::Transmit(json::value msg) {
	string test = json::to_string(msg);

	Plugin::Transmit(json::to_string(msg));
}

void LoopTimer() {
	while (poolReady) {
		SendData();
		this_thread::sleep_for(chrono::milliseconds(100));
	}
}

class C_TFGameRules;

using Raw = int(__thiscall*)(void*);
static Raw fn = nullptr;

C_TFGameRules* tfGameRulesClass = nullptr;

C_TFGameRules* GetTFGameRulesClass() {
#if defined _WIN32
	static DWORD pointer = NULL;

	if (!tfGameRulesClass) {
		tfGameRulesClass = **(C_TFGameRules ***)(SignatureScan("client", C_TFGAMERULES_SIG, C_TFGAMERULES_MASK) + C_TFGAMERULES_OFFSET);
	}

	return tfGameRulesClass;
#else
	throw bad_pointer("C_TFGameRules");

	return nullptr;
#endif
}

int GetMapTimeLeft() {
#if defined _WIN32
	using Raw = int(__thiscall*)(void*);

	if (!fn) {
		fn = (Raw)SignatureScan("client", GETTIME_SIG, GETTIME_MASK);

		if (!fn) {
			throw bad_pointer("C_TFGameRules::GetTimeLeft");
		}
	}

	return fn((void*) GetTFGameRulesClass());
#else
	throw bad_pointer("C_TFGameRules::GetTimeLeft");

	return nullptr;
#endif
}

void SendData() {
	Interfaces::GetEngineTools()->ForceSend();
	Interfaces::GetEngineTools()->ForceUpdateDuringPause();

	Player localPlayer = Player::GetLocalPlayer();
	Player targetPlayer = Player::GetTargetPlayer();

	bool isInGame = Interfaces::GetEngineClient()->IsInGame();
	string mapName = Interfaces::GetEngineClient()->GetLevelName();
	
	json::value data = tao::json::empty_object;

	data["provider"] = {
		{ "name", "Team Fortress 2" },
		{ "appid", g_AppId },
		{ "version", g_Version },
		{ "gsi", PLUGIN_VERSION },
	};

	if (g_EventId > 5) {
		data["events"] = tao::json::empty_object;
		data["events"]["count"] = g_EventId;

		for (int i = g_EventId; i > g_EventId - 5; i--) {
			string key = "event" + to_string(i - 1);
			data["events"][key] = g_EventData[key];
		}
	}
	else {
		data["events"] = g_EventData;
		data["events"]["count"] = g_EventId;
	}

	if (targetPlayer) {
		data["player"] = {
			{ "name", targetPlayer.GetName().c_str() },
			{ "steamid", to_string(targetPlayer.GetSteamID().ConvertToUint64()) },
			{ "mode", Player::GetTargetObserverMode() },
		};
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
		}

		data["map"] = {
			{ "name", mapName }
		};

		if (ObjectiveResource::Get()->IsValid()) {
			ObjectiveResource* objective = ObjectiveResource::Get();
			bool isKoth = false, is5cp = false;
			string type;
			int numOfCaps = -1;

			type = "Unknown";

			if (objective->IsValid()) {
				numOfCaps = objective->GetNumCP();

				if (numOfCaps == 1) {
					type = "KOTH";
					isKoth = true;
				}
				else if (numOfCaps == 5) {
					type = "5CP";
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

			bool isPaused = false;

			if (Interfaces::GetEngineTools()->ClientTime() - g_LastUpdate > 3) {
				isPaused = true;
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

				data["round"] = {
					{ "gameType", type },
					{ "isPaused", isPaused },
					{ "redTimeLeft", redTime },
					{ "blueTimeLeft", blueTime },
					{ "noOfCaps", numOfCaps },
				};
			}
			else if (is5cp) {
				RoundTimer::Find(1);
				RoundTimer* timer = RoundTimer::Get(0);

				data["round"] = {
					{ "gameType", type },
					{ "isPaused", isPaused },
					{ "matchTimeLeft", GetMapTimeLeft() },
					{ "maxLength",  timer->GetMaxLength() },
					{ "roundTimeLeft", timer->GetEndTime() - Interfaces::GetEngineTools()->ClientTime() },
					{ "noOfCaps", numOfCaps },
				};

			}
			else {
				RoundTimer::Find(1);
				RoundTimer* timer = RoundTimer::Get(0);

				data["round"] = {
					{ "gameType", type },
					{ "isPaused", isPaused },
				};
			}

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

				string key = "cap" + to_string(i);

				data["round"][key] = {
					{ "locked", objective->IsCapLocked(i) },
					{ "blocked", objective->IsCapBlocked(i) },
					{ "cappedTeam", objective->CapOwner(i) },
					{ "cappingTeam", cappingTeam },
					{ "unlockTime", objective->CapUnlockTime(i) - Interfaces::GetEngineTools()->ClientTime() },
					{ "playersOnCap", playersOnCap },
					{ "percentage", percentage },
					{ "mapResetTime1", TeamPlayRoundRules::Get()->GetMapResetTime() },
					{ "mapResetTime2", TFGameRules::Get()->GetMapResetTime() },
				};
			}
		}

		if (Team::GetRedTeam()->IsValid() && Team::GetBlueTeam()->IsValid()) {
			data["teams"] = {
				{ "team_blue", {
					{ "name", Team::GetBlueTeam()->GetName().c_str() },
					{ "score", Team::GetBlueTeam()->GetScore() },
					{ "rounds", Team::GetBlueTeam()->GetRoundsWon() },
				}},
				{ "team_red", {
					{ "name", Team::GetRedTeam()->GetName().c_str() },
					{ "score", Team::GetRedTeam()->GetScore() },
					{ "rounds", Team::GetRedTeam()->GetRoundsWon() },
				}}
			};
		}

		data["allplayers"] = json::empty_object;
		for (Player player : Player::Iterable()) {
			Vector position = player.GetPosition();
			const char* name = player.GetName().c_str();

			if (!tvFlag) {
				tvFlag = true;
				continue;
			}

			string steamid = to_string(player.GetSteamID().ConvertToUint64());

			data["allplayers"][steamid] = {
				{ "name", player.GetName().c_str() },
				{ "steamid", steamid },
				{ "team", static_cast<int>(player.GetTeam()) },
				{ "health", player.GetHealth() },
				{ "maxHealth", player.GetMaxHealth() },
				{ "class", static_cast<int>(player.GetClass()) },
				{ "alive", player.IsAlive() },
				{ "score", player.GetTotalScore() },
				{ "kills", player.GetScore() },
				{ "deaths", player.GetDeaths() },
				{ "assists", player.GetKillAssists() },
				{ "damage", player.GetDamage() },
				{ "totalHeadshots", player.GetTotalHeadshots() },
				{ "totalDamage", player.GetTotalDamage() },
				{ "healing", player.GetHealing() },
				{ "respawnTime", player.GetRespawnTime() - Interfaces::GetEngineTools()->ClientTime() },
				{ "position", { { "x", position.x }, { "y", position.y }, { "z", position.z } } }
			};

			if (player.FindCondition()) {
				if (player.CheckCondition(TFCond::TFCond_Slowed)) {
					data["allplayers"][steamid]["isSlowed"] = true;
				}

				if (player.CheckCondition(TFCond::TFCond_Ubercharged)) {
					data["allplayers"][steamid]["isUbered"] = true;
				}

				if (player.CheckCondition(TFCond::TFCond_Bleeding)) {
					data["allplayers"][steamid]["isBleeding"] = true;
				}

				if (player.CheckCondition(TFCond::TFCond_Bonked)) {
					data["allplayers"][steamid]["isBonked"] = true;
				}

				if (player.CheckCondition(TFCond::TFCond_MarkedForDeath)) {
					data["allplayers"][steamid]["isMarkedForDeath"] = true;
				}

				if (player.CheckCondition(TFCond::TFCond_MarkedForDeathSilent)) {
					data["allplayers"][steamid]["isMarkedForDeath"] = true;
					data["allplayers"][steamid]["isMarkedForDeathSilent"] = true;
				}

				if (player.CheckCondition(TFCond::TFCond_SpeedBuffAlly)) {
					data["allplayers"][steamid]["isAllySpeedBuffed"] = true;
				}

				if (player.CheckCondition(TFCond::TFCond_Cloaked)) {
					data["allplayers"][steamid]["isCloaked"] = true;
				}

				if (player.CheckCondition(TFCond::TFCond_CloakFlicker)) {
					data["allplayers"][steamid]["isCloakFlickering"] = true;
				}

				if (player.CheckCondition(TFCond::TFCond_Disguised)) {
					data["allplayers"][steamid]["isDisguised"] = true;
				}

				if (player.CheckCondition(TFCond::TFCond_Zoomed)) {
					data["allplayers"][steamid]["isZoomed"] = true;
				}
			}

			if (player.IsAlive()) {
				CHandle<IClientEntity> weapon = CHandle<IClientEntity>(player.GetActiveWeapon());

				if (weapon) {
					int type = *Entities::GetEntityProp<int*>(weapon, { "m_iPrimaryAmmoType" });
					int ammo = -1;

					if (type != -1)
						ammo = player.GetWeaponAmmo(type);

					data["allplayers"][steamid]["weapon"] = {
						{ "class", Entities::GetEntityClassname(weapon) },
						{ "clip1", *Entities::GetEntityProp<int*>(weapon.Get(), { "m_iClip1" }) },
						{ "clip2", *Entities::GetEntityProp<int*>(weapon.Get(), { "m_iClip2" }) },
						{ "index", *Entities::GetEntityProp<int*>(weapon.Get(), { "m_iItemDefinitionIndex" }) },
						{ "reserve", ammo },
					};
				}
			}

			if (player.GetClass() == TFClassType::TFClass_Medic) {
				int type = player.GetMedigunType();
				float charge = player.GetMedigunCharge();
				bool isHealing = player.IsMedigunHealing();

				string name;
				name = "Unknown";

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
					name = "Unknown";
					break;
				case TFMedigun_MediGun:
					name = "MediGun";
					break;
				case TFMedigun_Kritzkrieg:
					name = "Kritzkrieg";
					break;
				case TFMedigun_QuickFix:
					name = "QuickFix";
					break;
				case TFMedigun_Vaccinator:
					name = "Vaccinator";
					break;
				default:
					name = "Unknown";
				}

				data["allplayers"][steamid]["medigun"] = {
					{ "type", name },
					{ "charge", charge },
					{ "target",  targetId.str() },
				};
			}
			else if (player.GetClass() == TFClassType::TFClass_Spy) {

				data["allplayers"][steamid]["disguise"] = {
					{ "class", player.GetDisguiseClass() },
					{ "team", player.GetDisguiseTeam() },
				};
			}
		}
	}
	else {
		for (int i = 0; i < MAX_CONTROL_POINTS; i++) {
			m_flCapTimeLeft[i] = 0.0f;
			m_flCapLastThinkTime[i] = 0.0f;
			m_nPlayersOnCap[i] = 0;
		}

		g_EventData = json::empty_object;
		g_EventId = 0;

		inOvertime = false;
		inGameFlag = false;
	}

	g_Plugin.Transmit(data);
}

string GetEventKey() {
	string key = "event" + to_string(g_EventId);
	g_EventId++;
	return key;
}