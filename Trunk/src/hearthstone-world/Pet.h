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

#ifndef _PET_H
#define _PET_H

#define PET_SPELL_SPAM_COOLDOWN 2000 //applied only to spells that have no cooldown

/* Taken from ItemPetFood.dbc
 * Each value is equal to a flag
 * so 1 << PET_FOOD_BREAD for example
 * will result in a usable value.
 */
enum PET_FOOD
{
	PET_FOOD_NONE,
	PET_FOOD_MEAT,
	PET_FOOD_FISH,
	PET_FOOD_CHEESE,
	PET_FOOD_BREAD,
	PET_FOOD_FUNGUS,
	PET_FOOD_FRUIT,
	PET_FOOD_RAW_MEAT, // not used in pet diet
	PET_FOOD_RAW_FISH  // not used in pet diet
};

/* Loyalty and happiness */
static const char LoyaltyTicks[3] = { -10, 5, 20 };//loyalty_ticks for unhappy, content, happy - PetPersonality.dbc
static const uint32 HappinessTicks[6] = { 2783, 2088, 1670, 1392, 1193, 1044 };//loose_happiness ticks per loyalty lvl for 7.5s timer
static const uint16 LoyLvlRange[7] = { 0, 150, 300, 450, 600, 900, 1200 };//loyalty level ranges (1200 is guessed)

enum PET_ACTION
{
	PET_ACTION_STAY,
	PET_ACTION_FOLLOW,
	PET_ACTION_ATTACK,
	PET_ACTION_DISMISS,
	PET_ACTION_CASTING,
};

enum PET_STATE
{
	PET_STATE_PASSIVE,
	PET_STATE_DEFENSIVE,
	PET_STATE_AGGRESSIVE
};

enum PET_SPELL
{
	PET_SPELL_PASSIVE = 0x06000000,
	PET_SPELL_DEFENSIVE,
	PET_SPELL_AGRESSIVE,
	PET_SPELL_STAY = 0x07000000,
	PET_SPELL_FOLLOW,
	PET_SPELL_ATTACK
};

enum StableState
{
	STABLE_STATE_ACTIVE		= 1,
	STABLE_STATE_PASSIVE	= 2
};

enum StableResult
{
	STABLERESULT_FAIL_CANT_AFFORD		= 0x01,
	STABLERESULT_FAIL					= 0x06,
	STABLERESULT_STABLE_SUCCESS			= 0x08,
	STABLERESULT_UNSTABLE_SUCCESS		= 0x09,
	STABLERESULT_BUY_SLOT_SUCCESS		= 0x0A
};

#define MAX_STABLE_SLOTS 4

enum HappinessState
{
	UNHAPPY		=1,
	CONTENT		=2,
	HAPPY		=3
};

enum AutoCastEvents
{
	AUTOCAST_EVENT_NONE					= 0,
	AUTOCAST_EVENT_ATTACK				= 1,
	AUTOCAST_EVENT_ON_SPAWN				= 2,
	AUTOCAST_EVENT_OWNER_ATTACKED		= 3,
	AUTOCAST_EVENT_LEAVE_COMBAT			= 4,
	AUTOCAST_EVENT_COUNT				= 5,
};

#define PET_DELAYED_REMOVAL_TIME 60000  // 1 min

#define DEFAULT_SPELL_STATE 0x8100
#define AUTOCAST_SPELL_STATE 0xC100

typedef map<SpellEntry*, uint16> PetSpellMap;
typedef map<uint32, uint8> PetTalentMap;
struct PlayerPet;

class SERVER_DECL Pet : public Creature
{
	friend class Player;
	friend class Creature;
	friend class WorldSession;

public:
	Pet(uint64 guid);
	~Pet();
	virtual void Init();
	virtual void Destructor();

	void LoadFromDB(Player* owner, PlayerPet * pi);
	void CreateAsSummon(uint32 entry, CreatureInfo *ci, Creature* created_from_creature, Unit* owner, SpellEntry *created_by_spell, uint32 type, uint32 expiretime);

	virtual void Update(uint32 time);

	HEARTHSTONE_INLINE uint32 GetXP(void) { return m_PetXP; }

	void InitializeSpells();
	void ReInitializeSpells();
	void InitializeMe(bool first);
	void SendSpellsToOwner();
	void SendNullSpellsToOwner();

	HEARTHSTONE_INLINE void SetPetAction(uint32 act) { m_Action = act; }
	HEARTHSTONE_INLINE uint32 GetPetAction(void) { return m_Action; }

	HEARTHSTONE_INLINE void SetPetState(uint32 state) { m_State = state; }
	HEARTHSTONE_INLINE uint32 GetPetState(void) { return m_State; }

	HEARTHSTONE_INLINE void SetPetDiet(uint32 diet) { m_Diet = diet; }
	HEARTHSTONE_INLINE void SetPetDiet() { m_Diet = myFamily->petdietflags; }

	HEARTHSTONE_INLINE uint32 GetPetDiet(void) { return m_Diet; }
	
	HEARTHSTONE_INLINE AI_Spell* GetAISpellForSpellId(uint32 spellid)
	{
		std::map<uint32, AI_Spell*>::iterator itr = m_AISpellStore.find(spellid);
		if(itr != m_AISpellStore.end())
			return itr->second;
		else
			return NULL;
	}

	void UpdatePetInfo(bool bSetToOffline);
	void Remove(bool bSafeDelete, bool bUpdate, bool bSetOffline);
	void Dismiss(bool bSafeDelete = false);

	void DelayedRemove(bool bTime, bool bDeath);

	HEARTHSTONE_INLINE Player* GetPetOwner() { return m_Owner; }
	HEARTHSTONE_INLINE void ClearPetOwner() { m_Owner = NULLPLR; }

	/* Level and XP related functions */
	void GiveXP(uint32 xp);
	uint32 GetNextLevelXP(uint32 currentlevel);
	void LevelUpTo(uint32 level);
	void ApplyStatsForLevel();
	void ApplySummonLevelAbilities();
	void ApplyPetLevelAbilities();

	void PetSafeDelete();

	void SetDefaultSpells();
	void SetDefaultActionbar();
	void SetActionBarSlot(uint32 slot, uint32 spell){ ActionBar[ slot ] = spell; }

	void LoadSpells();
	void AddSpell(SpellEntry * sp, bool learning, bool sendspells = true);
	void LearnSpell(uint32 spellid);
	void LearnLevelupSpells();
	void RemoveSpell(SpellEntry * sp);
	void WipeSpells();
	uint32 GetUntrainCost();
	void SetSpellState(SpellEntry * sp, uint16 State);
	uint16 GetSpellState(SpellEntry * sp);
	HEARTHSTONE_INLINE void RemoveSpell(uint32 SpellID)
	{
		SpellEntry * sp = dbcSpell.LookupEntry(SpellID);
		if(sp)
			RemoveSpell(sp);
	}
	HEARTHSTONE_INLINE void SetSpellState(uint32 SpellID, uint16 State)
	{
		SpellEntry * sp = dbcSpell.LookupEntry(SpellID);
		if(sp) SetSpellState(sp, State);
	}
	HEARTHSTONE_INLINE uint16 GetSpellState(uint32 SpellID)
	{
		if(SpellID == 0)
			return DEFAULT_SPELL_STATE;

		SpellEntry * sp = dbcSpell.LookupEntry(SpellID);
		if(sp)
			return GetSpellState(sp);
		return DEFAULT_SPELL_STATE;
	}
	
	AI_Spell*CreateAISpell(SpellEntry * info);
	HEARTHSTONE_INLINE PetSpellMap* GetSpells() { return &mSpells; }
	HEARTHSTONE_INLINE bool IsSummonedPet() { return Summon; }
	bool IsWarlockPet()
	{
		return GetEntry() == 691 ? true : GetEntry() == 688 ? true : GetEntry() == 712 ? true : GetEntry() == 697 ? true : false;
	}

	void __fastcall SetAutoCastSpell(AI_Spell*sp);
	void Rename(string NewName);
	HEARTHSTONE_INLINE string& GetName() { return m_name; }
	void AddPetSpellToOwner(uint32 spellId);
	
	void HandleAutoCastEvent(uint32 Type);
	AI_Spell*HandleAutoCastEvent();
	void SetAutoCast(AI_Spell*sp, bool on);
	uint32 GetHappiness() { return GetUInt32Value(UNIT_FIELD_POWER5); };
	void SetHappiness(float amount) { SetUInt32Value(UNIT_FIELD_POWER5, amount); };
	void IncreaseHappiness(float amount) { SetHappiness(amount + GetHappiness()); };
	float GetHappinessDmgMod() { return 0.25f * GetHappinessState() + 0.5f; };
	const char* GetPetName() { return m_name.c_str(); }

	/* Pet Talents! */
	HEARTHSTONE_INLINE uint8 GetUnspentPetTalentPoints() { return GetByte( UNIT_FIELD_BYTES_1, 1); }
	HEARTHSTONE_INLINE void SetUnspentPetTalentPoints(uint8 points) { SetByte(UNIT_FIELD_BYTES_1, 1, points);}
	HEARTHSTONE_INLINE uint8 GetSpentPetTalentPoints() { return GetPetTalentPointsAtLevel() - GetUnspentPetTalentPoints(); }
	void InitTalentsForLevel(bool creating = false);
	bool ResetTalents(bool costs);
	void InitializeTalents();
	std::map<uint32, uint8> m_talents;

protected:
	Player* m_Owner;
	uint32 m_PetXP;
	PetSpellMap mSpells;
	PlayerPet * m_PlayerPetInfo;
	uint32 ActionBar[10];   // 10 slots
	
	
	std::map<uint32, AI_Spell*> m_AISpellStore;

	uint32 m_AutoCombatSpell;

	uint32 m_PartySpellsUpdateTimer;
	uint32 m_HappinessTimer;
	uint32 m_PetNumber;

	uint32 m_Action;
	uint32 m_State;
	uint32 m_ExpireTime;
	uint32 m_Diet;
	uint64 m_OwnerGuid;
	bool bExpires;
	bool Summon;
	string m_name;
	HappinessState GetHappinessState();
	uint32 GetHighestRankSpell(uint32 spellId);
	uint8 GetPetTalentPointsAtLevel();
	list<AI_Spell*> m_autoCastSpells[AUTOCAST_EVENT_COUNT];
	bool m_dismissed;
};

#define PET_HAPPINESS_UPDATE_VALUE 333000
#define PET_HAPPINESS_UPDATE_TIMER 7500
#define PET_PARTY_SPELLS_UPDATE_TIMER 10000

#define PET_ACTION_ACTION   0x700
#define PET_ACTION_STATE	0x600

//TODO: grep see the way pet spells contain the same flag?
#define PET_ACTION_SPELL	0xC100
#define PET_ACTION_SPELL_1  0x8100
#define PET_ACTION_SPELL_2  0x0100
#define PET_SPELL_AUTOCAST_CHANCE 50
#endif
