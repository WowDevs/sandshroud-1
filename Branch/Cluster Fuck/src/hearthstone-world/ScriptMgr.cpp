/*
 * Sandshroud Hearthstone
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

#include "StdAfx.h"

#define SCRIPTLIB_HIPART(x) ((x >> 16))
#define SCRIPTLIB_LOPART(x) ((x & 0x0000ffff))
#define SCRIPTLIB_VERSION_MINOR (BUILD_REVISION % 1000)
#define SCRIPTLIB_VERSION_MAJOR (BUILD_REVISION / 1000)

initialiseSingleton(ScriptMgr);
initialiseSingleton(HookInterface);

ScriptMgr::ScriptMgr()
{
	DefaultGossipScript = new GossipScript();
}

ScriptMgr::~ScriptMgr()
{

}

struct ScriptingEngine
{
#ifdef WIN32
	HMODULE Handle;
#else
	void* Handle;
#endif
	exp_script_register InitializeCall;
	uint32 Type;
};

void ScriptMgr::LoadScripts()
{
	if(!HookInterface::getSingletonPtr())
		new HookInterface;

	Log.Notice("ScriptMgr","Loading External Script Libraries..." );
	string start_path = string(Config.MainConfig.GetStringDefault( "Script", "BinaryLocation", "script_bin" ).c_str()) + "\\";
	string search_path = start_path + "*.";

	vector< ScriptingEngine > ScriptEngines;

	/* Loading system for win32 */
#ifdef WIN32
	search_path += "dll";

	WIN32_FIND_DATA data;
	uint32 count = 0;
	HANDLE find_handle = FindFirstFile( search_path.c_str(), &data );
	if(find_handle == INVALID_HANDLE_VALUE)
		Log.Notice("ScriptMgr","No external scripts found." );
	else
	{
		do
		{
			string full_path = start_path + data.cFileName;
			HMODULE mod = LoadLibrary( full_path.c_str() );
			if( mod == 0 )
				Log.Error("ScriptMgr","Loading %s failed, crc=0x%p", data.cFileName, reinterpret_cast< uint32* >( mod ));
			else
			{
				if(HEARTHSTONE_TOLOWER_RETURN(data.cFileName) == "lacrimi.dll")
				{
					Log.Error("ScriptMgr", "Lacrimi must be loaded from world directory.");
					FreeLibrary(mod);
					continue;
				}

				// find version import
				exp_get_version vcall = (exp_get_version)GetProcAddress(mod, "_exp_get_version");
				exp_script_register rcall = (exp_script_register)GetProcAddress(mod, "_exp_script_register");
				exp_get_script_type scall = (exp_get_script_type)GetProcAddress(mod, "_exp_get_script_type");
				if(vcall == 0 || rcall == 0 || scall == 0)
				{
					Log.Error("ScriptMgr","Loading %s failed, version info not found", data.cFileName);
					FreeLibrary(mod);
				}
				else
				{
					uint32 version = vcall();
					uint32 stype = scall();
					if(SCRIPTLIB_LOPART(version) == SCRIPTLIB_VERSION_MINOR && SCRIPTLIB_HIPART(version) == SCRIPTLIB_VERSION_MAJOR)
					{
						std::stringstream cmsg;
						cmsg << "Loading " << data.cFileName << ", crc:0x" << reinterpret_cast< uint32* >( mod );

						if( stype & SCRIPT_TYPE_SCRIPT_ENGINE )
						{
							cmsg << ", Version:" << SCRIPTLIB_HIPART(version) << SCRIPTLIB_LOPART(version) << " delayed loading.";

							ScriptingEngine se;
							se.Handle = mod;
							se.InitializeCall = rcall;
							se.Type = stype;

							ScriptEngines.push_back( se );
						}
						else
						{
							_handles.push_back(((SCRIPT_MODULE)mod));
							cmsg << ", Version:" << SCRIPTLIB_HIPART(version) << SCRIPTLIB_LOPART(version);
							rcall(this);
						}
						Log.Success("ScriptMgr",cmsg.str().c_str());
						++count;
					}
					else
					{
						Log.Error("ScriptMgr","Loading %s failed, version mismatch", data.cFileName);
						FreeLibrary(mod);
					}
				}
			}
		}
		while(FindNextFile(find_handle, &data));
		FindClose(find_handle);
		Log.Notice("ScriptMgr","Loaded %u external libraries.", count);
		Log.Notice("ScriptMgr","Loading optional scripting engines...");
		for(vector<ScriptingEngine>::iterator itr = ScriptEngines.begin(); itr != ScriptEngines.end(); itr++)
		{
			if( itr->Type & SCRIPT_TYPE_SCRIPT_ENGINE_LUA )
			{
				// lua :O
				if( Config.MainConfig.GetBoolDefault("ScriptBackends", "LUA", false) )
				{
					Log.Notice("ScriptMgr","Initializing LUA script engine...");
					itr->InitializeCall(this);
					_handles.push_back( (SCRIPT_MODULE)itr->Handle );
				}
				else
				{
					FreeLibrary( itr->Handle );
				}
			}
			else if( itr->Type & SCRIPT_TYPE_SCRIPT_ENGINE_GM )
			{
				if( Config.MainConfig.GetBoolDefault("ScriptBackends", "GM", false) )
				{
					Log.Notice("ScriptMgr","Initializing GameMonkey script engine...");
					itr->InitializeCall(this);
					_handles.push_back( (SCRIPT_MODULE)itr->Handle );
				}
				else
				{
					FreeLibrary( (*itr).Handle );
				}
			}
			else
			{
				Log.Error("ScriptMgr","Unknown script engine type: 0x%.2X, please contact developers.", (*itr).Type );
				FreeLibrary( itr->Handle );
			}
		}
		Log.Success("ScriptMgr","Done loading script engines...");
	}
#else
	/* Loading system for *nix */
	struct dirent ** list;
	int filecount = scandir(PREFIX "/lib/", &list, 0, 0);
	uint32 count = 0;

	if(!filecount || !list || filecount < 0)
		printf("  No external scripts found.");
	else
	{
char *ext;
		while(filecount--)
		{
			ext = strrchr(list[filecount]->d_name, '.');
#ifdef HAVE_DARWIN
			if (ext != NULL && strstr(list[filecount]->d_name, ".0.dylib") == NULL && !strcmp(ext, ".dylib")) {
#else
			if (ext != NULL && !strcmp(ext, ".so")) {
#endif
				string full_path = "../lib/" + string(list[filecount]->d_name);
				SCRIPT_MODULE mod = dlopen(full_path.c_str(), RTLD_NOW);
				printf("  %s : 0x%p : ", list[filecount]->d_name, mod);
				if(mod == 0)
					printf("error! [%s]\n", dlerror());
				else
				{
					if(HEARTHSTONE_TOLOWER_RETURN(list[filecount]->d_name) == "lacrimi.dll")
					{
						printf("Lacrimi must be loaded from world directory.");
						free(list[filecount]);
						dlclose(mod);
						continue;
					}

					// find version import
					exp_get_version vcall = (exp_get_version)dlsym(mod, "_exp_get_version");
					exp_script_register rcall = (exp_script_register)dlsym(mod, "_exp_script_register");
					exp_get_script_type scall = (exp_get_script_type)dlsym(mod, "_exp_get_script_type");
					if(vcall == 0 || rcall == 0 || scall == 0)
					{
						printf("version functions not found!\n");
						dlclose(mod);
					}
					else
					{
						int32 version = vcall();
						uint32 stype = scall();
						if(SCRIPTLIB_LOPART(version) == SCRIPTLIB_VERSION_MINOR && SCRIPTLIB_HIPART(version) == SCRIPTLIB_VERSION_MAJOR)
						{
							if( stype & SCRIPT_TYPE_SCRIPT_ENGINE )
							{
								printf("v%u.%u : ", SCRIPTLIB_HIPART(version), SCRIPTLIB_LOPART(version));
								printf("delayed load.\n");

								ScriptingEngine se;
								se.Handle = mod;
								se.InitializeCall = rcall;
								se.Type = stype;

								ScriptEngines.push_back( se );
							}
							else
							{
								_handles.push_back(((SCRIPT_MODULE)mod));
								printf("v%u.%u : ", SCRIPTLIB_HIPART(version), SCRIPTLIB_LOPART(version));
								rcall(this);
								printf("loaded.\n");
							}

							++count;
						}
						else
						{
							dlclose(mod);
							printf("version mismatch!\n");
						}
					}
				}
			}
			free(list[filecount]);
		}
		free(list);
		sLog.outString("");
		sLog.outString("Loaded %u external libraries.", count);
		sLog.outString("");

		sLog.outString("Loading optional scripting engines...");
		for(vector<ScriptingEngine>::iterator itr = ScriptEngines.begin(); itr != ScriptEngines.end(); itr++)
		{
			if( itr->Type & SCRIPT_TYPE_SCRIPT_ENGINE_LUA )
			{
				// lua :O
				if( Config.MainConfig.GetBoolDefault("ScriptBackends", "LUA", false) )
				{
					sLog.outString("   Initializing LUA script engine...");
					itr->InitializeCall(this);
					_handles.push_back( (SCRIPT_MODULE)itr->Handle );
				}
				else
				{
					dlclose( itr->Handle );
				}
			}
			else
			{
				sLog.outString("  Unknown script engine type: 0x%.2X, please contact developers.", (*itr).Type );
				dlclose( itr->Handle );
			}
		}
		Log.Success("ScriptMgr","Done loading script engines...");
	}
#endif

#ifdef WIN32 // Win

	HMODULE mod = LoadLibrary("Lacrimi.dll");
	if( mod != 0 )
	{
		Log.Notice("ScriptMgr","Initializing Lacrimi...");
		// find version import
		exp_get_version vcall = (exp_get_version)GetProcAddress(mod, "_exp_get_version");
		exp_script_register rcall = (exp_script_register)GetProcAddress(mod, "_exp_script_register");
		exp_get_script_type scall = (exp_get_script_type)GetProcAddress(mod, "_exp_get_script_type");
		if(vcall == 0 || rcall == 0 || scall == 0)
		{
			Log.Error("ScriptMgr","Lacrimi loading failed, version info not found");
			FreeLibrary(mod);
		}
		else
		{
			uint32 version = vcall();
			if(SCRIPTLIB_LOPART(version) == SCRIPTLIB_VERSION_MINOR && SCRIPTLIB_HIPART(version) == SCRIPTLIB_VERSION_MAJOR)
			{
				_handles.push_back(((SCRIPT_MODULE)mod));
				rcall(this);
			}
			else
			{
				Log.Error("ScriptMgr","Lacrimi loading failed, version mismatch");
				FreeLibrary(mod);
			}
		}
	}

#else // Nux

#ifdef LOAD_LACRIMI

	SCRIPT_MODULE mod = dlopen("Lacrimi.dll", RTLD_NOW);
	if(mod != 0)
	{
		// find version import
		exp_get_version vcall = (exp_get_version)dlsym(mod, "_exp_get_version");
		exp_script_register rcall = (exp_script_register)dlsym(mod, "_exp_script_register");
		exp_get_script_type scall = (exp_get_script_type)dlsym(mod, "_exp_get_script_type");
		if(vcall == 0 || rcall == 0 || scall == 0)
		{
			printf("version functions not found!\n");
			dlclose(mod);
		}
		else
		{
			int32 version = vcall();
			uint32 stype = scall();
			if(SCRIPTLIB_LOPART(version) == SCRIPTLIB_VERSION_MINOR && SCRIPTLIB_HIPART(version) == SCRIPTLIB_VERSION_MAJOR)
			{
				_handles.push_back(((SCRIPT_MODULE)mod));
				printf("v%u.%u : ", SCRIPTLIB_HIPART(version), SCRIPTLIB_LOPART(version));
				rcall(this);
				printf("loaded.\n");
			}
			else
			{
				dlclose(mod);
				printf("version mismatch!\n");
			}
		}
	}

#endif // LOAD_LACRIMI

#endif // Win/Nux

	while(sWorld.LacrimiLoading)
		Sleep(100);
}

void ScriptMgr::UnloadScripts(bool safe)
{
	if(HookInterface::getSingletonPtr())
		delete HookInterface::getSingletonPtr();

	QuestScript* script = NULL;
	for(QuestScriptMap::iterator qitr = EntryQuestScriptMap.begin(); qitr != EntryQuestScriptMap.end(); qitr++)
	{
		script = qitr->second;
		EntryQuestScriptMap.erase(qitr->first);
		delete script;
		script = NULL;
	}
	EntryQuestScriptMap.clear();

	for(uint8 type = 0; type < 3; type++)
		GossipMaps[type].clear(); // Do not delete, just clean em out, they will be deleted by customgossip map
	for(CustomGossipScripts::iterator itr = _customgossipscripts.begin(); itr != _customgossipscripts.end(); itr++)
		(*itr)->Destroy();
	_customgossipscripts.clear();
	if(safe == false)
	{
		delete DefaultGossipScript;
		DefaultGossipScript = NULL;
	}

	LibraryHandleMap::iterator itr = _handles.begin();
	for(; itr != _handles.end(); itr++)
	{
#ifdef WIN32
		FreeLibrary(((HMODULE)*itr));
#else
		dlclose(*itr);
#endif
	}
	_handles.clear();
}

void ScriptMgr::ReloadScripts()
{
	UnloadScripts(true);
	LoadScripts();
}

void ScriptMgr::register_creature_script(uint32 entry, exp_create_creature_ai callback)
{
	CreatureProto* cn = CreatureProtoStorage.LookupEntry(entry);
	if( cn )
	{
		for(uint8 i = 0; i < 4; i++)
		{
			if( !cn->spells[i].empty() )
			{
				for(list<AI_Spell*>::iterator it = cn->spells[i].begin(); it != cn->spells[i].end(); it++)
					delete (*it);

				cn->spells[i].clear();
			}
		}
	}
	_creatures.insert( CreatureCreateMap::value_type( entry, callback ) );
}

void ScriptMgr::register_gameobject_script(uint32 entry, exp_create_gameobject_ai callback)
{
	_gameobjects.insert( GameObjectCreateMap::value_type( entry, callback ) );
}

void ScriptMgr::register_dummy_aura(uint32 entry, exp_handle_dummy_aura callback)
{
	_auras.insert( HandleDummyAuraMap::value_type( entry, callback ) );
}

void ScriptMgr::register_dummy_spell(uint32 entry, exp_handle_dummy_spell callback)
{
	_spells.insert( HandleDummySpellMap::value_type( entry, callback ) );
}

void ScriptMgr::register_gossip_script(uint32 entry, GossipScript * gs)
{
	CreatureInfo * ci = CreatureNameStorage.LookupEntry(entry);
	if(ci == NULL)
		return;

	CreatureProto * cp = CreatureProtoStorage.LookupEntry(entry);
	if(cp == NULL)
		return;

	cp->TeleportInfoList.clear();
	GossipMaps[GTYPEID_CTR][entry] = gs;
	if(_customgossipscripts.find(gs) == _customgossipscripts.end())
		_customgossipscripts.insert(gs);
}

void ScriptMgr::register_item_gossip_script(uint32 entry, GossipScript * gs)
{
	ItemPrototype * proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
		return;

	GossipMaps[GTYPEID_ITEM][entry] = gs;
	if(_customgossipscripts.find(gs) == _customgossipscripts.end())
		_customgossipscripts.insert(gs);
}

void ScriptMgr::register_go_gossip_script(uint32 entry, GossipScript * gs)
{
	GameObjectInfo * gi = GameObjectNameStorage.LookupEntry(entry);
	if(gi == NULL)
		return;

	GossipMaps[GTYPEID_GAMEOBJECT][entry] = gs;
	if(_customgossipscripts.find(gs) == _customgossipscripts.end())
		_customgossipscripts.insert(gs);
}

void ScriptMgr::register_quest_script(uint32 entry, QuestScript * qs)
{
	EntryQuestScriptMap[entry] = qs;
	_questscripts.insert( qs );
}

void ScriptMgr::register_instance_script( uint32 pMapId, exp_create_instance_ai pCallback )
{
	mInstances.insert( InstanceCreateMap::value_type( pMapId, pCallback ) );
};

CreatureAIScript* ScriptMgr::CreateAIScriptClassForEntry(Creature* pCreature)
{
	CreatureCreateMap::iterator itr = _creatures.find(pCreature->GetEntry());
	if(itr == _creatures.end())
		return NULL;

	exp_create_creature_ai function_ptr = itr->second;
	return (function_ptr)(pCreature);
}

GameObjectAIScript * ScriptMgr::CreateAIScriptClassForGameObject(uint32 uEntryId, GameObject* pGameObject)
{
	GameObjectCreateMap::iterator itr = _gameobjects.find(pGameObject->GetEntry());
	if(itr == _gameobjects.end())
		return NULL;

	exp_create_gameobject_ai function_ptr = itr->second;
	return (function_ptr)(pGameObject);
}

InstanceScript* ScriptMgr::CreateScriptClassForInstance( uint32 pMapId, MapMgr* pMapMgr )
{
	InstanceCreateMap::iterator Iter = mInstances.find( pMapMgr->GetMapId() );
	if ( Iter == mInstances.end() )
		return NULL;

	exp_create_instance_ai function_ptr = Iter->second;
	return ( function_ptr )( pMapMgr );
};

bool ScriptMgr::CallScriptedDummySpell(uint32 uSpellId, uint32 i, Spell* pSpell)
{
	HandleDummySpellMap::iterator itr = _spells.find(uSpellId);
	if(itr == _spells.end())
		return false;

	exp_handle_dummy_spell function_ptr = itr->second;
	return (function_ptr)(i, pSpell);
}

bool ScriptMgr::CallScriptedDummyAura(uint32 uSpellId, uint32 i, Aura* pAura, bool apply)
{
	HandleDummyAuraMap::iterator itr = _auras.find(uSpellId);
	if(itr == _auras.end())
		return false;

	exp_handle_dummy_aura function_ptr = itr->second;
	return (function_ptr)(i, pAura, apply);
}

bool ScriptMgr::CallScriptedItem(Item* pItem, Player* pPlayer)
{
	GossipScript* script = NULL;
	if((script = GetRegisteredGossipScript(GTYPEID_ITEM, pItem->GetEntry(), false)) != NULL)
	{
		script->GossipHello(pItem,pPlayer,true);
		return true;
	}

	return false;
}

/* CreatureAI Stuff */
CreatureAIScript::CreatureAIScript(Creature* creature) : _unit(creature)
{

}

void CreatureAIScript::RegisterAIUpdateEvent(uint32 frequency)
{
	sEventMgr.AddEvent(_unit, &Creature::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, frequency, 0,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void CreatureAIScript::ModifyAIUpdateEvent(uint32 newfrequency)
{
	sEventMgr.ModifyEventTimeAndTimeLeft(_unit, EVENT_SCRIPT_UPDATE_EVENT, newfrequency);
}

void GameObjectAIScript::RemoveAIUpdateEvent()
{
	sEventMgr.RemoveEvents( _gameobject, EVENT_SCRIPT_UPDATE_EVENT );
}

void CreatureAIScript::RemoveAIUpdateEvent()
{
	sEventMgr.RemoveEvents(_unit, EVENT_SCRIPT_UPDATE_EVENT);
}

/* GameObjectAI Stuff */

GameObjectAIScript::GameObjectAIScript(GameObject* goinstance) : _gameobject(goinstance)
{

}

void GameObjectAIScript::RegisterAIUpdateEvent(uint32 frequency)
{
	sEventMgr.AddEvent(_gameobject, &GameObject::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, frequency, 0,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

/* QuestScript Stuff */

/* Gossip Stuff*/

GossipScript::GossipScript()
{

}

void GossipScript::GossipEnd(Object* pObject, Player* Plr)
{
	Plr->CleanupGossipMenu();
}

bool CanTrainAt(Player* plr, Trainer * trn);

void GossipScript::GossipHello(Object* pObject, Player* Plr, bool AutoSend)
{
	GossipMenu *Menu = NULL;
	uint32 TextID = 68; //Hi there how can I help you $N	Greetings $N

	switch(pObject->GetTypeId())
	{
	case TYPEID_UNIT:
		{
			Creature* pCreature = TO_CREATURE(pObject);
			if(!pCreature)
				return;

			Trainer *pTrainer = pCreature->GetTrainer();
			uint32 Text = objmgr.GetGossipTextForNpc(pCreature->GetEntry());
			if(Text != 0)
			{
				GossipText* text = NpcTextStorage.LookupEntry(Text);
				if(text != NULL)
					TextID = Text;
			}

			objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), TextID, Plr);
			std::set<TeleportInfo*> Telelist = pCreature->GetProto()->TeleportInfoList;
			if(Telelist.size())
			{
				std::set<TeleportInfo*>::iterator itr;
				for(itr = Telelist.begin(); itr != Telelist.end(); itr++)
					Menu->AddItem((*itr)->iconid, (*itr)->textinfo.c_str(), (*itr)->intid);
			}
			else // Use normal gossip functions
			{
				uint32 flags = pCreature->GetUInt32Value(UNIT_NPC_FLAGS);
				if( flags & UNIT_NPC_FLAG_VENDOR && !pCreature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE))
				{
					if(pTrainer != NULL)
					{
						if(CanTrainAt(Plr, pTrainer) || Plr->vendorpass_cheat)
							Menu->AddItem(GOSSIP_ICON_GOSSIP_VENDOR, "I would like to browse your goods", 1);
					}
					else
						Menu->AddItem(GOSSIP_ICON_GOSSIP_VENDOR, "I would like to browse your goods", 1);
				}

				if(pTrainer != NULL && (flags & UNIT_NPC_FLAG_TRAINER || flags & UNIT_NPC_FLAG_TRAINER_PROF))
				{
					string name = pCreature->GetCreatureInfo()->Name;
					string::size_type pos = name.find(" ");	  // only take first name
					if(pos != string::npos)
						name = name.substr(0, pos);

					if(!CanTrainAt(Plr, pTrainer))
					{
						if(pTrainer->Cannot_Train_GossipTextId)
						{
							//replace normal gossipid by Cannot_Train_GossipTextId.
							Menu->SetTextID(pTrainer->Cannot_Train_GossipTextId);
						}
					}
					else
					{
						if(pTrainer->Can_Train_Gossip_TextId)
						{
							//replace normal gossipid by Can_Train_GossipTextId.
							Menu->SetTextID(pTrainer->Can_Train_Gossip_TextId);
						}

						string msg = "I seek ";
						if(pTrainer->RequiredClass)
						{
							switch(Plr->getClass())
							{
							case WARRIOR:
								msg += "warrior ";
								break;
							case PALADIN:
								msg += "paladin ";
								break;
							case HUNTER:
								msg += "hunter ";
								break;
							case ROGUE:
								msg += "rogue ";
								break;
							case PRIEST:
								msg += "priest ";
								break;
							case DEATHKNIGHT:
								msg += "death knight ";
								break;
							case SHAMAN:
								msg += "shaman ";
								break;
							case MAGE:
								msg += "mage ";
								break;
							case WARLOCK:
								msg += "warlock ";
								break;
							case DRUID:
								msg += "druid ";
								break;
							}
						}
						msg += "training, ";
						msg += name;
						msg += ".";

						Menu->AddItem(GOSSIP_ICON_GOSSIP_TRAINER, msg.c_str(), 2);
					}
				}

				if(flags & UNIT_NPC_FLAG_TAXIVENDOR)
					Menu->AddItem(GOSSIP_ICON_GOSSIP_FLIGHT, "Give me a ride.", 3);

				if(flags & UNIT_NPC_FLAG_AUCTIONEER)
					Menu->AddItem(GOSSIP_ICON_GOSSIP_AUCTION, "I would like to make a bid.", 4);

				if(flags & UNIT_NPC_FLAG_INNKEEPER)
				{
					Menu->AddItem(GOSSIP_ICON_GOSSIP_ENGINEER2, "Make this inn your home.", 5);
					Menu->AddItem(GOSSIP_ICON_GOSSIP_NORMAL, "What can I do at an Inn.", 15);
					if(sWorld.HallowsEnd)
					{
						if(!Plr->HasAura(24755)) // Trick or Treat
							Menu->AddItem(0, "Trick or Treat!", 19);
					}
				}

				if(flags & UNIT_NPC_FLAG_BANKER)
					Menu->AddItem(GOSSIP_ICON_GOSSIP_COIN, "I would like to check my deposit box.", 6);

				if(flags & UNIT_NPC_FLAG_SPIRITHEALER || (pCreature->GetEntry() == 6491 || pCreature->GetEntry() == 32537))
					Menu->AddItem(GOSSIP_ICON_GOSSIP_NORMAL, "Bring me back to life.", 7);

				if(flags & UNIT_NPC_FLAG_ARENACHARTER)
					Menu->AddItem(GOSSIP_ICON_GOSSIP_ARENA, "How do I create a guild/arena team?", 8);

				if(flags & UNIT_NPC_FLAG_TABARDCHANGER)
					Menu->AddItem(GOSSIP_ICON_GOSSIP_TABARD, "I want to create a guild crest.", 9);

				if(flags & UNIT_NPC_FLAG_BATTLEFIELDPERSON)
				{
					if(pCreature->GetProto() && pCreature->GetProto()->BattleMasterType)
					{
						BattleMasterListEntry* Battlemaster = dbcBattleMasterList.LookupEntry(pCreature->GetProto()->BattleMasterType);
						if(Battlemaster != NULL)
						{	// Do people even use battlemasters anymore?
							if(Plr->getLevel() >= Battlemaster->minLevel)
								Menu->AddItem(GOSSIP_ICON_GOSSIP_ARENA, "I would like to enter the battleground.", 10);
						}
					}
				}

				if( pTrainer && pTrainer->RequiredClass )
				{
					if( pTrainer->RequiredClass == Plr->getClass() && pCreature->getLevel() > 10 && Plr->getLevel() > 11 )
					{
						Menu->AddItem(GOSSIP_ICON_GOSSIP_NORMAL, "I would like to reset my talents.", 11);
						if( Plr->getLevel() >= 40 && Plr->m_talentSpecsCount < 2)
							Menu->AddItem(GOSSIP_ICON_GOSSIP_NORMAL, "Learn about Dual Talent Specialization.", 16);
					}
				}

				if( pTrainer &&
						pTrainer->TrainerType == TRAINER_TYPE_PET &&	// pet trainer type
						Plr->getClass() == HUNTER &&					// hunter class
						Plr->GetSummon() != NULL )						// have pet
					Menu->AddItem(GOSSIP_ICON_GOSSIP_NORMAL, "I would like to untrain my pet.", 13);

				if( pCreature->GetEntry() == 35364 || pCreature->GetEntry() == 35365 )
				{
					if(Plr->getLevel() > 9 && Plr->getLevel() < sWorld.GetMaxLevel(Plr))
					{
						if(Plr->m_XPoff)
						{
							Menu->AddItem(GOSSIP_ICON_GOSSIP_NORMAL, "I wish to start gaining experience again.",
								18, false, 100000, "Are you certain you wish to start gaining experience?" );
						}
						else
						{
							Menu->AddItem(GOSSIP_ICON_GOSSIP_NORMAL, "I no longer wish to gain experience.", 18,
								false, 100000, "Are you certain you wish to stop gaining experience?" );
						}
					}
				}
			}
		}break;
	case TYPEID_ITEM:
	case TYPEID_GAMEOBJECT:
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), TextID, Plr);
		}break;
	}

	if(Menu != NULL && AutoSend)
		Menu->SendTo(Plr);
}

void GossipScript::GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char * EnteredCode)
{
	if( pObject->GetTypeId() != TYPEID_UNIT )
		return;

	Creature* pCreature = TO_CREATURE( pObject );
	std::set<TeleportInfo*> Telelist = pCreature->GetProto()->TeleportInfoList;

	if(Telelist.begin() != Telelist.end())
	{
		std::set<TeleportInfo*>::iterator itr;
		for(itr = Telelist.begin(); itr != Telelist.end(); itr++)
		{
			TeleportInfo* teleinfo = (*itr);
			if(teleinfo->intid == IntId)
			{
				if(teleinfo->teleport)
					Plr->SafeTeleport(teleinfo->teleportmapid, 0, teleinfo->teleportx, teleinfo->teleporty, teleinfo->teleportz, teleinfo->teleporto);

				SpellEntry* sp = NULL;
				if(teleinfo->castspellid && (sp = dbcSpell.LookupEntry(teleinfo->castspellid)))
				{	// Create the spell so we can get around forced targetting.
					Spell* spell = new Spell(pCreature, sp, true, NULLAURA);
					SpellCastTargets targets;
					targets.m_unitTarget = Plr->GetGUID();
					spell->prepare(&targets);
				}

				if(teleinfo->removetargetaura)
					Plr->RemoveAura(teleinfo->removetargetaura);

				if(teleinfo->sndchtmessage)
				{
					switch(teleinfo->messagetype)
					{
					case 0: // System Message
						{
							pCreature->SendChatMessageToPlayer(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, teleinfo->Message.c_str(), Plr);
						}break;

					case 1: // Creature Say
						{
							pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, Plr->GetTeam() ? LANG_ORCISH : LANG_COMMON, teleinfo->Message.c_str());
						}break;

					case 2: // Creature Yell
						{
							pCreature->SendChatMessage(CHAT_MSG_MONSTER_YELL, Plr->GetTeam() ? LANG_ORCISH : LANG_COMMON, teleinfo->Message.c_str());
						}break;

					case 3: // Creature Emote
						{
							pCreature->SendChatMessage(CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, teleinfo->Message.c_str());
						}break;
					}
				}
				break;
			}
		}
		Plr->Gossip_Complete();
	}
	else // Normal int selections
	{
		switch( IntId )
		{
		case 1:
			// vendor
			Plr->GetSession()->SendInventoryList(pCreature);
			break;
		case 2:
			// trainer
			Plr->GetSession()->SendTrainerList(pCreature);
			break;
		case 3:
			// taxi
			Plr->GetSession()->SendTaxiList(pCreature);
			break;
		case 4:
			// auction
			Plr->GetSession()->SendAuctionList(pCreature);
			break;
		case 5:
			// innkeeper
			Plr->GetSession()->SendInnkeeperBind(pCreature);
			break;
		case 6:
			// banker
			Plr->GetSession()->SendBankerList(pCreature);
			break;
		case 7:
			// spirit
			Plr->GetSession()->SendSpiritHealerRequest(pCreature);
			break;
		case 8:
			// petition
			Plr->GetSession()->SendCharterRequest(pCreature);
			break;
		case 9:
			// guild crest
			Plr->GetSession()->SendTabardHelp(pCreature);
			break;
		case 10:
			{
				// battlefield
				uint32 battlegroundid = pCreature->GetProto() ? pCreature->GetProto()->BattleMasterType : 0;
				if(battlegroundid)
					Plr->GetSession()->SendBattlegroundList(pCreature, battlegroundid);
			}break;
		case 11:
			// switch to talent reset message
			{
				GossipMenu *Menu;
				objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 5674, Plr);
				Menu->AddItem(GOSSIP_ICON_GOSSIP_ENGINEER2, "Yes, I understand, continue.", 12);
				Menu->AddItem(GOSSIP_ICON_GOSSIP_NORMAL, "No thanks.",99);
				Menu->SendTo(Plr);
			}break;
		case 12:
			// talent reset
			{
				Plr->Gossip_Complete();
				Plr->SendTalentResetConfirm();
			}break;
		case 13:
			// switch to untrain message
			{
				GossipMenu *Menu;
				objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 7722, Plr);
				Menu->AddItem(GOSSIP_ICON_GOSSIP_ENGINEER2, "Yes, please do.", 14);
				Menu->AddItem(GOSSIP_ICON_GOSSIP_NORMAL, "Not right now, thanks.",99);
				Menu->SendTo(Plr);
			}break;
		case 14:
			// untrain pet
			{
				Plr->Gossip_Complete();
				Plr->SendPetUntrainConfirm();
			}break;
		case 15:
			// switch Innkeeper help menu
			{
				Plr->bHasBindDialogOpen = false;
				GossipMenu *Menu;
				objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 1853, Plr);
				Menu->AddItem(GOSSIP_ICON_GOSSIP_ENGINEER2, "Make this inn your home", 5);
				Menu->AddItem(GOSSIP_ICON_GOSSIP_NORMAL, "Goodbye",99);
				Menu->SendTo(Plr);
			}break;
		case 16:
			{
				GossipMenu *Menu;
				objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 14391, Plr);
				Menu->AddItem(GOSSIP_ICON_GOSSIP_NORMAL, "Purchase a Dual Talent Specialization.", 17, false, 10000000, "Are you sure you would like to purchase your second talent specialization?" );
				Menu->SendTo(Plr);
			}break;
		case 17:
			{
				GossipMenu *Menu;
				objmgr.CreateGossipMenuForPlayer(&Menu, pCreature->GetGUID(), 14393, Plr);
				Menu->SendTo(Plr);
				Plr->CastSpell(Plr, 63624, true);
				Plr->GetAchievementInterface()->ForceEarnedAchievement(2716);
			}break;
		case 18: // Enable and disable XP.
			{
				Plr->Gossip_Complete();
				Plr->SendXPToggleConfirm();
			}break;
		case 19: // Trick or fucking treat.
			{
				if(!Plr->HasAura(24755))
				{
					pCreature->CastSpell(Plr, 24755, true);

					// either trick or treat, 50% chance
					if(rand()%2)
					{
						Plr->CastSpell(Plr, 24715, true);
					}
					else
					{
						int32 trickspell = 0;
						switch (rand()%9)
						{
						case 0:
							trickspell = 24753; // cannot cast, random 30sec
							break;

						case 1:
							trickspell = 24713; // lepper gnome costume
							break;

						case 2:
							{
								if(!Plr->getGender())
									trickspell = 24735; // male ghost costume
								else
									trickspell = 24736; // female ghostcostume
							}break;

						case 3:
							{
								if(!Plr->getGender())
									trickspell = 24711; // male ninja costume
								else
									trickspell = 24710; // female ninja costume
							}break;

						case 4:
							{
								if(!Plr->getGender())
									trickspell = 24708; // male pirate costume
								else
									trickspell = 24709; // female pirate costume
							}break;

						case 5:
							trickspell = 24723; // skeleton costume
							break;
						}
						if(trickspell)
							pCreature->CastSpell(Plr, trickspell, true);
					}
				}
				Plr->Gossip_Complete();
			}break;
		case 98:		// Resend original gossip.
			{
				Plr->Gossip_Complete(); // This should be used instead of GossipHello for quest givers.
				Plr->GetSession()->SendGossipForObject(pCreature);
			}break;
		case 99:		// Aborting current action
			{
				Plr->Gossip_Complete();
			}break;
		default:
			DEBUG_LOG("GossipSelectOption","Unknown menuitem %u on npc %u", IntId, pCreature->GetEntry());
			break;
		}
	}
}

void GossipScript::Destroy()
{
	delete this;
}

/* InstanceAI Stuff */

InstanceScript::InstanceScript( MapMgr* pMapMgr ) : mInstance( pMapMgr )
{
};

void InstanceScript::RegisterUpdateEvent( uint32 pFrequency )
{
	sEventMgr.AddEvent( mInstance, &MapMgr::CallScriptUpdate, EVENT_SCRIPT_UPDATE_EVENT, pFrequency, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );
};

void InstanceScript::ModifyUpdateEvent( uint32 pNewFrequency )
{
	sEventMgr.ModifyEventTimeAndTimeLeft( mInstance, EVENT_SCRIPT_UPDATE_EVENT, pNewFrequency );
};

void InstanceScript::RemoveUpdateEvent()
{
	sEventMgr.RemoveEvents( mInstance, EVENT_SCRIPT_UPDATE_EVENT );
};

/* Hook Stuff */


void ScriptMgr::register_hook(ServerHookEvents event, void * function_pointer)
{
	ASSERT(event < NUM_SERVER_HOOKS);
	_hooks[event].push_back(function_pointer);
}

/* Hook Implementations */
#define OUTER_LOOP_BEGIN(type, fptr_type) if(!sScriptMgr._hooks[type].size()) { \
	return; } \
	fptr_type call; \
	for(ServerHookList::iterator itr = sScriptMgr._hooks[type].begin(); itr != sScriptMgr._hooks[type].end(); itr++) { \
	call = ((fptr_type)*itr);

#define OUTER_LOOP_END }

#define OUTER_LOOP_BEGIN_COND(type, fptr_type) if(!sScriptMgr._hooks[type].size()) { \
	return true; } \
	fptr_type call; \
	bool ret_val = true; \
	for(ServerHookList::iterator itr = sScriptMgr._hooks[type].begin(); itr != sScriptMgr._hooks[type].end(); itr++) { \
		call = ((fptr_type)*itr);

#define OUTER_LOOP_END_COND } return ret_val;

bool HookInterface::OnNewCharacter(uint32 Race, uint32 Class, WorldSession * Session, const char * Name)
{
	OUTER_LOOP_BEGIN_COND(SERVER_HOOK_EVENT_ON_NEW_CHARACTER, tOnNewCharacter)
		ret_val = (call)(Race, Class, Session, Name);
	OUTER_LOOP_END_COND;
}

void HookInterface::OnKillPlayer(Player* pPlayer, Player* pVictim)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_KILL_PLAYER, tOnKillPlayer)
		(call)(pPlayer, pVictim);
	OUTER_LOOP_END;
}

void HookInterface::OnFirstEnterWorld(Player* pPlayer)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_FIRST_ENTER_WORLD, tOnFirstEnterWorld)
		(call)(pPlayer);
	OUTER_LOOP_END;
}

void HookInterface::OnCharacterCreate(Player* pPlayer)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_CHARACTER_CREATE, tOCharacterCreate)
		(call)(pPlayer);
	OUTER_LOOP_END;
}

void HookInterface::OnEnterWorld(Player* pPlayer)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_ENTER_WORLD, tOnEnterWorld)
		(call)(pPlayer);
	OUTER_LOOP_END;
}

void HookInterface::OnGuildCreate(Player* pLeader, Guild * pGuild)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_GUILD_CREATE, tOnGuildCreate)
		(call)(pLeader, pGuild);
	OUTER_LOOP_END;
}

void HookInterface::OnGuildJoin(Player* pPlayer, Guild * pGuild)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_GUILD_JOIN, tOnGuildJoin)
		(call)(pPlayer, pGuild);
	OUTER_LOOP_END;
}

void HookInterface::OnDeath(Player* pPlayer)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_DEATH, tOnDeath)
		(call)(pPlayer);
	OUTER_LOOP_END;
}

bool HookInterface::OnRepop(Player* pPlayer)
{
	OUTER_LOOP_BEGIN_COND(SERVER_HOOK_EVENT_ON_REPOP, tOnRepop)
		ret_val = (call)(pPlayer);
	OUTER_LOOP_END_COND;
}

void HookInterface::OnEmote(Player* pPlayer, uint32 Emote, Unit* pUnit)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_EMOTE, tOnEmote)
		(call)(pPlayer, Emote, pUnit);
	OUTER_LOOP_END;
}

void HookInterface::OnEnterCombat(Player* pPlayer, Unit* pTarget)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_ENTER_COMBAT, tOnEnterCombat)
		(call)(pPlayer, pTarget);
	OUTER_LOOP_END;
}

bool HookInterface::OnCastSpell(Player* pPlayer, SpellEntry* pSpell)
{
	OUTER_LOOP_BEGIN_COND(SERVER_HOOK_EVENT_ON_CAST_SPELL, tOnCastSpell)
		ret_val = (call)(pPlayer, pSpell);
	OUTER_LOOP_END_COND;
}

bool HookInterface::OnLogoutRequest(Player* pPlayer)
{
	OUTER_LOOP_BEGIN_COND(SERVER_HOOK_EVENT_ON_LOGOUT_REQUEST, tOnLogoutRequest)
		ret_val = (call)(pPlayer);
	OUTER_LOOP_END_COND;
}

void HookInterface::OnLogout(Player* pPlayer)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_LOGOUT, tOnLogout)
		(call)(pPlayer);
	OUTER_LOOP_END;
}

void HookInterface::OnQuestAccept(Player* pPlayer, Quest * pQuest, Object* pObject)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_QUEST_ACCEPT, tOnQuestAccept)
		(call)(pPlayer, pQuest, pObject);
	OUTER_LOOP_END;
}

void HookInterface::OnZone(Player* pPlayer, uint32 Zone, uint32 OldZone)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_ZONE, tOnZone)
		(call)(pPlayer, Zone, OldZone);
	OUTER_LOOP_END;
}

bool HookInterface::OnChat(Player* pPlayer, uint32 Type, uint32 Lang, string Message, string Misc)
{
	OUTER_LOOP_BEGIN_COND(SERVER_HOOK_EVENT_ON_CHAT, tOnChat)
		ret_val = (call)(pPlayer, Type, Lang, Message, Misc);
	OUTER_LOOP_END_COND;
}

void HookInterface::OnLoot(Player * pPlayer, Object * pTarget, uint32 money, uint32 itemId)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_LOOT, tOnLoot)
		(call)(pPlayer, pTarget, money, itemId);
	OUTER_LOOP_END;
}

void HookInterface::OnObjectLoot(Player * pPlayer, Object * pTarget, uint32 money, uint32 itemId)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_OBJECTLOOT, tOnObjectLoot)
		(call)(pPlayer, pTarget, money, itemId);
	OUTER_LOOP_END;
}

void HookInterface::OnFullLogin(Player* pPlayer)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_FULL_LOGIN, tOnFullLogin)
		(call)(pPlayer);
	OUTER_LOOP_END;
}

void HookInterface::OnQuestCancelled(Player* pPlayer, Quest * pQuest)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_QUEST_CANCELLED, tOnQuestCancel)
		(call)(pPlayer, pQuest);
	OUTER_LOOP_END;
}

void HookInterface::OnQuestFinished(Player* pPlayer, Quest * pQuest, Object* pObject)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_QUEST_FINISHED, tOnQuestFinished)
		(call)(pPlayer, pQuest, pObject);
	OUTER_LOOP_END;
}

void HookInterface::OnHonorableKill(Player* pPlayer, Player* pKilled)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_HONORABLE_KILL, tOnHonorableKill)
		(call)(pPlayer, pKilled);
	OUTER_LOOP_END;
}

void HookInterface::OnArenaFinish(Player* pPlayer, uint32 type, ArenaTeam* pTeam, bool victory, bool rated)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_ARENA_FINISH, tOnArenaFinish)
		(call)(pPlayer, type, pTeam, victory, rated);
	OUTER_LOOP_END;
}

void HookInterface::OnPostLevelUp(Player * pPlayer)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_POST_LEVELUP, tOnPostLevelUp)
		(call)(pPlayer);
	OUTER_LOOP_END;
}

bool HookInterface::OnPreUnitDie(Unit *Killer, Unit *Victim)
{
	OUTER_LOOP_BEGIN_COND(SERVER_HOOK_EVENT_ON_PRE_DIE,tOnPreUnitDie)
		ret_val = (call)(Killer, Victim);
	OUTER_LOOP_END_COND;
}

void HookInterface::OnAdvanceSkillLine(Player * pPlayer, uint32 SkillLine, uint32 Current)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_ADVANCE_SKILLLINE, tOnAdvanceSkillLine)
		(call)(pPlayer, SkillLine, Current);
	OUTER_LOOP_END;
}

void HookInterface::OnDuelFinished(Player * Winner, Player * Looser)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_DUEL_FINISHED, tOnDuelFinished)
		(call)(Winner, Looser);
	OUTER_LOOP_END;
}

void HookInterface::OnUpdate(Player* pPlayer)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_UPDATE, tOnUpdate)
		(call)(pPlayer);
	OUTER_LOOP_END;
}

void HookInterface::OnContinentCreate(MapMgr* pMgr)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_CONTINENT_CREATE, tOnContinentCreate)
		(call)(pMgr);
	OUTER_LOOP_END;
}

void HookInterface::OnPostSpellCast(Player* pPlayer, SpellEntry * pSpell, Unit* pTarget)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_POST_SPELL_CAST, tOnPostSpellCast)
		(call)(pPlayer, pSpell, pTarget);
	OUTER_LOOP_END;
}

void HookInterface::OnAreaTrigger(Player* plr, uint32 areatrigger)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_AREATRIGGER, tOnAreaTrigger)
		(call)(plr, areatrigger);
	OUTER_LOOP_END;
}

void HookInterface::OnPlayerSaveToDB(Player* pPlayer, QueryBuffer* buf)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_PLAYER_SAVE_TO_DB, tOnPlayerSaveToDB)
		(call)(pPlayer, buf);
	OUTER_LOOP_END;
}

void HookInterface::OnPlayerChangeArea(Player* pPlayer, uint32 ZoneID, uint32 NewAreaID, uint32 OldAreaID)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_PLAYER_CHANGE_AREA, tOnPlayerChangeArea)
		(call)(pPlayer, ZoneID, NewAreaID, OldAreaID);
	OUTER_LOOP_END;
}

void HookInterface::OnAuraRemove(Player* pPlayer, uint32 spellID)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_AURA_REMOVE, tOnAuraRemove)
		(call)(pPlayer, spellID);
	OUTER_LOOP_END;
}

bool HookInterface::OnResurrect(Player * pPlayer)
{
	OUTER_LOOP_BEGIN_COND(SERVER_HOOK_EVENT_ON_RESURRECT,tOnResurrect)
		ret_val = (call)(pPlayer);
	OUTER_LOOP_END_COND;
}

void HookInterface::OnDestroyBuilding(GameObject* go)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_DESTROY_BUILDING,tOnDestroyBuilding)
		(call)(go);
	OUTER_LOOP_END;
}

void HookInterface::OnDamageBuilding(GameObject* go)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_DAMAGE_BUILDING,tOnDamageBuilding)
		(call)(go);
	OUTER_LOOP_END;
}

bool HookInterface::OnMountFlying(Player* plr)
{
	OUTER_LOOP_BEGIN_COND(SERVER_HOOK_EVENT_ON_MOUNT_FLYING, tOnMountFlying)
		ret_val = (call)(plr);
	OUTER_LOOP_END_COND;
}

bool HookInterface::OnPreAuraRemove(Player* plr,uint32 spellID)
{
	OUTER_LOOP_BEGIN_COND(SERVER_HOOK_EVENT_ON_PRE_AURA_REMOVE,tOnPreAuraRemove)
		ret_val = (call)(plr,spellID);
	OUTER_LOOP_END_COND;
}

void HookInterface::OnSlowLockOpen(GameObject* go,Player* plr)
{
	OUTER_LOOP_BEGIN(SERVER_HOOK_EVENT_ON_SLOW_LOCK_OPEN,tOnSlowLockOpen)
		(call)(go,plr);
	OUTER_LOOP_END;
}
