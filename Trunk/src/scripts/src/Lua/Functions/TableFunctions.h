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

#ifndef __TABLEFUNCTIONS_H
#define __TABLEFUNCTIONS_H

#include "../LUAEngine.h"
// Crow: Some of the following functions are based off of functions created by Hypersniper of LuaHypeArc.
// Areas where this applies, credit has been given in the form of a name declaration.
#include "FunctionGameObjects.h"
#include "FunctionGlobal.h"
#include "FunctionItems.h"
#include "FunctionPacket.h"
#include "FunctionSpell.h"
#include "FunctionAura.h"
#include "FunctionTaxi.h"
#include "FunctionUnits.h"
#include "LuaSqlApi.h"

template<typename T>
struct RegType
{
	const char * name;
	int(*mfunc)(lua_State*,T*);
};

/*	Empty Functions: Both of these are used as placeholders for the actual code
	so we can have the functions, but they won't do anything. */

template<typename T> int EmptyLuaFunction(lua_State * L, T*)
{
	return 1;
}

int EmptyGlobalFunction(lua_State*L)
{
	return 1;
}

// Crow: Some of the following functions are based off of functions created by Hypersniper of LuaHypeArc.
// Areas where this applies, credit has been given in the form of a name declaration.
/************************************************************************/
/* SCRIPT FUNCTION TABLES                                               */
/************************************************************************/
RegType<Item> ItemMethods[] = 
{
	{ "GossipCreateMenu", &luaItem_GossipCreateMenu },
	{ "GossipMenuAddItem", &luaItem_GossipMenuAddItem },
	{ "GossipSendMenu", &luaItem_GossipSendMenu },
	{ "GossipComplete", &luaItem_GossipComplete },
	{ "GossipSendPOI", &luaItem_GossipSendPOI },
	{ "PerformWorldDBQuery", &luaItem_PerformWorldDBQuery },
	{ "PerformCharDBQuery", &luaItem_PerformCharDBQuery },
	{ "GetOwner", &luaItem_GetOwner },
	{ "AddEnchantment", &luaItem_AddEnchantment },
	{ "RemoveEnchantment", &luaItem_RemoveEnchantment },
	{ "GetEntryId", &luaItem_GetEntryId },
	{ "GetName", &luaItem_GetName },
	{ "GetSpellId", &luaItem_GetSpellId },
	{ "GetSpellTrigger", &luaItem_GetSpellTrigger },
	{ "GetGUID", &luaItem_GetGUID },
	{ "AddLoot", &luaItem_AddLoot},
	{ "SetByteValue", &luaItem_SetByteValue },
	{ "GetByteValue", &luaItem_GetByteValue },
	{ "GetItemLevel", &luaItem_GetItemLevel },
	{ "GetRequiredLevel", &luaItem_GetRequiredLevel },
	{ "GetBuyPrice", &luaItem_GetBuyPrice },
	{ "GetSellPrice", &luaItem_GetSellPrice },
	{ "RepairItem", &luaItem_RepairItem },
	{ "GetMaxDurability", &luaItem_GetMaxDurability },
	{ "GetDurability", &luaItem_GetDurability },
	{ "HasEnchantment", &luaItem_HasEnchantment },
	{ "ModifyEnchantmentTime", &luaItem_ModifyEnchantmentTime },
	{ "SetStackCount", &luaItem_SetStackCount },
	{ "HasFlag", &luaItem_HasFlag },
	{ "IsSoulbound", &luaItem_IsSoulbound },
	{ "IsAccountbound", &luaItem_IsAccountbound },
	{ "IsContainer", &luaItem_IsContainer },
	{ "GetContainerItemCount", &luaItem_GetContainerItemCount },
	{ "GetEquippedSlot", &luaItem_GetEquippedSlot },
	{ "GetObjectType", &luaItem_GetObjectType },
	{ "Remove", &luaItem_Remove },
	{ "Create", &luaItem_Create },
	{ NULL, NULL },
};

RegType<Unit> UnitMethods[] = 
{
	{ "GossipCreateMenu", &LuaUnit_GossipCreateMenu },
	{ "GossipMenuAddItem", &LuaUnit_GossipMenuAddItem },
	{ "GossipSendMenu", &LuaUnit_GossipSendMenu },
	{ "GossipComplete", &LuaUnit_GossipComplete },
	{ "GossipSendPOI", &LuaUnit_GossipSendPOI },
	{ "GetName", &LuaUnit_GetName },
	{ "SendChatMessage", &LuaUnit_SendChatMessage },
	{ "MoveTo", &LuaUnit_MoveTo },
	{ "SetMovementType", &LuaUnit_SetMovementType },
	{ "CastSpell", &LuaUnit_CastSpell },
	{ "FullCastSpell", &LuaUnit_FullCastSpell },
	{ "CastSpellOnTarget", &LuaUnit_CastSpellOnTarget },
	{ "FullCastSpellOnTarget", &LuaUnit_FullCastSpellOnTarget },
	{ "SpawnCreature", &LuaUnit_SpawnCreature },
	{ "SpawnGameObject", &LuaUnit_SpawnGameObject },
	{ "GetX", &LuaUnit_GetX },
	{ "GetY", &LuaUnit_GetY },
	{ "GetZ", &LuaUnit_GetZ },
	{ "GetO", &LuaUnit_GetO },
	{ "IsPlayer", &LuaUnit_IsPlayer },
	{ "IsCreature", &LuaUnit_IsCreature },
	{ "RegisterEvent", &LuaUnit_RegisterEvent },
	{ "RemoveEvents", &LuaUnit_RemoveEvents },
	{ "SendBroadcastMessage", &LuaUnit_SendBroadcastMessage },
	{ "SendAreaTriggerMessage", &LuaUnit_SendAreaTriggerMessage },
	{ "KnockBack", &LuaUnit_KnockBack },
	{ "MarkQuestObjectiveAsComplete", &LuaUnit_MarkQuestObjectiveAsComplete },
	{ "LearnSpell", &LuaUnit_LearnSpell },
	{ "UnlearnSpell", &LuaUnit_UnlearnSpell },
	{ "HasFinishedQuest", &LuaUnit_HasFinishedQuest },
	{ "GetItemCount", &LuaUnit_GetItemCount },
	{ "IsInCombat", &LuaUnit_IsInCombat },
	{ "GetMainTank", &LuaUnit_GetMainTank },
	{ "GetAddTank", &LuaUnit_GetAddTank },
	{ "ClearThreatList", &LuaUnit_ClearThreatList },
	{ "GetTauntedBy", &LuaUnit_GetTauntedBy },
	{ "SetTauntedBy", &LuaUnit_SetTauntedBy },
	{ "ChangeTarget", &LuaUnit_ChangeTarget },
	{ "GetHealthPct", &LuaUnit_GetHealthPct },
	{ "SetHealthPct", &LuaUnit_SetHealthPct },
	{ "GetManaPct", &LuaUnit_GetManaPct },
	{ "Despawn", &LuaUnit_Despawn },
	{ "GetUnitBySqlId", &LuaUnit_GetUnitBySqlId },
	{ "PlaySoundToSet", &LuaUnit_PlaySoundToSet },
	{ "RemoveAura", &LuaUnit_RemoveAura },
	{ "StopMovement", &LuaUnit_StopMovement },
	{ "Emote", &LuaUnit_Emote },
	{ "GetInstanceID", &LuaUnit_GetInstanceID },
	{ "GetClosestPlayer", &LuaUnit_GetClosestPlayer },
	{ "GetRandomPlayer", &LuaUnit_GetRandomPlayer },
	{ "GetRandomFriend", &LuaUnit_GetRandomFriend },
	{ "AddItem", &LuaUnit_AddItem },
	{ "RemoveItem", &LuaUnit_RemoveItem },
	{ "CreateCustomWaypointMap", &LuaUnit_CreateCustomWaypointMap },
	{ "CreateWaypoint", &LuaUnit_CreateWaypoint },
	{ "MoveToWaypoint", &LuaUnit_MoveToWaypoint },
	{ "DestroyCustomWaypointMap", &LuaUnit_DestroyCustomWaypointMap },
	{ "SetCombatCapable", &LuaUnit_SetCombatCapable },
	{ "SetCombatMeleeCapable", &LuaUnit_SetCombatMeleeCapable },
	{ "SetCombatRangedCapable", &LuaUnit_SetCombatRangedCapable },
	{ "SetCombatSpellCapable", &LuaUnit_SetCombatSpellCapable },
	{ "SetCombatTargetingCapable", &LuaUnit_SetCombatTargetingCapable },
	{ "SetNPCFlags", &LuaUnit_SetNPCFlags },
	{ "SetModel", &LuaUnit_SetModel },
	{ "SetScale", &LuaUnit_SetScale },
	{ "SetFaction", &LuaUnit_SetFaction },
	{ "SetStandState", &LuaUnit_SetStandState },
	{ "Teleport" , &LuaUnit_TeleportUnit },
	{ "GetPlayerClass", &LuaUnit_GetPlayerClass },
	{ "ClearThreatList", &LuaUnit_ClearHateList },
	{ "WipeThreatList", &LuaUnit_WipeHateList },
	{ "WipeTargetList", &LuaUnit_WipeTargetList },
	{ "WipeCurrentTarget", &LuaUnit_WipeCurrentTarget },
	{ "GetHealth", &LuaUnit_GetHealth },
	{ "GetMaxHealth", &LuaUnit_GetMaxHealth },
	{ "SetHealth", &LuaUnit_SetHealth },
	{ "SetMaxHealth", &LuaUnit_SetMaxHealth },
	{ "HasAura", &LuaUnit_HasAura },
	{ "Land", &LuaUnit_Land },
	{ "SetFlying", &LuaUnit_SetFlying },
	{ "SetMana", &LuaUnit_SetMana },
	{ "SetMaxMana", &LuaUnit_SetMaxMana },
	{ "GetDistance", &LuaUnit_GetDistance },
	{ "GetGUID", &LuaUnit_GetGUID },
	{ "GetCreatureNearestCoords", &LuaUnit_GetCreatureNearestCoords },
    { "CastSpellAoF", &LuaUnit_CastSpellAoF },
	{ "GetGameObjectNearestCoords", &LuaUnit_GetGameObjectNearestCoords },
	{ "SetInFront", &LuaUnit_SetInFront },
	{ "RemoveAllAuras", &LuaUnit_RemoveAllAuras },
	{ "ReturnToSpawnPoint", &LuaUnit_ReturnToSpawnPoint },
	{ "CancelSpell", &LuaUnit_CancelSpell },
	{ "IsAlive", &LuaUnit_IsAlive },
	{ "IsDead", &LuaUnit_IsDead },
	{ "IsInWorld", &LuaUnit_IsInWorld },
	{ "GetZoneId", &LuaUnit_GetZoneId },
	{ "GetMana", &LuaUnit_GetMana },
	{ "GetMaxMana", &LuaUnit_GetMaxMana },
	{ "Root", &LuaUnit_Root },
	{ "Unroot", &LuaUnit_Unroot },
	{ "IsCreatureMoving", &LuaUnit_IsCreatureMoving },
	{ "SetOutOfCombatRange", &LuaUnit_SetOutOfCombatRange },
	{ "ModifyRunSpeed", &LuaUnit_ModifyRunSpeed },
	{ "ModifyWalkSpeed", &LuaUnit_ModifyWalkSpeed },
	{ "ModifyFlySpeed" , &LuaUnit_ModifyFlySpeed },
	{ "GetCurrentSpell", &LuaUnit_GetCurrentSpell },
	{ "IsFlying", &LuaUnit_IsFlying },
	{ "SetRotation", &LuaUnit_SetRotation },
	{ "SetOrientation", &LuaUnit_SetOrientation },
	{ "GetSpawnX", &LuaUnit_GetSpawnX },
	{ "GetSpawnY", &LuaUnit_GetSpawnY },
	{ "GetSpawnZ", &LuaUnit_GetSpawnZ },
	{ "GetSpawnO", &LuaUnit_GetSpawnO },
	{ "GetInRangePlayersCount", &LuaUnit_GetInRangePlayersCount },
	{ "GetEntry", &LuaUnit_GetEntry },
	{ "SetMoveRunFlag", &LuaUnit_SetMoveRunFlag },
	{ "GetAIState", &LuaUnit_GetAIState },
	{ "ModUInt32Value", &LuaUnit_ModUInt32Value },
	{ "ModFloatValue", &LuaUnit_ModFloatValue },
	{ "SetUInt32Value", &LuaUnit_SetUInt32Value },
	{ "SetUInt64Value", &LuaUnit_SetUInt64Value },
	{ "SetFloatValue", &LuaUnit_SetFloatValue },
	{ "GetUInt32Value", &LuaUnit_GetUInt32Value },
	{ "GetUInt64Value", &LuaUnit_GetUInt64Value },
	{ "GetFloatValue", &LuaUnit_GetFloatValue },
	{ "InitPacket", &LuaUnit_InitPacket },
	{ "AddDataToPacket", &LuaUnit_AddDataToPacket },
	{ "AddGuidDataToPacket", &LuaUnit_AddGuidDataToPacket },
	{ "AdvanceQuestObjective", &LuaUnit_AdvanceQuestObjective },
	{ "Heal", &LuaUnit_Heal },
	{ "Energize", &LuaUnit_Energize },
	{ "SendChatMessageAlternateEntry", &LuaUnit_SendChatMessageAlternateEntry },
	{ "SendChatMessageToPlayer", &LuaUnit_SendChatMessageToPlayer },
	{ "SetPowerType", &LuaUnit_SetPowerType },
	{ "Strike", &LuaUnit_Strike },
	{ "SetAttackTimer", &LuaUnit_SetAttackTimer },
	{ "Kill", &LuaUnit_Kill },
	{ "DealDamage", &LuaUnit_DealDamage },
	{ "IsInFront", &LuaUnit_IsInFront },
	{ "IsInBack", &LuaUnit_IsInBack },
	{ "IsPacified", &LuaUnit_IsPacified },
	{ "IsStunned", &LuaUnit_IsStunned },
	{ "IsFeared", &LuaUnit_IsFeared },
	{ "CreateGuardian", &LuaUnit_CreateGuardian },
	{ "HandleEvent", &LuaUnit_HandleEvent },
	{ "HasInRangeObjects", &LuaUnit_HasInRangeObjects },
	{ "IsInWater", &LuaUnit_IsInWater },
	{ "IsInArc", &LuaUnit_IsInArc },
	{ "GetInRangeObjects", &LuaUnit_GetInRangeGameObjects },
	{ "GetInRangeObjectsCount", &LuaUnit_GetInRangeObjectsCount },
	{ "GetAITargetsCount", &LuaUnit_GetAITargetsCount },
	{ "SetUnitToFollow", &LuaUnit_SetUnitToFollow },
	{ "GetUnitToFollow", &LuaUnit_GetUnitToFollow },
	{ "DismissPet", &LuaUnit_DismissPet },
	{ "IsPet", &LuaUnit_IsPet },
	{ "SetNextTarget", &LuaUnit_SetNextTarget },
	{ "GetNextTarget", &LuaUnit_GetNextTarget },
	{ "SetPetOwner", &LuaUnit_SetPetOwner },
	{ "GetPetOwner", &LuaUnit_GetPetOwner },
	{ "CalcToDistance", &LuaUnit_CalcToDistance },
	{ "CalcAngle", &LuaUnit_CalcAngle },
	{ "CalcRadAngle", &LuaUnit_CalcRadAngle },
	{ "SetFacing", &LuaUnit_SetFacing },
	{ "SetDeathState", &LuaUnit_SetDeathState },
	{ "IsInvisible", &LuaUnit_IsInvisible },
	{ "IsInvincible", &LuaUnit_IsInvincible },
	{ "ResurrectPlayer", &LuaUnit_ResurrectPlayer },
	{ "KickPlayer", &LuaUnit_KickPlayer },
	{ "CanCallForHelp", &LuaUnit_CanCallForHelp },
	{ "CallForHelpHp", &LuaUnit_CallForHelpHp },
	{ "SetCreatureNameById", &LuaUnit_SetCreatureName },
	{ "GetInRangePlayers", &LuaUnit_GetInRangePlayers },
	{ "GetUnitByGUID", &LuaUnit_GetUnitByGUID },
	{ "RemoveFromWorld", &LuaUnit_RemoveFromWorld },
	{ "GetFaction", &LuaUnit_GetFaction },
	{ "EnableMoveFly", &LuaUnit_MoveFly },
	{ "SpellNonMeleeDamageLog", &LuaUnit_SpellNonMeleeDamageLog },
	{ "DisableRespawn", &LuaUnit_NoRespawn },
	{ "ModThreat", &LuaUnit_ModThreat },
	{ "GetThreat", &LuaUnit_GetThreatByPtr },
	{ "GetInRangeFriends", &LuaUnit_GetInRangeFriends },
	{ "GetPowerType", &LuaUnit_GetPowerType },
	{ "GetMapId", &LuaUnit_GetMapId },
	{ "CalcDistance" , &LuaUnit_CalcDistance },
	{ "AttackReaction", &LuaUnit_AttackReaction },
	{ "EventCastSpell", &LuaUnit_EventCastSpell },
	{ "IsPlayerMoving", &LuaUnit_IsPlayerMoving },
	{ "IsPlayerAttacking", &LuaUnit_IsPlayerAttacking },
	{ "RemoveThreat", &LuaUnit_RemoveThreatByPtr },
	{ "SetPlayerAtWar", &LuaUnit_SetPlayerAtWar },
	{ "GetFactionStanding", &LuaUnit_GetFactionStanding },
	{ "PlaySpellVisual", &LuaUnit_PlaySpellVisual },
	{ "GetPlayerLevel", &LuaUnit_GetPlayerLevel },
	{ "SetPlayerLevel", &LuaUnit_SetPlayerLevel },
	{ "SetStanding", &LuaUnit_SetPlayerStanding },
	{ "GetStanding", &LuaUnit_GetStanding },
	{ "HasItem", &LuaUnit_HasItem },
	{ "AdvanceSkill", &LuaUnit_AdvanceSkill },
	{ "AddSkill", &LuaUnit_AddSkill },
	{ "RemoveSkill", &LuaUnit_RemoveSkill },
	{ "EnableFlyCheat", &LuaUnit_FlyCheat },
	{ "AddAssistTarget", &LuaUnit_AddAssistTargets },
	{ "GetCurrentSpellId", &LuaUnit_GetCurrentSpellId },
	{ "GetPlayerRace", &LuaUnit_GetPlayerRace },
	{ "RemoveAurasByMechanic", &LuaUnit_RemoveAurasByMechanic },
	{ "RemoveAurasType", &LuaUnit_RemoveAurasType },

	/* LuaHypArc: Crow: Judging by the comments, this is contributions by Hypersniper(ofc), Halestorm and Alvanaar. */
	{ "AddAura", &LuaUnit_AddAura },
	{ "SetAIState", &LuaUnit_SetAIState },
	{ "InterruptSpell", &LuaUnit_InterruptSpell },
	{ "RemoveStealth", &LuaUnit_RemoveStealth },
	{ "IsPoisoned", &LuaUnit_IsPoisoned },
	{ "SetStealthLevel", &LuaUnit_SetStealth },
	{ "GetStealthLevel", &LuaUnit_GetStealthLevel },
	{ "IsStealthed", &LuaUnit_IsStealthed },
	{ "RemoveFlag", &LuaUnit_RemoveFlag },
	{ "RegisterAIUpdateEvent", &LuaUnit_RegisterAIUpdateEvent },
	{ "ModifyAIUpdateEvent", &LuaUnit_ModifyAIUpdateEvent },
	{ "RemoveAIUpdateEvent", &LuaUnit_RemoveAIUpdateEvent },
	{ "DeleteWaypoint", &LuaUnit_deleteWaypoint },
	{ "DealGoldCost", &LuaUnit_DealGoldCost },
	{ "DealGoldMerit", &LuaUnit_DealGoldMerit },
	{ "CanUseCommand", &LuaUnit_CanUseCommand },
	{ "DeMorph", &LuaUnit_DeMorph },
	{ "Attack", &LuaUnit_Attack },
	{ "GetTarget", &LuaUnit_GetTarget },
	{ "GetSelection", &LuaUnit_GetSelection },
	{ "SetMount", &LuaUnit_SetMount },
	{ "StartQuest", &LuaUnit_StartQuest },
	{ "FinishQuest", &LuaUnit_FinishQuest },
	{ "RepairAllPlayerItems", &LuaUnit_RepairAllPlayerItems },
	{ "SetKnownTitle", &LuaUnit_SetKnownTitle },
	{ "LifeTimeKills", &LuaUnit_LifeTimeKills },
	{ "HasTitle", &LuaUnit_HasTitle },
	{ "GetMaxSkill", &LuaUnit_GetMaxSkill },
	{ "GetCurrentSkill", &LuaUnit_GetCurrentSkill },
	{ "HasSkill", &LuaUnit_HasSkill },
	{ "GetGuildName", &LuaUnit_GetGuildName },
	{ "ClearCooldownForSpell", &LuaUnit_ClearCooldownForSpell },
	{ "HasSpell", &LuaUnit_HasSpell },
	{ "ClearAllCooldowns", &LuaUnit_ClearAllCooldowns },
	{ "ResetAllTalents", &LuaUnit_ResetAllTalents },
	{ "GetAccountName", &LuaUnit_GetAccountName },
	{ "SavePlayer", &LuaUnit_SavePlayer },
	{ "HasQuest", &LuaUnit_HasQuest },
	{ "RemovePvPFlag", &LuaUnit_RemovePvPFlag },
	{ "RemoveNegativeAuras", &LuaUnit_RemoveNegativeAuras },
	{ "GossipMiscAction", &LuaUnit_GossipMiscAction },
	{ "EquipWeapons", &LuaUnit_EquipWeapons },
	{ "Dismount", &LuaUnit_Dismount },
	{ "AdvanceAllSkills", &LuaUnit_AdvanceAllSkills },
	{ "GetTeam", &LuaUnit_GetTeam },
	{ "Possess", &LuaUnit_Possess },
	{ "Unpossess", &LuaUnit_Unpossess },
	{ "StartTaxi", &LuaUnit_StartTaxi },
	{ "ChannelSpell", &LuaUnit_ChannelSpell },
	{ "StopChannel", &LuaUnit_StopChannel },
	{ "EnableFlight", &LuaUnit_EnableFlight },
	{ "GetCoinage", &LuaUnit_GetCoinage },
	{ "FlagPvP", &LuaUnit_FlagPvP },
	{ "GetDisplay", &LuaUnit_GetDisplay },
	{ "GetNativeDisplay", &LuaUnit_GetNativeDisplay },
	{ "IsMounted", &LuaUnit_IsMounted },
	{ "GetGameTime", &LuaUnit_GetGameTime },
	{ "PlaySoundToPlayer", &LuaUnit_PlaySoundToPlayer },
	{ "GetDuelState", &LuaUnit_GetDuelState }, 
	{ "SetPosition", &LuaUnit_SetPosition},
	{ "CastSpellOnTarget", &LuaUnit_CastSpellOnTarget},
	{ "GetLandHeight", &LuaUnit_GetLandHeight},
	{ "QuestAddStarter", &LuaUnit_QuestAddStarter},
	{ "QuestAddFinisher", &LuaUnit_QuestAddFinisher},
	{ "SetPlayerSpeed", &LuaUnit_SetPlayerSpeed},
	{ "GiveHonor", &LuaUnit_GiveHonor},
	{ "SetBindPoint", &LuaUnit_SetBindPoint},
	{ "SoftDisconnect", &LuaUnit_SoftDisconnect},
	{ "SetZoneWeather", &LuaUnit_SetZoneWeather},
	{ "SetPlayerWeather", &LuaUnit_SetPlayerWeather},
	{ "SendPacketToPlayer", &LuaUnit_SendPacketToPlayer},
	{ "PlayerSendChatMessage", &LuaUnit_PlayerSendChatMessage},
	{ "GetDistanceYards", &LuaUnit_GetDistanceYards},
	{ "VendorAddItem", &LuaUnit_VendorAddItem},
	{ "VendorRemoveItem", &LuaUnit_VendorRemoveItem},
	{ "VendorRemoveAllItems", &LuaUnit_VendorRemoveAllItems},
	{ "CreatureHasQuest", &LuaUnit_CreatureHasQuest},
	{ "SendVendorWindow", &LuaUnit_SendVendorWindow},
	{ "SendTrainerWindow", &LuaUnit_SendTrainerWindow},
	{ "SendInnkeeperWindow", &LuaUnit_SendInnkeeperWindow},
	{ "SendBankWindow", &LuaUnit_SendBankWindow},
	{ "SendAuctionWindow", &LuaUnit_SendAuctionWindow},
	{ "SendBattlegroundWindow", &LuaUnit_SendBattlegroundWindow},
	{ "GetInventoryItem", &LuaUnit_GetInventoryItem},
	{ "GetInventoryItemById", &LuaUnit_GetInventoryItemById},
	{ "PhaseSet", &LuaUnit_PhaseSet},
	{ "PhaseAdd", &LuaUnit_PhaseAdd},
	{ "PhaseDelete", &LuaUnit_PhaseDelete},
	{ "GetPhase", &LuaUnit_GetPhase},
	{ "AggroWithInRangeFriends", &LuaUnit_AggroWithInRangeFriends},
	{ "MoveRandomArea", &LuaUnit_MoveRandomArea},
	{ "SendLootWindow", &LuaUnit_SendLootWindow},
	{ "AddLoot", &LuaUnit_AddLoot},
	{ "SetPacified", &LuaUnit_SetPacified},
	{ "SpawnVehicle", &LuaUnit_SpawnVehicle},
	{ "SetVehicle", &LuaUnit_SetVehicle},
	{ "GetVehicle", &LuaUnit_GetVehicle},
	{ "RemoveFromVehicle", &LuaUnit_RemoveFromVehicle},
	{ "GetVehicleSeat", &LuaUnit_GetVehicleSeat},
	{ "IsVehicle", &LuaUnit_IsVehicle},
	{ "GetPassengerCount", &LuaUnit_GetPassengerCount},
	{ "MoveVehicle", &LuaUnit_MoveVehicle},
	{ "SetPlayerLock", &LuaUnit_SetPlayerLock},
	{ "GetGroupPlayers", &LuaUnit_GetGroupPlayers},
	{ "IsGm", &LuaUnit_IsGm},
	{ "GetDungeonDifficulty", &LuaUnit_GetDungeonDifficulty},
	{ "GetGroupLeader", &LuaUnit_GetGroupLeader},
	{ "SetGroupLeader", &LuaUnit_SetGroupLeader},
	{ "AddGroupMember", &LuaUnit_AddGroupMember},
	{ "SetDungeonDifficulty", &LuaUnit_SetDungeonDifficulty},
	{ "ExpandToRaid", &LuaUnit_ExpandToRaid},
	{ "SendPacketToGroup", &LuaUnit_SendPacketToGroup},
	{ "IsGroupFull", &LuaUnit_IsGroupFull},
	{ "IsGroupedWith", &LuaUnit_IsGroupedWith},
	{ "GetTotalHonor", &LuaUnit_GetTotalHonor},
	{ "GetHonorToday", &LuaUnit_GetHonorToday},
	{ "GetHonorYesterday", &LuaUnit_GetHonorYesterday},
	{ "GetArenaPoints", &LuaUnit_GetArenaPoints},
	{ "AddArenaPoints", &LuaUnit_AddArenaPoints},
	{ "AddLifetimeKills", &LuaUnit_AddLifetimeKills},
	{ "GetGender", &LuaUnit_GetGender}, //9
	{ "SetGender", &LuaUnit_SetGender},
	{ "GetGroupType", &LuaUnit_GetGroupType},
	{ "SendPacketToGuild", &LuaUnit_SendPacketToGuild },
	{ "GetGuildId", &LuaUnit_GetGuildId },
	{ "GetGuildRank", &LuaUnit_GetGuildRank },
	{ "SetGuildRank", &LuaUnit_SetGuildRank },
	{ "IsInGuild", &LuaUnit_IsInGuild }, //5
	{ "SendGuildInvite", &LuaUnit_SendGuildInvite },
	{ "DemoteGuildMember", &LuaUnit_DemoteGuildMember },
	{ "PromoteGuildMember", &LuaUnit_PromoteGuildMember },
	{ "SetGuildMotd", &LuaUnit_SetGuildMotd },
	{ "GetGuildMotd", &LuaUnit_GetGuildMotd },
	{ "SetGuildInformation", &LuaUnit_SetGuildInformation },
	{ "AddGuildMember", &LuaUnit_AddGuildMember },
	{ "RemoveGuildMember", &LuaUnit_RemoveGuildMember },
	{ "SetPublicNote", &LuaUnit_SetPublicNote },
	{ "SetOfficerNote", &LuaUnit_SetOfficerNote },
	{ "DisbandGuild", &LuaUnit_DisbandGuild },
	{ "ChangeGuildMaster", &LuaUnit_ChangeGuildMaster },
	{ "SendGuildChatMessage", &LuaUnit_SendGuildChatMessage },
	{ "SendGuildLog", &LuaUnit_SendGuildLog },
	{ "GuildBankDepositMoney", &LuaUnit_GuildBankDepositMoney },
	{ "GuildBankWithdrawMoney", &LuaUnit_GuildBankWithdrawMoney },
	{ "GetInstanceOwner", &LuaUnit_GetInstanceOwner },
	{ "GetGmRank", &LuaUnit_GetGmRank },
	{ "SetByteValue", &LuaUnit_SetByteValue },
	{ "GetByteValue", &LuaUnit_GetByteValue },
	{ "IsPvPFlagged", &LuaUnit_IsPvPFlagged },
	{ "IsFFAPvPFlagged", &LuaUnit_IsFFAPvPFlagged },
	{ "IsFFAFlagged", &LuaUnit_IsFFAPvPFlagged },
	{ "GetGuildLeader", &LuaUnit_GetGuildLeader },
	{ "GetGuildMemberCount", &LuaUnit_GetGuildMemberCount },
	{ "CanAttack", &LuaUnit_CanAttack },
	{ "GetInRangeUnits", &LuaUnit_GetInRangeUnits },
	{ "GetInRangeEnemies", &LuaUnit_GetInRangeEnemies },
	{ "IsFriendly", &LuaUnit_IsFriendly },
	{ "MovePlayerTo", &LuaUnit_MovePlayerTo },
	{ "IsInChannel", &LuaUnit_IsInChannel },
	{ "JoinChannel", &LuaUnit_JoinChannel },
	{ "LeaveChannel", &LuaUnit_LeaveChannel },
	{ "SetChannelName", &LuaUnit_SetChannelName },
	{ "SetChannelPassword", &LuaUnit_SetChannelPassword },
	{ "GetChannelPassword", &LuaUnit_GetChannelPassword },
	{ "KickFromChannel", &LuaUnit_KickFromChannel },
	{ "BanFromChannel", &LuaUnit_BanFromChannel },
	{ "UnbanFromChannel", &LuaUnit_UnbanFromChannel },
	{ "GetChannelMemberCount", &LuaUnit_GetChannelMemberCount },
	{ "GetPlayerMovementVector", &LuaUnit_GetPlayerMovementVector},
	{ "UnsetKnownTitle", &LuaUnit_UnsetKnownTitle},
	{ "IsInPhase", &LuaUnit_IsInPhase },
	{ "HasFlag", &LuaUnit_HasFlag },
	{ "Repop", &LuaUnit_Repop },
	{ "SetMovementFlags", &LuaUnit_SetMovementFlags },
	{ "GetSpawnId", &LuaUnit_GetSpawnId },
	{ "ResetTalents", &LuaUnit_ResetTalents },
	{ "SetTalentPoints", &LuaUnit_SetTalentPoints },
	{ "GetTalentPoints", &LuaUnit_GetTalentPoints },
	{ "EventChat", &LuaUnit_EventChat },
	{ "GetEquippedItemBySlot", &LuaUnit_GetEquippedItemBySlot },
	{ "GetGuildMembers", &LuaUnit_GetGuildMembers },
	{ "AddAchievement", &LuaUnit_AddAchievement },
	{ "RemoveAchievement", &LuaUnit_RemoveAchievement },
	{ "HasAchievement", &LuaUnit_HasAchievement },
	{ "RemoveArenaPoints", &LuaUnit_RemoveArenaPoints},
	{ "TakeHonor", &LuaUnit_TakeHonor},
	{ "SetPhase", &LuaUnit_PhaseSet},
	{ "DeletePhase", &LuaUnit_PhaseDelete},
	{ "AddToPhase", &LuaUnit_PhaseAdd},
	{ "GetAreaId", &LuaUnit_GetAreaId},
	{ "ResetPetTalents", &LuaUnit_ResetPetTalents},
	{ "IsDazed", &LuaUnit_IsDazed },
	{ "GetAura", &LuaUnit_GetAura },
	{ "IsRooted", &LuaUnit_IsRooted },
	{ "HasAuraWithMechanic", &LuaUnit_HasAuraWithMechanic },
	{ "HasNegativeAura", &LuaUnit_HasNegativeAura },
	{ "HasPositiveAura", &LuaUnit_HasPositiveAura },
	{ "SetActionBar", &LuaUnit_SetActionBar },
	{ "GetClosestEnemy", &LuaUnit_GetClosestEnemy },
	{ "GetClosestFriend", &LuaUnit_GetClosestFriend },
	{ "IsOnTaxi", &LuaUnit_IsOnTaxi },
	{ "GetTaxi", &LuaUnit_GetTaxi },
	{ "GetObjectType", &LuaUnit_GetObjectType },
	{ "GiveXp", &LuaUnit_GiveXp },
	{ "GetPower", &LuaUnit_GetPower },
	{ "GetPowerPct", &LuaUnit_GetPowerPct },
	{ "GetMaxPower", &LuaUnit_GetMaxPower },
	{ "SetPower", &LuaUnit_SetPower },
	{ "SetPowerPct", &LuaUnit_SetPowerPct },
	{ "SetMaxPower", &LuaUnit_SetMaxPower },
	{ "LearnSpells", &LuaUnit_LearnSpells },
	{ "GetCurrentWaypoint", &LuaUnit_GetCurrentWaypoint },
	{ "GetSelectedGO", &LuaUnit_GetSelectedGO },
	{ "FullCastSpell", &LuaUnit_FullCastSpell },
	{ "FullCastSpellOnTarget", &LuaUnit_FullCastSpellOnTarget },
	{ "DisableMelee", &LuaUnit_DisableMelee },
	{ "DisableRanged", &LuaUnit_DisableRanged },
	{ "DisableSpells", &LuaUnit_DisableSpells },
	{ "DisableCombat", &LuaUnit_DisableCombat },
	{ "DisableTargeting", &LuaUnit_DisableTargeting },
	{ "IsInGroup", &LuaUnit_IsInGroup },
	{ "GetLocation", &LuaUnit_GetLocation },
	{ "GetSpawnLocation", &LuaUnit_GetSpawnLocation },
	{ "GetPlayerMovementFlags", &LuaUnit_GetPlayerMovementFlags},
	{ "GetObject", &LuaUnit_GetObject },
	{ "GetSecondHated", &LuaUnit_GetSecondHated },
	{ "UseAI", &LuaUnit_UseAI },
	{ "FlagFFA", &LuaUnit_FlagFFA },
	{ "TeleportCreature", &LuaUnit_TeleportCreature },
	{ "IsInDungeon", &LuaUnit_IsInDungeon },
	{ "IsInRaid", &LuaUnit_IsInRaid },
	{ "CreateLuaEvent", &LuaUnit_CreateLuaEvent },
	{ "IsHostile", &LuaUnit_IsHostile },
	{ "IsAttackable", &LuaUnit_IsAttackable },
	{ "GetNumWaypoints", &LuaUnit_GetNumWaypoints },
	{ "GetWayPointsCount", &LuaUnit_GetNumWaypoints },
	{ "GetMoveType", &LuaUnit_GetMovementType },
	{ "GetQuestLogSlot", &LuaUnit_GetQuestLogSlot },
	{ "GetAuraObject", &LuaUnit_GetAuraObject },
	{ "GetAuraStackCount", &LuaUnit_GetAuraStackCount },
	{ "AddAuraObject", &LuaUnit_AddAuraObject },
	{ "GetAuraObjectById", &LuaUnit_GetAuraObjectById },
	{ "GetNativeFaction", &LuaUnit_GetNativeFaction },
	{ NULL, NULL },
};


RegType<GameObject> GOMethods[] = 
{
	{ "GossipCreateMenu", &LuaGameObject_GossipCreateMenu },
	{ "GossipSendMenu", &LuaGameObject_GossipSendMenu },
	{ "GossipComplete", &LuaGameObject_GossipComplete },
	{ "GossipSendPOI", &LuaGameObject_GossipSendPOI },
	{ "GetX", &LuaGameObject_GetX },
	{ "GetY", &LuaGameObject_GetY },
	{ "GetZ", &LuaGameObject_GetZ },
	{ "GetO", &LuaGameObject_GetO },
	{ "PlaySoundToSet", &LuaGameObject_PlaySoundToSet },
 	{ "GetName", &LuaGameObject_GetName },
	{ "Teleport" , &LuaGameObject_TeleportPlr },
	{ NULL, NULL },
};

RegType<TaxiPath> LuaTaxiMethods[] = 
{
	{ "CreateTaxi", &LuaTaxi_CreateTaxi },
	{ "GetNodeCount", &LuaTaxi_GetNodeCount },
	{ "AddPathNode", &LuaTaxi_AddPathNode },
	{ "GetId", &LuaTaxi_GetId },
	{ "GetObjectType", &LuaTaxi_GetObjectType},
	{NULL, NULL},
};

RegType<WorldPacket> LuaPacketMethods[] = 
{
	{ "CreatePacket", &LuaPacket_CreatePacket },
	{ "GetOpcode", &LuaPacket_GetOpcode },
	{ "GetSize", &LuaPacket_GetSize },
	//READ OPERATIONS
	{ "ReadByte", &LuaPacket_ReadByte },
	{ "ReadUByte", &LuaPacket_ReadUByte },
	{ "ReadShort", &LuaPacket_ReadShort },
	{ "ReadUShort", &LuaPacket_ReadUShort },
	{ "ReadLong", &LuaPacket_ReadLong },
	{ "ReadULong", LuaPacket_ReadULong },
	{ "ReadFloat", &LuaPacket_ReadFloat },
	{ "ReadDouble", &LuaPacket_ReadDouble },
	{ "ReadGUID", &LuaPacket_ReadGUID },
	{ "ReadWoWGuid", &LuaPacket_ReadWoWGuid },
	{ "ReadString", &LuaPacket_ReadString },
	//WRITE OPERATIONS
	{ "WriteUByte", &LuaPacket_WriteUByte },
	{ "WriteByte", &LuaPacket_WriteByte },
	{ "WriteShort", &LuaPacket_WriteShort },
	{ "WriteUShort", &LuaPacket_WriteUShort },
	{ "WriteLong", &LuaPacket_WriteLong },
	{ "WriteULong", &LuaPacket_WriteULong },
	{ "WriteFloat", &LuaPacket_WriteFloat },
	{ "WriteDouble", &LuaPacket_WriteDouble },
	{ "WriteGUID", &LuaPacket_WriteGUID },
	{ "WriteWoWGuid", &LuaPacket_WriteWoWGuid },
	{ "WriteString", &LuaPacket_WriteString },
	{ "GetObjectType", &LuaPacket_GetObjectType },
	{NULL, NULL},
};

RegType<Spell> SpellMethods[] = 
{
	{ "GetCaster", &LuaSpell_GetCaster },
	{ "GetEntry", &LuaSpell_GetEntry },
	{ "IsDuelSpell", &LuaSpell_IsDuelSpell },
	{ "GetSpellType", &LuaSpell_GetSpellType },
	{ "GetSpellState", &LuaSpell_GetSpellState },
	{ "Cancel", &LuaSpell_Cancel },
	{ "Cast", &LuaSpell_Cast },
	{ "CanCast", &LuaSpell_CanCast },
	{ "Finish", &LuaSpell_Finish },
	{ "GetTarget", &LuaSpell_GetTarget },
	{ "IsStealthSpell", &LuaSpell_IsStealthSpell },
	{ "IsInvisibilitySpell", &LuaSpell_IsInvisibilitySpell },
	{ "GetPossibleEnemy", &LuaSpell_GetPossibleEnemy },
	{ "GetPossibleFriend", &LuaSpell_GetPossibleFriend },
	{ "HasPower", &LuaSpell_HasPower },
	{ "IsAspect", &LuaSpell_IsAspect },
	{ "IsSeal", &LuaSpell_IsSeal },
	{ "GetObjectType", &LuaSpell_GetObjectType },
	{ "SetVar", &LuaSpell_SetVar },
	{ "GetVar", &LuaSpell_GetVar },
	{ "ResetVar", &LuaSpell_ResetVar },
	{ "ResetAllVars", &LuaSpell_ResetAllVars },
	{ "GetCastedItemId", &LuaSpell_GetCastedItemId },
	{NULL, NULL},
};

RegType<QueryResult> QResultMethods[] = 
{
	{ "GetColumn", &LuaSql_GetColumn },
	{ "NextRow", &LuaSql_NextRow },
	{ "GetColumnCount", &LuaSql_GetColumnCount },
	{ "GetRowCount", &LuaSql_GetRowCount },
	{NULL, NULL},
};

RegType<Field> SQLFieldMethods[] = 
{
	{ "GetByte", &LuaSql_GetByte },
	{ "GetUByte", &LuaSql_GetUByte },
	{ "GetShort", &LuaSql_GetShort },
	{ "GetUShort", &LuaSql_GetUShort },
	{ "GetLong", &LuaSql_GetLong },
	{ "GetULong", &LuaSql_GetULong },
	{ "GetString", &LuaSql_GetString },
	{ "GetGUID", &LuaSql_GetGUID },
	{ "GetFloat", &LuaSql_GetFloat },
	{ "GetBool", &LuaSql_GetBool },
	{NULL, NULL},
};

RegType<Aura> AuraMethods[] = 
{
	{ "GetObjectType", &LuaAura_GetObjectType },
	{ "GetSpellId", &LuaAura_GetSpellId },
	{ "GetCaster", &LuaAura_GetCaster },
	{ "GetTarget", &LuaAura_GetTarget },
	{ "GetDuration", &LuaAura_GetDuration },
	{ "SetDuration", &LuaAura_SetDuration },
	{ "GetTimeLeft", &LuaAura_GetTimeLeft },
	{ "SetNegative", &LuaAura_SetNegative },
	{ "SetPositive", &LuaAura_SetPositive },
	{ "Remove", &LuaAura_Remove },
	{ "SetVar", &LuaAura_SetVar },
	{ "GetVar", &LuaAura_GetVar },
	{ "GetAuraSlot", &LuaAura_GetAuraSlot },
	{ "SetAuraSlot", &LuaAura_SetAuraSlot },
	{NULL, NULL},
};

template<typename T> RegType<T>* GetMethodTable() { return NULL; }
template<> RegType<Item>* GetMethodTable<Item>() { return ItemMethods; }
template<> RegType<Unit>* GetMethodTable<Unit>() { return UnitMethods; }
template<> RegType<GameObject>* GetMethodTable<GameObject>() { return GOMethods; }
template<> RegType<TaxiPath>* GetMethodTable<TaxiPath>() { return LuaTaxiMethods; }
template<> RegType<WorldPacket>* GetMethodTable<WorldPacket>() { return LuaPacketMethods; }
template<> RegType<Spell>* GetMethodTable<Spell>() { return SpellMethods; }
template<> RegType<QueryResult>* GetMethodTable<QueryResult>() { return QResultMethods; }
template<> RegType<Field> * GetMethodTable<Field>() { return SQLFieldMethods; }
template<> RegType<Aura> * GetMethodTable<Aura>() { return AuraMethods; }

void RegisterGlobalFunctions(lua_State *L)
{
	lua_register(L, "PerformIngameSpawn", &luaGlobalFunctions_PerformIngameSpawn);
	lua_register(L, "GetPlayer", &luaGlobalFunctions_GetPlayer);
	lua_register(L, "GetLUAEngine", &luaGlobalFunctions_GetLUAEngine);
	lua_register(L, "GetLuaEngine", &luaGlobalFunctions_GetLUAEngine);
	lua_register(L, "GetLuaEngineVersion", &luaGlobalFunctions_GetLuaEngineVersion);
	lua_register(L, "GetGameTime", &luaGlobalFunctions_GetGameTime);
	lua_register(L, "WorldDBQuery", &luaGlobalFunctions_WorldDBQuery);
	lua_register(L, "CharDBQuery", &luaGlobalFunctions_CharDBQuery);
	lua_register(L, "WorldDBQueryTable", &luaGlobalFunctions_WorldDBQueryTable);
	lua_register(L, "CharDBQueryTable", &luaGlobalFunctions_CharDBQueryTable);
	lua_register(L, "SendWorldMessage", &luaGlobalFunctions_SendWorldMessage);
	lua_register(L, "ReloadTable", &luaGlobalFunctions_ReloadTable);
	lua_register(L, "ReloadLuaEngine", &luaGlobalFunctions_ReloadLuaEngine);
	lua_register(L, "Rehash", &luaGlobalFunctions_Rehash);
	lua_register(L, "logcol", &luaGlobalFunctions_logcol);
	lua_register(L, "GetPlayersInWorld", &luaGlobalFunctions_GetPlayersInWorld);
	lua_register(L, "GetSandshroudRevision", &EmptyGlobalFunction /*&luaGlobalFunctions_GetSandshroudRevision*/);
	lua_register(L, "GetPlayersInMap", &luaGlobalFunctions_GetPlayersInMap);
	lua_register(L, "GetPlayersInZone", &luaGlobalFunctions_GetPlayersInZone);
	lua_register(L, "SendMail", &luaGlobalFunctions_SendMail);
	lua_register(L, "GetTaxiPath", &luaGlobalFunctions_GetTaxiPath);
	lua_register(L, "SetDBCSpellVar", &EmptyGlobalFunction /*&luaGlobalFunctions_SetDBCSpellVar*/);
	lua_register(L, "GetDBCSpellVar", &EmptyGlobalFunction /*&luaGlobalFunctions_GetDBCSpellVar*/);

	//Lua's bit instructions
	lua_register(L, "bit_and", &luaGlobalFunctions_bit_and);
	lua_register(L, "bit_or", &luaGlobalFunctions_bit_or);
	lua_register(L, "bit_xor", &luaGlobalFunctions_bit_xor);
	lua_register(L, "bit_not", &luaGlobalFunctions_bit_not);
	lua_register(L, "bit_shiftleft", &luaGlobalFunctions_bit_shiftleft);
	lua_register(L, "bit_shiftright", &luaGlobalFunctions_bit_shiftright);

	lua_register(L, "RemoveTimedEventsInTable", &EmptyGlobalFunction /*&luaGlobalFunctions_RemoveTimedEventsInTable*/);
	lua_register(L, "RemoveTimedEventsWithName", &EmptyGlobalFunction /*&luaGlobalFunctions_RemoveTimedEventsWithName*/);
	lua_register(L, "RemoveTimedEvent", &EmptyGlobalFunction /*&luaGlobalFunctions_RemoveTimedEvent*/);
	lua_register(L, "HasTimedEvents", &EmptyGlobalFunction /*&luaGlobalFunctions_HasTimedEvents*/);
	lua_register(L, "HasTimedEventInTable", &EmptyGlobalFunction /*&luaGlobalFunctions_HasTimedEventInTable*/);
	lua_register(L, "HasTimedEventWithName", &EmptyGlobalFunction /*&luaGlobalFunctions_HasTimedEventWithName*/);
	lua_register(L, "HasTimedEvent", &EmptyGlobalFunction /*&luaGlobalFunctions_HasTimedEvent*/);
	lua_register(L, "GetPlatform", &EmptyGlobalFunction /*&luaGlobalFunctions_GetPlatform*/);
	lua_register(L, "NumberToGUID", &EmptyGlobalFunction /*&luaGlobalFunctions_NumberToGUID*/);
	lua_register(L, "SendPacketToWorld", &EmptyGlobalFunction /*&luaGlobalFunctions_SendPacketToWorld*/);
	lua_register(L, "SendPacketToInstance", &EmptyGlobalFunction /*&luaGlobalFunctions_SendPacketToInstance*/);
	lua_register(L, "SendPacketToZone", &EmptyGlobalFunction /*&luaGlobalFunctions_SendPacketToZone*/);
	lua_register(L, "SendPacketToChannel", &EmptyGlobalFunction /*&luaGlobalFunctions_SendPacketToChannel*/);
}

#endif // __TABLEFUNCTIONS_H
