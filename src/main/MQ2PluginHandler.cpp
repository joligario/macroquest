/*
 * MacroQuest2: The extension platform for EverQuest
 * Copyright (C) 2002-2019 MacroQuest Authors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "pch.h"
#include "MQ2Main.h"

//#define DEBUG_PLUGINS

namespace mq {

#ifdef DEBUG_PLUGINS
#define PluginDebug DebugSpew
#else
#define PluginDebug //
#endif

#define NO_TIMESTAMP_CHECK

static std::atomic_bool s_pluginsInitialized = false;
static uint32_t s_mq2mainstamp = 0;
static std::recursive_mutex s_pluginsMutex;

static uint32_t checkme(char* module)
{
	PIMAGE_DOS_HEADER pd = (PIMAGE_DOS_HEADER)module;
	PIMAGE_FILE_HEADER pf;

	uint8_t* p = reinterpret_cast<uint8_t*>(module) + pd->e_lfanew;
	p += 4;  // skip sig

	pf = reinterpret_cast<PIMAGE_FILE_HEADER>(p);
	return pf->TimeDateStamp;
}

int LoadMQ2Plugin(const char* pszFilename, bool bCustom /* = false */)
{
	std::string strFileName = pszFilename;
	const int Pos = ci_find_substr(strFileName, ".dll");
	if (Pos != -1)
	{
		strFileName = strFileName.substr(0, Pos);
	}

	if (HMODULE hThemod = GetModuleHandle((strFileName + ".dll").data()))
	{
		DebugSpew("LoadMQ2Plugin(0)(%s) already loaded", (strFileName + ".dll").data());
		return 2;
	}

	std::scoped_lock lock(s_pluginsMutex);

	DebugSpew("LoadMQ2Plugin(%s)", strFileName.data());

	std::filesystem::path pathToPlugin = mq::internal_paths::Plugins / (strFileName + ".dll");
	HMODULE hmod = LoadLibrary(pathToPlugin.string().data());
	if (!hmod)
	{
		DebugSpew("LoadMQ2Plugin(%s) Failed", strFileName.data());
		return 0;
	}

	// im disabling this check in debug builds because i can't just make a minor change and rebuild just mq2main
	// without having to actually rebuild ALL my plugins even though its technically not needed for them to work -eqmule sep 11 2016
#if !defined(_DEBUG) && !defined(NO_TIMESTAMP_CHECK)
	if (!s_mq2mainstamp)
	{
		s_mq2mainstamp = checkme((char*)GetCurrentModule());
	}

	uint32_t timestamp = checkme((char*)hmod);
	if (s_mq2mainstamp > timestamp)
	{
		char tmpbuff[MAX_PATH];
		sprintf_s(tmpbuff, "Please recompile %s -- it is out of date with respect to mq2main (%d > %d)",
			pathToPlugin.string().data(), s_mq2mainstamp, timestamp);
		DebugSpew("%s", tmpbuff);
		MessageBoxA(NULL, tmpbuff, "Plugin Load Failed", MB_OK);

		FreeLibrary(hmod);
		return 0;
	}
#endif // !_DEBUG

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (hmod == pPlugin->hModule)
		{
			DebugSpew("LoadMQ2Plugin(%s) already loaded", strFileName.data());

			// LoadLibrary count must match FreeLibrary count for unloading to work.
			FreeLibrary(hmod);
			return 2; // already loaded
		}
		pPlugin = pPlugin->pNext;
	}

	pPlugin = new MQPlugin;
	memset(pPlugin, 0, sizeof(MQPlugin));
	pPlugin->bCustom           = bCustom;
	pPlugin->hModule           = hmod;
	strcpy_s(pPlugin->szFilename, strFileName.data());
	pPlugin->Initialize        = (fMQInitializePlugin)GetProcAddress(hmod, "InitializePlugin");
	pPlugin->Shutdown          = (fMQShutdownPlugin)GetProcAddress(hmod, "ShutdownPlugin");
	pPlugin->IncomingChat      = (fMQIncomingChat)GetProcAddress(hmod, "OnIncomingChat");
	pPlugin->Pulse             = (fMQPulse)GetProcAddress(hmod, "OnPulse");
	pPlugin->WriteChatColor    = (fMQWriteChatColor)GetProcAddress(hmod, "OnWriteChatColor");
	pPlugin->Zoned             = (fMQZoned)GetProcAddress(hmod, "OnZoned");
	pPlugin->CleanUI           = (fMQCleanUI)GetProcAddress(hmod, "OnCleanUI");
	pPlugin->ReloadUI          = (fMQReloadUI)GetProcAddress(hmod, "OnReloadUI");
	pPlugin->DrawHUD           = (fMQDrawHUD)GetProcAddress(hmod, "OnDrawHUD");
	pPlugin->SetGameState      = (fMQSetGameState)GetProcAddress(hmod, "SetGameState");
	pPlugin->AddSpawn          = (fMQSpawn)GetProcAddress(hmod, "OnAddSpawn");
	pPlugin->RemoveSpawn       = (fMQSpawn)GetProcAddress(hmod, "OnRemoveSpawn");
	pPlugin->AddGroundItem     = (fMQGroundItem)GetProcAddress(hmod, "OnAddGroundItem");
	pPlugin->RemoveGroundItem  = (fMQGroundItem)GetProcAddress(hmod, "OnRemoveGroundItem");
	pPlugin->BeginZone         = (fMQBeginZone)GetProcAddress(hmod, "OnBeginZone");
	pPlugin->EndZone           = (fMQEndZone)GetProcAddress(hmod, "OnEndZone");

	float* ftmp = (float*)GetProcAddress(hmod, "?MQ2Version@@3MA");
	if (ftmp)
		pPlugin->fpVersion = *ftmp;
	else
		pPlugin->fpVersion = 1.0;

	// initialize plugin
	if (pPlugin->Initialize)
		pPlugin->Initialize();

	// init gamestate
	if (pPlugin->SetGameState)
		pPlugin->SetGameState(GetGameState());

	// init spawns
	if (pPlugin->AddSpawn && GetGameState() == GAMESTATE_INGAME)
	{
		SPAWNINFO* pSpawn = (SPAWNINFO*)pSpawnList;
		while (pSpawn)
		{
			pPlugin->AddSpawn(pSpawn);
			pSpawn = pSpawn->pNext;
		}
	}

	// init ground items
	if (pPlugin->AddGroundItem && GetGameState() == GAMESTATE_INGAME)
	{
		PGROUNDITEM pItem = *(PGROUNDITEM*)pItemList;
		while (pItem)
		{
			pPlugin->AddGroundItem(pItem);
			pItem = pItem->pNext;
		}
	}

	// add to plugin list
	pPlugin->pLast = nullptr;
	pPlugin->pNext = pPlugins;
	if (pPlugins)
		pPlugins->pLast = pPlugin;
	pPlugins = pPlugin;

	// load cfg file if exists
	LoadCfgFile((strFileName + "-AutoExec").data(), false);

	return 1;
}

bool UnloadMQ2Plugin(const char* pszFilename)
{
	DebugSpew("UnloadMQ2Plugin");

	char Filename[MAX_PATH] = { 0 };
	strcpy_s(Filename, pszFilename);
	_strlwr_s(Filename);
	char* Temp = strstr(Filename, ".dll");
	if (Temp) *Temp = 0;

	// lock before accessing the list
	std::scoped_lock lock(s_pluginsMutex);

	// find plugin in list
	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (!_stricmp(Filename, pPlugin->szFilename))
		{
			// unlink from list
			if (pPlugin->pLast)
				pPlugin->pLast->pNext = pPlugin->pNext;
			else
				pPlugins = pPlugin->pNext;
			if (pPlugin->pNext)
				pPlugin->pNext->pLast = pPlugin->pLast;

			// call Plugin:CleanUI
			if (pPlugin->CleanUI)
				pPlugin->CleanUI();

			// call Plugin:Shutdown
			if (pPlugin->Shutdown)
				pPlugin->Shutdown();

			FreeLibrary(pPlugin->hModule);

			delete pPlugin;
			return true;
		}

		pPlugin = pPlugin->pNext;
	}

	return false;
}

// FIXME: Uses too much stack space
void InitializeMQ2Plugins()
{
	DebugSpew("Initializing plugins");

	bmWriteChatColor = AddMQ2Benchmark("WriteChatColor");
	bmPluginsIncomingChat = AddMQ2Benchmark("PluginsIncomingChat");
	bmPluginsPulse = AddMQ2Benchmark("PluginsPulse");
	bmPluginsOnZoned = AddMQ2Benchmark("PluginsOnZoned");
	bmPluginsCleanUI = AddMQ2Benchmark("PluginsCleanUI");
	bmPluginsReloadUI = AddMQ2Benchmark("PluginsReloadUI");
	bmPluginsDrawHUD = AddMQ2Benchmark("PluginsDrawHUD");
	bmPluginsSetGameState = AddMQ2Benchmark("PluginsSetGameState");
	bmCalculate = AddMQ2Benchmark("Calculate");
	bmBeginZone = AddMQ2Benchmark("BeginZone");
	bmEndZone = AddMQ2Benchmark("EndZone");

	// Until we clean this up -- longest plugin name last time someone looked at this is 17 characters.  Assume 30 chars * 250 plugins = 7500.
	char PluginList[MAX_STRING * 4] = { 0 };
	char* pPluginList = nullptr;

	// lock plugin list before manipulating it
	std::scoped_lock lock(s_pluginsMutex);
	s_pluginsInitialized = true;

	GetPrivateProfileString("Plugins", nullptr, "", PluginList, MAX_STRING * 4, mq::internal_paths::MQini.string().data());
	pPluginList = PluginList;

	while (pPluginList[0] != 0)
	{
		if (GetPrivateProfileBool("Plugins", pPluginList, false, mq::internal_paths::MQini))
		{
			LoadMQ2Plugin(pPluginList);
		}

		pPluginList += strlen(pPluginList) + 1;
	}
}

void UnloadMQ2Plugins()
{
	std::scoped_lock lock(s_pluginsMutex);

	while (pPlugins)
	{
		DebugSpew("%s->Unload()", pPlugins->szFilename);
		UnloadMQ2Plugin(pPlugins->szFilename);
	}
}

void ShutdownMQ2Plugins()
{
	std::scoped_lock lock(s_pluginsMutex);
	s_pluginsInitialized = false;

	MQPlugin* pPlugin = nullptr;
	while (pPlugins)
	{
		if (!_stricmp(pPlugins->szFilename, "mq2ic")) // has to be the last one we unload...
		{
			pPlugin = pPlugins;
			pPlugins = pPlugins->pNext;
			continue;
		}

		DebugSpew("%s->Unload()", pPlugins->szFilename);
		UnloadMQ2Plugin(pPlugins->szFilename);
	}
	pPlugins = pPlugin;

	UnloadMQ2Plugin("mq2ic");
}

void PluginsWriteChatColor(const char* Line, int Color, int Filter)
{
	if (!s_pluginsInitialized)
		return;
	if (gFilterMQ)
		return;

	PluginDebug("Begin WriteChatColor()");
	EnterMQ2Benchmark(bmWriteChatColor);

	if (size_t len = strlen(Line))
	{
		std::unique_ptr<char[]> plainText = std::make_unique<char[]>(len + 1);

		StripMQChat(Line, plainText.get());
		CheckChatForEvent(plainText.get());

		DebugSpew("WriteChatColor(%s)", Line);
	}

	// enter lock before accessing the plugin list
	std::scoped_lock lock(s_pluginsMutex);

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->WriteChatColor)
		{
			pPlugin->WriteChatColor(Line, Color, Filter);
		}

		pPlugin = pPlugin->pNext;
	}

	ExitMQ2Benchmark(bmWriteChatColor);
}

bool PluginsIncomingChat(const char* Line, DWORD Color)
{
	if (!s_pluginsInitialized)
		return false;
	if (!Line[0])
		return false;

	std::scoped_lock lock(s_pluginsMutex);
	PluginDebug("PluginsIncomingChat()");

	bool Ret = false;

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->IncomingChat)
		{
			Ret |= pPlugin->IncomingChat(Line, Color);
		}

		pPlugin = pPlugin->pNext;
	}

	return Ret;
}

void PulsePlugins()
{
	if (!s_pluginsInitialized)
		return;

	std::scoped_lock lock(s_pluginsMutex);
	PluginDebug("PulsePlugins()");

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->Pulse)
		{
			pPlugin->Pulse();
		}

		pPlugin = pPlugin->pNext;
	}
}

void PluginsZoned()
{
	if (!s_pluginsInitialized)
		return;

	std::scoped_lock lock(s_pluginsMutex);
	PluginDebug("PluginsZoned()");

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->Zoned)
		{
			DebugSpew("%s->Zoned()", pPlugin->szFilename);
			pPlugin->Zoned();
		}

		pPlugin = pPlugin->pNext;
	}

	if (pZoneInfo)
	{
		char szTemp[128];
		sprintf_s(szTemp, "You have entered %s.", ((PZONEINFO)pZoneInfo)->LongName);

		CheckChatForEvent(szTemp);
	}
}

void PluginsCleanUI()
{
	if (!s_pluginsInitialized)
		return;

	std::scoped_lock lock(s_pluginsMutex);
	PluginDebug("PluginsCleanUI()");

	DeleteMQ2NewsWindow();
	RemoveAutoBankMenu();

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->CleanUI)
		{
			DebugSpew("%s->CleanUI()", pPlugin->szFilename);
			pPlugin->CleanUI();
		}
		pPlugin = pPlugin->pNext;
	}
}

void PluginsReloadUI()
{
	if (!s_pluginsInitialized)
		return;

	std::scoped_lock lock(s_pluginsMutex);
	PluginDebug("PluginsReloadUI()");

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->ReloadUI)
		{
			DebugSpew("%s->ReloadUI()", pPlugin->szFilename);
			pPlugin->ReloadUI();
		}

		pPlugin = pPlugin->pNext;
	}
}

// Defined in MQ2Utilities.cpp
DWORD CALLBACK InitializeMQ2SpellDb(void* pData);

void PluginsSetGameState(DWORD GameState)
{
	if (!s_pluginsInitialized)
		return;

	std::scoped_lock lock(s_pluginsMutex);

	PluginDebug("PluginsSetGameState()");

	static bool AutoExec = false;
	static bool CharSelect = true;

	DrawHUDParams[0] = 0;
	gGameState = GameState;

	if (GameState != GAMESTATE_INGAME && GameState != GAMESTATE_LOGGINGIN)
	{
		gbSpelldbLoaded = false;
		ghInitializeSpellDbThread = nullptr;
	}

	if (GameState == GAMESTATE_INGAME)
	{
		if (!gbSpelldbLoaded && ghInitializeSpellDbThread == nullptr)
		{
			ghInitializeSpellDbThread = CreateThread(nullptr, 0, InitializeMQ2SpellDb, (void*)1, 0, nullptr);
		}

		gZoning = false;
		gbDoAutoRun = true;

		if (!AutoExec)
		{
			AutoExec = true;
			LoadCfgFile("AutoExec", false);
		}

		if (CharSelect)
		{
			CharSelect = false;
			char szBuffer[MAX_STRING] = { 0 };

			DebugSpew("PluginsSetGameState(%s server)", EQADDR_SERVERNAME);

			if (CHARINFO* pCharInfo = GetCharInfo())
			{
				DebugSpew("PluginsSetGameState(%s name)", pCharInfo->Name);

				sprintf_s(szBuffer, "%s_%s", EQADDR_SERVERNAME, pCharInfo->Name);
				LoadCfgFile(szBuffer, false);
			}

			if (PcProfile* pProfile = GetPcProfile())
			{
				DebugSpew("PluginsSetGameState(%d class)", pProfile->Class);

				sprintf_s(szBuffer, "%s", GetClassDesc(pProfile->Class));
				LoadCfgFile(szBuffer, false);

				if (IC_ClassLvl)
				{
					IC_ClassLvl(pProfile->Class, pProfile->Level, GetCurrentProcessId());
				}
			}
		}
	}
	else if (GameState == GAMESTATE_CHARSELECT)
	{
		if (!AutoExec)
		{
			AutoExec = true;
			LoadCfgFile("AutoExec", false);
		}
		CharSelect = true;
		LoadCfgFile("CharSelect", false);
	}

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->SetGameState)
		{
			DebugSpew("%s->SetGameState(%d)", pPlugin->szFilename, GameState);
			pPlugin->SetGameState(GameState);
		}

		pPlugin = pPlugin->pNext;
	}
}

void PluginsDrawHUD()
{
	if (!s_pluginsInitialized)
		return;

	std::scoped_lock lock(s_pluginsMutex);

	PluginDebug("PluginsDrawHUD()");

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->DrawHUD)
		{
			pPlugin->DrawHUD();
		}

		pPlugin = pPlugin->pNext;
	}
}

void PluginsAddSpawn(SPAWNINFO* pNewSpawn)
{
	if (!s_pluginsInitialized)
		return;

	std::scoped_lock lock(s_pluginsMutex);

	DWORD BodyType = GetBodyType(pNewSpawn);
	PluginDebug("PluginsAddSpawn(%s,%d,%d)", pNewSpawn->Name, pNewSpawn->mActorClient.Race, BodyType);

	if (GetGameState() > GAMESTATE_CHARSELECT)
		SetNameSpriteState(pNewSpawn, true);

	if (GetBodyTypeDesc(BodyType)[0] == '*')
		WriteChatf("Spawn '%s' has unknown bodytype %d", pNewSpawn->Name, BodyType);

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->AddSpawn)
		{
			pPlugin->AddSpawn(pNewSpawn);
		}

		pPlugin = pPlugin->pNext;
	}
}

void PluginsRemoveSpawn(SPAWNINFO* pSpawn)
{
	if (!s_pluginsInitialized)
		return;

	std::scoped_lock lock(s_pluginsMutex);
	PluginDebug("PluginsRemoveSpawn(%s)", pSpawn->Name);

	SpawnByName.erase(pSpawn->Name);

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->RemoveSpawn)
		{
			pPlugin->RemoveSpawn(pSpawn);
		}
		pPlugin = pPlugin->pNext;
	}
}

void PluginsAddGroundItem(GROUNDITEM* pNewGroundItem)
{
	if (!s_pluginsInitialized)
		return;

	if (!pNewGroundItem)
	{
		DebugSpew("PluginsAddGroundItem was NULL");
		return;
	}

	std::scoped_lock lock(s_pluginsMutex);
	PluginDebug("PluginsAddGroundItem()");

	DebugSpew("PluginsAddGroundItem(%s) %.1f,%.1f,%.1f", pNewGroundItem->Name, pNewGroundItem->X, pNewGroundItem->Y, pNewGroundItem->Z);

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->AddGroundItem)
		{
			pPlugin->AddGroundItem(pNewGroundItem);
		}
		pPlugin = pPlugin->pNext;
	}
}

void PluginsRemoveGroundItem(GROUNDITEM* pGroundItem)
{
	if (!s_pluginsInitialized)
		return;

	std::scoped_lock lock(s_pluginsMutex);
	PluginDebug("PluginsRemoveGroundItem()");

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->RemoveGroundItem)
		{
			pPlugin->RemoveGroundItem(pGroundItem);
		}

		pPlugin = pPlugin->pNext;
	}
}

void PluginsBeginZone()
{
	if (!s_pluginsInitialized)
		return;

	std::scoped_lock lock(s_pluginsMutex);
	PluginDebug("PluginsBeginZone()");

	gbInZone = false;
	gZoning = true;

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->BeginZone)
		{
			DebugSpew("%s->BeginZone()", pPlugin->szFilename);
			pPlugin->BeginZone();
		}
		pPlugin = pPlugin->pNext;
	}
}

void PluginsEndZone()
{
	if (!s_pluginsInitialized)
		return;

	std::scoped_lock lock(s_pluginsMutex);
	PluginDebug("PluginsEndZone()");

	gbInZone = true;
	WereWeZoning = true;
	LastEnteredZone = MQGetTickCount64();

	MQPlugin* pPlugin = pPlugins;
	while (pPlugin)
	{
		if (pPlugin->EndZone)
		{
			DebugSpew("%s->EndZone()", pPlugin->szFilename);
			pPlugin->EndZone();
		}

		pPlugin = pPlugin->pNext;
	}

	LoadCfgFile("zoned", true);

	if (PZONEINFO pthezone = (PZONEINFO)pZoneInfo)
	{
		LoadCfgFile(pthezone->ShortName, false);
	}
}

bool IsPluginsInitialized()
{
	return s_pluginsInitialized;
}

void* GetPluginProc(const char* plugin, const char* proc)
{
	unsigned int uiLength = strlen(plugin) + 1;
	auto pLook = pPlugins;
	while (pLook)
	{
		if (!_strnicmp(plugin, pLook->szFilename, uiLength))
			return GetProcAddress(pLook->hModule, proc);
		pLook = pLook->pNext;
	}

	return nullptr;
}

} // namespace mq
