
#pragma once

#include "interface.h"

class C_HLTVCamera;
class CSteamAPIContext;
class IBaseClientDLL;
class IClientEngineTools;
class IClientEntityList;
class IClientMode;
class IEngineTool;
class IGameEventManager2;
class IPrediction;
class IVEngineClient;
class IVModelInfoClient;
class IVRenderView;

class Interfaces {
	public:
		static void Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
		static void Unload();

		static IBaseClientDLL* GetClientDLL() { return pClientDLL; }
		static IClientEngineTools* GetClientEngineTools() { return pClientEngineTools; }
		static IEngineTool* GetEngineTools() { return pEngineTool; }
		static IVEngineClient* GetEngineClient() { return pEngineClient; }

		static IBaseClientDLL *pClientDLL;
		static IClientEngineTools *pClientEngineTools;
		static IClientEntityList *pClientEntityList;
		static IVEngineClient *pEngineClient;
		static IEngineTool *pEngineTool;
		static IGameEventManager2 *pGameEventManager;
		static IVModelInfoClient *pModelInfoClient;
		static IPrediction *pPrediction;
		static IVRenderView *pRenderView;
		static CSteamAPIContext *pSteamAPIContext;

		static bool steamLibrariesAvailable;
		static bool vguiLibrariesAvailable;

		static IClientMode *GetClientMode();
		static C_HLTVCamera *GetHLTVCamera();
};