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

#include "StdAfx.h"
#include "../Setup.h"

// Ulduar Teleporter
class SCRIPT_DECL UlduarTeleporter : public GossipScript
{
public:
	void GossipHello(Object *pObject, Player * Plr, bool AutoSend);
	void GossipSelectOption(Object *pObject, Player *Plr, uint32 Id, uint32 IntId, const char * Code);

	void GossipEnd(Object * pObject, Player *Plr)
	{
		GossipScript::GossipEnd(pObject, Plr);
	};
	void Destroy()
	{
		delete this;
	};
};

void SetupUlduar(ScriptMgr* mgr);
