/*
 * Sun++ Scripts for Sandshroud MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2007-2008 Moon++ Team <http://www.moonplusplus.info/>
 * Copyright (C) 2008-2009 Sun++ Team <http://www.sunscripting.com/>
 * Copyright (C) 2009-2011 Sandshroud <http://www.sandshroud.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptStdAfx.h"
#include "Setup.h"
#define SKIP_ALLOCATOR_SHARING 1
#include <ScriptSetup.h>

extern "C" SCRIPT_DECL uint32 _exp_get_script_type()
{
	return SCRIPT_TYPE_MISC;
}

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* mgr)	// Comment any script to disable it
{
	SetupNeutralGuards(mgr);
	SetupGoHandlers(mgr);
	SetupQDGoHandlers(mgr);
	SetupRandomScripts(mgr);
	SetupMiscCreatures(mgr);
	SetupDarkmoonFaireObjects(mgr);
	SetupWinterVeil(mgr);
	SetupHalloween(mgr);
	#if _MSC_VER <= 1500
	SetupBattlemaster(mgr);
	SetupBoreanTundraGossip(mgr);
	SetupDalaranGossip(mgr);
	SetupDarkmoonFaireBarker(mgr);
	SetupGuardGossip(mgr);
	SetupInnkeepers(mgr);
	SetupMoongladeGossip(mgr);
	SetupShattrathGossip(mgr);
	SetupStormwindGossip(mgr);
	SetupTanarisGossip(mgr);
	SetupTeldrassilGossip(mgr);
	SetupTheramoreGossip(mgr);
	SetupTrainerScript(mgr);
	SetupWyrmrestTempleGossip(mgr);
	//SetupEventDarkPortal(mgr);
	#endif
}

#ifdef WIN32

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    return TRUE;
}

#endif