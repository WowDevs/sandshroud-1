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

#define SPELL_CHANNEL_UPDATE_INTERVAL 1000

/// externals for spell system
extern pSpellEffect SpellEffectsHandler[TOTAL_SPELL_EFFECTS];
extern pSpellTarget SpellTargetHandler[TOTAL_SPELL_TARGET];

enum SpellTargetSpecification
{
	TARGET_SPECT_NONE		= 0,
	TARGET_SPEC_INVISIBLE	= 1,
	TARGET_SPEC_DEAD		= 2,
};

void SpellCastTargets::read( WorldPacket & data, uint64 caster, uint8 castFlags )
{
	WoWGuid guid;
	m_unitTarget = m_itemTarget = 0;
	m_srcX = m_srcY = m_srcZ = m_destX = m_destY = m_destZ = missilespeed = missilepitch = traveltime = 0.0f;
	m_strTarget = "";

	data >> m_targetMask;

	if( m_targetMask == TARGET_FLAG_SELF  || m_targetMask & TARGET_FLAG_GLYPH )
	{
		m_unitTarget = caster;
	}

	if( m_targetMask & (TARGET_FLAG_OBJECT | TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 ) )
	{
		data >> guid;
		m_unitTarget = guid.GetOldGuid();
	}

	if( m_targetMask & ( TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM ) )
	{
		data >> guid;
		m_itemTarget = guid.GetOldGuid();
	}

	if( m_targetMask & ( TARGET_FLAG_ITEM2 ) )
	{
		uint32 unk2;
		uint8 unk;
		data >> unk >> unk >> unk;
		data >> unk2;
		data >> guid;
		m_itemTarget = guid.GetOldGuid();
	}

	if( m_targetMask & TARGET_FLAG_SOURCE_LOCATION )
	{
		data >> guid >> m_srcX >> m_srcY >> m_srcZ;

		if( !( m_targetMask & TARGET_FLAG_DEST_LOCATION ) )
		{
			m_destX = m_srcX;
			m_destY = m_srcY;
			m_destZ = m_srcZ;
		}
	}

	if( m_targetMask & TARGET_FLAG_DEST_LOCATION )
	{
		data >> guid >> m_destX >> m_destY >> m_destZ;
		m_unitTarget = guid.GetOldGuid();
		if( !( m_targetMask & TARGET_FLAG_SOURCE_LOCATION ) )
		{
			m_srcX = m_destX;
			m_srcY = m_destY;
			m_srcZ = m_destZ;
		}
	}

	if( m_targetMask & TARGET_FLAG_STRING )
		data >> m_strTarget;

	if(castFlags & 0x2)
	{
		uint8 missileunkcheck;
		data >> missilepitch >> missilespeed >> missileunkcheck;
		if(missileunkcheck == 1)
		{
			uint32 unkdoodah, unkdoodah2;
			data >> unkdoodah;
			data >> unkdoodah2;
		}

		float dx = m_destX - m_srcX;
		float dy = m_destY - m_srcY;
		if((missilepitch != (M_PI / 4)) && (missilepitch != -M_PI / 4))
			traveltime = (sqrtf(dx * dx + dy * dy) / (cosf(missilepitch) * missilespeed)) * 1000;
	}
}

void SpellCastTargets::write( WorldPacket& data )
{
	data << m_targetMask;

	if( m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 | TARGET_FLAG_OBJECT | TARGET_FLAG_GLYPH) )
		FastGUIDPack( data, m_unitTarget );

	if( m_targetMask & ( TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM ) )
		FastGUIDPack( data, m_itemTarget );

	if( m_targetMask & TARGET_FLAG_SOURCE_LOCATION )
		data << m_srcX << m_srcY << m_srcZ;

	if( m_targetMask & TARGET_FLAG_DEST_LOCATION )
		if(m_unitTarget)
			{FastGUIDPack( data, m_unitTarget ); data << m_destX << m_destY << m_destZ;}
		else
			data << uint8(0) << m_destX << m_destY << m_destZ;
 
	if (m_targetMask & TARGET_FLAG_STRING)
        data << m_strTarget;
}

void SpellCastTargets::write( StackPacket& data )
{
	data << m_targetMask;

	if( m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE | TARGET_FLAG_CORPSE2 | TARGET_FLAG_OBJECT | TARGET_FLAG_GLYPH) )
		FastGUIDPack( data, m_unitTarget );

	if( m_targetMask & ( TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM ) )
		FastGUIDPack( data, m_itemTarget );

	if( m_targetMask & TARGET_FLAG_SOURCE_LOCATION )
		data << m_srcX << m_srcY << m_srcZ;

	if( m_targetMask & TARGET_FLAG_DEST_LOCATION )
	{
		if(m_unitTarget)
			FastGUIDPack( data, m_unitTarget );
		else
			data << uint8(0);
		data << m_destX << m_destY << m_destZ;
	}
    if (m_targetMask & TARGET_FLAG_STRING)
        data << m_strTarget;
}

Spell::Spell(Object* Caster, SpellEntry *info, bool triggered, Aura* aur)
{
	ASSERT( Caster != NULL && info != NULL );

	m_spellInfo = info;
	m_spellInfo_override = NULL;
	m_caster = Caster;
	duelSpell = false;
	m_pushbackCount = 0;
	m_missilePitch = 0;
	m_missileTravelTime = 0;

	switch( Caster->GetTypeId() )
	{
	case TYPEID_PLAYER:
		{
			g_caster = NULLGOB;
			i_caster = NULLITEM;
			u_caster = TO_UNIT( Caster );
			p_caster = TO_PLAYER( Caster );
			v_caster = NULLVEHICLE;
			if(Caster->IsVehicle())
				v_caster = TO_VEHICLE( Caster );
			if( p_caster->GetDuelState() == DUEL_STATE_STARTED )
				duelSpell = true;
		}break;

	case TYPEID_UNIT:
		{
			g_caster = NULLGOB;
			i_caster = NULLITEM;
			p_caster = NULLPLR;
			v_caster = NULLVEHICLE;
			u_caster = TO_UNIT( Caster );
			if(Caster->IsVehicle())
			{
				v_caster = TO_VEHICLE( Caster );
				if(!p_caster)
					p_caster = v_caster->m_redirectSpellPackets;
			}
			if( u_caster->IsPet() && TO_PET( u_caster)->GetPetOwner() != NULL && TO_PET( u_caster )->GetPetOwner()->GetDuelState() == DUEL_STATE_STARTED )
				duelSpell = true;
		}break;

	case TYPEID_ITEM:
	case TYPEID_CONTAINER:
		{
			g_caster = NULLGOB;
			u_caster = NULLUNIT;
			p_caster = NULLPLR;
			v_caster = NULLVEHICLE;
			i_caster = TO_ITEM( Caster );
			if( i_caster->GetOwner() && i_caster->GetOwner()->GetDuelState() == DUEL_STATE_STARTED )
				duelSpell = true;
		}break;

	case TYPEID_GAMEOBJECT:
		{
			u_caster = NULLUNIT;
			p_caster = NULLPLR;
			v_caster = NULLVEHICLE;
			i_caster = NULLITEM;
			g_caster = TO_GAMEOBJECT( Caster );
		}break;

	default:
		{
			if(sLog.IsOutDevelopement())
				printf("[DEBUG][SPELL] Incompatible object type, please report this to the dev's\n");
			else
				OUT_DEBUG("[DEBUG][SPELL] Incompatible object type, please report this to the dev's");
		}break;
	}

	m_spellState = SPELL_STATE_NULL;

	m_castPositionX = m_castPositionY = m_castPositionZ = 0;
	m_triggeredSpell = triggered;
	m_AreaAura = false;

	m_triggeredByAura = aur;

	damageToHit = 0;
	castedItemId = 0;

	m_usesMana = false;
	m_Spell_Failed = false;
	bDurSet = false;
	bRadSet[0] = false;
	bRadSet[1] = false;
	bRadSet[2] = false;

	cancastresult = SPELL_CANCAST_OK;

	unitTarget = NULLUNIT;
	itemTarget = NULLITEM;
	gameObjTarget = NULLGOB;
	playerTarget = NULLPLR;
	corpseTarget = NULLCORPSE;
	damage = 0;
	add_damage = 0;
	m_Delayed = false;
	m_ForceConsumption = false;
	pSpellId = 0;
	m_cancelled = false;
	ProcedOnSpell = NULL;
	forced_basepoints[0] = forced_basepoints[1] = forced_basepoints[2] = 0;
	extra_cast_number = 0;
	m_glyphIndex = 0;
	m_reflectedParent = NULLSPELL;
	m_isCasting = false;
	m_hitTargetCount = 0;
	m_missTargetCount = 0;
	m_targetList.reserve(3);
	m_magnetTarget = NULLUNIT;
	m_projectileWait = false;
}

Spell::~Spell()
{
	if( u_caster != NULL && u_caster->GetCurrentSpell() == this )
		u_caster->SetCurrentSpell(NULLSPELL);

	v_caster = NULLVEHICLE;
	g_caster = NULLGOB;
	u_caster = NULLUNIT;
	i_caster = NULLITEM;
	p_caster = NULLPLR;
	m_caster = NULLOBJ;
	v_caster = NULLVEHICLE;
	m_triggeredByAura = NULLAURA;
	unitTarget = NULLUNIT;
	itemTarget = NULLITEM;
	gameObjTarget = NULLGOB;
	playerTarget = NULLPLR;
	corpseTarget = NULLCORPSE;
	m_magnetTarget = NULLUNIT;
	m_reflectedParent = NULLSPELL;
}

//i might forget conditions here. Feel free to add them
bool Spell::IsStealthSpell()
{
	//check if aura name is some stealth aura
	if( GetSpellProto()->EffectApplyAuraName[0] == 16 ||
		GetSpellProto()->EffectApplyAuraName[1] == 16 ||
		GetSpellProto()->EffectApplyAuraName[2] == 16 )
		return true;
	return false;
}

//i might forget conditions here. Feel free to add them
bool Spell::IsInvisibilitySpell()
{
	//check if aura name is some invisibility aura
	if( GetSpellProto()->EffectApplyAuraName[0] == 18 ||
		GetSpellProto()->EffectApplyAuraName[1] == 18 ||
		GetSpellProto()->EffectApplyAuraName[2] == 18 )
		return true;
	return false;
}

void Spell::FillSpecifiedTargetsInArea( float srcx, float srcy, float srcz, uint32 ind, uint32 specification )
{
	FillSpecifiedTargetsInArea( ind, srcx, srcy, srcz, GetRadius(ind), specification );
}

// for the moment we do invisible targets
void Spell::FillSpecifiedTargetsInArea(uint32 i,float srcx,float srcy,float srcz, float range, uint32 specification)
{
	//TargetsList *tmpMap=&m_targetUnits[i];
	//InStealth()
	float r = range * range;
	//uint8 did_hit_result;
	for(unordered_set<Object* >::iterator itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); itr++ )
	{
		// don't add objects that are units and dead
		if( (*itr)->IsUnit() && !(TO_UNIT( *itr )->isAlive()))
			continue;


		//TO_UNIT(*itr)->InStealth()
		if( GetSpellProto()->TargetCreatureType && (*itr)->IsUnit())
		{
			Unit* Target = TO_UNIT((*itr));
			if(!(1<<(Target->GetCreatureType()-1) & GetSpellProto()->TargetCreatureType))
				continue;
		}

		if(IsInrange(srcx,srcy,srcz,(*itr),r))
		{
			if( v_caster != NULL ) // Vehicles can destroy gameobjects
			{
				if( (*itr)->IsUnit() )
				{
					if( isAttackable( u_caster, TO_UNIT( *itr ),!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
					{
						_AddTarget((TO_UNIT(*itr)), i);
					}
				}
				else if((*itr)->IsGameObject())
				{
					_AddTargetForced((*itr), i);
				}
			}
			else if( u_caster != NULL )
			{
				if( (*itr)->IsUnit() )
				{
					if( isAttackable( u_caster, TO_UNIT( *itr ),!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
					{
						_AddTarget((TO_UNIT(*itr)), i);
					}
				}
			}
			else //cast from GO
			{
				if(g_caster && g_caster->GetUInt32Value(OBJECT_FIELD_CREATED_BY) && g_caster->m_summoner)
				{
					if((*itr)->IsUnit())
					{	//trap, check not to attack owner and friendly
						if(isAttackable(g_caster->m_summoner,TO_UNIT(*itr),!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
							_AddTarget((TO_UNIT(*itr)), i);
					}
				}
				else
					_AddTargetForced((*itr), i);
			}
			if( GetSpellProto()->MaxTargets)
			{
				if( m_hitTargetCount >= GetSpellProto()->MaxTargets )
					return;
			}
		}
	}
}
void Spell::FillAllTargetsInArea(LocationVector & location,uint32 ind)
{
	FillAllTargetsInArea(ind,location.x,location.y,location.z,GetRadius(ind));
}

void Spell::FillAllTargetsInArea(float srcx,float srcy,float srcz,uint32 ind)
{
	FillAllTargetsInArea(ind,srcx,srcy,srcz,GetRadius(ind));
}

/// We fill all the targets in the area, including the stealth ed one's
void Spell::FillAllTargetsInArea(uint32 i,float srcx,float srcy,float srcz, float range, bool includegameobjects)
{
	//TargetsList *tmpMap=&m_targetUnits[i];
	float r = range*range;
	//uint8 did_hit_result;

	for(unordered_set<Object* >::iterator itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); itr++ )
	{
		// don't add objects that are units and dead
		if( (*itr)->IsUnit() && (!(TO_UNIT( *itr )->isAlive())))
			continue;

		if( GetSpellProto()->TargetCreatureType && (*itr)->IsUnit())
		{
			Unit* Target = TO_UNIT((*itr));
			if(!(1<<(Target->GetCreatureType()-1) & GetSpellProto()->TargetCreatureType))
				continue;
		}

		if(IsInrange(srcx,srcy,srcz,(*itr),r))
		{
			if( includegameobjects )
			{
				if( (*itr)->IsUnit() )
				{
					if( isAttackable( m_caster, TO_UNIT( *itr ),!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
					{
						_AddTarget((TO_UNIT(*itr)), i);
					}
				}
				else if((*itr)->IsGameObject())
				{
					_AddTargetForced((*itr), i);
				}
			}
			else if( u_caster != NULL )
			{
				if( (*itr)->IsUnit() )
				{
					if( isAttackable( u_caster, TO_UNIT( *itr ),!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
					{
						_AddTarget((TO_UNIT(*itr)), i);
					}
				}
			}
			else //cast from GO
			{
				if(g_caster && g_caster->GetUInt32Value(OBJECT_FIELD_CREATED_BY) && g_caster->m_summoner)
				{
					if((*itr)->IsUnit())
					{	//trap, check not to attack owner and friendly
						if(isAttackable(g_caster->m_summoner,TO_UNIT(*itr),!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
							_AddTarget((TO_UNIT(*itr)), i);
					}
				}
				else
					_AddTargetForced((*itr), i);
			}

			if( GetSpellProto()->MaxTargets)
			{
				if( m_hitTargetCount >= GetSpellProto()->MaxTargets )
					return;
			}
		}
	}
}

// We fill all the targets in the area, including the stealthed one's
void Spell::FillAllFriendlyInArea( uint32 i, float srcx, float srcy, float srcz, float range )
{
	float r = range*range;

	for( unordered_set<Object* >::iterator itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); itr++ )
	{
		if((*itr)->IsUnit())
			if( (!(TO_UNIT(*itr)->isAlive())) || ( (*itr)->IsCreature() && TO_CREATURE(*itr)->IsTotem() ))
				continue;

		if((*itr)->PhasedCanInteract(m_caster) == false)
			continue;

		if( GetSpellProto()->TargetCreatureType && (*itr)->IsUnit())
		{
			Unit* Target = TO_UNIT((*itr));
			if(!(1<<(Target->GetCreatureType()-1) & GetSpellProto()->TargetCreatureType))
				continue;
		}

		if( IsInrange( srcx, srcy, srcz, (*itr), r ))
		{
			if( u_caster != NULL )
			{
				if( isAttackable( u_caster, (*itr), !(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED) ) )
				{
					if((*itr)->IsUnit())
						_AddTarget((TO_UNIT(*itr)), i);
					else
					{
						_AddTargetForced((*itr)->GetGUID(), i);
					}
				}
			}
			else //cast from GO
			{
				if( g_caster != NULL && g_caster->GetUInt32Value( OBJECT_FIELD_CREATED_BY ) && g_caster->m_summoner != NULL )
				{
					//trap, check not to attack owner and friendly
					if( isAttackable( g_caster->m_summoner, TO_UNIT(*itr), !(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED) ) )
						_AddTarget((TO_UNIT(*itr)), i);
				}
				else
					_AddTargetForced((*itr)->GetGUID(), i);
			}
			if( GetSpellProto()->MaxTargets )
				if( m_hitTargetCount >= GetSpellProto()->MaxTargets )
					return;
		}
	}
}

/// We fill all the gameobject targets in the area
void Spell::FillAllGameObjectTargetsInArea(uint32 i,float srcx,float srcy,float srcz, float range)
{
	float r = range*range;

	for(unordered_set<Object* >::iterator itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); itr++ )
	{
		// don't add objects that are units and dead
		if(!(*itr)->IsGameObject())
			continue;
		GameObject * o = TO_GAMEOBJECT(*itr);

		if( IsInrange( srcx, srcy, srcz, o, r ))
		{
			_AddTargetForced(o, i);
		}
	}
}

uint64 Spell::GetSinglePossibleEnemy(uint32 i,float prange)
{
	float r;
	if(prange)
		r = prange;
	else
	{
		r = GetSpellProto()->base_range_or_radius_sqr;
		if( GetSpellProto()->SpellGroupType && u_caster)
		{
			SM_FFValue(u_caster->SM[SMT_RADIUS][0],&r,GetSpellProto()->SpellGroupType);
			SM_PFValue(u_caster->SM[SMT_RADIUS][1],&r,GetSpellProto()->SpellGroupType);
		}
	}
	float srcx = m_caster->GetPositionX(), srcy = m_caster->GetPositionY(), srcz = m_caster->GetPositionZ();
	for( unordered_set<Object* >::iterator itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); itr++ )
	{
		if( !( (*itr)->IsUnit() ) || !TO_UNIT(*itr)->isAlive() || !(*itr)->PhasedCanInteract(m_caster))
			continue;

		if( GetSpellProto()->TargetCreatureType && (*itr)->IsUnit())
		{
			Unit* Target = TO_UNIT((*itr));
			if(!(1<<(Target->GetCreatureType()-1) & GetSpellProto()->TargetCreatureType))
				continue;
		}
		if(IsInrange(srcx,srcy,srcz,(*itr),r))
		{
			if( u_caster != NULL || (g_caster && g_caster->GetType() == GAMEOBJECT_TYPE_TRAP) )
			{
				if(isAttackable(u_caster, TO_UNIT(*itr),!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)) && _DidHit(TO_UNIT(*itr))==SPELL_DID_HIT_SUCCESS)
					return (*itr)->GetGUID(); 
			}
			else //cast from GO
			{
				if(g_caster && g_caster->GetUInt32Value(OBJECT_FIELD_CREATED_BY) && g_caster->m_summoner)
				{
					//trap, check not to attack owner and friendly
					if( isAttackable( g_caster->m_summoner, TO_UNIT(*itr),!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
						return (*itr)->GetGUID();
				}
			}
		}
	}
	return 0;
}

uint64 Spell::GetSinglePossibleFriend(uint32 i,float prange)
{
	float r;
	if(prange)
		r = prange;
	else
	{
		r = GetSpellProto()->base_range_or_radius_sqr;
		if( GetSpellProto()->SpellGroupType && u_caster)
		{
			SM_FFValue(u_caster->SM[SMT_RADIUS][0],&r,GetSpellProto()->SpellGroupType);
			SM_PFValue(u_caster->SM[SMT_RADIUS][1],&r,GetSpellProto()->SpellGroupType);
		}
	}
	float srcx=m_caster->GetPositionX(),srcy=m_caster->GetPositionY(),srcz=m_caster->GetPositionZ();
	for(unordered_set<Object* >::iterator itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); itr++ )
	{
		if( !( (*itr)->IsUnit() ) || !TO_UNIT(*itr)->isAlive() )
			continue;
		if( GetSpellProto()->TargetCreatureType && (*itr)->IsUnit())
		{
			Unit* Target = TO_UNIT((*itr));
			if(!(1<<(Target->GetCreatureType()-1) & GetSpellProto()->TargetCreatureType))
				continue;
		}
		if(IsInrange(srcx,srcy,srcz,(*itr),r))
		{
			if( u_caster != NULL )
			{
				if( isFriendly( u_caster, TO_UNIT(*itr) ) && _DidHit(TO_UNIT(*itr))==SPELL_DID_HIT_SUCCESS)
					return (*itr)->GetGUID(); 
			}
			else //cast from GO
			{
				if(g_caster && g_caster->GetUInt32Value(OBJECT_FIELD_CREATED_BY) && g_caster->m_summoner)
				{
					//trap, check not to attack owner and friendly
					if( isFriendly( g_caster->m_summoner, TO_UNIT(*itr) ) )
						return (*itr)->GetGUID();
				}
			}
		}
	}
	return 0;
}

uint8 Spell::_DidHit(const Unit* target)
{
	//note resistchance is vise versa, is full hit chance
	Unit* u_victim = TO_UNIT(target);
	Player* p_victim = ( target->GetTypeId() == TYPEID_PLAYER ) ? TO_PLAYER(target) : NULLPLR;

	//
	float baseresist[3] = { 4.0f, 5.0f, 6.0f };
	int32 lvldiff;
	float resistchance ;
	if( u_victim == NULL )
		return SPELL_DID_HIT_MISS;


	/************************************************************************/
	/* Can't resist non-unit												*/
	/************************************************************************/
	if(!u_caster)
		return SPELL_DID_HIT_SUCCESS;

	/************************************************************************/
	/* Can't reduce your own spells										 */
	/************************************************************************/
	if(u_caster == u_victim)
		return SPELL_DID_HIT_SUCCESS;

	/************************************************************************/
	/* Check if the unit is evading										 */
	/************************************************************************/
	if(u_victim->GetTypeId()==TYPEID_UNIT && u_victim->GetAIInterface()->getAIState()==STATE_EVADE)
		return SPELL_DID_HIT_EVADE;

	/************************************************************************/
	/* Check if the target is immune to this spell school				   */
	/************************************************************************/
	if(u_victim->SchoolImmunityList[GetSpellProto()->School])
		return SPELL_DID_HIT_IMMUNE;

	/*************************************************************************/
	/* Check if the target is immune to this mechanic						*/
	/*************************************************************************/
	if(u_victim->MechanicsDispels[GetSpellProto()->MechanicsType])
	{
		return SPELL_DID_HIT_IMMUNE; // Moved here from Spell::CanCast
	}

	/************************************************************************/
	/* Check if the target has a % resistance to this mechanic			  */
	/************************************************************************/
	if( GetSpellProto()->MechanicsType < MECHANIC_COUNT)
	{
		float res;
		if(p_victim)
			res = p_victim->MechanicsResistancesPCT[Spell::GetMechanic(m_spellInfo)];
		else
			res = u_victim->MechanicsResistancesPCT[Spell::GetMechanic(m_spellInfo)];
		if( !(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_NOT_RESISTABLE) && Rand(res))
			return SPELL_DID_HIT_RESIST;
	}

	/************************************************************************/
	/* Check if the spell is a melee attack and if it was missed/parried	*/
	/************************************************************************/
	uint32 melee_test_result;
	if( GetSpellProto()->is_melee_spell || GetSpellProto()->is_ranged_spell )
	{
		uint32 _type;
		if( GetType() == SPELL_DMG_TYPE_RANGED )
			_type = RANGED;
		else
		{
			if (GetSpellProto()->Flags4 & FLAGS4_OFFHAND)
				_type =  OFFHAND;
			else
				_type = MELEE;
		}

		melee_test_result = u_caster->GetSpellDidHitResult( u_victim, _type, m_spellInfo );
		if(melee_test_result != SPELL_DID_HIT_SUCCESS)
			return (uint8)melee_test_result;
	}

	/************************************************************************/
	/* Check if the spell is resisted.									  */
	/************************************************************************/
	if( GetSpellProto()->School==0  || GetSpellProto()->is_ranged_spell ) // all ranged spells are physical too...
		return SPELL_DID_HIT_SUCCESS;

	bool pvp =(p_caster && p_victim);

	if(pvp)
		lvldiff = p_victim->getLevel() - p_caster->getLevel();
	else
		lvldiff = u_victim->getLevel() - u_caster->getLevel();
	if (lvldiff < 0)
		resistchance = baseresist[0] +lvldiff;
	else
	{
		if(lvldiff < 3)
			resistchance = baseresist[lvldiff];
		else
		{
			if(pvp)
				resistchance = baseresist[2] + (((float)lvldiff-2.0f)*7.0f);
			else
				resistchance = baseresist[2] + (((float)lvldiff-2.0f)*11.0f);
		}
	}
	if (GetSpellProto()->DispelType < NUM_DISPELS)
		resistchance += u_victim->DispelResistancesPCT[GetSpellProto()->DispelType];
	//rating bonus
	if( p_caster != NULL )
	{
		resistchance -= p_caster->CalcRating( PLAYER_RATING_MODIFIER_SPELL_HIT );
		resistchance -= p_caster->GetHitFromSpell();
	}

	if(p_victim)
		resistchance += p_victim->m_resist_hit[2];

	if( GetSpellProto()->c_is_flags & SPELL_FLAG_IS_DISPEL_SPELL && GetSpellProto()->SpellGroupType && u_caster)
	{
		SM_FFValue(u_caster->SM[SMT_RESIST_DISPEL][0],&resistchance,GetSpellProto()->SpellGroupType);
		SM_PFValue(u_caster->SM[SMT_RESIST_DISPEL][1],&resistchance,GetSpellProto()->SpellGroupType);
	}

	if(GetSpellProto()->SpellGroupType && u_caster)
	{
		float hitchance=0;
		SM_FFValue(u_caster->SM[SMT_HITCHANCE][0],&hitchance,GetSpellProto()->SpellGroupType);
		SM_PFValue(u_caster->SM[SMT_HITCHANCE][1],&hitchance,GetSpellProto()->SpellGroupType);
		resistchance -= hitchance;
	}

	if(IsBinary(m_spellInfo))
	{ // need to apply resistance mitigation
		float mitigation = 1.0f - float (u_caster->GetResistanceReducion(u_victim, GetSpellProto()->School, 0.0f));
		resistchance = 100 - (100 - resistchance) * mitigation; // meaning hitchance * mitigation
	}

	if(resistchance < 1.0f)
		resistchance = 1.0f;

	if (GetSpellProto()->Attributes & ATTRIBUTES_IGNORE_INVULNERABILITY)
		resistchance = 0.0f;

	if(resistchance > 99.0f)
		resistchance = 99.0f;

	if( GetSpellProto()->c_is_flags & SPELL_FLAG_IS_NOT_RESISTABLE )
		resistchance = 0.0f;

	uint32 res = (Rand(resistchance) ? SPELL_DID_HIT_RESIST : SPELL_DID_HIT_SUCCESS);

	// spell reflect handler
	if(res == SPELL_DID_HIT_SUCCESS && Reflect(u_victim) )
		res = SPELL_DID_HIT_REFLECT;

	return res;
}

//generate possible target list for a spell. Use as last resort since it is not acurate
//this function makes a rough estimation for possible target !
//!!!disabled parts that were not tested !!
void Spell::GenerateTargets(SpellCastTargets *store_buff)
{
	float r = GetSpellProto()->base_range_or_radius_sqr;
	if( GetSpellProto()->SpellGroupType && u_caster)
	{
		SM_FFValue(u_caster->SM[SMT_RADIUS][0],&r,GetSpellProto()->SpellGroupType);
		SM_PFValue(u_caster->SM[SMT_RADIUS][1],&r,GetSpellProto()->SpellGroupType);
	}
	uint32 cur;
	for(uint32 i=0;i<3;++i)
		for(uint32 j=0;j<2;j++)
		{
			if(j==0)
				cur = GetSpellProto()->EffectImplicitTargetA[i];
			else // if(j==1)
				cur = GetSpellProto()->EffectImplicitTargetB[i];
			switch(cur)
			{
				case EFF_TARGET_NONE:
					{
					//this is bad for us :(
					}break;
				case EFF_TARGET_SELF:
					{
						if(m_caster->IsUnit())
							store_buff->m_unitTarget = m_caster->GetGUID();
					}break;
					// need more research
				case 4:
					{ // dono related to "Wandering Plague", "Spirit Steal", "Contagion of Rot", "Retching Plague" and "Copy of Wandering Plague"
					}break;
				case EFF_TARGET_PET:
					{// Target: Pet
						if(p_caster && p_caster->GetSummon())
							store_buff->m_unitTarget = p_caster->GetSummon()->GetGUID();
					}break;
				case EFF_TARGET_SINGLE_ENEMY:// Single Target Enemy
				case 77:					// grep: i think this fits
				case 8: // related to Chess Move (DND), Firecrackers, Spotlight, aedm, Spice Mortar
				case EFF_TARGET_ALL_ENEMY_IN_AREA: // All Enemies in Area of Effect (TEST)
				case EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT: // All Enemies in Area of Effect instant (e.g. Flamestrike)
				case EFF_TARGET_ALL_ENEMIES_AROUND_CASTER:
				case EFF_TARGET_IN_FRONT_OF_CASTER:
				case EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED:// All Enemies in Area of Effect(Blizzard/Rain of Fire/volley) channeled
				case 31:// related to scripted effects
				case 53:// Target Area by Players CurrentSelection()
				case 54:// Targets in Front of the Caster
					{
						if( p_caster != NULL )
						{
							Unit* selected = p_caster->GetMapMgr()->GetUnit(p_caster->GetSelection());
							if(isAttackable(p_caster,selected,!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)) && selected != p_caster )
								store_buff->m_unitTarget = p_caster->GetSelection();
						}
						else if( u_caster != NULL )
						{
							if(	u_caster->GetAIInterface()->GetNextTarget() &&
								isAttackable(u_caster,u_caster->GetAIInterface()->GetNextTarget(),!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)) &&
								u_caster->GetDistanceSq(u_caster->GetAIInterface()->GetNextTarget()) <= r)
							{
								store_buff->m_unitTarget = u_caster->GetAIInterface()->GetNextTarget()->GetGUID();
							}
							if(u_caster->GetAIInterface()->getAITargetsCount())
							{
								//try to get most hated creature
								TargetMap *m_aiTargets = u_caster->GetAIInterface()->GetAITargets();
								TargetMap::iterator itr;
								for(itr = m_aiTargets->begin(); itr != m_aiTargets->end();itr++)
								{
									if( /*m_caster->GetMapMgr()->GetUnit(itr->first->GetGUID()) &&*/ itr->first->GetMapMgr() == m_caster->GetMapMgr() &&
										itr->first->isAlive() &&
										m_caster->GetDistanceSq(itr->first) <= r &&
										isAttackable(u_caster,itr->first,!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED))
										)
									{
										store_buff->m_unitTarget=itr->first->GetGUID();
										break;
									}
								}
							}
						}
						//try to get a whatever target
						if(!store_buff->m_unitTarget)
						{
							store_buff->m_unitTarget=GetSinglePossibleEnemy(i);
						}
					}break;
					// spells like 17278:Cannon Fire and 21117:Summon Son of Flame A
				case 17: // A single target at a xyz location or the target is a possition xyz
				case 18:// Land under caster.Maybe not correct
					{
						store_buff->m_srcX=m_caster->GetPositionX();
						store_buff->m_srcY=m_caster->GetPositionY();
						store_buff->m_srcZ=m_caster->GetPositionZ();
						store_buff->m_targetMask |= TARGET_FLAG_SOURCE_LOCATION;
					}break;
				case EFF_TARGET_ALL_PARTY_AROUND_CASTER:
					{// All Party Members around the Caster in given range NOT RAID!
						Player* p = p_caster;
						if( p == NULL)
						{
							if( TO_CREATURE( u_caster )->IsTotem() )
								p = TO_PLAYER( TO_CREATURE(u_caster)->GetSummonOwner());
						}
						if( p == NULL )
							break;

						if(IsInrange(m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),p,r))
						{
							store_buff->m_unitTarget = m_caster->GetGUID();
							break;
						}
						SubGroup * subgroup = p->GetGroup() ?
							p->GetGroup()->GetSubGroup(p->GetSubGroup()) : 0;

						if(subgroup)
						{
							p->GetGroup()->Lock();
							for(GroupMembersSet::iterator itr = subgroup->GetGroupMembersBegin(); itr != subgroup->GetGroupMembersEnd(); itr++)
							{
								if(!(*itr)->m_loggedInPlayer || m_caster == (*itr)->m_loggedInPlayer)
									continue;
								if(IsInrange(m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),(*itr)->m_loggedInPlayer,r))
								{
									store_buff->m_unitTarget = (*itr)->m_loggedInPlayer->GetGUID();
									break;
								}
							}
							p->GetGroup()->Unlock();
						}
					}break;
				case EFF_TARGET_SINGLE_FRIEND:
				case 45:// Chain,!!only for healing!! for chain lightning =6
				case 57:// Targeted Party Member
					{// Single Target Friend
						if( p_caster != NULL )
						{
							if(isFriendly(p_caster,p_caster->GetMapMgr()->GetUnit(p_caster->GetSelection())))
								store_buff->m_unitTarget = p_caster->GetSelection();
							else store_buff->m_unitTarget = p_caster->GetGUID();
						}
						else if( u_caster != NULL )
						{
							if(u_caster->GetUInt64Value(UNIT_FIELD_CREATEDBY))
								store_buff->m_unitTarget = u_caster->GetUInt64Value(UNIT_FIELD_CREATEDBY);
							else store_buff->m_unitTarget = u_caster->GetGUID();
						}
						else store_buff->m_unitTarget=GetSinglePossibleFriend(i,r);
					}break;
				case EFF_TARGET_GAMEOBJECT:
					{
						if(p_caster && p_caster->GetSelection())
							store_buff->m_unitTarget = p_caster->GetSelection();
					}break;
				case EFF_TARGET_DUEL:
					{// Single Target Friend Used in Duel
						if(p_caster && p_caster->DuelingWith != NULL && p_caster->DuelingWith->isAlive() && IsInrange(p_caster,p_caster->DuelingWith,r))
							store_buff->m_unitTarget = p_caster->GetSelection();
					}break;
				case EFF_TARGET_GAMEOBJECT_ITEM:{// Gameobject/Item Target
						//shit
					}break;
				case 27:{ // target is owner of pet
					// please correct this if not correct does the caster variablen need a Pet caster variable?
						if(u_caster && u_caster->IsPet())
							store_buff->m_unitTarget = TO_PET( u_caster )->GetPetOwner()->GetGUID();
					}break;
				case EFF_TARGET_MINION:
				case 73:
					{// Minion Target
						if(m_caster->GetUInt64Value(UNIT_FIELD_SUMMON) == 0)
							store_buff->m_unitTarget = m_caster->GetGUID();
						else store_buff->m_unitTarget = m_caster->GetUInt64Value(UNIT_FIELD_SUMMON);
					}break;
				case 33://Party members of totem, inside given range
				case EFF_TARGET_SINGLE_PARTY:// Single Target Party Member
				case EFF_TARGET_ALL_PARTY: // all Members of the targets party
					{
						Player* p=NULLPLR;
						if( p_caster != NULL )
								p = p_caster;
						else if( u_caster && u_caster->GetTypeId() == TYPEID_UNIT && TO_CREATURE( u_caster )->IsTotem() )
								p = TO_PLAYER( TO_CREATURE(u_caster)->GetSummonOwner());
						if( p_caster != NULL )
						{
							if(IsInrange(m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),p,r))
							{
								store_buff->m_unitTarget = p->GetGUID();
								break;
							}
							SubGroup * pGroup = p_caster->GetGroup() ?
								p_caster->GetGroup()->GetSubGroup(p_caster->GetSubGroup()) : 0;

							if( pGroup )
							{
								p_caster->GetGroup()->Lock();
								for(GroupMembersSet::iterator itr = pGroup->GetGroupMembersBegin();
									itr != pGroup->GetGroupMembersEnd(); itr++)
								{
									if(!(*itr)->m_loggedInPlayer || p == (*itr)->m_loggedInPlayer)
										continue;
									if(IsInrange(m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),(*itr)->m_loggedInPlayer,r))
									{
										store_buff->m_unitTarget = (*itr)->m_loggedInPlayer->GetGUID();
										break;
									}
								}
								p_caster->GetGroup()->Unlock();
							}
						}
					}break;
				case 38:{//Dummy Target
					//have no idea
					}break;
				case EFF_TARGET_SELF_FISHING://Fishing
				case 46://Unknown Summon Atal'ai Skeleton
				case 47:// Portal
				case 52:	// Lightwells, etc
					{
						store_buff->m_unitTarget = m_caster->GetGUID();
					}break;
				case 40://Activate Object target(probably based on focus)
				case EFF_TARGET_TOTEM_EARTH:
				case EFF_TARGET_TOTEM_WATER:
				case EFF_TARGET_TOTEM_AIR:
				case EFF_TARGET_TOTEM_FIRE:// Totem
					{
						if( u_caster != NULL )
						{
							SummonPropertiesEntry* summonprop = dbcSummonProps.LookupEntryForced( GetSpellProto()->EffectMiscValueB[i] );
							if(!summonprop)
								return;
							uint32 slot = summonprop->slot;

							if(u_caster->m_SummonSlots[slot] != NULL )
								store_buff->m_unitTarget = u_caster->m_SummonSlots[slot]->GetGUID();
						}
					}break;
				case 61:{ // targets with the same group/raid and the same class
						if( p_caster != NULL )
						{
							SubGroup * pGroup = p_caster->GetGroup() ?
								p_caster->GetGroup()->GetSubGroup(p_caster->GetSubGroup()) : 0;

							if( pGroup )
							{
								p_caster->GetGroup()->Lock();
								for(GroupMembersSet::iterator itr = pGroup->GetGroupMembersBegin();
									itr != pGroup->GetGroupMembersEnd(); itr++)
								{
									if(!(*itr)->m_loggedInPlayer)
										continue;
									if(IsInrange(m_caster->GetPositionX(),m_caster->GetPositionY(),m_caster->GetPositionZ(),(*itr)->m_loggedInPlayer,r) && (*itr)->m_loggedInPlayer->getClass() == p_caster->getClass())
									{
										store_buff->m_unitTarget = (*itr)->m_loggedInPlayer->GetGUID();
										break;
									}
								}
								p_caster->GetGroup()->Unlock();
							}
						}
				}break;
				case EFF_TARGET_ALL_FRIENDLY_IN_AREA:{

				}break;

			}//end switch
		}//end for
	if(store_buff->m_unitTarget)
		store_buff->m_targetMask |= TARGET_FLAG_UNIT;
	if(store_buff->m_srcX)
		store_buff->m_targetMask |= TARGET_FLAG_SOURCE_LOCATION;
	if(store_buff->m_destX)
		store_buff->m_targetMask |= TARGET_FLAG_DEST_LOCATION;
}//end function

uint8 Spell::prepare( SpellCastTargets * targets )
{
	uint8 ccr;
	chaindamage = 0;

	// check for current difficulty an change spellentry if needed
	if( u_caster && !p_caster && u_caster->GetMapMgr() && u_caster->GetMapMgr()->pInstance && GetSpellProto()->SpellDifficulty )
	{
		uint32 spellid = GetDifficultySpell(GetSpellProto(), u_caster->GetMapMgr()->iInstanceMode);
		if( spellid != GetSpellProto()->Id && spellid != 0 )
			m_spellInfo = dbcSpell.LookupEntry( spellid );
	}

	if( p_caster && GetSpellProto()->Id == 51514 || GetSpellProto()->NameHash == SPELL_HASH_ARCANE_SHOT || GetSpellProto()->NameHash == SPELL_HASH_MIND_FLAY )
	{
		targets->m_unitTarget = 0;
		GenerateTargets( targets );
	}
	if(UseMissileDelay())
	{
		m_missileTravelTime = (uint32)m_targets.traveltime;
		m_missilePitch = m_targets.missilepitch;
	}
	m_targets = *targets;

	if( !m_triggeredSpell && p_caster != NULL && p_caster->CastTimeCheat )
		m_castTime = 0;
	else
	{
		if(dbcSpellCastTime.LookupEntry( GetSpellProto()->CastingTimeIndex ))
			m_castTime = GetCastTime( dbcSpellCastTime.LookupEntry( GetSpellProto()->CastingTimeIndex ) );
		else
			m_castTime = 0;

		if( m_castTime && GetSpellProto()->SpellGroupType && u_caster != NULL )
		{
			SM_FIValue( u_caster->SM[SMT_CAST_TIME][0], (int32*)&m_castTime, GetSpellProto()->SpellGroupType );
			SM_PIValue( u_caster->SM[SMT_CAST_TIME][1], (int32*)&m_castTime, GetSpellProto()->SpellGroupType );
		}

		// handle MOD_CAST_TIME
		if( u_caster != NULL && m_castTime )
			m_castTime = float2int32( m_castTime * u_caster->GetFloatValue( UNIT_MOD_CAST_SPEED ) );
	}

	uint8 forced_cancast_failure = 0;
	if( u_caster != NULL )
	{
		if( GetGameObjectTarget() || GetSpellProto()->Id == 21651)
		{
			if( u_caster->InStealth() )
				u_caster->RemoveAura( u_caster->m_stealth );

			if( (GetSpellProto()->Effect[0] == SPELL_EFFECT_OPEN_LOCK ||
				GetSpellProto()->Effect[1] == SPELL_EFFECT_OPEN_LOCK ||
				GetSpellProto()->Effect[2] == SPELL_EFFECT_OPEN_LOCK) &&
				p_caster != NULL && p_caster->m_bgFlagIneligible)
				forced_cancast_failure = SPELL_FAILED_BAD_TARGETS;
		}
	}

	//let us make sure cast_time is within decent range
	//this is a hax but there is no spell that has more then 10 minutes cast time
	if( m_castTime < 0 )
		m_castTime = 0;
	else if( m_castTime > 60 * 10 * 1000)
		m_castTime = 60 * 10 * 1000; //we should limit cast time to 10 minutes right ?

	m_timer = m_castTime;

	if( !m_triggeredSpell && p_caster != NULL && p_caster->CooldownCheat )
		p_caster->ClearCooldownForSpell( GetSpellProto()->Id );

	if( m_triggeredSpell )
		cancastresult = SPELL_CANCAST_OK;
	else
		cancastresult = CanCast(false);

	//sLog.outString( "CanCast result: %u. Refer to SpellFailure.h to work out why." , cancastresult );

	ccr = cancastresult;

	if( forced_cancast_failure )
		cancastresult = forced_cancast_failure;

	if( cancastresult != SPELL_CANCAST_OK )
	{
		SendCastResult( cancastresult );

		if( m_triggeredByAura )
		{
			SendChannelUpdate( 0 );
			if( u_caster != NULL )
				u_caster->RemoveAura( m_triggeredByAura );
		}
		else
		{
			// HACK, real problem is the way spells are handled
			// when a spell is channeling and a new spell is casted
			// that is a channeling spell, but not triggert by a aura
			// the channel bar/spell is bugged
			if( u_caster && u_caster->GetUInt64Value( UNIT_FIELD_CHANNEL_OBJECT) > 0 && u_caster->GetCurrentSpell() )
			{
				u_caster->GetCurrentSpell()->cancel();
				SendChannelUpdate( 0 );
				cancel();
				return ccr;
			}
		}
		finish();
		return ccr;
	}
	else if( !m_triggeredSpell )
	{
		if( !HasPower() )
		{
			SendCastResult(SPELL_FAILED_NO_POWER);
			// in case we're out of sync
			if( p_caster )
				u_caster->SendPowerUpdate();

			return SPELL_FAILED_NO_POWER;
		}
		SendSpellStart();

		// start cooldown handler
		if( p_caster != NULL && !p_caster->CastTimeCheat )
			AddStartCooldown();

		if( i_caster == NULL )
		{
			if( p_caster != NULL && m_timer > 0 )
				p_caster->delayAttackTimer( m_timer + 1000 );
		}

		// aura state removal
		if( GetSpellProto()->CasterAuraState && GetSpellProto()->CasterAuraState != AURASTATE_FLAG_JUDGEMENT )
			u_caster->RemoveFlag( UNIT_FIELD_AURASTATE, GetSpellProto()->CasterAuraState );
	}

	m_spellState = SPELL_STATE_PREPARING;

	//instant cast(or triggered) and not channeling
	if( u_caster != NULL && ( m_castTime > 0 || GetSpellProto()->ChannelInterruptFlags ) && !m_triggeredSpell )
	{
		m_castPositionX = m_caster->GetPositionX();
		m_castPositionY = m_caster->GetPositionY();
		m_castPositionZ = m_caster->GetPositionZ();
		u_caster->CastSpell( this );
	}
	else
		cast( false );

	return ccr;
}

void Spell::cancel()
{
	SendInterrupted(0);
	SendCastResult(SPELL_FAILED_INTERRUPTED);

	if(m_spellState == SPELL_STATE_CASTING)
	{
		if( u_caster != NULL )
			u_caster->RemoveAura(GetSpellProto()->Id);

		if(m_timer > 0 || m_Delayed)
		{
			if(p_caster && p_caster->IsInWorld() || v_caster && v_caster->IsInWorld())
			{
				Unit* pTarget = p_caster->GetMapMgr()->GetUnit(m_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT));
				if(!pTarget)
					pTarget = p_caster->GetMapMgr()->GetUnit(p_caster->GetSelection());

				if(pTarget)
					pTarget->RemoveAura(GetSpellProto()->Id, m_caster->GetGUID());

				if(m_AreaAura)//remove of blizz and shit like this
				{
					DynamicObject* dynObj = m_caster->GetMapMgr()->GetDynamicObject(m_caster->GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT));
					if(dynObj)
					{
						dynObj->RemoveFromWorld(true);
						delete dynObj;
						dynObj = NULLOBJ;
					}
				}

				if(p_caster && p_caster->GetSummonedObject())
				{
					if(p_caster->GetSummonedObject()->IsInWorld())
						p_caster->GetSummonedObject()->RemoveFromWorld(true);
					// for now..
					ASSERT(p_caster->GetSummonedObject()->GetTypeId() == TYPEID_GAMEOBJECT);
					delete TO_GAMEOBJECT(p_caster->GetSummonedObject());
					p_caster->SetSummonedObject(NULL);
				}
				if (p_caster &&  m_timer > 0)
					p_caster->delayAttackTimer(-m_timer);
			 }
		}
		SendChannelUpdate(0);
	}
	// Ensure the item gets consumed once the channel has started
	if (m_timer > 0)
		m_ForceConsumption = true;

	if( !m_isCasting )
		finish();
}

void Spell::AddCooldown()
{
	if( p_caster != NULL && !p_caster->CooldownCheat)
		p_caster->Cooldown_Add( m_spellInfo, i_caster );
}

void Spell::AddStartCooldown()
{
	if( p_caster != NULL && !p_caster->CastTimeCheat)
		p_caster->Cooldown_AddStart( m_spellInfo );
}

void Spell::cast(bool check)
{
	if( duelSpell && (
		( p_caster != NULL && p_caster->GetDuelState() != DUEL_STATE_STARTED ) ||
		( u_caster != NULL && u_caster->IsPet() && TO_PET( u_caster )->GetPetOwner() && TO_PET( u_caster )->GetPetOwner()->GetDuelState() != DUEL_STATE_STARTED ) ) )
	{
		// Can't cast that!
		SendInterrupted( SPELL_FAILED_TARGET_FRIENDLY );
		finish();
		return;
	}

	DEBUG_LOG("Spell","Cast %u, Unit: %u", GetSpellProto()->Id, m_caster->GetLowGUID());

	if(u_caster != NULL )
		if(u_caster->CallOnCastSpell != NULL)
			u_caster->CallOnCastSpell->UnitOnCastSpell(u_caster, m_spellInfo);

	if(check)
		cancastresult = CanCast(true);
	else
		cancastresult = SPELL_CANCAST_OK;

	if(cancastresult == SPELL_CANCAST_OK)
	{
		if (p_caster && !m_triggeredSpell && p_caster->IsInWorld() && GET_TYPE_FROM_GUID(m_targets.m_unitTarget) == HIGHGUID_TYPE_CREATURE)
		{
			sQuestMgr.OnPlayerCast(p_caster,GetSpellProto()->Id,m_targets.m_unitTarget);
		}

		// trigger on next attack
		if( GetSpellProto()->Attributes & ATTRIBUTE_ON_NEXT_ATTACK && !m_triggeredSpell )
		{
			// check power
			if(!HasPower())
			{
				SendInterrupted(SPELL_FAILED_NO_POWER);
				SendCastResult(SPELL_FAILED_NO_POWER);
				finish();
				return;
			}

			// we're much better to remove this here, because otherwise spells that change powers etc,
			// don't get applied.

			u_caster->RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_CAST_SPELL, GetSpellProto()->Id);

			m_isCasting = false;
			SendCastResult(cancastresult);
			if( u_caster != NULL )
				u_caster->SetOnMeleeSpell(GetSpellProto()->Id, extra_cast_number);

			finish();
			return;
		}

		if( !m_projectileWait )
		{
			if(!TakePower() && !m_triggeredSpell) //not enough mana
			{
				SendInterrupted(SPELL_FAILED_NO_POWER);
				SendCastResult(SPELL_FAILED_NO_POWER);
				finish();
				return;
			}

			for(uint32 i=0;i<3;++i)
			{
				if( GetSpellProto()->Effect[i])
					 FillTargetMap(i);
			}

			SendCastResult(cancastresult);
			if(cancastresult != SPELL_CANCAST_OK)
			{
				finish();
				return;
			}

			if(m_magnetTarget){ // Spell was redirected
				// Grounding Totem gets destroyed after redirecting 1 spell
				if ( m_magnetTarget && m_magnetTarget->IsCreature()){
					Creature* MagnetCreature = TO_CREATURE(m_magnetTarget);
					if(MagnetCreature->IsTotem()){
						sEventMgr.ModifyEventTimeLeft(MagnetCreature, EVENT_TOTEM_EXPIRE, 0);
					}
				}
			}

			if(p_caster && GetSpellProto()->c_is_flags & SPELL_FLAG_ON_ONLY_ONE_TARGET &&
			   m_targetList.size() == 1)
			{
				SpellTargetList::iterator itr = m_targetList.begin();
				p_caster->CheckSpellUniqueTargets(m_spellInfo, itr->Guid);
			}

			m_isCasting = true;

			//sLog.outString( "CanCastResult: %u" , cancastresult );
			if(!m_triggeredSpell)
				AddCooldown();

			if( u_caster )
			{
				if (u_caster->InStealth() && !(GetSpellProto()->AttributesEx & ATTRIBUTESEX_NOT_BREAK_STEALTH) && !m_triggeredSpell)
					u_caster->RemoveStealth();
			}


			if( p_caster != NULL )
			{
				if( GetSpellProto()->NameHash == SPELL_HASH_SLAM)
				{
					/* slam - reset attack timer */
					p_caster->setAttackTimer( 0, true );
					p_caster->setAttackTimer( 0, false );
				}

				// Arathi Basin opening spell, remove stealth, invisibility, etc.
				// hacky but haven't found a better way that works
				// Note: Same stuff but for picking flags is over AddAura
				if (p_caster->m_bg && GetSpellProto()->Id == 21651)
				{
					p_caster->RemoveStealth();
					p_caster->RemoveInvisibility();
					p_caster->RemoveAllAuraByNameHash(SPELL_HASH_ICE_BLOCK);
					p_caster->RemoveAllAuraByNameHash(SPELL_HASH_DIVINE_SHIELD);
					p_caster->RemoveAllAuraByNameHash(SPELL_HASH_BLESSING_OF_PROTECTION);
				}
			}

			SendSpellGo();

			//******************** SHOOT SPELLS ***********************
			//* Flags are now 1,4,19,22 (4718610) //0x480012

			if(GetSpellProto()->Flags4 & 0x8000 && (p_caster != NULL) && m_caster->IsInWorld())
			{
				/// Part of this function contains a hack fix
				/// hack fix for shoot spells, should be some other resource for it
				//p_caster->SendSpellCoolDown(GetSpellProto()->Id, GetSpellProto()->RecoveryTime ? GetSpellProto()->RecoveryTime : 2300);
				WorldPacket data(SMSG_SPELL_COOLDOWN, 14);
				data << GetSpellProto()->Id;
				data << m_caster->GetNewGUID();
				data << uint32(GetSpellProto()->RecoveryTime ? GetSpellProto()->RecoveryTime : 2300);
				p_caster->GetSession()->SendPacket(&data);
			}

			// Note: [Warlock] Immolation Aura somehow has these flags, but it's not channeled
			if( GetSpellProto()->ChannelInterruptFlags != 0 && !m_triggeredSpell && GetSpellProto()->Id != 50589 )
			{
				/*
				Channeled spells are handled a little differently. The five second rule starts when the spell's channeling starts; i.e. when you pay the mana for it.
				The rule continues for at least five seconds, and longer if the spell is channeled for more than five seconds. For example,
				Mind Flay channels for 3 seconds and interrupts your regeneration for 5 seconds, while Tranquility channels for 10 seconds
				and interrupts your regeneration for the full 10 seconds.
				*/

				int32 channelDuration = GetDuration();
				m_spellState = SPELL_STATE_CASTING;
				SendChannelStart(channelDuration);
				if( p_caster != NULL )
				{
					//Use channel interrupt flags here
					if(m_targets.m_targetMask == TARGET_FLAG_DEST_LOCATION || m_targets.m_targetMask == TARGET_FLAG_SOURCE_LOCATION)
						u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, p_caster->GetSelection());
					else if(p_caster->GetSelection() == m_caster->GetGUID())
					{
						if(p_caster->GetSummon())
							u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, p_caster->GetSummon()->GetGUID());
						else if(m_targets.m_unitTarget)
							u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, m_targets.m_unitTarget);
						else
							u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, p_caster->GetSelection());
					}
					else
					{
						if(p_caster->GetSelection())
						{
							if( GetSpellProto()->Id == 60931 )
								u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, p_caster->GetGUID());
							else
								u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, p_caster->GetSelection());
						}
						else if(p_caster->GetSummon())
							u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, p_caster->GetSummon()->GetGUID());
						else if(m_targets.m_unitTarget)
							u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, m_targets.m_unitTarget);
						else
						{
							m_isCasting = false;
							cancel();
							return;
						}
					}
				}
				if(u_caster && u_caster->GetPowerType()==POWER_TYPE_MANA)
				{
					if(channelDuration <= 5000)
						u_caster->DelayPowerRegeneration(5000);
					else
						u_caster->DelayPowerRegeneration(channelDuration);
				}

				if( u_caster != NULL )
					u_caster->SetCurrentSpell(this);
			}
			if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
			{
				float dist = 0.0f;

				if(m_missileTravelTime)
					sEventMgr.AddEvent(this, &Spell::HandleDestLocationHit, EVENT_SPELL_HIT, m_missileTravelTime, 1, 0);
				else
				{
					if(GetSpellProto()->speed <= 0 || !UseMissileDelay())
						HandleDestLocationHit();
					else
					{
						dist = m_caster->CalcDistance(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ);
						float time = ((dist*1000.0f)/GetSpellProto()->speed);
						if(time <= 100)
							HandleDestLocationHit();
						else
							sEventMgr.AddEvent(this, &Spell::HandleDestLocationHit, EVENT_SPELL_HIT, float2int32(time), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
					}
				}
			}
		}

		if (p_caster)
		{
			if (m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM)
			{
				Player* plr = p_caster->GetTradeTarget();
				if(plr!=NULL)
					itemTarget = plr->getTradeItem((uint32)m_targets.m_itemTarget);
			} else
			if (m_targets.m_targetMask & TARGET_FLAG_ITEM)
				itemTarget = p_caster->GetItemInterface()->GetItemByGUID(m_targets.m_itemTarget);
		}

		// if the spell is not reflected
		uint32 x;
		bool effects_done[3];
		SpellTargetList::iterator itr = m_targetList.begin();
		effects_done[0] = effects_done[1] = effects_done[2] = false;

		for(; itr != m_targetList.end(); itr++)
		{
			if( itr->HitResult != SPELL_DID_HIT_SUCCESS )
				continue;

			// set target pointers
			_SetTargets(itr->Guid);

			// call effect handlers
			for( x = 0; x < 3; ++x )
			{
				if(!CanHandleSpellEffect(x, m_spellInfo->NameHash))
					continue;
				//Don't handle effect now
				if(GetSpellProto()->Effect[x] == SPELL_EFFECT_SUMMON)
				{
					effects_done[x] = false;
					continue;
				}
				else if(itr->EffectMask & (1 << x))
				{
					HandleEffects(x);
					effects_done[x] = true;
				}
			}

			// handle the rest of shit
			if( unitTarget != NULL )
			{
				// aura state
				if( GetSpellProto()->TargetAuraState )
					unitTarget->RemoveFlag(UNIT_FIELD_AURASTATE, uint32(1) << (GetSpellProto()->TargetAuraState - 1) );

				// proc!
				if(!m_triggeredSpell)
				{
					if( u_caster != NULL && u_caster->IsInWorld() )
					{
							u_caster->HandleProc(PROC_ON_CAST_SPECIFIC_SPELL | PROC_ON_CAST_SPELL, NULL, unitTarget, m_spellInfo);
							u_caster->m_procCounter = 0; //this is required for to be able to count the depth of procs (though i have no idea where/why we use proc on proc)
					}
				}
				if( unitTarget != NULL && unitTarget->IsInWorld() )
				{
					unitTarget->HandleProc(PROC_ON_SPELL_LAND_VICTIM, NULL, u_caster, m_spellInfo);
					unitTarget->m_procCounter = 0; //this is required for to be able to count the depth of procs (though i have no idea where/why we use proc on proc)
				}
			}
		}

		//Handle remaining effects for which we did not find targets.
		for( x = 0; x < 3; ++x )
		{
			if(!CanHandleSpellEffect(x, m_spellInfo->NameHash))
				continue;
			if(!effects_done[x])
			{
				switch (GetSpellProto()->Effect[x])
				{
					// Target ourself for these effects
				case SPELL_EFFECT_TRIGGER_SPELL:
				case SPELL_EFFECT_SUMMON:
					{
						_SetTargets(m_caster->GetGUID());
						HandleEffects(x);
					}break;

					// No Target required for these effects
				case SPELL_EFFECT_PERSISTENT_AREA_AURA:
					{
						HandleEffects(x);
					}break;
				}
			}
		}

		/* don't call HandleAddAura unless we actually have auras... - Burlex*/
		if( GetSpellProto()->EffectApplyAuraName[0] != 0 || GetSpellProto()->EffectApplyAuraName[1] != 0 || GetSpellProto()->EffectApplyAuraName[2] != 0)
		{
			itr = m_targetList.begin();
			for(; itr != m_targetList.end(); itr++)
			{
				if( itr->HitResult != SPELL_DID_HIT_SUCCESS )
					continue;

				HandleAddAura(itr->Guid);
			}
		}

		if( u_caster )
			u_caster->SendPowerUpdate();

		// we're much better to remove this here, because otherwise spells that change powers etc,
		// don't get applied.
		if( u_caster && !m_projectileWait )
			u_caster->RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_CAST_SPELL, GetSpellProto()->Id);

		//i don't think that's good idea
		switch( GetSpellProto()->NameHash )
		{
		case SPELL_HASH_FIREBALL:
			{
				if( u_caster )
					u_caster->RemoveAura( 57761 );
			}break;
		case SPELL_HASH_PYROBLAST:
			{
				if( u_caster )
					u_caster->RemoveAura( 48108 );
			}break;
		case SPELL_HASH_GLYPH_OF_ICE_BLOCK:
			{
				if( p_caster && p_caster->HasDummyAura(SPELL_HASH_GLYPH_OF_ICE_BLOCK) )
				{
					//frost nova
					p_caster->ClearCooldownForSpell( 122 );
					p_caster->ClearCooldownForSpell( 865 );
					p_caster->ClearCooldownForSpell( 6131 );
					p_caster->ClearCooldownForSpell( 10230 );
					p_caster->ClearCooldownForSpell( 27088 );
					p_caster->ClearCooldownForSpell( 42917 );
				}
			}break;
		case SPELL_HASH_GLYPH_OF_ICY_VEINS:
			{
				if( u_caster && u_caster->HasDummyAura(SPELL_HASH_GLYPH_OF_ICY_VEINS) )
				{
					Aura* pAura = NULL;
					for(uint32 i = MAX_POSITIVE_AURAS; i < MAX_AURAS; i++)
					{
						pAura = u_caster->m_auras[i];
						if( pAura != NULL && !pAura->IsPositive() )
						{
							for(uint32 j = 0; j < 3; ++j)
							{
								if( pAura->GetSpellProto()->EffectApplyAuraName[j] == SPELL_AURA_MOD_DECREASE_SPEED ||
									pAura->GetSpellProto()->EffectApplyAuraName[j] == SPELL_AURA_MOD_CASTING_SPEED )
								{
									u_caster->RemoveAuraBySlot(i);
									break;
								}
							}
						}
					}
				}
			}break;
		case SPELL_HASH_HAND_OF_FREEDOM:
			{
				if( u_caster && u_caster->HasDummyAura(SPELL_HASH_DIVINE_PURPOSE) )
				{
					if( Rand( u_caster->GetDummyAura(SPELL_HASH_DIVINE_PURPOSE)->RankNumber * 50 ) )
					{
						Unit* u_target = u_caster->GetMapMgr()->GetUnit(m_targets.m_unitTarget);
						if( u_target )
						{
							Aura* pAura;
							for(uint32 i = MAX_POSITIVE_AURAS; i < MAX_AURAS; i++)
							{
								pAura = u_target->m_auras[i];
								if( pAura != NULL )
								{
									for(uint32 j = 0; j < 3; ++j)
									{
										if( Spell::GetMechanic(pAura->GetSpellProto()) == MECHANIC_STUNNED )
										{
											u_caster->RemoveAuraBySlot(i);
											break;
										}
									}
								}
							}
						}
					}
				}
			}break;
		case SPELL_HASH_TIGER_S_FURY:
			{
				if( p_caster && p_caster->HasDummyAura(SPELL_HASH_KING_OF_THE_JUNGLE) )
				{
					SpellEntry * spellInfo = dbcSpell.LookupEntry( 51178 );
					if( spellInfo )
					{
						Spell* spell = NULLSPELL;
						spell = (new Spell(p_caster, spellInfo ,true, NULLAURA));
						spell->forced_basepoints[0] = p_caster->GetDummyAura(SPELL_HASH_KING_OF_THE_JUNGLE)->RankNumber * 20;
						SpellCastTargets targets(p_caster->GetGUID());
						spell->prepare(&targets);
					}
				}
			}break;
		case SPELL_HASH_EARTHBIND:
			{
				if( GetSpellProto()->Id == 3600 && u_caster && u_caster->IsCreature() && TO_CREATURE( u_caster )->IsTotem() )
				{
					Player * p_totemOwner = NULL;
					p_totemOwner = TO_PLAYER( TO_CREATURE(u_caster)->GetSummonOwner());
					if( p_totemOwner != NULL && p_totemOwner->HasDummyAura(SPELL_HASH_EARTHEN_POWER) )
					{
						if( Rand( p_totemOwner->GetDummyAura(SPELL_HASH_EARTHEN_POWER)->RankNumber * 50 ) )
						{
							SpellEntry* totemSpell = dbcSpell.LookupEntryForced( 59566 );
							Spell* pSpell = NULLSPELL;
							pSpell = (new Spell(u_caster, totemSpell, true, NULLAURA));
							SpellCastTargets targets;
							pSpell->GenerateTargets( &targets );
							pSpell->prepare(&targets);
						}
					}
				}
			}break;
		}

		if( p_caster && p_caster->HasDummyAura(SPELL_HASH_DEADLY_BREW) &&
			GetSpellProto()->poison_type && (
			GetSpellProto()->poison_type == POISON_TYPE_WOUND ||
			GetSpellProto()->poison_type == POISON_TYPE_INSTANT ||
			GetSpellProto()->poison_type == POISON_TYPE_MIND_NUMBING
			))
		{
			Unit* u_target = p_caster->GetMapMgr()->GetUnit(m_targets.m_unitTarget);
			if( u_target && u_target != u_caster && isAttackable( u_target, u_caster ) )
			{
				uint32 chance = p_caster->GetDummyAura(SPELL_HASH_DEADLY_BREW)->RankNumber == 1 ? 50 : 100;
				if( Rand( chance ) )
				{
					//apply Crippling poison
					SpellEntry * tmpsp = NULL;
					tmpsp = dbcSpell.LookupEntry(25809);
					if(tmpsp != NULL)
						sEventMgr.AddEvent(u_caster, &Unit::EventCastSpell, u_target, tmpsp, EVENT_AURA_APPLY, 250, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
				}
			}
		}

		m_isCasting = false;

		if(m_spellState != SPELL_STATE_CASTING)
			finish();
	}
	else
	{
		// cancast failed
		SendCastResult(cancastresult);
		finish();
	}
}

void Spell::HandleDestLocationHit()
{
	SpellTargetList::iterator i;
	for(uint32 x = 0; x < 3; ++x)
	{
		FillTargetMap(x);
		// check if we actualy have a effect
		if(GetSpellProto()->Effect[x])
		{
			bool hit = false;
			if (m_orderedObjects.size()>0)
			{
				for(std::vector<uint64>::iterator itr = m_orderedObjects.begin(); itr != m_orderedObjects.end(); itr++)
				{
					for(SpellTargetList::iterator itr2 = m_targetList.begin(); itr2 != m_targetList.end(); itr2++)
					{
						if((*itr2).Guid == *itr && ((*itr2).EffectMask & (1 << x)) && (m_caster && ((*itr2).Guid != m_caster->GetGUID())))
						{
							if((*itr2).Guid == m_caster->GetGUID() || TO_OBJECT((*itr))->GetGUID() == m_caster->GetGUID())
								continue;
							if(!CanHandleSpellEffect(x, m_spellInfo->NameHash))
								continue;
							HandleEffects(x);
							hit = true;
						}
					}
				}
			}
		}
	}
}

void Spell::AddTime(uint32 type)
{
	if(u_caster && u_caster->IsPlayer())
	{
		if( GetSpellProto()->InterruptFlags & CAST_INTERRUPT_ON_DAMAGE_TAKEN)
		{
			cancel();
			return;
		}
		if( GetSpellProto()->SpellGroupType && u_caster)
		{
			float ch=0;
			SM_FFValue(u_caster->SM[SMT_NONINTERRUPT][1],&ch,GetSpellProto()->SpellGroupType);
			if(Rand(ch))
				return;
		}
		if( p_caster != NULL )
		{
			if(Rand(p_caster->SpellDelayResist[type]))
				return;
		}
		if(m_pushbackCount > 1)
			return;
		m_pushbackCount++;
		if(m_spellState==SPELL_STATE_PREPARING)
		{
			int32 delay = 500;
			m_timer+=delay;
			if(m_timer>m_castTime)
			{
				delay -= (m_timer - m_castTime);
				m_timer=m_castTime;
				if(delay<0)
					delay = 1;
			}

			WorldPacket data(SMSG_SPELL_DELAYED, 13);
			data << u_caster->GetNewGUID();
			data << uint32(delay);
			u_caster->SendMessageToSet(&data, true);

			if(!p_caster)
			{
				if(m_caster->GetTypeId() == TYPEID_UNIT)
					u_caster->GetAIInterface()->AddStopTime(delay);
			}
			//in case cast is delayed, make sure we do not exit combat
			else
			{
//				sEventMgr.ModifyEventTimeLeft(p_caster,EVENT_ATTACK_TIMEOUT,PLAYER_ATTACK_TIMEOUT_INTERVAL,true);
				// also add a new delay to offhand and main hand attacks to avoid cutting the cast short
				p_caster->delayAttackTimer(delay);
			}
		}
		else if( GetSpellProto()->ChannelInterruptFlags != 48140)
		{
			int32 delay = GetDuration()/4;
			m_timer-=delay;
			if(m_timer<0)
				m_timer=0;
			else
				p_caster->delayAttackTimer(-delay);

			m_Delayed = true;
			if(m_timer>0)
				SendChannelUpdate(m_timer);
		}
	}
}

void Spell::update(uint32 difftime)
{
	// ffs stealth capping
	if( p_caster && GetGameObjectTarget() && p_caster->InStealth() )
	{
		p_caster->RemoveAura( p_caster->m_stealth );
	}

	// skip cast if we're more than 2/3 of the way through
	if(
		(((float)m_castTime / 1.5f) > (float)m_timer ) &&
//		float(m_castTime)/float(m_timer) >= 2.0f		&&
		(
		m_castPositionX != m_caster->GetPositionX() ||
		m_castPositionY != m_caster->GetPositionY() ||
		m_castPositionZ != m_caster->GetPositionZ()
		)
		)
	{
		if( u_caster != NULL )
		{
			if(u_caster->HasNoInterrupt() == 0 && GetSpellProto()->EffectMechanic[1] != 14)
			{
				cancel();
				return;
			}
		}
	}

	if(m_cancelled)
	{
		cancel();
		return;
	}

	switch(m_spellState)
	{
	case SPELL_STATE_PREPARING:
		{
			//printf("spell::update m_timer %u, difftime %d, newtime %d\n", m_timer, difftime, m_timer-difftime);
			if((int32)difftime >= m_timer)
			{
				if( u_caster != NULL && u_caster->GetCurrentSpell() == this )
					u_caster->SetCurrentSpell(NULLSPELL);

				cast(true);
			}
			else
			{
				m_timer -= difftime;
				if((int32)difftime >= m_timer)
				{
					if( u_caster != NULL && u_caster->GetCurrentSpell() == this )
						u_caster->SetCurrentSpell(NULLSPELL);

					m_timer = 0;
					cast(true);
				}
			}
		} break;
	case SPELL_STATE_CASTING:
		{
			if(m_timer > 0)
				m_timer = ( (int32)difftime >= m_timer ? 0 : m_timer - difftime );

			if(m_timer <= 0)
			{
				if( u_caster != NULL && u_caster->GetCurrentSpell() == this )
					u_caster->SetCurrentSpell(NULLSPELL);

				SendChannelUpdate(0);
				finish();
			}
		} break;
	}
}

void Spell::finish()
{
	m_spellState = SPELL_STATE_FINISHED;
	if( u_caster != NULL )
	{
		u_caster->m_canMove = true;
		// mana		   channeled													 power type is mana
		if(m_usesMana && (GetSpellProto()->ChannelInterruptFlags == 0 && !m_triggeredSpell) && u_caster->GetPowerType()==POWER_TYPE_MANA)
		{
			/*
			Five Second Rule
			After a character expends mana in casting a spell, the effective amount of mana gained per tick from spirit-based regeneration becomes a ratio of the normal
			listed above, for a period of 5 seconds. During this period mana regeneration is said to be interrupted. This is commonly referred to as the five second rule.
			By default, your interrupted mana regeneration ratio is 0%, meaning that spirit-based mana regeneration is suspended for 5 seconds after casting.
			Several effects can increase this ratio, including:
			*/

			u_caster->DelayPowerRegeneration(5000);
		}
	}
	/* Mana Regenerates while in combat but not for 5 seconds after each spell */
	/* Only if the spell uses mana, will it cause a regen delay.
	   is this correct? is there any spell that doesn't use mana that does cause a delay?
	   this is for creatures as effects like chill (when they have frost armor on) prevents regening of mana	*/

	//moved to spellhandler.cpp -> remove item when click on it! not when it finishes

	//enable pvp when attacking another player with spells
	if( p_caster != NULL )
	{
		if (GetSpellProto()->Attributes & ATTRIBUTES_STOP_ATTACK)
		{
			p_caster->EventAttackStop();
			p_caster->smsg_AttackStop( unitTarget );
		}

		if(RequiresComboPoints(GetSpellProto()) && !GetSpellFailed())
		{
			if(p_caster->m_spellcomboPoints)
			{
				p_caster->m_comboPoints = p_caster->m_spellcomboPoints;
				p_caster->UpdateComboPoints(); //this will make sure we do not use any wrong values here
			}
			else
			{
				p_caster->NullComboPoints();
			}
		}
		if(m_Delayed)
		{
			Unit* pTarget = NULLUNIT;
			if( p_caster->IsInWorld() )
			{
				pTarget = p_caster->GetMapMgr()->GetUnit(m_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT));
				if(!pTarget)
					pTarget = p_caster->GetMapMgr()->GetUnit(p_caster->GetSelection());
			}

			if(pTarget)
			{
				pTarget->RemoveAura(GetSpellProto()->Id, m_caster->GetGUID());
			}
		}
	}

	if( GetSpellProto()->Effect[0] == SPELL_EFFECT_SUMMON_OBJECT ||
		GetSpellProto()->Effect[1] == SPELL_EFFECT_SUMMON_OBJECT ||
		GetSpellProto()->Effect[2] == SPELL_EFFECT_SUMMON_OBJECT)
		if( p_caster != NULL )
			p_caster->SetSummonedObject(NULLOBJ);
	/*
	We set current spell only if this spell has cast time or is channeling spell
	otherwise it's instant spell and we delete it right after completion
	*/

	if( u_caster != NULL )
	{
		if( u_caster->GetCurrentSpell() == this )
			u_caster->SetCurrentSpell(NULLSPELL);
	}

	if( p_caster)
	{
		if(!GetSpellFailed())
			sHookInterface.OnPostSpellCast( p_caster, GetSpellProto(), unitTarget );

		if(p_caster->CooldownCheat && GetSpellProto())
			p_caster->ClearCooldownForSpell(GetSpellProto()->Id);

		if( m_ForceConsumption || ( cancastresult == SPELL_CANCAST_OK && !GetSpellFailed() ) )
			RemoveItems();
	}
	delete this;
}

void Spell::SendCastResult(uint8 result)
{
	uint32 Extra = 0;

	if(result == SPELL_CANCAST_OK)
		return;

	SetSpellFailed();

	if(!m_caster->IsInWorld())
		return;

	Player* plr = p_caster;

	if( plr == NULL && u_caster != NULL)
		plr = u_caster->m_redirectSpellPackets;

	if( plr == NULL)
		return;

	// reset cooldowns
	if( m_spellState == SPELL_STATE_PREPARING )
		plr->Cooldown_OnCancel(m_spellInfo);

	// for some reason, the result extra is not working for anything, including SPELL_FAILED_REQUIRES_SPELL_FOCUS
	switch( result )
	{
	case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
		Extra = GetSpellProto()->RequiresSpellFocus;
		break;

	case SPELL_FAILED_REQUIRES_AREA:
		if( GetSpellProto()->AreaGroupId > 0 )
		{
			uint16 area_id = plr->GetAreaID();
			AreaGroup *GroupEntry = dbcAreaGroup.LookupEntry( GetSpellProto()->AreaGroupId );

			for( uint8 i = 0; i < 7; i++ )
				if( GroupEntry->AreaId[i] != 0 && GroupEntry->AreaId[i] != area_id )
				{
					Extra = GroupEntry->AreaId[i];
					break;
				}
		}
		break;

	case SPELL_FAILED_TOTEMS:
		Extra = GetSpellProto()->Totem[1] ? GetSpellProto()->Totem[1] : GetSpellProto()->Totem[0];
		break;

	//case SPELL_FAILED_TOTEM_CATEGORY: seems to be fully client sided.
	}

	//plr->SendCastResult(GetSpellProto()->Id, result, extra_cast_number, Extra);
	if( Extra )
	{
		packetSMSG_CASTRESULT_EXTRA pe;
		pe.SpellId = GetSpellProto()->Id;
		pe.ErrorMessage = result;
		pe.MultiCast = extra_cast_number;
		pe.Extra = Extra;
		plr->GetSession()->OutPacket( SMSG_CAST_FAILED, sizeof( packetSMSG_CASTRESULT_EXTRA ), &pe );
	}
	else
	{
		packetSMSG_CASTRESULT pe;
		pe.SpellId = GetSpellProto()->Id;
		pe.ErrorMessage = result;
		pe.MultiCast = extra_cast_number;
		plr->GetSession()->OutPacket( SMSG_CAST_FAILED, sizeof( packetSMSG_CASTRESULT ), &pe );
	}
}

// uint16 0xFFFF
enum SpellStartFlags
{
	SPELL_START_FLAGS_NONE				= 0x00000000,
	SPELL_START_FLAGS_UNKNOWN0			= 0x00000001,	// may be pending spell cast
	SPELL_START_FLAG_DEFAULT			= 0x00000002,	// atm set as default flag
	SPELL_START_FLAGS_UNKNOWN2			= 0x00000004,
	SPELL_START_FLAGS_UNKNOWN3			= 0x00000008,
	SPELL_START_FLAGS_UNKNOWN4			= 0x00000010,
	SPELL_START_FLAG_RANGED				= 0x00000020,
	SPELL_START_FLAGS_UNKNOWN6			= 0x00000040,
	SPELL_START_FLAGS_UNKNOWN7			= 0x00000080,
	SPELL_START_FLAGS_UNKNOWN8			= 0x00000100,
	SPELL_START_FLAGS_UNKNOWN9			= 0x00000200,
	SPELL_START_FLAGS_UNKNOWN10			= 0x00000400, //TARGET MISSES AND OTHER MESSAGES LIKE "Resist"
	SPELL_START_FLAGS_POWER_UPDATE		= 0x00000800,
	SPELL_START_FLAGS_UNKNOWN12			= 0x00001000,
	SPELL_START_FLAGS_UNKNOWN13			= 0x00002000,
	SPELL_START_FLAGS_UNKNOWN14			= 0x00004000,
	SPELL_START_FLAGS_UNKNOWN15			= 0x00008000,
	SPELL_START_FLAGS_UNKNOWN16			= 0x00010000,
	SPELL_START_FLAGS_UNKNOWN17			= 0x00020000,
	SPELL_START_FLAGS_UNKNOWN18			= 0x00040000,
	SPELL_START_FLAGS_UNKNOWN19			= 0x00080000,
	SPELL_START_FLAGS_UNKNOWN20			= 0x00100000,
	SPELL_START_FLAGS_RUNE_UPDATE		= 0x00200000,
	SPELL_START_FLAGS_UNKNOWN21			= 0x00400000,
};

void Spell::SendSpellStart()
{
	// no need to send this on passive spells
	if( !m_caster->IsInWorld() || GetSpellProto()->Attributes & 64 || m_triggeredSpell )
		return;

	WorldPacket data(SMSG_SPELL_START, 2000);

	uint32 cast_flags = SPELL_START_FLAG_DEFAULT;

	if(u_caster && !m_triggeredSpell && !(GetSpellProto()->AttributesEx & ATTRIBUTESEX_AREA_OF_EFFECT))
		cast_flags |= SPELL_START_FLAGS_POWER_UPDATE;

	if(p_caster && p_caster->getClass() == DEATHKNIGHT)
		cast_flags |= SPELL_START_FLAGS_RUNE_UPDATE;

	if( GetType() == SPELL_DMG_TYPE_RANGED )
		cast_flags |= SPELL_START_FLAG_RANGED;


	// hacky yeaaaa
	if( GetSpellProto()->Id == 8326 ) // death
		cast_flags = 0x0F;
	if (m_missileTravelTime)
		cast_flags = 0x6000E;

	if( i_caster != NULL )
		data << i_caster->GetNewGUID() << u_caster->GetNewGUID();
	else
		data << m_caster->GetNewGUID() << m_caster->GetNewGUID();

	data << extra_cast_number;
	data << GetSpellProto()->Id;
	data << cast_flags;
	data << (uint32)m_castTime;

	m_targets.write( data );

	if(cast_flags & SPELL_START_FLAGS_POWER_UPDATE) //send new mana
		data << uint32( u_caster ? u_caster->GetUInt32Value(UNIT_FIELD_POWER1 + u_caster->GetPowerType()) : 0);
	if( (p_caster != NULL) && cast_flags & SPELL_START_FLAGS_RUNE_UPDATE && dbcSpellRuneCost.LookupEntry(GetSpellProto()->runeCostID) != NULL) //send new runes
	{
		SpellRuneCostEntry * runecost = dbcSpellRuneCost.LookupEntry(GetSpellProto()->runeCostID);
		uint8 theoretical = p_caster->TheoreticalUseRunes(runecost->bloodRuneCost, runecost->frostRuneCost, runecost->unholyRuneCost);
		data << p_caster->m_runemask << theoretical;

		for (uint8 i=0; i<6; i++)
		{
			if ((1 << i) & p_caster->m_runemask)
				if (!((1 << i) & theoretical))
					data << uint8(0); // I love you, Andy--in a gay way.
		}
	}

	if (cast_flags & 0x20000)
		data << m_missilePitch << m_missileTravelTime;
	if( GetType() == SPELL_DMG_TYPE_RANGED )
	{
		ItemPrototype* ip = NULL;
		if( GetSpellProto()->Id == SPELL_RANGED_THROW ) // throw
		{
			if( p_caster != NULL )
			{
				Item* itm = p_caster->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_RANGED );
				if( itm != NULL )
				{
					ip = itm->GetProto();
					/* Throwing Weapon Patch by Supalosa
					p_caster->GetItemInterface()->RemoveItemAmt(it->GetEntry(),1);
					(Supalosa: Instead of removing one from the stack, remove one from durability)
					We don't need to check if the durability is 0, because you can't cast the Throw spell if the thrown weapon is broken, because it returns "Requires Throwing Weapon" or something.
					*/

					// burlex - added a check here anyway (wpe suckers :P)
					if( itm->GetDurability() > 0 )
					{
						itm->SetDurability( itm->GetDurability() - 1 );
						if( itm->GetDurability() == 0 )
							p_caster->ApplyItemMods( itm, EQUIPMENT_SLOT_RANGED, false, true );
					}
				}
				else
				{
					ip = ItemPrototypeStorage.LookupEntry( 2512 );	/*rough arrow*/
				}
			}
		}
		else if( GetSpellProto()->Flags4 & FLAGS4_PLAYER_RANGED_SPELLS )
		{
			if( p_caster != NULL )
				ip = ItemPrototypeStorage.LookupEntry( p_caster->GetUInt32Value( PLAYER_AMMO_ID ) );
			else
				ip = ItemPrototypeStorage.LookupEntry( 2512 );	/*rough arrow*/
		}

		if( ip != NULL )
			data << ip->DisplayInfoID << ip->InventoryType;
	}

	m_caster->SendMessageToSet( &data, true );
}

/************************************************************************/
/* General Spell Go Flags, for documentation reasons					*/
/************************************************************************/
enum SpellGoFlags
{
	SPELL_GO_FLAGS_NONE					= 0x00000000,
	SPELL_GO_FLAGS_UNKNOWN0				= 0x00000001,				// may be pending spell cast
	SPELL_GO_FLAGS_UNKNOWN1				= 0x00000002,
	SPELL_GO_FLAGS_UNKNOWN2				= 0x00000004,
	SPELL_GO_FLAGS_UNKNOWN3				= 0x00000008,
	SPELL_GO_FLAGS_UNKNOWN4				= 0x00000010,
	SPELL_GO_FLAGS_RANGED				= 0x00000020,
	SPELL_GO_FLAGS_UNKNOWN6				= 0x00000040,
	SPELL_GO_FLAGS_UNKNOWN7				= 0x00000080,
	SPELL_GO_FLAGS_ITEM_CASTER			= 0x00000100,
	SPELL_GO_FLAGS_UNKNOWN9				= 0x00000200,
	SPELL_GO_FLAGS_EXTRA_MESSAGE		= 0x00000400, //TARGET MISSES AND OTHER MESSAGES LIKE "Resist"
	SPELL_GO_FLAGS_POWER_UPDATE			= 0x00000800,
	SPELL_GO_FLAGS_UNKNOWN12			= 0x00001000,
	SPELL_GO_FLAGS_UNKNOWN13			= 0x00002000,
	SPELL_GO_FLAGS_UNKNOWN14			= 0x00004000,
	SPELL_GO_FLAGS_UNKNOWN15			= 0x00008000,
	SPELL_GO_FLAGS_UNKNOWN16			= 0x00010000,
	SPELL_GO_FLAGS_UNKNOWN17			= 0x00020000,
	SPELL_GO_FLAGS_UNKNOWN18			= 0x00040000,
	SPELL_GO_FLAGS_UNKNOWN19			= 0x00080000,
	SPELL_GO_FLAGS_UNKNOWN20			= 0x00100000,
	SPELL_GO_FLAGS_RUNE_UPDATE			= 0x00200000,
	SPELL_GO_FLAGS_UNKNOWN22			= 0x00400000,
};

void Spell::SendSpellGo()
{
	// no need to send this on passive spells
	if( !m_caster->IsInWorld() || GetSpellProto()->Attributes & 64 )
		return;

	ItemPrototype* ip = NULL;
	uint32 cast_flags = (m_triggeredSpell && !(GetSpellProto()->Attributes & ATTRIBUTE_ON_NEXT_ATTACK)) ? 0x0105 : 0x0100;
	if(u_caster && !m_triggeredSpell && (u_caster->GetPowerType() != POWER_TYPE_MANA || m_usesMana) &&
		!(GetSpellProto()->AttributesEx & ATTRIBUTESEX_AREA_OF_EFFECT))	// Hackfix for client bug which displays mana as 0 after receiving update for these spells
		cast_flags |= SPELL_GO_FLAGS_POWER_UPDATE;
	SpellTargetList::iterator itr;
	uint32 counter;

	if( i_caster != NULL )
		cast_flags |= SPELL_GO_FLAGS_ITEM_CASTER; // 0x100 ITEM CASTER

	if(p_caster && p_caster->getClass() == DEATHKNIGHT && m_spellInfo->runeCostID && m_spellInfo->powerType == POWER_RUNE)
		cast_flags |= SPELL_GO_FLAGS_RUNE_UPDATE;

	for(uint8 i = 0; i < 3; i++)
	{
		if( GetSpellProto()->Effect[i] == SPELL_EFFECT_ACTIVATE_RUNE)
		{
			cast_flags |= SPELL_GO_FLAGS_RUNE_UPDATE;
		}
	}

	if (m_missileTravelTime)
		cast_flags |= 0x20000;
	// hacky..
	if( GetSpellProto()->Id == 8326 ) // death
		cast_flags = SPELL_GO_FLAGS_ITEM_CASTER | 0x0D;

	if( GetType() == SPELL_DMG_TYPE_RANGED )
	{
		cast_flags |=SPELL_GO_FLAGS_RANGED;
		if( GetSpellProto()->Id == SPELL_RANGED_THROW )
		{
			if( p_caster != NULL )
			{
				Item* it = p_caster->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_RANGED );
				if( it != NULL )
					ip = it->GetProto();
			}
			else
			{
				ip = ItemPrototypeStorage.LookupEntry(2512);	//rough arrow
			}
		}
		else
		{
			if( p_caster != NULL )
				ip = ItemPrototypeStorage.LookupEntry(p_caster->GetUInt32Value( PLAYER_AMMO_ID ) );
			else // HACK FIX
				ip = ItemPrototypeStorage.LookupEntry(2512);	//rough arrow
		}
	}

	// stack-based solution
	// this has room for every type of spell target type,
	// as well as 100 missed and 100 hit targets.
	// if you're doing that, you're NUTS.
	WorldPacket data(SMSG_SPELL_GO, 3764);

	if( i_caster != NULL && u_caster != NULL ) // this is needed for correct cooldown on items
		data << i_caster->GetNewGUID() << u_caster->GetNewGUID();
	else
		data << m_caster->GetNewGUID() << m_caster->GetNewGUID();

	data << extra_cast_number;
	data << GetSpellProto()->Id;
	data << cast_flags;
	data << getMSTime();

	/************************************************************************/
	/* spell go targets													 */
	/************************************************************************/
	// Make sure we don't hit over 100 targets.
	// It's fine internally, but sending it to the client will REALLY cause it to freak.

	data << uint8(m_hitTargetCount);
	if( m_hitTargetCount > 0 )
	{
		counter = 0;
		for( itr = m_targetList.begin(); itr != m_targetList.end() && counter < 100; itr++ )
		{
			if( itr->HitResult == SPELL_DID_HIT_SUCCESS )
			{
				data << itr->Guid;
				++counter;
			}
		}
	}

	data << uint8(m_missTargetCount);
	if( m_missTargetCount > 0 )
	{
		counter = 0;
		for( itr = m_targetList.begin(); itr != m_targetList.end() && counter < 100; itr++ )
		{
			if( itr->HitResult != SPELL_DID_HIT_SUCCESS )
			{
				data << itr->Guid;
				data << itr->HitResult;
				if( itr->HitResult == SPELL_DID_HIT_REFLECT )
					data << uint8(SPELL_DID_HIT_SUCCESS);

				++counter;
			}
		}
	}

	m_targets.write( data ); // this write is included the target flag

	if (cast_flags & SPELL_GO_FLAGS_POWER_UPDATE) //send new power
		data << uint32( u_caster ? u_caster->GetUInt32Value(UNIT_FIELD_POWER1 + u_caster->GetPowerType()) : 0);

	if( (p_caster != NULL) && cast_flags & SPELL_GO_FLAGS_RUNE_UPDATE && dbcSpellRuneCost.LookupEntry(GetSpellProto()->runeCostID) != NULL) //send new runes
	{
		SpellRuneCostEntry * runecost = dbcSpellRuneCost.LookupEntry(GetSpellProto()->runeCostID);
		uint8 theoretical = p_caster->TheoreticalUseRunes(runecost->bloodRuneCost, runecost->frostRuneCost, runecost->unholyRuneCost);
		data << p_caster->m_runemask << theoretical;

		for (uint8 i=0; i<6; i++)
		{
			if ((1 << i) & p_caster->m_runemask)
				if (!((1 << i) & theoretical))
					data << uint8(0);
		}
	}

	if( ip != NULL)
		data << ip->DisplayInfoID << ip->InventoryType;

	m_caster->SendMessageToSet( &data, (m_caster->IsPlayer() ? true : false) );
	// spell log execute is still send 2.08
	// as I see with this combination, need to test it more
	//if (flags != 0x120 && GetSpellProto()->Attributes & 16) // not ranged and flag 5
	  //  SendLogExecute(0,m_targets.m_unitTarget);
}
void Spell::writeSpellGoTargets( WorldPacket * data )
{
	SpellTargetList::iterator itr;
	uint32 counter;

	// Make sure we don't hit over 100 targets.
	// It's fine internally, but sending it to the client will REALLY cause it to freak.

	*data << uint8(m_hitTargetCount);
	if( m_hitTargetCount > 0 )
	{
		counter = 0;
		for( itr = m_targetList.begin(); itr != m_targetList.end() && counter < 100; itr++ )
		{
			if( itr->HitResult == SPELL_DID_HIT_SUCCESS )
			{
				*data << itr->Guid;
				++counter;
			}
		}
	}

	*data << uint8(m_missTargetCount);
	if( m_missTargetCount > 0 )
	{
		counter = 0;
		for( itr = m_targetList.begin(); itr != m_targetList.end() && counter < 100; itr++ )
		{
			if( itr->HitResult != SPELL_DID_HIT_SUCCESS )
			{
				*data << itr->Guid;
				++counter;
			}
		}
	}
}

void Spell::SendLogExecute(uint32 damage, uint64 & targetGuid)
{
	WorldPacket data(SMSG_SPELLLOGEXECUTE, 37);
	data << m_caster->GetNewGUID();
	data << GetSpellProto()->Id;
	data << uint32(1);
//	data << GetSpellProto()->SpellVisual[0];
//	data << uint32(1);
	if (m_caster->GetGUID() != targetGuid)
		data << targetGuid;
	if (damage)
		data << damage;
	m_caster->SendMessageToSet(&data,true);
}

void Spell::SendInterrupted(uint8 result)
{
	SetSpellFailed();

	if(!m_caster->IsInWorld()) 
		return;

	WorldPacket data(SMSG_SPELL_FAILURE, 100);

	// send the failure to pet owner if we're a pet
	Player* plr = p_caster;
	if(!plr && m_caster->IsPet())
		plr = TO_PET(m_caster)->GetPetOwner();
	if(!plr && u_caster)
		plr = u_caster->m_redirectSpellPackets;

	if(plr && plr->IsPlayer())
	{
		data << m_caster->GetNewGUID();
		data << uint8(extra_cast_number);
		data << uint32(GetSpellProto()->Id);
		data << uint8(result);
		plr->GetSession()->SendPacket(&data);
	}

	data.Initialize(SMSG_SPELL_FAILED_OTHER);
	data << m_caster->GetNewGUID();
	data << uint8(extra_cast_number);
	data << uint32(GetSpellProto()->Id);
	data << uint8(result);
	m_caster->SendMessageToSet(&data, false);
}

void Spell::SendChannelUpdate(uint32 time)
{
	if(u_caster && time == 0)
	{
		if( u_caster->IsInWorld() && u_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT))
		{
			DynamicObject* dynObj = u_caster->GetMapMgr()->GetDynamicObject(u_caster->GetUInt32Value(UNIT_FIELD_CHANNEL_OBJECT));
			if(dynObj)
			{
				dynObj->RemoveFromWorld(true);
				delete dynObj;
				dynObj = NULLGOB;
			}
		}
		if( p_caster && p_caster->IsInWorld() && p_caster->GetUInt64Value(PLAYER_FARSIGHT) )
		{
			DynamicObject* dynObj = p_caster->GetMapMgr()->GetDynamicObject(p_caster->GetUInt32Value(PLAYER_FARSIGHT));
			if( dynObj )
			{
				dynObj->RemoveFromWorld(true);
				delete dynObj;
				dynObj = NULLGOB;
				p_caster->SetUInt32Value(PLAYER_FARSIGHT, 0);
			}
			p_caster->SetUInt64Value(PLAYER_FARSIGHT, 0);
			p_caster->GetMapMgr()->ChangeFarsightLocation(p_caster, p_caster->GetPositionX(), p_caster->GetPositionY(), false);
		}

		u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT,0);
		u_caster->SetUInt32Value(UNIT_CHANNEL_SPELL,0);
	}

	WorldPacket data(MSG_CHANNEL_UPDATE, 40);
	data << m_caster->GetNewGUID();
	data << time;
	m_caster->SendMessageToSet(&data, (m_caster->IsPlayer() ? true : false));
}

void Spell::SendChannelStart(int32 duration)
{
	if (m_caster->GetTypeId() != TYPEID_GAMEOBJECT)
	{
		WorldPacket data(MSG_CHANNEL_START, 60);
		data << m_caster->GetNewGUID();
		data << GetSpellProto()->Id;
		data << duration;
		m_caster->SendMessageToSet(&data, true);
	}

	m_castTime = m_timer = duration;

	if( u_caster != NULL )
		u_caster->SetUInt32Value(UNIT_CHANNEL_SPELL,GetSpellProto()->Id);
}

void Spell::SendResurrectRequest(Player* target)
{
	const char* name = m_caster->IsCreature() ? TO_CREATURE(m_caster)->GetCreatureInfo()->Name : "";
	WorldPacket data(SMSG_RESURRECT_REQUEST, 12+strlen(name)+3);
	data << m_caster->GetGUID();
	data << uint32(strlen(name) + 1);
	data << name;
	data << uint8(0);
	data << uint8(m_caster->IsCreature() ? 1 : 0);
	target->GetSession()->SendPacket(&data);
}

bool Spell::HasPower()
{
	//trainers can always cast
	if( u_caster != NULL && u_caster->HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_TRAINER) )
		return true;
	//Powercheaters too
	if(p_caster && p_caster->PowerCheat)
		return true;
	//Seems to be an issue since 3.0.9, as many elixers/potions got powertype 4
	//Haven't found any items taking power, so guess it's safe to skip them.
	if(i_caster || g_caster)
		return true;

	int32 powerField;
	switch(GetSpellProto()->powerType)
	{
	case POWER_TYPE_HEALTH:	{ powerField = UNIT_FIELD_HEALTH; }break;
	case POWER_TYPE_MANA:	{ powerField = UNIT_FIELD_POWER1; m_usesMana=true; }break;
	case POWER_TYPE_RAGE:	{ powerField = UNIT_FIELD_POWER2; }break;
	case POWER_TYPE_FOCUS:	{ powerField = UNIT_FIELD_POWER3; }break;
	case POWER_TYPE_ENERGY:	{ powerField = UNIT_FIELD_POWER4; }break;
	case POWER_TYPE_RUNE:
		{
			if(GetSpellProto()->runeCostID && p_caster)
			{
				SpellRuneCostEntry * runecost = dbcSpellRuneCost.LookupEntry(GetSpellProto()->runeCostID);
				if( !p_caster->CanUseRunes( runecost->bloodRuneCost, runecost->frostRuneCost, runecost->unholyRuneCost) )
					return false;
			}
			return true;
		}
	case POWER_TYPE_RUNIC:	{ powerField = UNIT_FIELD_POWER7; }break;
	default:{
		DEBUG_LOG("Spell","unknown power type %d", GetSpellProto()->powerType);
		// we should'nt be here to return
		return false;
			}break;
	}

	if(!m_caster)
		return true;

	int32 currentPower = m_caster->GetUInt32Value(powerField);
	int32 cost = m_caster->GetSpellBaseCost(m_spellInfo);

	if((int32)GetSpellProto()->powerType == POWER_TYPE_HEALTH)
		cost -= GetSpellProto()->baseLevel;//FIX for life tap
	else if( u_caster != NULL )
	{
		if( GetSpellProto()->powerType == POWER_TYPE_MANA)
			cost += u_caster->PowerCostMod[GetSpellProto()->School];//this is not percent!
		else
			cost += u_caster->PowerCostMod[0];

		cost +=float2int32(cost*u_caster->GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+GetSpellProto()->School));
	}

	//apply modifiers
	if( GetSpellProto()->SpellGroupType && u_caster)
	{
		SM_FIValue(u_caster->SM[SMT_COST][0],&cost,GetSpellProto()->SpellGroupType);
		SM_PIValue(u_caster->SM[SMT_COST][1],&cost,GetSpellProto()->SpellGroupType);
	}

	if (cost <=0)
	{
		m_usesMana = false; // no mana regen interruption for free spells
		return true;
	}

	//Stupid shiv
	if( GetSpellProto()->NameHash == SPELL_HASH_SHIV )
	{
		Item* Offhand = p_caster->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND);

		if( Offhand != NULL && Offhand->GetProto() != NULL )
			cost += Offhand->GetProto()->Delay / 100;
	}

	//FIXME:DK:if field value < cost what happens
	if(powerField == UNIT_FIELD_HEALTH)
	{
		return true;
	}
	else
	{
		if(cost <= currentPower) // Unit has enough power (needed for creatures)
		{
			return true;
		}
		else
			return false;
	}
}

bool Spell::TakePower()
{
	//trainers can always cast
	if( u_caster != NULL && u_caster->HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_TRAINER) )
		return true;
	//Powercheaters too
	if(p_caster && p_caster->PowerCheat)
		return true;
	//Seems to be an issue since 3.0.9, as many elixers/potions got powertype 4
	//Haven't found any items taking power, so guess it's safe to skip them.
	if(i_caster)
		return true;
	if(g_caster)
		return true; //GO's Don't use power.

	int32 powerField = 0;
	switch(GetSpellProto()->powerType)
	{
		case POWER_TYPE_HEALTH:	{ powerField = UNIT_FIELD_HEALTH; }break;
		case POWER_TYPE_MANA:	{ powerField = UNIT_FIELD_POWER1; m_usesMana=true; }break;
		case POWER_TYPE_RAGE:	{ powerField = UNIT_FIELD_POWER2; }break;
		case POWER_TYPE_FOCUS:	{ powerField = UNIT_FIELD_POWER3; }break;
		case POWER_TYPE_ENERGY:	{ powerField = UNIT_FIELD_POWER4; }break;
		case POWER_TYPE_RUNIC:	{ powerField = UNIT_FIELD_POWER7; }break;
		case POWER_TYPE_RUNE:
		{
			if(GetSpellProto()->runeCostID && p_caster)
			{
				SpellRuneCostEntry * runecost = dbcSpellRuneCost.LookupEntry(GetSpellProto()->runeCostID);
				if( !p_caster->CanUseRunes( runecost->bloodRuneCost, runecost->frostRuneCost, runecost->unholyRuneCost) )
					return false;

				p_caster->UseRunes( runecost->bloodRuneCost, runecost->frostRuneCost, runecost->unholyRuneCost, m_spellInfo);
				if(runecost->runePowerGain)
					u_caster->SetPower(POWER_TYPE_RUNIC, runecost->runePowerGain + u_caster->GetUInt32Value(UNIT_FIELD_POWER7));
			}
			return true;
		}break;
		default:
		{
			DEBUG_LOG("Spell","Unknown power type %u for spell %u", GetSpellProto()->powerType, GetSpellProto()->Id);
			// we shouldn't be here to return
			return false;
		}break;
	}

	//FIXME: add handler for UNIT_FIELD_POWER_COST_MODIFIER
	//UNIT_FIELD_POWER_COST_MULTIPLIER

	int32 cost = 0;
	int32 currentPower = m_caster->GetUInt32Value(powerField);

	cost = m_caster->GetSpellBaseCost(m_spellInfo);

	if( u_caster != NULL )
	{
		if( GetSpellProto()->AttributesEx & ATTRIBUTESEX_DRAIN_WHOLE_MANA ) // Uses %100 mana
		{
			m_caster->SetUInt32Value(powerField, 0);
			return true;
		}
		cost += u_caster->PowerCostMod[ m_usesMana ? GetSpellProto()->School : 0 ];//this is not percent!
		cost += float2int32(float(cost)* u_caster->GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + GetSpellProto()->School));
	}

	if( powerField == UNIT_FIELD_HEALTH )
		cost -= GetSpellProto()->baseLevel;//FIX for life tap

	//apply modifiers
	if( GetSpellProto()->SpellGroupType && u_caster)
	{
		SM_FIValue(u_caster->SM[SMT_COST][0],&cost,GetSpellProto()->SpellGroupType);
		SM_PIValue(u_caster->SM[SMT_COST][1],&cost,GetSpellProto()->SpellGroupType);
	}

	//Stupid shiv
	if( GetSpellProto()->NameHash == SPELL_HASH_SHIV )
	{
		Item* Offhand = p_caster->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND);

		if( Offhand != NULL && Offhand->GetProto() != NULL )
			cost += Offhand->GetProto()->Delay / 100;
	}

	if (cost <=0)
	{
		m_usesMana = false; // no mana regen interruption for free spells
		return true;
	}

	if( u_caster && GetSpellProto()->powerType == POWER_TYPE_MANA )
		u_caster->m_LastSpellManaCost = cost;

	//FIXME:DK:if field value < cost what happens
	if(powerField == UNIT_FIELD_HEALTH)
	{
		m_caster->DealDamage(u_caster, cost, 0, 0, 0,true);
		return true;
	}
	else
	{
		if(cost <= currentPower) // Unit has enough power (needed for creatures)
		{
			m_caster->SetUInt32Value(powerField, currentPower - cost);
			return true;
		}
	}
	return false;
}

Object* Spell::_LookupObject(const uint64& guid)
{
	if( guid == m_caster->GetGUID() )
		return m_caster;

	switch(GET_TYPE_FROM_GUID(guid))
	{
	case HIGHGUID_TYPE_ITEM:
		{
			if( p_caster != NULL )
				return p_caster->GetItemInterface()->GetItemByGUID( (uint64)guid );
		}break;

	case HIGHGUID_TYPE_CORPSE:
		{
			return objmgr.GetCorpse((uint32)guid);
		}break;

	default:
		{
			if( m_caster->IsInWorld() )
				return m_caster->GetMapMgr()->GetUnit(guid);
		}break;
	}

	return NULLOBJ;
}

void Spell::_SetTargets(const uint64& guid)
{
	if(guid == m_caster->GetGUID())
	{
		unitTarget = u_caster;
		gameObjTarget = g_caster;
		playerTarget = p_caster;
	}
	else
	{
		if(!m_caster->IsInWorld())
		{
			unitTarget = NULLUNIT;
			playerTarget = NULLPLR;
			gameObjTarget = NULLGOB;
			corpseTarget = NULLCORPSE;
		}
		else
		{
			unitTarget = NULLUNIT;
			MapMgr* mgr = m_caster->GetMapMgr();
			switch(GET_TYPE_FROM_GUID(guid))
			{
			case HIGHGUID_TYPE_VEHICLE:
				unitTarget = mgr->GetVehicle(GET_LOWGUID_PART(guid));
				break;
			case HIGHGUID_TYPE_CREATURE:
				unitTarget = mgr->GetCreature(GET_LOWGUID_PART(guid));
				break;
			case HIGHGUID_TYPE_PET:
				unitTarget = mgr->GetPet(GET_LOWGUID_PART(guid));
				break;
			case HIGHGUID_TYPE_PLAYER:
				{
					unitTarget = mgr->GetPlayer((uint32)guid);
					playerTarget = TO_PLAYER(unitTarget);
				}break;
			case HIGHGUID_TYPE_GAMEOBJECT:
				gameObjTarget = mgr->GetGameObject(GET_LOWGUID_PART(guid));
				break;
			case HIGHGUID_TYPE_CORPSE:
				corpseTarget = objmgr.GetCorpse((uint32)guid);
				break;
			case HIGHGUID_TYPE_ITEM:
				if( p_caster != NULL )
					itemTarget = p_caster->GetItemInterface()->GetItemByGUID(guid);
				break;
			}
		}
	}
}

void Spell::HandleEffects(uint32 i)
{
	// Try a dummy SpellHandler
	if(sScriptMgr.CallScriptedDummySpell(GetSpellProto()->Id, i, this ))
	{
		DEBUG_LOG( "Spell","Redirecting Spell %u Effect id = %u to sScriptMgr", GetSpellProto()->Id, GetSpellProto()->Effect[i]);
		return;
	}
	damage = CalculateEffect(i, unitTarget);
	DEBUG_LOG( "Spell","Handling Effect id = %u, damage = %d", GetSpellProto()->Effect[i], damage);

	if( GetSpellProto()->Effect[i] < TOTAL_SPELL_EFFECTS)
		(*this.*SpellEffectsHandler[GetSpellProto()->Effect[i]])(i);
	else
		if(sLog.IsOutDevelopement())
			printf("Unknown spell effect %u in spell %u.\n",GetSpellProto()->Effect[i],GetSpellProto()->Id);
		else
			DEBUG_LOG("Spell","Unknown effect %u spellid %u",GetSpellProto()->Effect[i], GetSpellProto()->Id);
}

void Spell::HandleAddAura(uint64 guid)
{
	Unit* Target = NULLUNIT;
	if(guid == 0)
		return;

	if(u_caster && u_caster->GetGUID() == guid)
		Target = u_caster;
	else if(m_caster->IsInWorld())
		Target = m_caster->GetMapMgr()->GetUnit(guid);

	if(!Target)
		return;

	// Applying an aura to a flagged target will cause you to get flagged.
	// self casting doesnt flag himself.
	if( p_caster && p_caster->GetGUID() != Target->GetGUID() )
	{
		if( Target->IsPvPFlagged() )
		{
			if( !p_caster->IsPvPFlagged() )
				p_caster->PvPToggle();
			else
				p_caster->SetPvPFlag();
		}
	}

	uint32 spellid = 0;

	if( GetSpellProto()->MechanicsType == 25 && GetSpellProto()->Id != 25771 || GetSpellProto()->Id == 31884 ) // Cast spell Forbearance
	{
		if( GetSpellProto()->Id != 31884 )
			spellid = 25771;

		if( Target->IsPlayer() )
		{
			sEventMgr.AddEvent(TO_PLAYER(Target), &Player::AvengingWrath, EVENT_PLAYER_AVENGING_WRATH, 30000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
			TO_PLAYER(Target)->mAvengingWrath = false;
		}
	}
	else if( GetSpellProto()->MechanicsType == 16 && GetSpellProto()->Id != 11196) // Cast spell Recently Bandaged
		spellid = 11196;
	else if( GetSpellProto()->MechanicsType == 19 && GetSpellProto()->Id != 6788) // Cast spell Weakened Soul
		spellid = 6788;
	else if( GetSpellProto()->Id == 45438) // Cast spell Hypothermia
		spellid = 41425;
	else if( GetSpellProto()->AdditionalAura )
		spellid = GetSpellProto()->AdditionalAura;
	else if( GetSpellProto()->NameHash == SPELL_HASH_HEROISM )
		spellid = 57723;
	else if( GetSpellProto()->NameHash == SPELL_HASH_BLOODLUST )
		spellid = 57724;
	else if( GetSpellProto()->NameHash == SPELL_HASH_STEALTH )
	{
		if( Target->HasDummyAura(SPELL_HASH_MASTER_OF_SUBTLETY) )
			spellid = 31665;
	}
	else if( GetSpellProto()->Id == 62124 && u_caster )
	{
		if( u_caster->HasDummyAura(SPELL_HASH_VINDICATION) )
			spellid = u_caster->GetDummyAura(SPELL_HASH_VINDICATION)->RankNumber == 2 ? 26017 : 67;
	}
	else if( GetSpellProto()->Id == 5229 &&
		p_caster && (
		p_caster->GetShapeShift() == FORM_BEAR ||
		p_caster->GetShapeShift() == FORM_DIREBEAR ) &&
		p_caster->HasDummyAura(SPELL_HASH_KING_OF_THE_JUNGLE) )
	{
		SpellEntry *spellInfo = dbcSpell.LookupEntry( 51185 );
		if(!spellInfo)
			return;

		Spell* spell = NULLSPELL;
		spell = (new Spell(p_caster, spellInfo ,true, NULLAURA));
		spell->forced_basepoints[0] = p_caster->GetDummyAura(SPELL_HASH_KING_OF_THE_JUNGLE)->RankNumber * 5;
		SpellCastTargets targets(p_caster->GetGUID());
		spell->prepare(&targets);
	}
	else if( GetSpellProto()->Id == 19574 )
	{
		if( u_caster->HasDummyAura(SPELL_HASH_THE_BEAST_WITHIN) )
			u_caster->CastSpell(u_caster, 34471, true);
	}
	else if( GetSpellProto()->NameHash == SPELL_HASH_RAPID_KILLING )
	{
		if( u_caster->HasDummyAura(SPELL_HASH_RAPID_RECUPERATION) )
			spellid = 56654;
	}

	switch( GetSpellProto()->NameHash )
	{
	case SPELL_HASH_CLEARCASTING:
	case SPELL_HASH_PRESENCE_OF_MIND:
		{
			if( Target->HasDummyAura(SPELL_HASH_ARCANE_POTENCY) )
				spellid = Target->GetDummyAura(SPELL_HASH_ARCANE_POTENCY)->RankNumber == 1 ? 57529 : 57531;
		}break;
	}

	if( spellid && Target )
	{
		SpellEntry *spellInfo = dbcSpell.LookupEntry( spellid );
		if(!spellInfo)
			return;

		Spell* spell = NULLSPELL;
		spell = (new Spell(Target, spellInfo ,true, NULLAURA));
		if( spellid == 31665 && Target->HasDummyAura(SPELL_HASH_MASTER_OF_SUBTLETY) )
			spell->forced_basepoints[0] = Target->GetDummyAura(SPELL_HASH_MASTER_OF_SUBTLETY)->EffectBasePoints[0];

		SpellCastTargets targets(Target->GetGUID());
		spell->prepare(&targets);
	}

	if( GetSpellProto()->MechanicsType == 31 )
		Target->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_ENRAGE);

	// avoid map corruption
	if(Target->GetInstanceID()!=m_caster->GetInstanceID())
		return;

	std::map<uint32,Aura* >::iterator itr=Target->tmpAura.find(GetSpellProto()->Id);
	if(itr!=Target->tmpAura.end())
	{
		Aura* aura = itr->second;
		if(aura != NULL)
		{
			// did our effects kill the target?
			if( Target->isDead() && !(GetSpellProto()->Flags4 & FLAGS4_DEATH_PERSISTENT))
			{
				// free pointer
				aura->m_tmpAuradeleted = true;
				Target->RemoveAura(aura);
				itr->second = NULLAURA;
				Target->tmpAura.erase(itr);
				return;
			}

			//make sure bg/arena preparation aura's are positive.
			if(GetSpellProto()->Id == 32727 || GetSpellProto()->Id == 44521)
				aura->SetPositive(100);

			Target->AddAura(aura);
			if(!aura->m_tmpAuradeleted && !Target->tmpAura.empty())
				Target->tmpAura.erase(itr);
		}
	}
}

bool Spell::IsAspect()
{
	return (GetSpellProto()->buffType == SPELL_TYPE_ASPECT);
}

bool Spell::IsSeal()
{
	return (GetSpellProto()->buffType == SPELL_TYPE_SEAL);
}

bool Spell::IsBinary(SpellEntry * sp)
{
	// Normally, damage spells are only binary if they have an additional non-damage effect
	// DoTs used to be binary spells, but this was changed. (WoWwiki)
	return !(sp->Effect[0] == SPELL_EFFECT_SCHOOL_DAMAGE ||
		sp->EffectApplyAuraName[0] == SPELL_AURA_PERIODIC_DAMAGE);
}

uint8 Spell::CanCast(bool tolerate)
{
	uint32 i;

	bool skip = (p_caster && (p_caster->m_skipCastCheck[0] & GetSpellProto()->SpellGroupType[0] ||
		p_caster && p_caster->m_skipCastCheck[1] & GetSpellProto()->SpellGroupType[1] ||
		p_caster && p_caster->m_skipCastCheck[2] & GetSpellProto()->SpellGroupType[2])); // related to aura 262

	if(objmgr.IsSpellDisabled(GetSpellProto()->Id))
		return SPELL_FAILED_SPELL_UNAVAILABLE;

	if( u_caster && u_caster->GetCurrentSpell() != NULL && u_caster->GetCurrentSpell() != this && !m_triggeredSpell )
		return SPELL_FAILED_SPELL_IN_PROGRESS;

	/* Spells for the zombie event */
	if( p_caster && p_caster->GetShapeShift() ==FORM_ZOMBIE && !( ((uint32)1 << (p_caster->GetShapeShift()-1)) & GetSpellProto()->RequiredShapeShift  ))
	{
		OUT_DEBUG("Invalid shapeshift: %u", GetSpellProto()->RequiredShapeShift);
		return SPELL_FAILED_SPELL_UNAVAILABLE;
	}

	if(m_caster->IsInWorld())
	{
		Unit *target = m_caster->GetMapMgr()->GetUnit( m_targets.m_unitTarget );

		if( target )
		{
			// GM flagged players should be immune to other players' casts, but not their own.
			if(target->IsPlayer() && (m_caster->GetTypeId() == TYPEID_ITEM ? TO_ITEM(m_caster)->GetOwner() != target : m_caster != target) && TO_PLAYER(target)->bGMTagOn)
				return SPELL_FAILED_BM_OR_INVISGOD;

			//you can't mind control someone already mind controlled
			if (GetSpellProto()->NameHash == SPELL_HASH_MIND_CONTROL && target->GetAuraSpellIDWithNameHash(SPELL_HASH_MIND_CONTROL))
				return SPELL_FAILED_CANT_BE_CHARMED;

			if(target == m_caster && GetSpellProto()->AttributesEx & ATTRIBUTESEX_CANT_TARGET_SELF)
				return SPELL_FAILED_BAD_TARGETS;

			//these spells can be cast only on certain objects. Otherwise cool exploit
			//Most of this things goes to spell_forced_target table
			switch (GetSpellProto()->Id)
			{
				case 27907:// Disciplinary Rod
				{
					if( target->IsPlayer() )
						return SPELL_FAILED_BAD_TARGETS;
				}break;
			}
		}
	}

	if( p_caster != NULL )
	{
		if( GetSpellProto()->Id == 51721 )
		{
			if( !(p_caster->GetPlayerAreaID() == 4281) )
				return SPELL_FAILED_NOT_HERE;
		}

		if(GetSpellProto()->Flags7 & FLAGS7_NOT_IN_RAID_INSTANCE && p_caster->GetMapMgr()->GetdbcMap()->israid())
			return SPELL_FAILED_NOT_HERE;

		// flying auras
		if( GetSpellProto()->c_is_flags & SPELL_FLAG_IS_FLYING )
		{
			if(!p_caster->CanFlyInCurrentZoneOrMap()) // Check our area
				return SPELL_FAILED_NOT_HERE;
		}
		else
		{
			if(GetSpellProto()->Flags5 & FLAGS5_ONLY_IN_OUTLAND && p_caster->GetMapId() != 530)
				return SPELL_FAILED_NOT_HERE;
		}

		if( GetSpellProto()->Id == 53822 && p_caster->getClass()!=DEATHKNIGHT)			// DeathGate
			return SPELL_FAILED_SPELL_UNAVAILABLE;

		if(p_caster->HasFlag(PLAYER_FLAGS,PLAYER_FLAG_ALLOW_ONLY_ABILITY) &&
			!((GetSpellProto()->SpellGroupType[0] & p_caster->m_castFilter[0]) ||
			(GetSpellProto()->SpellGroupType[1] & p_caster->m_castFilter[1]) ||
			(GetSpellProto()->SpellGroupType[2] & p_caster->m_castFilter[2])))
			return SPELL_FAILED_SPELL_IN_PROGRESS;	// Need to figure the correct message

		uint32 self_rez = p_caster->GetUInt32Value(PLAYER_SELF_RES_SPELL);
		// if theres any spells that should be cast while dead let me know
		if( !p_caster->isAlive() && self_rez != GetSpellProto()->Id)
		{
			if( (m_targets.m_targetMask & TARGET_FLAG_SELF  || m_targets.m_unitTarget == p_caster->GetGUID() || !IsHealingSpell(m_spellInfo)) && p_caster->GetShapeShift() == FORM_SPIRITOFREDEMPTION)		// not a holy spell
					return SPELL_FAILED_SPELL_UNAVAILABLE;

			if(!(GetSpellProto()->Attributes & ATTRIBUTES_CASTABLE_WHILE_DEAD))
				return SPELL_FAILED_NOT_WHILE_GHOST;
		}

		if( GetSpellProto()->NameHash == SPELL_HASH_HUNTER_S_MARK )
		{
			if( GetUnitTarget() && !isHostile( GetUnitTarget(), m_caster ))
				return SPELL_FAILED_BAD_TARGETS;
		}

		if (p_caster->GetMapMgr() && p_caster->GetMapMgr()->CanUseCollision(p_caster))
		{
			if (GetSpellProto()->MechanicsType == MECHANIC_MOUNTED)
			{
				// Qiraj battletanks work everywhere on map 531
				if ( p_caster->GetMapId() == 531 && ( GetSpellProto()->Id == 25953 || GetSpellProto()->Id == 26054 || GetSpellProto()->Id == 26055 || GetSpellProto()->Id == 26056 ) )
					return SPELL_CANCAST_OK;

				if (CollideInterface.IsIndoor( p_caster->GetMapId(), p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ() + 2.0f ))
					return SPELL_FAILED_NO_MOUNTS_ALLOWED;
			}
			else if( GetSpellProto()->Attributes & ATTRIBUTES_ONLY_OUTDOORS )
			{
				if(CollideInterface.IsIndoor( p_caster->GetMapId(),p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ() + 2.0f ) )
					return SPELL_FAILED_ONLY_OUTDOORS;
			}
		}
		//are we in an arena and the spell cooldown is longer then 15mins?
		if ( p_caster->m_bg && ( p_caster->m_bg->GetType() >= BATTLEGROUND_ARENA_2V2 && p_caster->m_bg->GetType() <= BATTLEGROUND_ARENA_5V5 ) )
		{
			if(  GetSpellProto()->Flags5 & FLAGS5_NOT_USABLE_IN_ARENA )
				return SPELL_FAILED_NOT_IN_ARENA;

			if( !p_caster->m_bg->HasStarted() )
			{
				// cannot cast spells in an arena if it hasn't started (blinking through gates, lalala)
				if( GetSpellProto()->NameHash == SPELL_HASH_BLINK )
					return SPELL_FAILED_SPELL_UNAVAILABLE;
			}
		}

		// What a waste.
		if( p_caster->m_bg && GetGameObjectTarget())
		{
			if(p_caster->SchoolImmunityList[0]) // Physical is all that really matters.
				return SPELL_FAILED_SPELL_UNAVAILABLE;
		}

		if(p_caster->m_bg == NULL && GetSpellProto()->Flags4 & FLAGS4_BG_ONLY)
			return SPELL_FAILED_ONLY_BATTLEGROUNDS;

		// Requires ShapeShift (stealth only atm, need more work)
		if( GetSpellProto()->RequiredShapeShift )
		{
			if( GetSpellProto()->RequiredShapeShift == (uint32)1 << (FORM_STEALTH-1) )
			{
				if( !(((uint32)1 << (p_caster->GetShapeShift()-1)) & GetSpellProto()->RequiredShapeShift) && !p_caster->HasDummyAura(SPELL_HASH_SHADOW_DANCE) )
					return SPELL_FAILED_ONLY_STEALTHED;
			}
		}

		if(!u_caster->CombatStatus.IsInCombat() && GetSpellProto()->NameHash == SPELL_HASH_DISENGAGE)
			return SPELL_FAILED_SPELL_UNAVAILABLE;;

		// Disarm
		if( u_caster!= NULL )
		{
			if (GetSpellProto()->Attributes == ATTRIBUTES_REQ_OOC && u_caster->CombatStatus.IsInCombat())
			{
				// Charge In Combat, it's broke since 3.3.5, ??? maybe an aura state that needs to be set now
				//if ((GetSpellProto()->Id !=  100 && GetSpellProto()->Id != 6178 && GetSpellProto()->Id != 11578 ) )
				return SPELL_FAILED_TARGET_IN_COMBAT;
			}


			if( u_caster->disarmed )
			{
				if( GetSpellProto()->is_melee_spell )
					return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
				else if( GetSpellProto()->is_ranged_spell )
					return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
			}
			if( u_caster->disarmedShield && GetSpellProto()->RequiredItemFlags && (GetSpellProto()->RequiredItemFlags & (1 << INVTYPE_SHIELD)) )
					return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
		}

		// check for cooldowns
		if(!tolerate && !p_caster->Cooldown_CanCast(m_spellInfo))
			return SPELL_FAILED_NOT_READY;

		if(p_caster->GetDuelState() == DUEL_STATE_REQUESTED)
		{
			for(i = 0; i < 3; i++)
			{
				if( GetSpellProto()->Effect[i] && GetSpellProto()->Effect[i] != SPELL_EFFECT_APPLY_AURA && GetSpellProto()->Effect[i] != SPELL_EFFECT_APPLY_PET_AURA
					&& GetSpellProto()->Effect[i] != SPELL_EFFECT_APPLY_AREA_AURA)
				{
					return SPELL_FAILED_TARGET_DUELING;
				}
			}
		}

		if( playerTarget != NULL && p_caster != playerTarget &&
			playerTarget->GetDuelState() != DUEL_STATE_FINISHED &&
			GetSpellProto()->c_is_flags & SPELL_FLAG_CASTED_ON_FRIENDS )
			return SPELL_FAILED_TARGET_DUELING;

		// check for duel areas
		if( GetSpellProto()->Id == 7266 )
		{
			AreaTable* at = dbcArea.LookupEntry( p_caster->GetPlayerAreaID() );
			if(at->AreaFlags & AREA_CITY_AREA)
				return SPELL_FAILED_NO_DUELING;

			if( p_caster->m_bg )
				return SPELL_FAILED_NO_DUELING;
		}

		// check if spell is allowed while player is on a taxi
		if(p_caster->m_onTaxi)
		{
			// This uses the same flag as ordinary mounts
			if(!(GetSpellProto()->Attributes & ATTRIBUTES_MOUNT_CASTABLE))
				return SPELL_FAILED_NOT_ON_TAXI;
		}

		// check if spell is allowed while not mounted
		if(!p_caster->IsMounted())
		{
			if( GetSpellProto()->Id == 25860) // Reindeer Transformation
				return SPELL_FAILED_ONLY_MOUNTED;
		}
		else
		{
			if (!(GetSpellProto()->Attributes & ATTRIBUTES_MOUNT_CASTABLE))
				return SPELL_FAILED_NOT_MOUNTED;
		}

		if((GetSpellProto()->Id == 1850 || GetSpellProto()->Id == 9821 || GetSpellProto()->Id == 33357) && p_caster->GetShapeShift() != FORM_CAT)
			return SPELL_FAILED_ONLY_SHAPESHIFT;

		// no mana drains on shifted druids :(
		if( GetPlayerTarget() && GetPlayerTarget()->getClass() == DRUID && (GetSpellProto()->Effect[0] == SPELL_EFFECT_POWER_DRAIN || GetSpellProto()->Effect[1] == SPELL_EFFECT_POWER_DRAIN || GetSpellProto()->Effect[2] == SPELL_EFFECT_POWER_DRAIN))
		{
			if( GetPlayerTarget()->GetShapeShift() == FORM_BEAR ||
				GetPlayerTarget()->GetShapeShift() == FORM_DIREBEAR ||
				GetPlayerTarget()->GetShapeShift() == FORM_CAT)
				return SPELL_FAILED_BAD_TARGETS;
		}

		// check if spell is allowed while shapeshifted
		if(p_caster->GetShapeShift())
		{
			switch(p_caster->GetShapeShift())
			{
			case FORM_TREE:
			case FORM_BATTLESTANCE:
			case FORM_DEFENSIVESTANCE:
			case FORM_BERSERKERSTANCE:
			case FORM_SHADOW:
			case FORM_STEALTH:
			case FORM_MOONKIN:
				break;

			case FORM_SWIFT:
			case FORM_FLIGHT:
				{
					// check if item is allowed (only special items allowed in flight forms)
					if(i_caster && !(i_caster->GetProto()->Flags & ITEM_FLAG_SHAPESHIFT_OK))
						return SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED;
				}break;

			default:
				{
					// check if item is allowed (only special & equipped items allowed in other forms)
					if(i_caster && !(i_caster->GetProto()->Flags & ITEM_FLAG_SHAPESHIFT_OK))
						if(i_caster->GetProto()->InventoryType == INVTYPE_NON_EQUIP)
							return SPELL_FAILED_NO_ITEMS_WHILE_SHAPESHIFTED;
				}
			}
		}


		for(uint8 i = 0; i < 3; i++)
		{
			if( GetSpellProto()->Effect[i] == SPELL_EFFECT_OPEN_LOCK && GetSpellProto()->EffectMiscValue[i] == LOCKTYPE_SLOW_OPEN )
			{
				if( p_caster->m_MountSpellId )
					p_caster->RemoveAura( p_caster->m_MountSpellId );

				if( p_caster->GetVehicle() )
					p_caster->GetVehicle()->RemovePassenger( p_caster );

				if( p_caster->m_stealth )
				{
					p_caster->RemovePositiveAura( p_caster->m_stealth );
					p_caster->RemoveAuraByNameHash( SPELL_HASH_VANISH );
				}
				break;
			}
		}

		// check if spell is allowed while we have a battleground flag
		if(p_caster->m_bgHasFlag)
		{
			if( (GetSpellProto()->NameHash == SPELL_HASH_DIVINE_SHIELD || GetSpellProto()->NameHash == SPELL_HASH_ICE_BLOCK) )
			{
					if(p_caster->m_bg && p_caster->m_bg->GetType() == BATTLEGROUND_WARSONG_GULCH)
						TO_WARSONGGULCH(p_caster->m_bg)->DropFlag( p_caster );
					else if(p_caster->m_bg && p_caster->m_bg->GetType() == BATTLEGROUND_EYE_OF_THE_STORM)
						TO_EYEOFTHESTORM(p_caster->m_bg)->DropFlag( p_caster );
			}
		}

		// item spell checks
		if(i_caster != NULL)
		{
			if( i_caster->GetProto()->MapID && i_caster->GetProto()->MapID != i_caster->GetMapId() )
				return SPELL_FAILED_NOT_HERE;

			if(i_caster->GetProto()->Spells[0].Charges != 0)
			{
				// check if the item has the required charges
				if(i_caster->GetUInt32Value(ITEM_FIELD_SPELL_CHARGES) == 0)
				{
					//Mounts have changed, they should be added to known spells
					if(i_caster->GetProto()->Class != ITEM_CLASS_MISCELLANEOUS && i_caster->GetProto()->SubClass != ITEM_SUBCLASS_MISCELLANEOUS_MOUNT )
						return SPELL_FAILED_SPELL_LEARNED;
					else
					{
						Unit* target = (m_caster->IsInWorld()) ? m_caster->GetMapMgr()->GetUnit(m_targets.m_unitTarget) : NULLUNIT;
						if(target && target->IsPlayer())
						{
							//Allow spell to be casted if player didn't have this mount yet in pet tab (iow has the spell).
							if(i_caster->GetProto()->Spells[1].Id && TO_PLAYER(target)->HasSpell(i_caster->GetProto()->Spells[1].Id))
								return SPELL_FAILED_SPELL_LEARNED;
						}
					}
				}

				// for items that combine to create a new item, check if we have the required quantity of the item
				if(i_caster->GetProto()->ItemId == GetSpellProto()->Reagent[0] && (i_caster->GetProto()->Flags != 268435520))
					if(p_caster->GetItemInterface()->GetItemCount(GetSpellProto()->Reagent[0]) < GetSpellProto()->ReagentCount[0] + 1)
						return SPELL_FAILED_NEED_MORE_ITEMS;
			}

			// heal checks are only applied to item casters
			if( GetSpellProto()->c_is_flags & SPELL_FLAG_IS_HEALING_SPELL && p_caster->GetUInt32Value(UNIT_FIELD_HEALTH) == p_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH) )
				return SPELL_FAILED_ALREADY_AT_FULL_HEALTH;

			if( p_caster->GetPowerType() == POWER_TYPE_MANA && GetSpellProto()->c_is_flags & SPELL_FLAG_IS_HEALING_MANA_SPELL && p_caster->GetUInt32Value(UNIT_FIELD_POWER1) == p_caster->GetUInt32Value(UNIT_FIELD_MAXPOWER1) )
				return SPELL_FAILED_ALREADY_AT_FULL_MANA;
		}

		bool CheckReagents = true;
		if( GetSpellProto()->SpellGroupType )
		{
			//lets check if we need regeants
			uint32 AffectedSpellGroupType[3] = {0,0,0};
			for(uint32 x=0;x<3;x++)
				AffectedSpellGroupType[x] |= p_caster->GetUInt32Value(PLAYER_NO_REAGENT_COST_1+x);

			if( AffectedSpellGroupType )
			{
				for(uint32 x=0;x<3;x++)
					if( AffectedSpellGroupType[x] & GetSpellProto()->SpellGroupType[x] )
						CheckReagents = false;
			}
		}

		// check if we have the required reagents
		if( CheckReagents && (!i_caster || (i_caster->GetProto() && i_caster->GetProto()->Flags != 268435520)) && !p_caster->NoReagentCost)
			for(i=0; i<8 ;++i)
			{
				if( GetSpellProto()->Reagent[i] <= 0 || GetSpellProto()->ReagentCount[i] <= 0)
					continue;

				if(p_caster->GetItemInterface()->GetItemCount(GetSpellProto()->Reagent[i]) < GetSpellProto()->ReagentCount[i])
					return SPELL_FAILED_NEED_MORE_ITEMS;
			}

		// check if we have the required tools, totems, etc
		if( GetSpellProto()->Totem[0] != 0)
		{
			if(!p_caster->GetItemInterface()->GetItemCount(GetSpellProto()->Totem[0]))
				return SPELL_FAILED_TOTEMS;
		}
		if( GetSpellProto()->Totem[1] != 0)
		{
			if(!p_caster->GetItemInterface()->GetItemCount(GetSpellProto()->Totem[1]))
				return SPELL_FAILED_TOTEMS;
		}

		// stealth check
		if( GetSpellProto()->NameHash == SPELL_HASH_STEALTH || GetSpellProto()->NameHash == SPELL_HASH_PROWL )
		{
			if( p_caster->CombatStatus.IsInCombat() )
				return SPELL_FAILED_TARGET_IN_COMBAT;
		}

		if( ( GetSpellProto()->NameHash == SPELL_HASH_CANNIBALIZE || GetSpellProto()->Id == 46584 ))
		{
			bool check = false;
			for(Object::InRangeSet::iterator i = p_caster->GetInRangeSetBegin(); i != p_caster->GetInRangeSetEnd(); i++)
			{
				if(p_caster->GetDistance2dSq((*i)) <= 25)
					if((*i)->GetTypeId() == TYPEID_UNIT || (*i)->IsPlayer() )
						if( TO_UNIT(*i)->isDead() )
							check = true;
			}

			if( !check && GetSpellProto()->Id == 46584 )
			{
				if( p_caster->HasDummyAura( SPELL_HASH_GLYPH_OF_RAISE_DEAD ) )
					check = true;
				else if( p_caster->GetItemInterface()->GetItemCount( 37201 ) )
				{
					check = true;
					p_caster->GetItemInterface()->RemoveItemAmt( 37201, 1 );
				}
			}
			if( !check )
			{
				return SPELL_FAILED_NO_EDIBLE_CORPSES;
			}
		}

		// check if we have the required gameobject focus
		if( GetSpellProto()->RequiresSpellFocus)
		{
			float focusRange;
			bool found = false;

			for( unordered_set<Object*>::iterator itr = p_caster->GetInRangeSetBegin(); itr != p_caster->GetInRangeSetEnd(); itr++ )
			{
				if((*itr)->GetTypeId() != TYPEID_GAMEOBJECT)
					continue;

				if(TO_GAMEOBJECT(*itr)->GetType() != GAMEOBJECT_TYPE_SPELL_FOCUS)
					continue;

				GameObjectInfo *info = TO_GAMEOBJECT(*itr)->GetInfo();
				if(!info)
				{
					DEBUG_LOG("Spell","Warning: could not find info about game object %u", (*itr)->GetEntry());
					continue;
				}

				// lets read the distance from database (sound 1)
				focusRange = (float)info->sound1;

				// is that possible?
				if( !focusRange )
					focusRange = 5.0f;

				if(!IsInrange(p_caster->GetPositionX(), p_caster->GetPositionY(), p_caster->GetPositionZ(), (*itr), (focusRange * focusRange)))
					continue;

				if(info->SpellFocus == GetSpellProto()->RequiresSpellFocus)
				{
					found = true;
					break;
				}
			}

			if(!found)
				return SPELL_FAILED_REQUIRES_SPELL_FOCUS;
		}

		if( GetSpellProto()->AreaGroupId > 0)
		{
			bool found = false;
			uint16 area_id = p_caster->GetAreaID();
			uint32 zone_id = p_caster->GetZoneId();

			AreaGroup const* groupEntry = dbcAreaGroup.LookupEntry( GetSpellProto()->AreaGroupId );
			if( groupEntry )
			{
				for ( uint8 i=0; i<7; i++ )
				{
					if( groupEntry->AreaId[i] == zone_id || groupEntry->AreaId[i] == area_id )
					{
						found = true;
						break;
					}
				}
			}
			if(!found)
				return SPELL_FAILED_REQUIRES_AREA;
		}

		// aurastate check
		if( GetSpellProto()->CasterAuraState)
		{
			if( !p_caster->HasFlag( UNIT_FIELD_AURASTATE, 1 << (GetSpellProto()->CasterAuraState-1) ) )
				return SPELL_FAILED_CASTER_AURASTATE;
		}
	}

	// Targetted Item Checks
	if(m_targets.m_itemTarget && p_caster)
	{
		Item* i_target = NULLITEM;

		// check if the targeted item is in the trade box
		if( m_targets.m_targetMask & TARGET_FLAG_TRADE_ITEM )
		{
				// only lockpicking and enchanting can target items in the trade box
			if(HasSpellEffect(SPELL_EFFECT_OPEN_LOCK) || HasSpellEffect(SPELL_EFFECT_ENCHANT_ITEM) || HasSpellEffect(SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY))
			{
				// check for enchants that can only be done on your own items
				if( GetSpellProto()->Flags3 & FLAGS3_ENCHANT_OWN_ONLY )
					return SPELL_FAILED_BAD_TARGETS;

				// get the player we are trading with
				Player* t_player = p_caster->GetTradeTarget();
				// get the targeted trade item
				if( t_player != NULL )
					i_target = t_player->getTradeItem((uint32)m_targets.m_itemTarget);
				}
		}
		// targeted item is not in a trade box, so get our own item
		else
		{
			i_target = p_caster->GetItemInterface()->GetItemByGUID( m_targets.m_itemTarget );
		}

		// check to make sure we have a targeted item
		if( i_target == NULL )
			return SPELL_FAILED_BAD_TARGETS;

		ItemPrototype* proto = i_target->GetProto();

		// check to make sure we have it's prototype info
		if(!proto) 
			return SPELL_FAILED_BAD_TARGETS;

		// check to make sure the targeted item is acceptable
		// Enchanting Targeted Item Check
		if(HasSpellEffect(SPELL_EFFECT_ENCHANT_ITEM) || HasSpellEffect(SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY))
		{
			// check for enchants that can only be done on your own items, make sure they are soulbound
			if( GetSpellProto()->Flags3 & FLAGS3_ENCHANT_OWN_ONLY && i_target->GetOwner() != p_caster)
				return SPELL_FAILED_BAD_TARGETS;
			// check if we have the correct class, subclass, and inventory type of target item
			if( GetSpellProto()->EquippedItemClass != (int32)proto->Class && proto->Class != 7)
				return SPELL_FAILED_BAD_TARGETS;

			if( GetSpellProto()->EquippedItemSubClass && !(GetSpellProto()->EquippedItemSubClass & (1 << proto->SubClass)) &&  GetSpellProto()->EffectMiscValueB[0] != (int32)proto->SubClass )
				return SPELL_FAILED_BAD_TARGETS;

			if( GetSpellProto()->RequiredItemFlags && !(GetSpellProto()->RequiredItemFlags & (1 << proto->InventoryType)) && proto->InventoryType != 0 )
				return SPELL_FAILED_BAD_TARGETS;

			if (GetSpellProto()->Effect[0] == SPELL_EFFECT_ENCHANT_ITEM &&
				GetSpellProto()->baseLevel && (GetSpellProto()->baseLevel > proto->ItemLevel))
				return int8(SPELL_FAILED_BAD_TARGETS); // maybe there is different err code
		}

			// Disenchanting Targeted Item Check
		if(HasSpellEffect(SPELL_EFFECT_DISENCHANT))
		{
			// check if item can be disenchanted
			if(proto->DisenchantReqSkill < 1)
				return SPELL_FAILED_CANT_BE_DISENCHANTED;
			// check if we have high enough skill
			if((int32)p_caster->_GetSkillLineCurrent(SKILL_ENCHANTING) < proto->DisenchantReqSkill)
				return SPELL_FAILED_CANT_BE_DISENCHANTED_SKILL;
		}

		// Feed Pet Targeted Item Check
		if(HasSpellEffect(SPELL_EFFECT_FEED_PET))
		{
			Pet* pPet = p_caster->GetSummon();
			// check if we have a pet
			if(!pPet)
				return SPELL_FAILED_NO_PET;
			// check if item is food
			if(!proto->FoodType)
				return SPELL_FAILED_BAD_TARGETS;

			// check if food type matches pets diet
			if(!(pPet->GetPetDiet() & (1 << (proto->FoodType - 1))))
				return SPELL_FAILED_WRONG_PET_FOOD;
			// check food level: food should be max 30 lvls below pets level
			if(pPet->getLevel() > proto->ItemLevel + 30)
				return SPELL_FAILED_FOOD_LOWLEVEL;
		}

			// Prospecting Targeted Item Check
		if(HasSpellEffect(SPELL_EFFECT_PROSPECTING))
		{
			// check if the item can be prospected
			if(!(proto->Flags & ITEM_FLAG_PROSPECTABLE))
				return SPELL_FAILED_CANT_BE_PROSPECTED;
			// check if we have at least 5 of the item
			if(p_caster->GetItemInterface()->GetItemCount(proto->ItemId) < 5)
				return SPELL_FAILED_NEED_MORE_ITEMS;
			// check if we have high enough skill
			if(p_caster->_GetSkillLineCurrent(SKILL_JEWELCRAFTING) < proto->RequiredSkillRank)
				return SPELL_FAILED_LOW_CASTLEVEL;
		}

			// Milling Targeted Item Check
		if(HasSpellEffect(SPELL_EFFECT_MILLING))
		{
			// check if the item can be milled
			if(!(proto->Flags & ITEM_FLAG_MILLABLE))
				return SPELL_FAILED_CANT_BE_MILLED;
			// check if we have at least 5 of the item
			if(p_caster->GetItemInterface()->GetItemCount(proto->ItemId) < 5)
				return SPELL_FAILED_NEED_MORE_ITEMS;
			// check if we have high enough skill
			if(p_caster->_GetSkillLineCurrent(SKILL_INSCRIPTION) < proto->RequiredSkillRank)
				return SPELL_FAILED_LOW_CASTLEVEL;
		}
	}

	// set up our max Range
	//float maxRange = GetMaxRange( dbcSpellRange.LookupEntry( GetSpellProto()->rangeIndex ) );
	float maxRange = GetSpellProto()->base_range_or_radius;
	if(unitTarget && u_caster && !isAttackable( u_caster,unitTarget,!(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
		maxRange = GetSpellProto()->base_range_or_radius_friendly;

	if( GetSpellProto()->SpellGroupType && u_caster != NULL )
	{
		SM_FFValue( u_caster->SM[SMT_RANGE][0], &maxRange, GetSpellProto()->SpellGroupType );
		SM_PFValue( u_caster->SM[SMT_RANGE][1], &maxRange, GetSpellProto()->SpellGroupType );
	}

	// Targeted Location Checks (AoE spells)
	if( m_targets.m_targetMask == TARGET_FLAG_DEST_LOCATION )
	{
		if( !IsInrange( m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, m_caster, ( maxRange * maxRange ) ) )
			return SPELL_FAILED_OUT_OF_RANGE;
	}
	// Collision 2 broken for this :|
	//if(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION && !m_caster->IsInLineOfSight(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ))
		//return SPELL_FAILED_LINE_OF_SIGHT;

	Unit* target = NULLUNIT;
	if( m_targets.m_targetMask == TARGET_FLAG_SELF )
		target = u_caster;

	// Targeted Unit Checks
	if(m_targets.m_unitTarget)
	{
		if( m_targets.m_unitTarget == m_caster->GetGUID() && m_caster->IsUnit() )
			target = TO_UNIT(m_caster);
		else
			target = (m_caster->IsInWorld()) ? m_caster->GetMapMgr()->GetUnit(m_targets.m_unitTarget) : NULLUNIT;

		if(target!= NULL)
		{
			if( g_caster != NULL )
			{
				if(target->SchoolImmunityList[GetSpellProto()->School])
					return SPELL_FAILED_DAMAGE_IMMUNE;

				if(target->MechanicsDispels[GetSpellProto()->MechanicsType])
					return SPELL_FAILED_DAMAGE_IMMUNE;
			}

			if( target != m_caster )
			{
				// Partha: +2.52yds to max range, this matches the range the client is calculating.
				// see extra/supalosa_range_research.txt for more info

				if( tolerate ) // add an extra 33% to range on final check (squared = 1.78x)
				{
					if( !IsInrange( m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), target, ( ( maxRange + 2.52f ) * ( maxRange + 2.52f ) * 1.78f ) ) )
						return SPELL_FAILED_OUT_OF_RANGE;
				}
				else
				{
					// Added +2 because there's always someone who forgot to put CombatReach into the DB and latency compensation
					float targetRange = maxRange + target->GetSize() + (u_caster ? u_caster->GetSize() : 0 ) + 2;
					if( !IsInrange(m_caster, target, targetRange * targetRange ) )
						return SPELL_FAILED_OUT_OF_RANGE;
				}
			}

			if( p_caster != NULL )
			{
				if( GetSpellProto()->forced_creature_target )
				{
					if( !target->IsCreature() )
						return SPELL_FAILED_BAD_TARGETS;

					if( TO_CREATURE( target )->GetCreatureInfo() != NULL )
						if( GetSpellProto()->forced_creature_target != TO_CREATURE( target )->GetCreatureInfo()->Id )
							return SPELL_FAILED_BAD_TARGETS;
				}

				if( GetSpellProto()->Id == SPELL_RANGED_THROW)
				{
					Item* itm = p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
					if(!itm || ((itm->GetDurability() == 0) && itm->GetDurabilityMax()))
						return SPELL_FAILED_NO_AMMO;
				}

				if ( target != m_caster && !m_caster->IsInLineOfSight(target) )
				{
					return SPELL_FAILED_LINE_OF_SIGHT;
				}


				// check aurastate
				if( GetSpellProto()->TargetAuraState && !skip)
				{
					if( !target->HasFlag( UNIT_FIELD_AURASTATE, 1<<(GetSpellProto()->TargetAuraState-1) ) )
					{
						return SPELL_FAILED_TARGET_AURASTATE;
					}
				}

				if(target->IsPlayer())
				{
					// disallow spell casting in sanctuary zones
					// allow attacks in duels
					if( p_caster->DuelingWith != target && !isFriendly( p_caster, target ) )
					{
						AreaTable* atCaster = dbcArea.LookupEntry( p_caster->GetPlayerAreaID() );
						AreaTable* atTarget = dbcArea.LookupEntry( TO_PLAYER( target )->GetPlayerAreaID() );
						if( atCaster->AreaFlags & 0x800 || atTarget->AreaFlags & 0x800 )
							return SPELL_FAILED_NOT_HERE;
					}
				}

				if( GetSpellProto()->EffectApplyAuraName[0] == 2)//mind control
				{
					if( GetSpellProto()->EffectBasePoints[0])//got level req;
					{
						if((int32)target->getLevel() > GetSpellProto()->EffectBasePoints[0]+1 + int32(p_caster->getLevel() - GetSpellProto()->spellLevel))
							return SPELL_FAILED_HIGHLEVEL;
						else if(target->GetTypeId() == TYPEID_UNIT)
						{
							Creature* c =  TO_CREATURE(target);
							if (c&&c->GetCreatureInfo()&&c->GetCreatureInfo()->Rank >ELITE_ELITE)
								return SPELL_FAILED_HIGHLEVEL;
						}
					}
				}
			}

			// scripted spell stuff
			switch(GetSpellProto()->Id)
			{
				case 603: //curse of doom, can't be casted on players
				case 30910:
				case 47867:
				{
					if(target->IsPlayer())
						return SPELL_FAILED_TARGET_IS_PLAYER;
				}break;

				case 13907:
				{
					if (!target || target->IsPlayer() || target->GetCreatureType()!=TARGET_TYPE_DEMON )
						return SPELL_FAILED_SPELL_UNAVAILABLE;
				}break;

				// disable spell
				case 25997: // Eye for an Eye
				case 38554: //Absorb Eye of Grillok
				{
					// do not allow spell to be cast
					return SPELL_FAILED_SPELL_UNAVAILABLE;
				}break;

				//These spells are NPC only.
				case 25166: //Call Glyphs of Warding
				case 38892: //Shadow Bolt
				case 40536: //Chain Lightning
				case 41078: //Shadow Blast
				{
					if(u_caster->IsPlayer())
						return SPELL_FAILED_BAD_TARGETS;
				}break;

				case 982: //Revive Pet
				{
					Pet* pPet = p_caster->GetSummon();
					if(pPet && !pPet->isDead())
						return SPELL_FAILED_TARGET_NOT_DEAD;
				}break;
			}

			// if the target is not the unit caster and not the masters pet
			if(target != u_caster && !m_caster->IsPet())
			{

				/***********************************************************
				* Inface checks, these are checked in 2 ways
				* 1e way is check for damage type, as 3 is always ranged
				* 2e way is trough the data in the extraspell db
				*
				**********************************************************/

				/* burlex: units are always facing the target! */
				if(p_caster && GetSpellProto()->FacingCasterFlags)
				{
					if(!p_caster->isInFront(target))
						return SPELL_FAILED_UNIT_NOT_INFRONT;
					if((GetSpellProto()->Flags3 & FLAGS3_REQ_BEHIND_TARGET) && (GetSpellProto()->Id != SPELL_RANGED_THROW) &&
						target->isInFront(p_caster))
						return SPELL_FAILED_NOT_BEHIND;
				}
			}

			// if target is already skinned, don't let it be skinned again
			if( GetSpellProto()->Effect[0] == SPELL_EFFECT_SKINNING) // skinning
				if(target->IsUnit() && (TO_CREATURE(target)->Skinned) )
					return SPELL_FAILED_TARGET_UNSKINNABLE;

			// target 39 is fishing, all fishing spells are handled
			if( GetSpellProto()->EffectImplicitTargetA[0] == 39 )
			{
				uint32 entry = GetSpellProto()->EffectMiscValue[0];
				if(entry == GO_FISHING_BOBBER)
				{
					float px=u_caster->GetPositionX();
					float py=u_caster->GetPositionY();
					float orient = m_caster->GetOrientation();
					float posx = 0,posy = 0,posz = 0;
					float co = cos(orient);
					float si = sin(orient);
					MapMgr* map = m_caster->GetMapMgr();

					float r;
					for(r=20; r>10; r--)
					{
						posx = px + r * co;
						posy = py + r * si;
						posz = map->GetWaterHeight(posx,posy);
						if(posz > map->GetLandHeight(posx,posy))//water
							break;
					}
					if(r<=10)
						return SPELL_FAILED_NOT_FISHABLE;

					// if we are already fishing, dont cast it again
					if(p_caster->GetSummonedObject())
						if(p_caster->GetSummonedObject()->GetEntry() == GO_FISHING_BOBBER)
							return SPELL_FAILED_SPELL_IN_PROGRESS;
				}
			}

			if( p_caster != NULL )
			{
				if( GetSpellProto()->NameHash == SPELL_HASH_GOUGE )// Gouge
					if(!target->isInFront(p_caster))
						return SPELL_FAILED_NOT_INFRONT;

				if( GetSpellProto()->Category==1131)//Hammer of wrath, requires target to have 20- % of hp
				{
					if(target->GetUInt32Value(UNIT_FIELD_HEALTH) == 0)
						return SPELL_FAILED_BAD_TARGETS;

					if(target->GetUInt32Value(UNIT_FIELD_MAXHEALTH)/target->GetUInt32Value(UNIT_FIELD_HEALTH)<5)
						 return SPELL_FAILED_BAD_TARGETS;
				}
				else if( GetSpellProto()->NameHash == SPELL_HASH_CONFLAGRATE)//Conflagrate, requires immolation spell on victim
				{
					if(!target->HasAurasOfNameHashWithCaster(SPELL_HASH_IMMOLATION, NULL))
						return SPELL_FAILED_BAD_TARGETS;
				}

				if( GetSpellProto()->NameHash == SPELL_HASH_ENVENOM )
				{
					if( !target->HasAuraVisual(5100) )
						return SPELL_FAILED_BAD_TARGETS ;
				}

				if(target->dispels[GetSpellProto()->DispelType])
					return SPELL_FAILED_PREVENTED_BY_MECHANIC-1;			// hackfix - burlex
			}

			// if we're replacing a higher rank, deny it
			if( GetSpellProto()->buffType > 0 && target != m_caster && !(GetSpellProto()->Flags5 & FLAGS5_IGNORE_AURA_RANK_CHECK))
			{
				AuraCheckResponse acr = target->AuraCheck(m_spellInfo);
				if( acr.Error == AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT )
					return SPELL_FAILED_AURA_BOUNCED;
			}

			//check if we are trying to stealth or turn invisible but it is not allowed right now
			if( IsStealthSpell() || IsInvisibilitySpell() )
			{
				//if we have Faerie Fire, we cannot stealth or turn invisible
				if( u_caster->HasNegativeAuraWithNameHash( SPELL_HASH_FAERIE_FIRE ) || u_caster->HasNegativeAuraWithNameHash( SPELL_HASH_FAERIE_FIRE__FERAL_ ) )
					return SPELL_FAILED_SPELL_UNAVAILABLE;
			}

			if( target->IsPlayer() )
			{
				switch( GetSpellProto()->NameHash )
				{
				case SPELL_HASH_DIVINE_PROTECTION:
				case SPELL_HASH_DIVINE_SHIELD:
				case SPELL_HASH_HAND_OF_PROTECTION:
					{
						if( TO_PLAYER(target)->mForbearance )
							return SPELL_FAILED_DAMAGE_IMMUNE;

						if( !TO_PLAYER(target)->mAvengingWrath )
							return SPELL_FAILED_DAMAGE_IMMUNE;
					}break;

				case SPELL_HASH_AVENGING_WRATH:
					{
						if( !TO_PLAYER(target)->mAvengingWrath )
							return SPELL_FAILED_DAMAGE_IMMUNE;
					}break;

				case SPELL_HASH_ICE_BLOCK:
					{
						if( TO_PLAYER(target)->mHypothermia )
							return SPELL_FAILED_DAMAGE_IMMUNE;

					}break;

				case SPELL_HASH_POWER_WORD__SHIELD:
					{
						if( TO_PLAYER(target)->mWeakenedSoul )
							return SPELL_FAILED_DAMAGE_IMMUNE;
					}break;

				case SPELL_HASH_FIRST_AID:
					{
						if( TO_PLAYER(target)->mRecentlyBandaged )
							return SPELL_FAILED_DAMAGE_IMMUNE;
					}break;
				case SPELL_HASH_HEROISM:
					{
						if( TO_PLAYER(target)->mExhaustion )
							return SPELL_FAILED_DAMAGE_IMMUNE;
					}break;
				case SPELL_HASH_BLOODLUST:
					{
						if( TO_PLAYER(target)->mSated )
							return SPELL_FAILED_DAMAGE_IMMUNE;
					}break;
				}
			}

			if (GetSpellProto()->MechanicsType == 16 && target->mRecentlyBandaged)
				return SPELL_FAILED_DAMAGE_IMMUNE;
		}
	}

	// Special State Checks (for creatures & players)
	if( u_caster )
	{
		if( u_caster->SchoolCastPrevent[GetSpellProto()->School] )
		{
			uint32 now_ = getMSTime();
			if( now_ < u_caster->SchoolCastPrevent[GetSpellProto()->School] )
				return SPELL_FAILED_SILENCED;
		}

		if( u_caster->m_silenced && GetSpellProto()->School != NORMAL_DAMAGE )
		{
				// HACK FIX
				switch( GetSpellProto()->NameHash )
				{
					case SPELL_HASH_ICE_BLOCK: //Ice Block
					case 0x9840A1A6: //Divine Shield
						break;

					case 0x3DFA70E5: //Will of the Forsaken
						{
							if( u_caster->m_special_state & ( UNIT_STATE_FEAR | UNIT_STATE_CHARM | UNIT_STATE_SLEEP ) )
								break;
						}break;

					case 0xF60291F4: //Death Wish
					case 0x19700707: //Berserker Rage
						{
							if( u_caster->m_special_state & UNIT_STATE_FEAR )
								break;
						}break;

					// {Insignia|Medallion} of the {Horde|Alliance}
					case 0xC7C45478: //Immune Movement Impairment and Loss of Control
					case SPELL_HASH_PVP_TRINKET: // insignia of the alliance/horde 2.4.3
					case SPELL_HASH_EVERY_MAN_FOR_HIMSELF:
					case SPELL_HASH_DISPERSION:
						{
							break;
						}

					case 0xCD4CDF55: // Barksin
					{ // This spell is usable while stunned, frozen, incapacitated, feared or asleep.  Lasts 12 sec.
						if( u_caster->m_special_state & ( UNIT_STATE_STUN | UNIT_STATE_FEAR | UNIT_STATE_SLEEP ) ) // Uh, what unit_state is Frozen? (freezing trap...)
							break;
					}

					default:
							return SPELL_FAILED_SILENCED;
				}
		}

		if(target != NULL) /* -Supalosa- Shouldn't this be handled on Spell Apply? */
		{
			for( int i = 0; i < 3; i++ ) // if is going to cast a spell that breaks stun remove stun auras, looks a bit hacky but is the best way i can find
			{
				if( GetSpellProto()->EffectApplyAuraName[i] == SPELL_AURA_MECHANIC_IMMUNITY )
					target->RemoveAllAurasByMechanic( GetSpellProto()->EffectMiscValue[i] , -1 , true );
			}
		}

		if( u_caster->IsPacified() && GetSpellProto()->School == NORMAL_DAMAGE ) // only affects physical damage
		{
			// HACK FIX
			switch( GetSpellProto()->NameHash )
			{
				case SPELL_HASH_ICE_BLOCK: //Ice Block
				case 0x9840A1A6: //Divine Shield
				case 0x3DFA70E5: //Will of the Forsaken
				{
					if( u_caster->m_special_state & (UNIT_STATE_FEAR | UNIT_STATE_CHARM | UNIT_STATE_SLEEP))
						break;
				}break;
				case SPELL_HASH_PVP_TRINKET: // insignia of the alliance/horde 2.4.3
				case SPELL_HASH_EVERY_MAN_FOR_HIMSELF:
				case SPELL_HASH_DISPERSION:
					break;

				default:
					return SPELL_FAILED_PACIFIED;
			}
		}

		if( u_caster->IsStunned() || u_caster->IsFeared())
		{
			 //HACK FIX
			switch( GetSpellProto()->NameHash )
			{
				case SPELL_HASH_HAND_OF_FREEDOM:
				{
					if( !u_caster->HasDummyAura(SPELL_HASH_DIVINE_PURPOSE) )
						return SPELL_FAILED_STUNNED;
				}break;
				case SPELL_HASH_ICE_BLOCK: //Ice Block
				case SPELL_HASH_DIVINE_SHIELD: //Divine Shield
				case SPELL_HASH_DIVINE_PROTECTION: //Divine Protection
				case 0xCD4CDF55: //Barkskin
					break;
				/* -Supalosa- For some reason, being charmed or sleep'd is counted as 'Stunned'.
				Check it: http://www.wowhead.com/?spell=700 */

				case 0xC7C45478: /* Immune Movement Impairment and Loss of Control (PvP Trinkets) */
					break;

				case 0x3DFA70E5: /* Will of the Forsaken (Undead Racial) */
					break;

				case SPELL_HASH_PVP_TRINKET: // insignia of the alliance/horde 2.4.3*/
				case SPELL_HASH_EVERY_MAN_FOR_HIMSELF:
					break;

				case SPELL_HASH_BLINK:
					break;

				case SPELL_HASH_DISPERSION:
					break;

				default:
					{
						if(u_caster->IsStunned() && !(GetSpellProto()->Flags6 & FLAGS6_USABLE_WHILE_STUNNED) || 
							u_caster->IsFeared() && !(GetSpellProto()->Flags6 & FLAGS6_USABLE_WHILE_FEARED))
							return SPELL_FAILED_STUNNED;
					}
			}
		}

		if(u_caster->GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT) > 0)
		{
			SpellEntry * t_spellInfo = (u_caster->GetCurrentSpell() ? u_caster->GetCurrentSpell()->m_spellInfo : NULL);

			if(!t_spellInfo || !m_triggeredSpell)
				return SPELL_FAILED_SPELL_IN_PROGRESS;
			else if (t_spellInfo)
			{
				if(
					t_spellInfo->EffectTriggerSpell[0] != GetSpellProto()->Id &&
					t_spellInfo->EffectTriggerSpell[1] != GetSpellProto()->Id &&
					t_spellInfo->EffectTriggerSpell[2] != GetSpellProto()->Id)
				{
					return SPELL_FAILED_SPELL_IN_PROGRESS;
				}
			}
		}
	}

	// no problems found, so we must be ok
	return SPELL_CANCAST_OK;
}

void Spell::RemoveItems()
{
	// Item Charges & Used Item Removal
	if(i_caster)
	{
		// Stackable Item -> remove 1 from stack
		if(i_caster->GetUInt32Value(ITEM_FIELD_STACK_COUNT) > 1)
		{
			i_caster->ModUnsigned32Value(ITEM_FIELD_STACK_COUNT, -1);
			i_caster->m_isDirty = true;
		}
		// Expendable Item
		else if(i_caster->GetProto()->Spells[0].Charges < 0
			 || i_caster->GetProto()->Spells[1].Charges == -1) // hackfix for healthstones/mana gems/depleted items
		{
			// if item has charges remaining -> remove 1 charge
			if(((int32)i_caster->GetUInt32Value(ITEM_FIELD_SPELL_CHARGES)) < -1)
			{
				i_caster->ModSignedInt32Value(ITEM_FIELD_SPELL_CHARGES, 1);
				i_caster->m_isDirty = true;
			}
			// if item has no charges remaining -> delete item
			else
			{
				if(i_caster->GetOwner()) // wtf?
				{
					i_caster->GetOwner()->GetItemInterface()->SafeFullRemoveItemByGuid(i_caster->GetGUID());
					i_caster = NULLITEM;
				}
			}
		}
		// Non-Expendable Item -> remove 1 charge
		else if(i_caster->GetProto()->Spells[0].Charges > 0)
		{
			i_caster->ModSignedInt32Value(ITEM_FIELD_SPELL_CHARGES, -1);
			i_caster->m_isDirty = true;
		}
	}

	// Ammo Removal
	if( p_caster && p_caster->RequireAmmo && GetSpellProto()->Id != 53254 && (GetSpellProto()->Flags3 == FLAGS3_REQ_RANGED_WEAPON || GetSpellProto()->Flags4 & FLAGS4_PLAYER_RANGED_SPELLS))
	{
		p_caster->GetItemInterface()->RemoveItemAmt_ProtectPointer(p_caster->GetUInt32Value(PLAYER_AMMO_ID), 1, &i_caster);
	}

	// Reagent Removal
	for(uint32 i=0; i<8 ;++i)
	{
		if( p_caster && GetSpellProto()->Reagent[i] && !p_caster->NoReagentCost)
		{
			p_caster->GetItemInterface()->RemoveItemAmt_ProtectPointer(GetSpellProto()->Reagent[i], GetSpellProto()->ReagentCount[i], &i_caster);
		}
	}
}

int32 Spell::CalculateEffect(uint32 i,Unit* target)
{
	// TODO: Add ARMOR CHECKS; Add npc that have ranged weapons use them;
	// Range checks
/*	if(GetSpellProto()->Id == SPELL_RANGED_GUN) // this includes bow and gun
	{
		if(!u_caster || !unitTarget)
			return 0;

		return ::CalculateDamage( u_caster, unitTarget, RANGED, GetSpellProto()->SpellGroupType );
	}*/

	if( GetSpellProto()->Id == 3018 )
		m_spellInfo = dbcSpell.LookupEntry(75);

	int32 value = 0;

	float basePointsPerLevel = GetSpellProto()->EffectRealPointsPerLevel[i];
	int32 basePoints = GetSpellProto()->EffectBasePoints[i] + 1;
	int32 randomPoints = GetSpellProto()->EffectDieSides[i];

	//added by Zack : some talents inherit their basepoints from the previously casted spell: see mage - Master of Elements
	if(forced_basepoints[i])
		basePoints = forced_basepoints[i];

	if( u_caster != NULL )
	{
		int32 diff = -(int32)GetSpellProto()->baseLevel;
		if (GetSpellProto()->maxLevel && u_caster->getLevel()>GetSpellProto()->maxLevel)
			diff += GetSpellProto()->maxLevel;
		else
			diff += u_caster->getLevel();
//		Crow:
//		randomPoints += float2int32(diff * 1); // Wrong, we get random on spells without random figures, mounts, dual specs, etc.
//		randomPoints += float2int32(diff); // Wrong, we get random on spells we shouldn't.
//		The below is fine, but it is still incorrect.
//		We used to grab a dice figure from the dbc but it was removed in 3.3
//		This is fine though, we should be able to do a decent amount of damage, maybe even fix some issues.
//		randomPoints += float2int32(diff * 0);
		basePoints += float2int32(diff * basePointsPerLevel);
	}

	if(randomPoints <= 1)
		value = basePoints;
	else
		value = basePoints + RandomUInt((uint32)randomPoints);

	if( p_caster != NULL )
	{
		int32 comboDamage = (int32)GetSpellProto()->EffectPointsPerComboPoint[i];

		if(comboDamage)
		{
			value += ( comboDamage * p_caster->m_comboPoints );
			//this is ugly so i will explain the case maybe someone ha a better idea :
			// while casting a spell talent will trigger uppon the spell prepare faze
			// the effect of the talent is to add 1 combo point but when triggering spell finishes it will clear the extra combo point
			p_caster->m_spellcomboPoints = 0;
		}

		if( GetSpellProto()->Id == 49020 )
		{
			if( u_caster != NULL )
			{
				Player* plr = NULL;
				if(u_caster->IsPlayer())
					plr = TO_PLAYER(u_caster);

				uint32 diseasecount = 0;
				uint32 diseases[2] = { 55078, 55095 };
				for(int8 i = 0; i < 2; i++)
				{
					if(unitTarget->HasAura(diseases[i]))
					{
						diseasecount++;
						if(plr != NULL)
						{
							uint32 keepchance = plr->AnnihilationProcChance;
							if(keepchance > 0)
							{
								if(!Rand(keepchance))
									unitTarget->RemoveAura(diseases[i]);
							}
							else
								unitTarget->RemoveAura(diseases[i]);
						}
						else
							unitTarget->RemoveAura(diseases[i]);
					}
				}
				if(diseasecount)
					value += value*(0.125f*diseasecount);
			}
		}
		SpellOverrideMap::iterator itr = p_caster->mSpellOverrideMap.find(GetSpellProto()->Id);
		if(itr != p_caster->mSpellOverrideMap.end())
		{
			ScriptOverrideList::iterator itrSO;
			for(itrSO = itr->second->begin(); itrSO != itr->second->end(); itrSO++)
			{
				//DK:FIXME->yeni bir map olu�tur
				// Capt: WHAT THE FUCK DOES THIS MEAN....
				// Supa: WHAT THE FUCK DOES THIS MEAN?
				// Crow: FOUND OUT WHAT IT MEANS! yeni bir map olu tur = map it with a new round
				value += RandomUInt((*itrSO)->damage);
			}
		}
	}

	Unit* caster = u_caster;
	if( i_caster != NULL && target && target->GetMapMgr() && i_caster->GetUInt64Value( ITEM_FIELD_CREATOR ) )
	{
		//we should inherit the modifiers from the conjured food caster
		Unit* item_creator = target->GetMapMgr()->GetUnit( i_caster->GetUInt64Value( ITEM_FIELD_CREATOR ) );
		if( item_creator != NULL )
			caster = item_creator;
	}

	if( caster != NULL )
	{
		int32 spell_flat_modifers=0;
		int32 spell_pct_modifers=0;

		SM_FIValue(caster->SM[SMT_MISC_EFFECT][0],&spell_flat_modifers,GetSpellProto()->SpellGroupType);
		SM_FIValue(caster->SM[SMT_MISC_EFFECT][1],&spell_pct_modifers,GetSpellProto()->SpellGroupType);

		if( i == 0 )
		{
			SM_FIValue(caster->SM[SMT_FIRST_EFFECT_BONUS][0],&spell_flat_modifers,GetSpellProto()->SpellGroupType);
			SM_FIValue(caster->SM[SMT_FIRST_EFFECT_BONUS][1],&spell_pct_modifers,GetSpellProto()->SpellGroupType);
		}else if( i == 1 )
		{
			SM_FIValue(caster->SM[SMT_SECOND_EFFECT_BONUS][0],&spell_flat_modifers,GetSpellProto()->SpellGroupType);
			SM_FIValue(caster->SM[SMT_SECOND_EFFECT_BONUS][1],&spell_pct_modifers,GetSpellProto()->SpellGroupType);
		}
		if( ( i == 2 ) ||
			( i == 1 && GetSpellProto()->Effect[2] == 0 ) ||
			( i == 0 && GetSpellProto()->Effect[1] == 0 && GetSpellProto()->Effect[2] == 0 ) )
		{
			SM_FIValue(caster->SM[SMT_LAST_EFFECT_BONUS][0],&spell_flat_modifers,GetSpellProto()->SpellGroupType);
			SM_FIValue(caster->SM[SMT_LAST_EFFECT_BONUS][1],&spell_pct_modifers,GetSpellProto()->SpellGroupType);
		}
		value += float2int32(value*(float)(spell_pct_modifers/100.0f)) + spell_flat_modifers;
	}

	return value;
}

void Spell::HandleTeleport(uint32 id, Unit* Target)
{
	if(Target == NULL || Target->GetTypeId() != TYPEID_PLAYER || !id || id == 1)
		return;

	Player* pTarget = TO_PLAYER( Target );

	uint32 mapid;
	int32 phase = 1;
	float x,y,z,o;

	TeleportCoords* TC = TeleportCoordStorage.LookupEntry(id);
	if(TC == NULL)
	{
		switch(id)
		{
	/*	case :
			{
				mapid = ;
				x = f;
				y = f;
				z = f;
				o = 0.0f;
			}break;*/
		case 556: // Hearthstone effects.
		case 8690:
		case 39937:
			{
				mapid = pTarget->GetBindMapId();
				x = pTarget->GetBindPositionX();
				y = pTarget->GetBindPositionY();
				z = pTarget->GetBindPositionZ();
				o = pTarget->GetOrientation();
			}break;
		case 59901: // Portal Effect: Caverns Of Time
			{
				mapid = 1;
				x = -8164.8f;
				y = -4768.5f;
				z = 34.3f;
				o = 0.0f;
			}break;
		case 61419: // Portal Effect: The purple parlor
			{
				mapid = 571;
				x = 5848.48f;
				y = 853.706f;
				z = 843.182f;
				o = 0.0f;
			}break;
		case 61420: // Portal Effect: Violet Citadel
			{
				mapid = 571;
				x = 5819.26f;
				y = 829.774f;
				z = 680.22f;
				o = 0.0f;
			}break;

		default:
			if(m_targets.m_destX && m_targets.m_destY)
			{
				mapid = pTarget->GetMapId();
				x = m_targets.m_destX;
				y = m_targets.m_destY;
				z = m_targets.m_destZ;
				o = pTarget->GetOrientation();
			}
			else
			{
				if(sLog.IsOutDevelopement())
					printf("Unknown teleport spell: %u\n", id);
				else
					OUT_DEBUG("Unknown teleport spell: %u", id);
				return;
			}break;
		}
	}
	else
	{
		mapid = TC->mapId;
		x = TC->x;
		y = TC->y;
		z = TC->z;
		o = TC->o;
//		phase = TC->p;
	}

	pTarget->EventAttackStop();
	pTarget->SetSelection(NULL);

	// We use a teleport event on this one. Reason being because of UpdateCellActivity,
	// the game object set of the updater thread WILL Get messed up if we teleport from a gameobject caster.
	if(!sEventMgr.HasEvent(pTarget, EVENT_PLAYER_TELEPORT))
		sEventMgr.AddEvent(pTarget, &Player::EventTeleport, mapid, x, y, z, o, phase, EVENT_PLAYER_TELEPORT, 1, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Spell::CreateItem(uint32 itemId)
{
	if( !itemId )
		return;

	Player*			pUnit = TO_PLAYER( m_caster );
	Item*			newItem = NULLITEM;
	Item*			add = NULLITEM;
	SlotResult		slotresult;
	ItemPrototype*	m_itemProto;

	m_itemProto = ItemPrototypeStorage.LookupEntry( itemId );
	if( m_itemProto == NULL )
		return;

	if (pUnit->GetItemInterface()->CanReceiveItem(m_itemProto, 1, NULL))
	{
		SendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
		return;
	}

	add = pUnit->GetItemInterface()->FindItemLessMax(itemId, 1, false);
	if (!add)
	{
		slotresult = pUnit->GetItemInterface()->FindFreeInventorySlot(m_itemProto);
		if(!slotresult.Result)
		{
			 SendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
			 return;
		}

		newItem = objmgr.CreateItem(itemId, pUnit);
		AddItemResult result = pUnit->GetItemInterface()->SafeAddItem(newItem, slotresult.ContainerSlot, slotresult.Slot);
		if(!result)
		{
			delete newItem;
			newItem = NULLITEM;
			return;
		}

		newItem->SetUInt64Value(ITEM_FIELD_CREATOR,m_caster->GetGUID());
		newItem->SetUInt32Value(ITEM_FIELD_STACK_COUNT, damage);
		p_caster->GetSession()->SendItemPushResult(newItem,true,false,true,true,slotresult.ContainerSlot,slotresult.Slot,1);
		newItem->m_isDirty = true;

	}
	else
	{
		add->SetUInt32Value(ITEM_FIELD_STACK_COUNT,add->GetUInt32Value(ITEM_FIELD_STACK_COUNT) + damage);
		p_caster->GetSession()->SendItemPushResult(add,true,false,true,false,p_caster->GetItemInterface()->GetBagSlotByGuid(add->GetGUID()),0xFFFFFFFF,1);
		add->m_isDirty = true;
	}
}

void Spell::SendHealSpellOnPlayer( Object* caster, Object* target, uint32 dmg, bool critical, uint32 overheal, uint32 spellid)
{
	if( caster == NULL || target == NULL || !target->IsPlayer())
		return;

	WorldPacket data(SMSG_SPELLHEALLOG, 25);
	data << target->GetNewGUID();
	data << caster->GetNewGUID();
	data << uint32(spellid);
	data << uint32(dmg);
	data << uint32(overheal);
	data << uint32(0);
	data << uint8(critical);
	caster->SendMessageToSet(&data, true);
}

void Spell::SendHealManaSpellOnPlayer(Object* caster, Object* target, uint32 dmg, uint32 powertype, uint32 spellid)
{
	if( caster == NULL || target == NULL || !target->IsPlayer())
		return;

	WorldPacket data(SMSG_SPELLENERGIZELOG, 29);
	data.append(target->GetNewGUID());
	data.append(caster->GetNewGUID());
	data << uint32(spellid);
	data << uint32(powertype);
	data << uint32(dmg);
	caster->SendMessageToSet(&data, true);
}

void Spell::Heal(int32 amount)
{
	if( unitTarget == NULL || !unitTarget->isAlive() )
		return;

	if( p_caster != NULL )
		p_caster->last_heal_spell=m_spellInfo;

	//self healing shouldn't flag himself
	if( p_caster != NULL && p_caster->GetGUID() != unitTarget->GetGUID() )
	{
		// Healing a flagged target will flag you.
		if( unitTarget->IsPvPFlagged() )
		{
			if( !p_caster->IsPvPFlagged() )
				p_caster->PvPToggle();
			else
				p_caster->SetPvPFlag();
		}
	}

	//Make it critical
	bool critical = false;
	float critchance = 0;
	//int32 bonus = 0;
	if( u_caster != NULL )
	{
		// All calculations are done in getspellbonusdamage
		amount = u_caster->GetSpellBonusDamage(unitTarget, m_spellInfo, amount, false, true); // 3.0.2 Spellpower change: In order to keep the effective amount healed for a given spell the same, we’d expect the original coefficients to be multiplied by 1/0.532 or 1.88.

		// Healing Way fix
 		if(GetSpellProto()->NameHash == SPELL_HASH_HEALING_WAVE)
		{
			if(unitTarget->HasActiveAura(29203))
				amount += amount * 18 / 100;
		}
		else if( GetSpellProto()->NameHash == SPELL_HASH_HOLY_LIGHT )
		{
			if( unitTarget->HasDummyAura(SPELL_HASH_GLYPH_OF_HOLY_LIGHT) )
			{
				uint32 GHL = float2int32(amount * 0.1f);
				uint32 targetcnt = 0;
				unordered_set<Object* >::iterator itr;
				for( itr = unitTarget->GetInRangeSetBegin(); itr != unitTarget->GetInRangeSetEnd(); itr++ )
				{
					if( !(*itr)->IsUnit() || !TO_UNIT(*itr)->isAlive() || isAttackable(u_caster, (*itr), true) )
						continue;

					if( targetcnt > 4 )
						break;

					if(unitTarget->GetDistanceSq((*itr)) <= 64.0f)
					{
						SpellEntry* HLH = dbcSpell.LookupEntryForced( 54968 );
						Spell* pSpell(new Spell(u_caster, HLH, true, NULLAURA));
						pSpell->forced_basepoints[0] = GHL;
						SpellCastTargets tgt;
						tgt.m_unitTarget = (*itr)->GetGUID();
						pSpell->prepare(&tgt);
						targetcnt++;
					}
				}
			}
		}

		if(GetSpellProto()->spell_can_crit)
		{
			critchance = u_caster->spellcritperc + u_caster->SpellCritChanceSchool[GetSpellProto()->School];
			if( GetSpellProto()->SpellGroupType )
			{
				SM_FFValue(u_caster->SM[SMT_CRITICAL][0], &critchance, GetSpellProto()->SpellGroupType);
				SM_PFValue(u_caster->SM[SMT_CRITICAL][1], &critchance, GetSpellProto()->SpellGroupType);
			}

			// Sacred Shield HOAX
			if( unitTarget->HasDummyAura(SPELL_HASH_SACRED_SHIELD) && GetSpellProto()->NameHash == SPELL_HASH_FLASH_OF_LIGHT )
				critchance += 50.0f;
		}
		if(critical = Rand(critchance))
		{
			/*int32 critbonus = amount >> 1;
			if( GetSpellProto()->SpellGroupType)
					SM_PIValue(TO_UNIT(u_caster)->SM[SMT_CRITICAL_DAMAGE][1], &critbonus, GetSpellProto()->SpellGroupType);
			amount += critbonus;*/

			int32 critical_bonus = 100;
			if( GetSpellProto()->SpellGroupType )
				SM_FIValue( u_caster->SM[SMT_CRITICAL_DAMAGE][1], &critical_bonus, GetSpellProto()->SpellGroupType );

			if( critical_bonus > 0 )
			{
				// the bonuses are halved by 50% (funky blizzard math :S)
				float b = ( ( float(critical_bonus) / 2.0f ) / 100.0f );
				amount += float2int32( float(amount) * b );
			}

			if( u_caster->HasDummyAura(SPELL_HASH_LIVING_SEED) &&
				( GetSpellProto()->NameHash == SPELL_HASH_SWIFTMEND ||
				GetSpellProto()->NameHash == SPELL_HASH_REGROWTH ||
				GetSpellProto()->NameHash == SPELL_HASH_NOURISH ||
				GetSpellProto()->NameHash == SPELL_HASH_HEALING_TOUCH) )
			{
				uint32 chance = ( u_caster->GetDummyAura(SPELL_HASH_LIVING_SEED)->RankNumber * 33 ) + 1;
				if( Rand( chance ) )
				{
					SpellEntry *spellInfo = dbcSpell.LookupEntry( 48504 );
					Spell* sp(new Spell( u_caster, spellInfo, true, NULLAURA ));
					sp->forced_basepoints[0] = float2int32(amount * 0.3f);
					SpellCastTargets tgt;
					tgt.m_unitTarget = unitTarget->GetGUID();
					sp->prepare(&tgt);
				}
			}

			if( playerTarget && u_caster->HasDummyAura(SPELL_HASH_DIVINE_AEGIS) )
			{
				SpellEntry * spellInfo = dbcSpell.LookupEntry( 47753 );
				Spell* sp(new Spell( u_caster, spellInfo, true, NULLAURA ));
				sp->forced_basepoints[0] = float2int32(amount * ( 0.1f * u_caster->GetDummyAura(SPELL_HASH_DIVINE_AEGIS)->RankNumber ));
				SpellCastTargets tgt;
				tgt.m_unitTarget = playerTarget->GetGUID();
				sp->prepare(&tgt);
			}

			if( u_caster->HasDummyAura(SPELL_HASH_SHEATH_OF_LIGHT) && unitTarget )
			{
				SpellEntry * spellInfo = dbcSpell.LookupEntry( 54203 );
				Spell* sp(new Spell( u_caster, spellInfo, true, NULLAURA ));
				sp->forced_basepoints[0] = float2int32(amount * ( 0.05f * u_caster->GetDummyAura(SPELL_HASH_SHEATH_OF_LIGHT)->RankNumber ));
				SpellCastTargets tgt;
				tgt.m_unitTarget = unitTarget->GetGUID();
				sp->prepare(&tgt);
			}
			unitTarget->HandleProc(PROC_ON_SPELL_CRIT_HIT_VICTIM, NULL, u_caster, m_spellInfo, amount);
			u_caster->HandleProc(NULL, PROC_ON_SPELL_CRIT_HIT, unitTarget, m_spellInfo, amount);
		}

		if( unitTarget != NULL && (GetSpellProto()->NameHash == SPELL_HASH_GREATER_HEAL ||
			GetSpellProto()->NameHash == SPELL_HASH_FLASH_HEAL ||
			GetSpellProto()->NameHash == SPELL_HASH_PENANCE ) &&
			u_caster->HasDummyAura( SPELL_HASH_RAPTURE ) && u_caster->m_CustomTimers[CUSTOM_TIMER_RAPTURE] <= getMSTime() )
		{
			SpellEntry *spellInfo = dbcSpell.LookupEntry( 47755 );
			Spell* sp(new Spell( u_caster, spellInfo, true, NULLAURA ));
			uint32 maxmana = u_caster->GetUInt32Value(UNIT_FIELD_MAXPOWER1);
			float rapture_mod = u_caster->GetDummyAura( SPELL_HASH_RAPTURE )->RankNumber * 0.005f;
			sp->forced_basepoints[0] = float2int32( maxmana * rapture_mod );
			SpellCastTargets tgt;
			tgt.m_unitTarget = unitTarget->GetGUID();
			sp->prepare(&tgt);
			u_caster->m_CustomTimers[CUSTOM_TIMER_RAPTURE] = getMSTime() + 12000;
		}
	}

	if(amount < 0)
		amount = 0;

	uint32 overheal = 0;
	uint32 curHealth = unitTarget->GetUInt32Value(UNIT_FIELD_HEALTH);
	uint32 maxHealth = unitTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH);
	if((curHealth + amount) >= maxHealth)
	{
		unitTarget->SetUInt32Value(UNIT_FIELD_HEALTH, maxHealth);
		overheal = curHealth + amount - maxHealth;
	} else
		unitTarget->ModUnsigned32Value(UNIT_FIELD_HEALTH, amount);

	if( overheal && u_caster && u_caster->HasDummyAura(SPELL_HASH_SERENDIPITY) && u_caster->m_LastSpellManaCost &&
		( GetSpellProto()->NameHash == SPELL_HASH_GREATER_HEAL || GetSpellProto()->NameHash == SPELL_HASH_FLASH_HEAL ) )
	{
		int32 amt = float2int32( u_caster->m_LastSpellManaCost * ( 0.08f * u_caster->GetDummyAura(SPELL_HASH_SERENDIPITY)->RankNumber ));
		SpellEntry* SpellInfo = dbcSpell.LookupEntry( 47762 );
		if( SpellInfo )
		{
			Spell* sp = NULLSPELL;
			sp = (new Spell( u_caster, SpellInfo, true, NULLAURA ));
			sp->forced_basepoints[0] = amt;
			SpellCastTargets tgt;
			tgt.m_unitTarget = u_caster->GetGUID();
			sp->prepare( &tgt );
		}
	}
	if( u_caster && unitTarget && u_caster != unitTarget && unitTarget->HasDummyAura(SPELL_HASH_SPIRITUAL_ATTUNEMENT) && amount)
	{
		int32 amt = float2int32( amount * ( (unitTarget->GetDummyAura(SPELL_HASH_SPIRITUAL_ATTUNEMENT)->EffectBasePoints[0]+1) / 100.0f ));
		SpellEntry* SpellInfo = dbcSpell.LookupEntry( 31786 );
		if( SpellInfo )
		{
			Spell* sp(new Spell( u_caster, SpellInfo, true, NULLAURA ));
			sp->forced_basepoints[0] = amt;
			SpellCastTargets tgt;
			tgt.m_unitTarget = unitTarget->GetGUID();
			sp->prepare( &tgt );
		}
	}

	if( m_caster )
	{
		SendHealSpellOnPlayer( m_caster, unitTarget, amount, critical, overheal, GetSpellProto()->logsId ? GetSpellProto()->logsId : (pSpellId ? pSpellId : GetSpellProto()->Id) );
	}
	if( p_caster != NULL )
	{
		p_caster->m_bgScore.HealingDone += amount - overheal;
		if( p_caster->m_bg != NULL )
			p_caster->m_bg->UpdatePvPData();
	}

 	//Beacon of Light
	if(p_caster && p_caster->GetGroup())
 	{
		std::map<Player*,uint32> beaconmap = p_caster->GetGroup()->m_BeaconOfLightTargets;

		if(beaconmap.size())
		{
			for(std::map<Player*, uint32>::iterator itr = beaconmap.begin(); itr != beaconmap.end(); itr++)
			{
				if(itr->first != NULL)
				{
					Player* HealTarget = itr->first;
					if(HealTarget->GetGUID() == p_caster->GetGUID())	// don't heal ourself!
						continue;

					if((HealTarget->GetUInt32Value(UNIT_FIELD_HEALTH) + amount) >= HealTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH))
						amount = (HealTarget->GetUInt32Value(UNIT_FIELD_MAXHEALTH) - HealTarget->GetUInt32Value(UNIT_FIELD_HEALTH));

					HealTarget->ModUnsigned32Value(UNIT_FIELD_HEALTH, amount);
					SendHealSpellOnPlayer( p_caster, HealTarget, amount, critical, overheal, GetSpellProto()->logsId ? GetSpellProto()->logsId : (pSpellId ? pSpellId : GetSpellProto()->Id) );
				}
				else
					beaconmap.erase(itr);
			}
		}
 	}

	if (p_caster)
	{
		p_caster->m_casted_amount[GetSpellProto()->School]=amount;
	}

	// add threat
	if( u_caster != NULL )
	{
		if(GetSpellProto()->AttributesEx & ATTRIBUTESEX_NO_THREAT)
			return;
		uint32 base_threat= GetBaseThreat(amount);
		int count = 0;
		Unit* unit;
		std::vector<Unit* > target_threat;
		if(base_threat)
		{
			target_threat.reserve(u_caster->GetInRangeCount()); // this helps speed

			for(unordered_set<Object* >::iterator itr = u_caster->GetInRangeSetBegin(); itr != u_caster->GetInRangeSetEnd(); itr++)
			{
				if((*itr)->GetTypeId() != TYPEID_UNIT)
					continue;
				unit = TO_UNIT((*itr));
				if(unit->GetAIInterface()->GetNextTarget() == unitTarget)
				{
					target_threat.push_back(unit);
					++count;
				}
			}
			if(count == 0)
				count = 1;  // division against 0 protection
			/*
			When a tank hold multiple mobs, the threat of a heal on the tank will be split between all the mobs.
			The exact formula is not yet known, but it is more than the Threat/number of mobs.
			So if a tank holds 5 mobs and receives a heal, the threat on each mob will be less than Threat(heal)/5.
			Current speculation is Threat(heal)/(num of mobs *2)
			*/
			uint32 threat = base_threat / (count * 2);

			for(std::vector<Unit* >::iterator itr = target_threat.begin(); itr != target_threat.end(); itr++)
			{
				// for now we'll just use heal amount as threat.. we'll prolly need a formula though
				TO_UNIT(*itr)->GetAIInterface()->HealReaction( u_caster, unitTarget, threat, m_spellInfo );
			}
		}

		if(unitTarget->IsInWorld() && u_caster->IsInWorld())
			u_caster->CombatStatus.WeHealed(unitTarget);
	}
}

void Spell::DetermineSkillUp(uint32 skillid,uint32 targetlevel, uint32 multiplicator)
{
	if(p_caster == NULL)
		return;
	if(p_caster->GetSkillUpChance(skillid)<0.01)
		return;//to preven getting higher skill than max

	int32 diff = abs(int(p_caster->_GetSkillLineCurrent(skillid,false)/5 - targetlevel));
	float chance = ( diff <=5  ? 95.0f : diff <=10 ? 66.0f : diff <=15 ? 33.0f : 0.0f );
	if( Rand(int32(chance * sWorld.getRate(RATE_SKILLCHANCE) * (multiplicator?multiplicator:1))))
		p_caster->_AdvanceSkillLine(skillid, float2int32(1.0f * sWorld.getRate(RATE_SKILLRATE)));
}

void Spell::DetermineSkillUp(uint32 skillid)
{
	//This code is wrong for creating items and disenchanting.
	if(p_caster == NULL)
		return;
	float chance = 0.0f;
	skilllinespell* skill = objmgr.GetSpellSkill(GetSpellProto()->Id);
	if( skill != NULL && TO_PLAYER( m_caster )->_HasSkillLine( skill->skilline ) )
	{
		uint32 amt = TO_PLAYER( m_caster )->_GetSkillLineCurrent( skill->skilline, false );
		uint32 max = TO_PLAYER( m_caster )->_GetSkillLineMax( skill->skilline );
		if( amt >= max )
			return;
		if( amt >= skill->grey ) //grey
			chance = 0.0f;
		else if( ( amt >= ( ( ( skill->grey - skill->green) / 2 ) + skill->green ) ) ) //green
			chance = 33.0f;
		else if( amt >= skill->green ) //yellow
			chance = 66.0f;
		else //brown
			chance=100.0f;
	}
	if(Rand(chance*sWorld.getRate(RATE_SKILLCHANCE)))
		p_caster->_AdvanceSkillLine(skillid, float2int32( 1.0f * sWorld.getRate(RATE_SKILLRATE)));
}

bool Spell::Reflect(Unit* refunit)
{
	uint32 refspellid = 0;
	bool canreflect = false;
//	bool remove = false;

	if( m_reflectedParent != NULL || m_caster == refunit )
		return false;

	// if the spell to reflect is a reflect spell, do nothing.
	for(int i=0; i<3; i++)
	{
		if( GetSpellProto()->Effect[i] == 6 && (GetSpellProto()->EffectApplyAuraName[i] == 74 || GetSpellProto()->EffectApplyAuraName[i] == 28))
			return false;
	}

	for(std::list<struct ReflectSpellSchool*>::iterator i = refunit->m_reflectSpellSchool.begin();i != refunit->m_reflectSpellSchool.end();++i)
	{
		if(((*i)->school == -1 && GetSpellProto()->School) || (*i)->school == (int32)GetSpellProto()->School)
		{
			if(Rand((float)(*i)->chance))
			{
				//the god blessed special case : mage - Frost Warding = is an augmentation to frost warding
				if((*i)->require_aura_hash && u_caster && !u_caster->GetAuraSpellIDWithNameHash((*i)->require_aura_hash))
				{
					if( !(GetSpellProto()->c_is_flags & SPELL_FLAG_IS_DAMAGING ) )
						continue;

					int32 evilforce = 0;
					uint32 effectid = 0;
					for( uint32 loopnr = 0; loopnr < 3; loopnr++ )
					{
						if( GetSpellProto()->EffectBasePoints[loopnr] > evilforce )
						{
							evilforce = GetSpellProto()->EffectBasePoints[loopnr];
							effectid = loopnr;
						}
					}

					SpellEntry *spellInfo = dbcSpell.LookupEntry( 57776 );
					Spell* spell = NULLSPELL;
					spell = (new Spell( refunit, spellInfo, true, NULLAURA));
					uint32 manaregenamt = CalculateEffect(effectid, refunit);
					spell->forced_basepoints[0] = manaregenamt;
					SpellCastTargets targets;
					targets.m_unitTarget = refunit->GetGUID();
					spell->prepare( &targets );
				}
				else if( (*i)->infront )
				{
					if( m_caster->isInFront(refunit) )
					{
						canreflect = true;
					}
				}
				else
					canreflect = true;

				refspellid = (*i)->spellId;
				if( !(*i)->infinity )
					refunit->RemoveAura(refspellid);

				break;
			}
		}
	}

	if( !refspellid || !canreflect )
		return false;

	Spell* spell = NULLSPELL;
	spell = (new Spell(refunit, m_spellInfo, true, NULLAURA));
	SpellCastTargets targets;
	targets.m_unitTarget = m_caster->GetGUID();
	spell->m_reflectedParent = this;
	spell->prepare(&targets);
	return true;
}

void ApplyDiminishingReturnTimer(int32 * Duration, Unit* Target, SpellEntry * spell)
{
	uint32 status = GetDiminishingGroup(spell->NameHash);
	uint32 Grp = status & 0xFFFF;   // other bytes are if apply to pvp
	uint32 PvE = (status >> 16) & 0xFFFF;

	// Make sure we have a group
	if(Grp == 0xFFFF) return;

	// Check if we don't apply to pve
	if(!PvE && Target->GetTypeId() != TYPEID_PLAYER && !Target->IsPet())
		return;

	assert(Grp < DIMINISH_GROUPS);

	// TODO: check for spells that should do this
	float Qduration = float(*Duration);

	switch(Target->m_diminishCount[Grp])
	{
	case 0: // Full effect
		if (Target->IsPlayer() && Qduration > 10000)
		{
			Qduration = 10000;
		}
		break;

	case 1: // Reduced by 50%
		Qduration *= 0.5f;
		if (Target->IsPlayer() && Qduration > 5000)
		{
			Qduration = 5000;
		}
		break;

	case 2: // Reduced by 75%
		Qduration *= 0.25f;
		if (Target->IsPlayer() && Qduration > 2500)
		{
			Qduration = 2500;
		}
		break;

	default:// Target immune to spell
		{
			*Duration = 0;
			return;
		}break;
	}

	// Convert back
	*Duration = FL2UINT(Qduration);

	// Reset the diminishing return counter, and add to the aura count (we don't decrease the timer till we
	// have no auras of this type left.
	++Target->m_diminishAuraCount[Grp];
	++Target->m_diminishCount[Grp];
}

void UnapplyDiminishingReturnTimer(Unit* Target, SpellEntry * spell)
{
	uint32 status = GetDiminishingGroup(spell->NameHash);
	uint32 Grp = status & 0xFFFF;   // other bytes are if apply to pvp
	uint32 PvE = (status >> 16) & 0xFFFF;

	// Make sure we have a group
	if(Grp == 0xFFFF) return;

	// Check if we don't apply to pve
	if(!PvE && Target->GetTypeId() != TYPEID_PLAYER && !Target->IsPet())
		return;

	assert(Grp < DIMINISH_GROUPS);

	Target->m_diminishAuraCount[Grp]--;

	// start timer decrease
	if(!Target->m_diminishAuraCount[Grp])
	{
		Target->SetDiminishTimer(Grp);
	}
}

/// Calculate the Diminishing Group. This is based on a name hash.
/// this off course is very hacky, but as its made done in a proper way
/// I leave it here.
uint32 GetDiminishingGroup(uint32 NameHash)
{
	int32 grp = -1;
	bool pve = false;

	switch(NameHash)
	{
	case SPELL_HASH_CYCLONE:
	case SPELL_HASH_BLIND:
		grp = 0;
		pve = true;
		break;
	case SPELL_HASH_MIND_CONTROL:
		grp = 1;
		break;
	case SPELL_HASH_FEAR:
	case SPELL_HASH_PSYCHIC_SCREAM:
	case SPELL_HASH_HOWL_OF_TERROR:
	case SPELL_HASH_SEDUCTION:
		grp = 2;
		break;
	case SPELL_HASH_SAP:
	case SPELL_HASH_GOUGE:
	case SPELL_HASH_REPENTANCE:
	case SPELL_HASH_POLYMORPH:				// Polymorph
	case SPELL_HASH_POLYMORPH__CHICKEN:		// Chicken
	case SPELL_HASH_POLYMORPH__SHEEP:		// Good ol' sheep
		grp = 3;
		break;
	case SPELL_HASH_DEATH_COIL:
		grp = 4;
		break;
	case SPELL_HASH_KIDNEY_SHOT:
		grp = 5;
		pve = true;
		break;
	case SPELL_HASH_ENTRAPMENT:
		grp = 6;
		break;
	case SPELL_HASH_ENTANGLING_ROOTS:
	case SPELL_HASH_FROST_NOVA:
		grp = 7;
		break;
	case SPELL_HASH_FROSTBITE:
		grp = 8;
		break;
	case SPELL_HASH_HIBERNATE:
	case SPELL_HASH_WYVERN_STING:
	case SPELL_HASH_SLEEP:
	case SPELL_HASH_FROST_TRAP_AURA:
	case SPELL_HASH_FREEZING_TRAP_EFFECT:
		grp = 9;
		break;
	case SPELL_HASH_BASH:
	case SPELL_HASH_IMPACT:
	case SPELL_HASH_HAMMER_OF_JUSTICE:
	case SPELL_HASH_CHEAP_SHOT:
	case SPELL_HASH_SHADOWFURY:
	case SPELL_HASH_CHARGE_STUN:
	case SPELL_HASH_INTERCEPT:
	case SPELL_HASH_CONCUSSION_BLOW:
		grp = 10;
		pve = true;
		break;
	case SPELL_HASH_STARFIRE_STUN:
	case SPELL_HASH_STONECLAW_STUN:
	case SPELL_HASH_STUN:
	case SPELL_HASH_BLACKOUT:
		grp = 11;
		pve = true;
		break;
	case SPELL_HASH_HEX:
		grp = 12;
		break;


		/*case SPELL_HASH_BANISH:				// Banish
		grp = 19;
		break;

		case SPELL_HASH_FREEZING_TRAP_EFFECT:	// Freezing Trap Effect
		grp = 20;
		break;

		case SPELL_HASH_SCARE_BEAST:			// Scare Beast
		grp = 21;
		break;

		case SPELL_HASH_ENSLAVE_DEMON:			// Enslave Demon
		grp = 22;
		break;
		case SPELL_HASH_SLEEP:					// Sleep
		grp = 23;
		break;
		case SPELL_HASH_RIPOSTE:
		grp = 24;
		break;*/
	}
	uint32 ret;
	if( pve )
		ret = grp | (1 << 16);
	else
		ret = grp;

	return ret;
}

void Spell::_AddTarget(const Unit* target, const uint32 effectid)
{
	SpellTargetList::iterator itr;
	SpellTarget tgt;

	// look for the target in the list already
	for( itr = m_targetList.begin(); itr != m_targetList.end(); itr++ )
	{
		if( itr->Guid == target->GetGUID() )
		{
			// add the effect to it, hit result is already determined
			itr->EffectMask |= (1 << effectid);
			return;
		}
	}

	// setup struct
	tgt.Guid = target->GetGUID();
	tgt.EffectMask = (1 << effectid);

	// work out hit result (always true if we are a GO)
	tgt.HitResult = (g_caster || (g_caster && g_caster->GetType() != GAMEOBJECT_TYPE_TRAP) ) ? SPELL_DID_HIT_SUCCESS : _DidHit(target);

	// add to the list
	m_targetList.push_back(tgt);

	// add counter
	if( tgt.HitResult == SPELL_DID_HIT_SUCCESS )
		++m_hitTargetCount;
	else
		++m_missTargetCount;
}

void Spell::_AddTargetForced(const uint64& guid, const uint32 effectid)
{
	SpellTargetList::iterator itr;

	// look for the target in the list already
	for( itr = m_targetList.begin(); itr != m_targetList.end(); itr++ )
	{
		if( itr->Guid == guid )
		{
			// add the effect to it, hit result is already determined
			itr->EffectMask |= (1 << effectid);
			return;
		}
	}

	// setup struct
	SpellTarget tgt;
	tgt.Guid = guid;
	tgt.EffectMask = (1 << effectid);
	tgt.HitResult = SPELL_DID_HIT_SUCCESS;

	// add to the list
	m_targetList.push_back(tgt);

	// add counter
	if( tgt.HitResult == SPELL_DID_HIT_SUCCESS )
		++m_hitTargetCount;
	else
		++m_missTargetCount;
}

void Spell::_AddTargetForced(Object * target, const uint32 effectid)
{
	SpellTargetList::iterator itr;
	SpellTarget tgt;

	for( itr = m_targetList.begin(); itr != m_targetList.end(); itr++ )
	{
		if( itr->Guid == target->GetGUID() )
		{
			itr->EffectMask |= (1 << effectid);
			return;
		}
	}
	tgt.Guid = target->GetGUID();
	tgt.EffectMask = (1 << effectid);
	tgt.HitResult = SPELL_DID_HIT_SUCCESS;
	m_targetList.push_back(tgt);
	++m_hitTargetCount;
}

void Spell::DamageGosAround(uint32 i)
{
	uint32 spell_id = GetSpellProto()->Id;
	printf("gameObjTarget not found for spell Id %u report on mantis\n",spell_id);
	float r = GetRadius(i);
	r *= r;
	Object* o;

	for (Object::InRangeSet::iterator itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); ++itr)
	{
		o = *itr;
		if (!o->IsGameObject())
			continue;
		if(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION && o->GetDistance2dSq(m_targets.m_destX, m_targets.m_destY) <= r )
		{
			TO_GAMEOBJECT(o)->TakeDamage(damage,m_caster,p_caster,spell_id);
			return;
		}
		else if(m_targets.m_targetMask & TARGET_FLAG_SOURCE_LOCATION && o->GetDistance2dSq(m_targets.m_srcX, m_targets.m_srcY) <= r)
		{
			TO_GAMEOBJECT(o)->TakeDamage(damage,m_caster,p_caster,spell_id);
			return;
		}
		else
		{
			if(o->GetDistance2dSq(m_caster->GetPositionX(), m_caster->GetPositionY()) <= r)
				TO_GAMEOBJECT(o)->TakeDamage(damage,m_caster,p_caster,spell_id);
		}
	}
}

bool Spell::CanHandleSpellEffect(uint32 i, uint32 namehash)
{
	if(!u_caster)
		return false;
	switch(i)
	{
		case 1:
		{
			switch(namehash)
			{
				case SPELL_HASH_FROSTBOLT:
				{
					if(u_caster->HasDummyAura( SPELL_HASH_GLYPH_OF_FROSTBOLT ))
						return false;
				}break;
			}break;
		}break;
		case 2:
		{
			switch(namehash)
			{
				case SPELL_HASH_FIREBALL:
				{
					if(u_caster->HasDummyAura( SPELL_HASH_GLYPH_OF_FIREBALL ))
						return false;
				}break;
			}break;
		}break;
		case 3:
		{
			switch(namehash)
			{
				case SPELL_HASH_BLAST_WAVE:
				{
					if(u_caster->HasAura(62126))
						return false;
				}break;		
				case SPELL_HASH_THUNDERSTORM:
				{
					if(u_caster->HasAura(62132))
						return false;
				}break;
			}break;
		}break;
	}
	return true;
}

bool Spell::UseMissileDelay()
{
	if(HasSpellEffect(SPELL_EFFECT_CHARGE) || HasSpellEffect(SPELL_EFFECT_JUMP_TO_TARGET) ||
		HasSpellEffect(SPELL_EFFECT_JUMP_TO_DESTIONATION) || HasSpellEffect(SPELL_EFFECT_TRACTOR_BEAM_FROM_DEST) || 
		HasSpellEffect(SPELL_EFFECT_PLAYER_PULL))
		return false;
	return true;
}