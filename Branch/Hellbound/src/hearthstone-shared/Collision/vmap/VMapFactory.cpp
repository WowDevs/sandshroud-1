/*
 * Sandshroud Hearthstone
 * Copyright (C) 2005 - 2010 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2010 - 2011 Sandshroud <http://www.sandshroud.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "SharedStdAfx.h"
#include <sys/types.h>
#include "VMapFactory.h"
#include "VMapManager2.h"

using namespace G3D;

namespace VMAP
{
	void chompAndTrim(std::string& str)
	{
		while(str.length() >0)
		{
			char lc = str[str.length()-1];
			if(lc == '\r' || lc == '\n' || lc == ' ' || lc == '"' || lc == '\'')
			{
				str = str.substr(0,str.length()-1);
			}
			else
			{
				break;
			}
		}
		while(str.length() >0)
		{
			char lc = str[0];
			if(lc == ' ' || lc == '"' || lc == '\'')
			{
				str = str.substr(1,str.length()-1);
			}
			else
			{
				break;
			}
		}
	}

	VMapManager2 *gVMapManager = 0;
	Table<unsigned int , bool>* iIgnoreSpellIds=0;

	//===============================================
	// result false, if no more id are found

	bool getNextId(const std::string& pString, unsigned int& pStartPos, unsigned int& pId)
	{
		bool result = false;
		unsigned int i;
		for(i=pStartPos;i<pString.size(); ++i)
		{
			if(pString[i] == ',')
			{
				break;
			}
		}
		if(i>pStartPos)
		{
			std::string idString = pString.substr(pStartPos, i-pStartPos);
			pStartPos = i+1;
			chompAndTrim(idString);
			pId = atoi(idString.c_str());
			result = true;
		}
		return(result);
	}

	//===============================================
	/**
	parameter: String of map ids. Delimiter = ","
	*/

	void VMapFactory::preventSpellsFromBeingTestedForLoS(const char* pSpellIdString)
	{
		if(!iIgnoreSpellIds)
			iIgnoreSpellIds = new Table<unsigned int , bool>();
		if(pSpellIdString != NULL)
		{
			unsigned int pos =0;
			unsigned int id;
			std::string confString(pSpellIdString);
			chompAndTrim(confString);
			while(getNextId(confString, pos, id))
			{
				iIgnoreSpellIds->set(id, true);
			}
		}
	}

	//===============================================

	bool VMapFactory::checkSpellForLoS(unsigned int pSpellId)
	{
		return(!iIgnoreSpellIds->containsKey(pSpellId));
	}

	//===============================================
	// just return the instance
	VMapManager2* VMapFactory::createOrGetVMapManager()
	{
		if(gVMapManager == 0)
			gVMapManager = new VMapManager2();				// should be taken from config ... Please change if you like :-)
		return gVMapManager;
	}

	//===============================================
	// delete all internal data structures
	void VMapFactory::clear()
	{
		if(iIgnoreSpellIds)
		{
			delete iIgnoreSpellIds;
			iIgnoreSpellIds = NULL;
		}
		if(gVMapManager)
		{
			delete gVMapManager;
			gVMapManager = NULL;
		}
	}
}
