/*
 * MacroQuest: The extension platform for EverQuest
 * Copyright (C) 2002-2023 MacroQuest Authors
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

#include "MQActorAPI.h"

#include "routing/Routing.h"
#include "routing/PostOffice.h"

namespace mq {
using namespace postoffice;

static void UnloadPluginActorAPI(const char*);

static MQModule s_ActorAPIModule = {
	"ActorAPI",                      // Name
	false,                           // CanUnload
	nullptr,                         // Initialize
	nullptr,                         // Shutdown
	nullptr,                         // Pulse
	nullptr,                         // SetGameState
	nullptr,                         // UpdateImGui
	nullptr,                         // Zoned
	nullptr,                         // WriteChatColor
	nullptr,                         // SpawnAdded
	nullptr,                         // SpawnRemoved
	nullptr,                         // BeginZone
	nullptr,                         // EndZone
	nullptr,                         // LoadPlugin
	UnloadPluginActorAPI,            // UnloadPlugin
	false,                           // loaded
	false                            // manualUnload
};
MQModule* GetActorAPIModule() { return &s_ActorAPIModule; }

static void UnloadPluginActorAPI(const char* pluginName)
{
	MQPlugin* plugin = GetPlugin(pluginName);
	if (plugin != nullptr && pActorAPI != nullptr)
	{
		pActorAPI->OnUnloadPlugin(plugin);
	}
}

MQActorAPI* pActorAPI = nullptr;

postoffice::Dropbox* MQActorAPI::AddActor(const char* localAddress, ReceiveCallback&& receive, MailboxMutator&& mutator, MQPlugin* owner)
{
	auto dropbox = std::make_unique<Dropbox>(GetPostOffice().RegisterAddress(localAddress, std::move(receive), std::move(mutator)));

	// return even if it's invalid so users don't have to null check
	// note that owner can be nullptr here (which would be for mq2main)
	return m_dropboxes[owner].emplace_back(std::move(dropbox)).get();
}

void MQActorAPI::RemoveActor(postoffice::Dropbox*& dropbox, MQPlugin* owner)
{
	if (dropbox != nullptr)
	{
		dropbox->Remove();
		auto dropboxes_it = m_dropboxes.find(owner);
		if (dropboxes_it != m_dropboxes.end())
		{
			auto dropbox_it = std::find_if(dropboxes_it->second.begin(), dropboxes_it->second.end(),
				[dropbox](const std::unique_ptr<postoffice::Dropbox>& ptr) { return ptr.get() == dropbox; });

			if (dropbox_it != dropboxes_it->second.end())
				dropboxes_it->second.erase(dropbox_it);

			if (dropboxes_it->second.empty())
				m_dropboxes.erase(dropboxes_it);
		}

		dropbox = nullptr;
	}
}

void MQActorAPI::OnUnloadPlugin(MQPlugin* plugin)
{}

} // namespace mq
