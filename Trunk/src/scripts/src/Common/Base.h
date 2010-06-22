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

#ifndef _INSTANCE_SCRIPTS_BASE_H_
#define _INSTANCE_SCRIPTS_BASE_H_

#include "StdAfx.h"
#include "Instance_Base.h"

#include <string>
#include <vector>
#include <map>

#define INVALIDATE_TIMER			-1
#define DEFAULT_UPDATE_FREQUENCY	1000	//milliseconds
#define DEFAULT_DESPAWN_TIMER		2000	//milliseconds

#define MOONSCRIPT_FACTORY_FUNCTION(ClassName, ParentClassName)\
public:\
	ADD_CREATURE_FACTORY_FUNCTION(ClassName);\
	typedef ParentClassName ParentClass;

enum TextType
{
	Text_Say,
	Text_Yell,
	Text_Emote
};

enum EventType
{
	Event_OnCombatStart,
	Event_OnTargetDied,
	Event_OnDied,
	Event_OnTaunt
};

enum BehaviorType
{
	Behavior_Default,
	Behavior_Melee,
	Behavior_Ranged,
	Behavior_Spell,
	Behavior_Flee,
	Behavior_CallForHelp
};

enum MoveType
{
	Move_None,
	Move_RandomWP,
	Move_CircleWP,
	Move_WantedWP,
	Move_DontMoveWP,
	Move_Quest,
	Move_ForwardThenStop
};

enum MoveFlag
{
	Flag_Walk = 0,
	Flag_Run = 256,
	Flag_Fly = 768
};

struct EmoteDesc
{
	EmoteDesc(const char* pText, TextType pType, uint32 pSoundId, EmoteType pEmoteType)
	{
		mText = ( pText && strlen(pText) > 0 ) ? pText : "";
		mType = pType;
		mSoundId = pSoundId;
		mEmoteType = pEmoteType;
	}

	std::string	mText;
	TextType	mType;
	uint32		mSoundId;
	EmoteType   mEmoteType;
};

struct Coords
{
	float	mX;
	float	mY;
	float	mZ;
	float	mO;
	uint32	mAddition;
};

struct LootDesc
{
	uint32 mItemID;
	uint32 mChance;
	uint32 mMinCount;
	uint32 mMaxCount;
	uint32 mFFA;
};

enum TargetGenerator
{
	// Self
	TargetGen_Self,							// Target self ( Note: doesn't always mean self, also means the spell can choose various target )

	// Current
	TargetGen_Current,						// Current highest aggro ( attacking target )
	TargetGen_Destination,					// Target is a destination coordinates ( X, Y, Z )

	// Second most hated
	TargetGen_SecondMostHated,				// Second highest aggro

	// Predefined target
	TargetGen_Predefined,					// Pre-defined target unit

	// Random Unit
	TargetGen_RandomUnit,					// Random target unit ( players, totems, pets, etc. )
	TargetGen_RandomUnitDestination,		// Random destination coordinates ( X, Y, Z )
	TargetGen_RandomUnitApplyAura,			// Random target unit to self cast aura

	// Random Player
	TargetGen_RandomPlayer,					// Random target player
	TargetGen_RandomPlayerDestination,		// Random player destination coordinates ( X, Y, Z )
	TargetGen_RandomPlayerApplyAura,		// Random target player to self cast aura
};

enum TargetFilter
{
	// Standard filters
	TargetFilter_None						= 0,																	// 0
	TargetFilter_Closest					= 1 << 0,																// 1
	TargetFilter_Friendly					= 1 << 1,																// 2
	TargetFilter_NotCurrent					= 1 << 2,																// 4
	TargetFilter_Wounded					= 1 << 3,																// 8
	TargetFilter_SecondMostHated			= 1 << 4,																// 16
	TargetFilter_Aggroed					= 1 << 5,																// 32
	TargetFilter_Corpse						= 1 << 6,																// 64
	TargetFilter_InMeleeRange				= 1 << 7,																// 128
	TargetFilter_InRangeOnly				= 1 << 8,																// 256
	TargetFilter_IgnoreSpecialStates		= 1 << 9,																// 512 - not really a TargetFilter, more like requirement for spell
	TargetFilter_IgnoreLineOfSight			= 1 << 10,																// 1024

	// Predefined filters
	TargetFilter_ClosestFriendly			= TargetFilter_Closest | TargetFilter_Friendly,							// 3
	TargetFilter_ClosestNotCurrent			= TargetFilter_Closest | TargetFilter_NotCurrent,						// 5
	TargetFilter_WoundedFriendly			= TargetFilter_Wounded | TargetFilter_Friendly,							// 10
	TargetFilter_FriendlyCorpse				= TargetFilter_Corpse | TargetFilter_Friendly,							// 66
	TargetFilter_ClosestFriendlyCorpse		= TargetFilter_Closest | TargetFilter_FriendlyCorpse,					// 67
};

enum RangeStatus
{
	RangeStatus_Ok,
	RangeStatus_TooFar,
	RangeStatus_TooClose
};

class TargetType;
class SpellDesc;
class MoonScriptCreatureAI;
class MoonScriptBossAI;

typedef void(*SpellFunc)(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit *pTarget, TargetType pType);
typedef std::vector<EmoteDesc*> EmoteArray;
typedef std::vector<Player *> PlayerArray;
typedef std::vector<Unit *> UnitArray;
typedef std::vector<SpellDesc*> SpellDescArray;
typedef std::list<SpellDesc*> SpellDescList;
typedef std::pair<int32, SpellDesc*> PhaseSpellPair;
typedef std::vector<PhaseSpellPair> PhaseSpellArray;
typedef std::pair<int32, int32> TimerPair;
typedef std::vector<TimerPair> TimerArray;
typedef std::vector<LootDesc> LootTable;
typedef std::pair< RangeStatus, float > RangeStatusPair;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class TargetType
class TargetType
{
public:
	TargetType( uint32 pTargetGen = TargetGen_Self, TargetFilter pTargetFilter = TargetFilter_None, uint32 pMinTargetNumber = 0, uint32 pMaxTargetNumber = 0 );
	~TargetType();

	uint32			mTargetGenerator;	// Defines what kind of target should we try to find
	TargetFilter	mTargetFilter;		// Defines filter of target
	uint32			mTargetNumber[ 2 ];	// 0: Defines min. number of creature on hatelist ( 0 - any, 1 - the most hated etc. )
										// 1: Defines max. number of creature on hatelist ( 0 - any, HateList.size + 1 - the least hated etc. )
};

// Pre-made TargetTypes
#define Target_Self TargetType()
#define Target_Current TargetType( TargetGen_Current )
#define Target_SecondMostHated TargetType( TargetGen_SecondMostHated )
#define Target_Destination TargetType( TargetGen_Destination )
#define Target_Predefined TargetType( TargetGen_Predefined )
#define Target_RandomPlayer TargetType( TargetGen_RandomPlayer )
#define Target_RandomPlayerNotCurrent TargetType( TargetGen_RandomPlayer, TargetFilter_NotCurrent )
#define Target_RandomPlayerDestination TargetType( TargetGen_RandomPlayerDestination )
#define Target_RandomPlayerApplyAura TargetType( TargetGen_RandomPlayerApplyAura )
#define Target_RandomUnit TargetType( TargetGen_RandomUnit )
#define Target_RandomUnitNotCurrent TargetType( TargetGen_RandomUnit, TargetFilter_NotCurrent )
#define Target_RandomDestination TargetType( TargetGen_RandomUnitDestination )
#define Target_RandomUnitApplyAura TargetType( TargetGen_RandomUnitApplyAura )
#define Target_RandomFriendly TargetType( TargetGen_RandomUnit, TargetFilter_Friendly )
#define Target_WoundedPlayer TargetType( TargetGen_RandomPlayer, TargetFilter_Wounded )
#define Target_WoundedUnit TargetType( TargetGen_RandomUnit, TargetFilter_Wounded )
#define Target_WoundedFriendly TargetType( TargetGen_RandomUnit, TargetFilter_WoundedFriendly )
#define Target_ClosestPlayer TargetType( TargetGen_RandomPlayer, TargetFilter_Closest )
#define Target_ClosestPlayerNotCurrent TargetType( TargetGen_RandomPlayer, TargetFilter_ClosestNotCurrent )
#define Target_ClosestUnit TargetType( TargetGen_RandomUnit, TargetFilter_Closest )
#define Target_ClosestUnitNotCurrent TargetType( TargetGen_RandomUnit, TargetFilter_ClosestNotCurrent )
#define Target_ClosestFriendly TargetType( TargetGen_RandomUnit, TargetFilter_ClosestFriendly )
#define Target_ClosestCorpse TargetType( TargetGen_RandomUnit, TargetFilter_ClosestFriendlyCorpse )
#define Target_RandomCorpse TargetType( TargetGen_RandomUnit, TargetFilter_FriendlyCorpse )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class SpellDesc
class SpellDesc
{
public:
	SpellDesc(SpellEntry* pInfo, SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange, float pMaxRange, bool pStrictRange, const char* pText, TextType pTextType, uint32 pSoundId, EmoteType pEmoteType);
	virtual ~SpellDesc();

	EmoteDesc*	AddEmote(const char* pText, TextType pType=Text_Yell, uint32 pSoundId=0, EmoteType pEmoteType = EMOTE_ONESHOT_NONE);
	void		TriggerCooldown(uint32 pCurrentTime=0);

	SpellEntry*	mInfo;				//Spell Entry information (generally you either want a SpellEntry OR a SpellFunc, not both)
	SpellFunc	mSpellFunc;			//Spell Function to be executed (generally you either want a SpellEntry OR a SpellFunc, not both)
	TargetType	mTargetType;		//Target type (see class above)
	float		mChance;			//Percent of the cast of this spell in a total of 100% of the attacks
	float		mCastTime;			//Duration of the spell cast (seconds). Zero means the spell is instant cast
	int32		mCooldown;			//Spell cooldown (seconds)
	float		mMinRange;			//Minimum range for spell to be cast
	float		mMaxRange;			//Maximum range for spell to be cast
	bool		mStrictRange;		//If true, creature won't run to (or away of) target, it will instead skip that attack
	EmoteArray	mEmotes;			//Emotes (random) shouted on spell cast
	bool		mEnabled;			//True if the spell is enabled for casting, otherwise it will never be scheduled (useful for bosses with phases, etc.)
	Unit *	mPredefinedTarget;	//Pre-defined Target Unit (Only valid with target type TargetGen_Predefined);

	//Those are not properties, they are data member used by the evaluation system
	uint32		mLastCastTime;		//Last time at which the spell casted (used to implement cooldown), set to 0
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class MoonScriptCreatureAI
class MoonScriptCreatureAI : public CreatureAIScript
{
public:
	MoonScriptCreatureAI(Creature *pCreature);
	virtual ~MoonScriptCreatureAI();

	/* Gonna make a small documentation for scripts. 
	Currently i'm choosing where to do it. 
	First idea was to make a wiki for it, but it would be so slow to make it works.
	Second idea was to post an articles with docu on sandshroud forums. But I NEED MOAR FORMAT TOOLZ! :D
	Third idea is to put here comments, because while you 'brows' functions to use, you can read a small comments on what that function is doing.
	So, dunno what to do. Summarize all 'yes' and 'no' i'm beggin before wiki and maybe will do it some time later. */

	//Movement
	bool					GetCanMove();
	void					SetCanMove(bool pCanMove);
	void					MoveTo(MoonScriptCreatureAI* pCreature, RangeStatusPair pRangeStatus = make_pair( RangeStatus_TooFar, 0.0f ) );
	void					MoveTo(Unit *pUnit, RangeStatusPair pRangeStatus = make_pair( RangeStatus_TooFar, 0.0f ) );
	void					MoveTo(float pX, float pY, float pZ, bool pRun=true);
	void					MoveToSpawnOrigin();
	void					StopMovement();
	void					SetFlyMode(bool pValue);

	//Attack and Combat State
	bool					GetCanEnterCombat();
	void					SetCanEnterCombat(bool pCanEnterCombat);
	bool					IsInCombat();
	void					DelayNextAttack(int32 pMilliseconds);
	void					SetDespawnWhenInactive(bool pValue);

	//Behavior
	void					SetBehavior(BehaviorType pBehavior);
	BehaviorType			GetBehavior();
	void					SetAllowMelee(bool pAllow);
	bool					GetAllowMelee();
	void					SetAllowRanged(bool pAllow);
	bool					GetAllowRanged();
	void					SetAllowSpell(bool pAllow);
	bool					GetAllowSpell();
	void					SetAllowTargeting(bool pAllow);
	bool					GetAllowTargeting();
	void					AggroNearestUnit(int pInitialThreat=1);
	void					AggroRandomUnit(int pInitialThreat=1);
	void					AggroNearestPlayer(int pInitialThreat=1);
	void					AggroRandomPlayer(int pInitialThreat=1);
	void					AttackPlayer(Unit *pPlayer);

	//Status
	void					ClearHateList();
	void					WipeHateList();
	int32					GetHealthPercent();
	int32					GetManaPercent();
	void					Regenerate();
	bool					IsAlive();
	void					SetScale(float pScale);
	float					GetScale();
	void					SetDisplayId(uint32 pDisplayId);
	void					SetWieldWeapon(bool pValue);
	void					SetDisplayWeapon(bool pMainHand, bool pOffHand);
	void					SetDisplayWeaponIds(uint32 pItem1Id, uint32 pItem2Id);

	//Environment
	float					GetRange(MoonScriptCreatureAI* pCreature);
	float					GetRangeToUnit(Unit *pUnit);

	//Instances
	bool					IsHeroic();
	uint32					GetInstanceMode();
	
	Player *			GetNearestPlayer();
	GameObject *		GetNearestGameObject(uint32 pGameObjectId=0);
	MoonScriptCreatureAI*	GetNearestCreature(uint32 pCreatureId=0);
	MoonScriptCreatureAI*	SpawnCreature(uint32 pCreatureId, bool pForceSameFaction=false, Unit* pTarget=NULLUNIT);
	MoonScriptCreatureAI*	SpawnCreature(uint32 pCreatureId, float pX, float pY, float pZ, float pO=0, bool pForceSameFaction=false, Unit* pTarget=NULLUNIT);
	Unit *				ForceCreatureFind( uint32 pCreatureId );
	Unit *				ForceCreatureFind( uint32 pCreatureId, float pX, float pY, float pZ );
	void					Despawn(uint32 pDelay=0, uint32 pRespawnTime=0);


	SpellDesc*				AddSpell(uint32 pSpellId, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange=0, float pMaxRange=0, bool pStrictRange=false, const char* pText=NULL, TextType pTextType=Text_Yell, uint32 pSoundId=0, EmoteType pEmoteType = EMOTE_ONESHOT_NONE);
	SpellDesc*				AddSpellFunc(SpellFunc pFnc, TargetType pTargetType, float pChance, float pCastTime, int32 pCooldown, float pMinRange=0, float pMaxRange=0, bool pStrictRange=false, const char* pText=NULL, TextType pTextType=Text_Yell, uint32 pSoundId=0, EmoteType pEmoteType = EMOTE_ONESHOT_NONE);

	void					CastSpell(SpellDesc* pSpell);
	void					CastSpellNowNoScheduling(SpellDesc* pSpell);
    void                    CastSpellInRange(SpellDesc *pSpell);
	SpellDesc*				FindSpellById(uint32 pSpellId);
	SpellDesc*				FindSpellByFunc(SpellFunc pFnc);
	bool					IsCasting();
	void					ApplyAura(uint32 pSpellId);
	void					RemoveAura(uint32 pSpellId);
	void					RemoveAuraOnPlayers(uint32 pSpellId);
	void					RemoveAllAuras();
	void					TriggerCooldownOnAllSpells();
	void					CancelAllCooldowns();

	//Emotes
	EmoteDesc*				AddEmote(EventType pEventType, const char* pText, TextType pType, uint32 pSoundId=0, EmoteType pEmoteType = EMOTE_ONESHOT_NONE);
	void					RemoveEmote(EventType pEventType, EmoteDesc* pEmote);
	void					RemoveAllEmotes(EventType pEventType);
	void					Emote(EmoteDesc* pEmote);
	void					Emote(const char* pText, TextType pType=Text_Yell, uint32 pSoundId=0, EmoteType pEmoteType = EMOTE_ONESHOT_NONE);

	//Timers
	uint32					AddTimer(int32 pDurationMillisec);
	int32					GetTimer(int32 pTimerId);
	void					RemoveTimer(int32& pTimerId);
	void					ResetTimer(int32 pTimerId, int32 pDurationMillisec);
	bool					IsTimerFinished(int32 pTimerId);
	void					CancelAllTimers();

	//Waypoints
	WayPoint*				CreateWaypoint(int pId, uint32 pWaittime, uint32 pMoveFlag, Coords pCoords);
	void					AddWaypoint(WayPoint* pWayPoint);
	void					ForceWaypointMove(uint32 pWaypointId);
	void					SetWaypointToMove(uint32 pWaypointId);
	void					StopWaypointMovement();
	void					SetMoveType(MoveType pMoveType);
	MoveType				GetMoveType();
	uint32					GetCurrentWaypoint();
	size_t					GetWaypointCount();
	bool					HasWaypoints();

	//Others
	void					SetTargetToChannel(Unit *pTarget, uint32 pSpellId);
	Unit *					GetTargetToChannel();

	//Options
	void					SetAIUpdateFreq(uint32 pUpdateFreq);
	uint32					GetAIUpdateFreq();

	//Loot
	void					AddLootToTable(LootTable* pTable, uint32 pItemID, uint32 pChance, uint32 pMinCount, uint32 pMaxCount, uint32 pFFA);
	void					ClearLoot(Unit *pTarget);
	void					AddLootFromTable(Unit *pTarget, LootTable* pTable, uint32 pCount = 1);
	void					SetGoldLoot(Unit *pTarget, uint32 pMinGold, uint32 pMaxGold);
	void					AddLoot(Unit *pTarget, uint32 pItemID, uint32 pMinCount, uint32 pMaxCount, uint32 pFFA);
	void					AddRareLoot(Unit *pTarget, uint32 pItemID, float pPercentChance);

	//Reimplemented Events
	virtual void			OnCombatStart(Unit *pTarget);
	virtual void			OnCombatStop(Unit *pTarget);
	virtual void			OnTargetDied(Unit *pTarget);
	virtual void			OnDied(Unit *pKiller);
	virtual void			AIUpdate();
	virtual void			Destroy();

protected:

	bool					IsSpellScheduled(SpellDesc* pSpell);
	bool					CastSpellInternal(SpellDesc* pSpell, uint32 pCurrentTime=0);
	void					CastSpellOnTarget(Unit *pTarget, TargetType pType, SpellEntry* pEntry, bool pInstant);
	int32					CalcSpellAttackTime(SpellDesc* pSpell);
	void					CancelAllSpells();

	RangeStatusPair			GetSpellRangeStatusToUnit( Unit *pTarget, SpellDesc* pSpell );
	Unit *					GetTargetForSpell( SpellDesc* pSpell );
	Unit *					GetBestPlayerTarget( TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f );
	Unit *					GetBestUnitTarget( TargetFilter pFilter = TargetFilter_None, float pMinRange = 0.0f, float pMaxRange = 0.0f );
	Unit *					ChooseBestTargetInArray( UnitArray& pTargetArray, TargetFilter pFilter );
	Unit *					GetNearestTargetInArray( UnitArray& pTargetArray );
	Unit *					GetSecondMostHatedTargetInArray( UnitArray& pTargetArray );
	bool					IsValidUnitTarget( Object *pObject, TargetFilter pFilter, float pMinRange = 0.0f, float pMaxRange = 0.0f  );
	void					PushRunToTargetCache( Unit *pTarget, SpellDesc* pSpell, RangeStatusPair pRangeStatus = make_pair( RangeStatus_TooFar, 0.0f ) );
	void					PopRunToTargetCache();

	void					RandomEmote(EmoteArray& pEmoteArray);

	SpellDescArray			mSpells;
	SpellDescList			mQueuedSpells;
	SpellDescList			mScheduledSpells;

	Unit *					mRunToTargetCache;
	SpellDesc*				mRunToTargetSpellCache;

	EmoteArray				mOnCombatStartEmotes;
	EmoteArray				mOnTargetDiedEmotes;
	EmoteArray				mOnDiedEmotes;
	EmoteArray				mOnTauntEmotes;

	TimerArray				mTimers;
	int32					mTimerIdCounter;
	uint32					mAIUpdateFrequency;
	uint32					mBaseAttackTime;
	bool					mDespawnWhenInactive;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Class MoonScriptBossAI
class MoonScriptBossAI : public MoonScriptCreatureAI
{
public:
	MoonScriptBossAI(Creature *pCreature);
	virtual ~MoonScriptBossAI();

	//Basic Interface
	SpellDesc*		AddPhaseSpell(int32 pPhase, SpellDesc* pSpell);
	int32			GetPhase();
	void			SetPhase(int32 pPhase, SpellDesc* pPhaseChangeSpell=NULL);
	void			SetEnrageInfo(SpellDesc* pSpell, int32 pTriggerMilliseconds);

	//Reimplemented Events
	virtual void	OnCombatStart(Unit *pTarget);
	virtual void	OnCombatStop(Unit *pTarget);
	virtual void	AIUpdate();

protected:
	int32			mPhaseIndex;
	PhaseSpellArray	mPhaseSpells;
	SpellDesc*		mEnrageSpell;
	int32			mEnrageTimerDuration;
	int32			mEnrageTimer;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//STL Utilities
template <class Type> inline void DeleteArray(std::vector<Type> pVector)
{
	typename std::vector<Type>::iterator Iter = pVector.begin();
	for( ; Iter != pVector.end(); ++Iter )
	{
		delete (*Iter);
	}
	pVector.clear();
}

//Warning: do not use if item can reside more than once in same vector
template <class Type> inline void DeleteItem(std::vector<Type> pVector, Type pItem)
{
	typename std::vector<Type>::iterator Iter = std::find(pVector.begin(), pVector.end(), pItem);
	if( Iter != pVector.end() )
	{
		delete (*Iter);
		pVector.erase(Iter);
	}
}

#endif /* _INSTANCE_SCRIPTS_BASE_H_ */
