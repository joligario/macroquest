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

#include "../common/CXStr.h"
#include "../common/CXWnd.h"
#include "../common/Containers.h"
#include "../common/SharedClasses.h"
#include "../common/UI.h"
#include "EQData(Test).h"

namespace eqlib {

// ***************************************************************************
// Structures
// ***************************************************************************

struct EQCURRENTSELECTION
{
/*0x00*/ DWORD Unknown;
/*0x04*/ DWORD SelectedWnd;                      // address to selection in tree
/*0x08*/ DWORD Unknownlodncrap;
/*0x0c*/ DWORD NumberOfSlots;                    // merchantslots == 79
/*0x10*/ DWORD TextureAnim;
/*0x14*/
};
using PEQCURRENTSELECTION = EQCURRENTSELECTION*;

struct CSidlScreenWnd_VirtualFunctions// : public CXWnd_VirtualFunctions
{
/*0x16c*/ void* CSidlScreenWnd__OnPreZone1c;
/*0x170*/ void* CSidlScreenWnd__OnPreZone1d;
/*0x170*/ void* LoadIniInfo;
/*0x174*/ void* StoreIniInfo;
/*0x178*/ void* EQObject__AsObject;
/*0x17c*/ void* CTreeViewPropertySet__IsPropertySet;
/*0x180*/
};


struct CListWnd_VirtualFunctions// : public CXWnd_VirtualFunctions
{
/*0x168*/ void* CListWnd__OnHeaderClick;
/*0x16c*/ void* CListWnd__DrawColumnSeparators;
/*0x170*/ void* CListWnd__DrawSeparator;
/*0x174*/ void* CListWnd__DrawLine;
/*0x178*/ void* CListWnd__DrawHeader;
/*0x17c*/ void* CListWnd__DrawItem;
/*0x180*/ void* CListWnd__DeleteAll;
/*0x184*/ void* CListWnd__Compare;
/*0x188*/ void* CListWnd__Sort;
};

struct CContextMenu_VirtualFunctions : public CListWnd_VirtualFunctions
{
/*0x18c*/ void* CContextMenu__ShowAt;
};
using CCONTEXTMENUVFTABLE = CContextMenu_VirtualFunctions;
using PCCONTEXTMENUVFTABLE = CContextMenu_VirtualFunctions*;

struct CONTENTDATA
{
	CONTENTS*      pCont;
	int            Unknown;
};

struct CONTENTSARRAY
{
    struct CONTENTDATA Array[1];
};
using PCONTENTSARRAY = CONTENTSARRAY*;


enum eContextMenuFilterIDs
{
	CONTEXT_FILTER_SAY = 100,
	CONTEXT_FILTER_TELL,
	CONTEXT_FILTER_GROUP,
	CONTEXT_FILTER_RAID,
	CONTEXT_FILTER_GUILD,
	CONTEXT_FILTER_OOC,
	CONTEXT_FILTER_AUCTION,
	CONTEXT_FILTER_SHOUT,
	CONTEXT_FILTER_EMOTE,
	CONTEXT_FILTER_MELEE_YOUR_HITS,
	CONTEXT_FILTER_SPELLS_MINE,
	CONTEXT_FILTER_SKILLS,
	CONTEXT_FILTER_CHAT1,
	CONTEXT_FILTER_CHAT2,
	CONTEXT_FILTER_CHAT3,
	CONTEXT_FILTER_CHAT4,
	CONTEXT_FILTER_CHAT5,
	CONTEXT_FILTER_CHAT6,
	CONTEXT_FILTER_CHAT7,
	CONTEXT_FILTER_CHAT8,
	CONTEXT_FILTER_CHAT9,
	CONTEXT_FILTER_CHAT10,
	CONTEXT_FILTER_OTHER,
	CONTEXT_FILTER_MELEE_YOUR_MISSES,
	CONTEXT_FILTER_MELEE_YOU_BEING_HIT,
	CONTEXT_FILTER_MELEE_YOU_BEING_MISSED,
	CONTEXT_FILTER_MELEE_OTHERS_HITS,
	CONTEXT_FILTER_MELEE_OTHERS_MISSES,
	CONTEXT_FILTER_MELEE_MY_DEATH,
	CONTEXT_FILTER_MELEE_OTHER_PC_DEATH,
	CONTEXT_FILTER_MELEE_CRITICAL_HITS,
	CONTEXT_FILTER_MELEE_DISCIPLINES,
	CONTEXT_FILTER_MELEE_WARNINGS,
	CONTEXT_FILTER_MELEE_NPC_RAMPAGE,
	CONTEXT_FILTER_MELEE_NPC_FLURRY,
	CONTEXT_FILTER_MELEE_NPC_ENRAGE,
	CONTEXT_FILTER_SPELLS_OTHERS,
	CONTEXT_FILTER_SPELLS_FAILURES	,
	CONTEXT_FILTER_SPELLS_CRITICALS,
	CONTEXT_FILTER_SPELLS_WORN_OFF	,
	CONTEXT_FILTER_SPELLS_DD_YOURS,
	CONTEXT_FILTER_FOCUS_EFFECTS,
	CONTEXT_FILTER_RANDOM_YOUR_ROLLS,
	CONTEXT_FILTER_PET_MESSAGES,
	CONTEXT_FILTER_PET_RAMPAGE_FLURRY,
	CONTEXT_FILTER_PET_CRITICALS,
	CONTEXT_FILTER_DAMAGE_SHIELDS_YOU_ATTACKING,
	CONTEXT_FILTER_EXPERIENCE_MESSAGES,
	CONTEXT_FILTER_NPC_EMOTES,
	CONTEXT_FILTER_SYSTEM_MESSAGES,
	CONTEXT_FILTER_WHO,
	CONTEXT_FILTER_PET_SPELLS,
	CONTEXT_FILTER_PET_RESPONSES,
	CONTEXT_FILTER_ITEM_SPEECH,
	CONTEXT_FILTER_FELLOWSHIP_MESSAGES,
	CONTEXT_FILTER_MERCENARY_MESSAGES,
    CONTEXT_FILTER_PVP_MESSAGES,
    CONTEXT_FILTER_MELEE_YOUR_FLURRY,
	CONTEXT_FILTER_DEBUG,                        // todo: check this not 100% sure its it...
	CONTEXT_FILTER_MELEE_NPC_DEATH	,
    CONTEXT_FILTER_RANDOM_OTHERS_ROLLS,
    CONTEXT_FILTER_RANDOM_GROUP_RAID_ROLLS,
	CONTEXT_FILTER_ENVIRONMENTAL_DAMAGE_YOURS,
	CONTEXT_FILTER_ENVIRONMENTAL_DAMAGE_OTHERS,
	CONTEXT_FILTER_DAMAGE_SHIELDS_YOU_DEFENDING,
	CONTEXT_FILTER_DAMAGE_SHIELDS_OTHERS,
	CONTEXT_FILTER_EVENT_MESSAGES,
	CONTEXT_FILTER_OVERWRITTEN_DETRIMENTAL_SPELL_MESSAGES,
	CONTEXT_FILTER_OVERWRITTEN_BENEFICIAL_SPELL_MESSAGES,
	CONTEXT_FILTER_YOU_CANT_USE_THAT_COMMAND,     // Added chat color and filtering options for 'You can't use that command' messages.
	CONTEXT_FILTER_COMBAT_ABILITY_REUSE,          // Added chat color and filtering options for combat ability and AA ability reuse time messages.
	CONTEXT_FILTER_SPELLS_AA_ABILITY_REUSE,
	CONTEXT_FILTER_ITEM_DESTROYED,
	CONTEXT_FILTER_SPELLS_AURAS_YOU,
	CONTEXT_FILTER_SPELLS_AURAS_OTHERS,
	CONTEXT_FILTER_SPELLS_HEALS_YOURS,
	CONTEXT_FILTER_SPELLS_HEALS_OTHERS,
	CONTEXT_FILTER_SPELLS_DOTS_YOURS,
	CONTEXT_FILTER_SPELLS_DOTS_OTHERS,
	CONTEXT_FILTER_SPELLS_SONGS,
	CONTEXT_FILTER_SPELLS_DD_OTHERS,

	CONTEXT_FILTER_ALL_NORMAL,
	CONTEXT_FILTER_ALL_MELEE,
	CONTEXT_FILTER_ALL_SPELL,
	CONTEXT_FILTER_ALL_CHANNEL,
    CONTEXT_FILTER_ALL_RANDOM,
	CONTEXT_FILTER_ALL_DAMAGE_SHIELDS,
	CONTEXT_FILTER_ALL_ENVIRONMENTAL_DMG,

	// new timestamp menu ids goes here
	// todo check the ids.
};

// Actual Size 0x98  03/15/06
struct EQ_CONTAINERWND_MANAGER
{
/*0x000*/ DWORD       pvfTable;                            // NOT based on CXWnd. Contains only destructor
/*0x004*/ EQCONTAINERWINDOW* pPCContainers[0x22];          // All open containers, including World, in order of opening...
/*0x08c*/ CONTENTS*   pWorldContents;                      // Pointer to the contents of the world If NULL, world container isn't open;
/*0x090*/ DWORD       dwWorldContainerID;                  // ID of container in zone, starts at one (zero?) and goes up.
/*0x094*/ DWORD       dwTimeSpentWithWorldContainerOpen;   // Cumulative counter?
/*0x078*/
};
using PEQ_CONTAINERWND_MANAGER = EQ_CONTAINERWND_MANAGER*;



struct LOOTCORPSE
{
/*0x000*/ BYTE         Unknown0x000;             // 03 seems very common (for NPC anyway)
/*0x001*/ CHAR         Name[0x4b];
/*0x04c*/ FLOAT        Timer;                    // Uncertain the equation used here, but fairly certain it's the timer
/*0x050*/ FLOAT        X;
/*0x054*/ BYTE         Unknown0x054[0x4];
/*0x058*/ FLOAT        Y;
/*0x05c*/ FLOAT        Z;
/*0x060*/ BYTE         Unknown0x060[0x24];
/*0x084*/ DWORD        Flags;
/*0x088*/ BYTE         Unknown0x088[0x4];
/*0x08c*/ DWORD        Unknown0x08c;             // 256 seems very common
/*0x090*/
};
using PLOOTCORPSE = LOOTCORPSE*;


// size 0x28 4-24-2004 Lax
struct EQGRAPHICSENGINE
{
/*0x00*/ // Graphics file handler
/*0x04*/
};
using PEQGRAPHICSENGINE = EQGRAPHICSENGINE*;

//.text:005FC640 ; public: __thiscall CTargetIndicator::CTargetIndicator(void)
// size 0x48 see 4BA434 in Oct 26 2015 -eqmule
struct TARGETINDICATOR
{
/*0x00*/ bool          bVisible;
/*0x01*/ bool          bSettingsLoaded;
/*0x04*/ SPAWNINFO*    lasttarget;
/*0x04*/ SPAWNINFO*    MarkedTarget[4];          // Assist,FirstMarked,LastMarked
/*0x18*/ BYTE          IndicatorHidden;          // if this is 1 our mouse is over another window and not on the main one - shouldnt cast when this is 1
/*0x19*/ BYTE          CanActivate;              // if 0 the indicator is red if 1 its green, i.e. it can be activated.
/*0x1a*/ BYTE          Unknown0x1a[0x2];
/*0x1c*/ DWORD         TargetIndicatorSettings;  // CTargetIndicatorSettings class todo: figure it out
/*0x20*/ DWORD         MarkerIndicatorSettings;
/*0x24*/ ETargetType   IndicatorType;            // this is reliable IF you actually have a target
/*0x28*/ float         SegmentLength;
/*0x2c*/ float         ControlSegmentLength;
/*0x30*/ void*         pTargetThickLine;         // CThickLineInterface
/*0x34*/ void*         pFreeTargetThickLine;     // CThickLineInterface
/*0x38*/ void*         pMarkerThickLine[4];      // CThickLineInterface
/*0x48*/
};
using PTARGETINDICATOR = TARGETINDICATOR*;

struct TARGETDATA
{
/*0x00*/ SPAWNINFO*    pPlayer;
/*0x04*/ float         Dist;
/*0x08*/ bool          bUpdated;                 // not sure what this is for tbh
/*0x0c*/ 
};
using PTARGETDATA = TARGETDATA*;

// CTargetManager
// size 0x1a4 see 5FE489 in Oct 26 2015 -eqmule
struct TARGETMANAGER
{
/*0x000*/ TARGETDATA   CycleNPCList[0xa];        // size 0xc*0xa
/*0x078*/ TARGETDATA   CyclePCList[0xa];
/*0x0F0*/ TARGETDATA   CycleCorpseList[0xa];
/*0x168*/ DWORD        LastTargetID;
/*0x16c*/ DWORD        TabTargetID;
/*0x170*/ DWORD        AttackTargetID;
/*0x174*/ DWORD        LastHoverUpdate;
/*0x178*/ DWORD        HoverUpdateInterval;
/*0x17c*/ DWORD        LastFreeTargetUpdate;
/*0x180*/ DWORD        FreeTargetUpdateInterval;
/*0x184*/ DWORD        PreviousTargetID;
/*0x188*/ DWORD        HoverTargetID;            // id of the entity our cursor is over...
/*0x18c*/ bool         bFreeTargetingEnabled;
/*0x190*/ float        target_ring_range;        // squared for easier math
/*0x194*/ float        target_ring_x;            // CVector3 class, this is whats passed to castspell
/*0x198*/ float        target_ring_y;
/*0x19c*/ float        target_ring_z;
/*0x1a0*/ bool         target_ring_good;         // 0=red 1=green 
/*0x1a4*/
};
using PTARGETMANAGER = TARGETMANAGER*;

struct ScreenVector3
{
	union {
		float x; // left to right screen coordinate
		DWORD dx;
	};
	union {
		float y; // top to bottom screen coordinate
		DWORD dy;
	};
	union {
		float z;
		DWORD dz;
	};
};
using PScreenVector3 = ScreenVector3*;

} // namespace eqlib

using namespace eqlib;