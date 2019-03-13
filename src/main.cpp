// Fix INVALID_HANDLE_VALUE redefinition warning
#ifndef _LINUX
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifndef NULL
#define NULL nullptr
#endif

#include "common.h"
#include "player.h"
#include "ifaces.h"

#include "interface.h"
#include "filesystem.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
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

#include <vector>
#include <set>
#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IServerGameDLL* g_pGameDLL;
IFileSystem* g_pFileSystem;
IHLTVDirector* g_pHLTVDirector;
IVEngineServer* engine;

class Plugin : public IServerPluginCallbacks, IGameEventListener2 {
public:
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory) override;
    virtual void Unload() override { }
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
    Interfaces::pGameEventManager->AddListener(this, "teamplay_round_win", false);
    Interfaces::pGameEventManager->AddListener(this, "teamplay_round_stalemate", false);
    Interfaces::pGameEventManager->AddListener(this, "teamplay_game_over", false);

    g_pGameDLL = (IServerGameDLL*)gameServerFactory(INTERFACEVERSION_SERVERGAMEDLL, nullptr);
    if(!g_pGameDLL) {
        PRINT_TAG();
        ConColorMsg(Color(255, 0, 0, 255), "Could not find game DLL interface, aborting load\n");
        return false;
    }

    engine = (IVEngineServer*)interfaceFactory(INTERFACEVERSION_VENGINESERVER, nullptr);
    if (!engine) {
        PRINT_TAG();
        ConColorMsg(Color(255, 0, 0, 255), "Could not find engine interface, aborting load\n");
        return false;
    }

    g_pFileSystem = (IFileSystem*)interfaceFactory(FILESYSTEM_INTERFACE_VERSION, nullptr);
    if(!g_pFileSystem) {
        PRINT_TAG();
        ConColorMsg(Color(255, 0, 0, 255), "Could not find filesystem interface, aborting load\n");
        return false;
    }

    if(!Player::CheckDependencies()) {
        PRINT_TAG();
        ConColorMsg(Color(255, 0, 0, 255), "Required player helper class for module!\n");
        return false;
    }

    PRINT_TAG();
    ConColorMsg(Color(255, 255, 0, 255), "Successfully Started!\n");

    std::ostringstream os;
    os << "plugin started";
    this->Transmit(os.str().c_str());

    return true;
}

void Plugin::ClientPutInServer(edict_t *pEntity, char const *playername) {
    PRINT_TAG();
    ConColorMsg(Color(255, 255, 255, 255), "Client Joined: %s\n", playername);
    }

    void Plugin::FireGameEvent(IGameEvent* pEvent) {
    PRINT_TAG();
    ConColorMsg(Color(255, 255, 255, 255), "Event: %s\n", pEvent->GetName());

    std::ostringstream os;
    os << "event " << pEvent->GetName();
    this->Transmit(os.str().c_str());

    for (Player player : Player::Iterable()) {
        ConColorMsg(Color(255, 255, 255, 255), "Player: %s\n", player.GetName().c_str());

        std::ostringstream os;
        os << "player " << player.GetName().c_str();
        this->Transmit(os.str().c_str());
    }
}

void Plugin::Transmit(const char* msg) {
    HANDLE hPipe;
    LPTSTR lptPipeName = TEXT("\\\\.\\pipe\\tf2-gsi");

    hPipe = CreateFile(lptPipeName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    DWORD cbWritten;
    WriteFile(hPipe, msg, strlen(msg), &cbWritten, NULL);
}