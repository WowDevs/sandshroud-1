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

#ifndef UNIT_FUNCTIONS_H
#define UNIT_FUNCTIONS_H

////////////////////////////////////////////////////
//////////////UNIT GOSSIP///////////////////////////
////////////////////////////////////////////////////
int LuaUnit_GossipCreateMenu(lua_State * L, Unit * ptr);
int LuaUnit_GossipMenuAddItem(lua_State * L, Unit * ptr);
int LuaUnit_GossipSendMenu(lua_State * L, Unit * ptr);
int LuaUnit_GossipSendPOI(lua_State * L, Unit * ptr);
int LuaUnit_GossipComplete(lua_State * L, Unit * ptr);
int LuaUnit_IsPlayer(lua_State * L, Unit * ptr);
int LuaUnit_IsCreature(lua_State * L, Unit * ptr);
int LuaUnit_Emote(lua_State * L, Unit * ptr);
int LuaUnit_GetManaPct(lua_State * L, Unit * ptr);
int LuaUnit_GetName(lua_State * L, Unit * ptr);
int LuaUnit_SendChatMessage(lua_State * L, Unit * ptr);
int LuaUnit_MoveTo(lua_State * L, Unit * ptr);
int LuaUnit_SetMovementType(lua_State * L, Unit * ptr);
int LuaUnit_GetX(lua_State * L, Unit * ptr);
int LuaUnit_GetY(lua_State * L, Unit * ptr);
int LuaUnit_GetZ(lua_State * L, Unit * ptr);
int LuaUnit_GetO(lua_State * L, Unit * ptr);
int LuaUnit_CastSpell(lua_State * L, Unit * ptr);
int LuaUnit_FullCastSpell(lua_State * L, Unit * ptr);
int LuaUnit_CastSpellOnTarget(lua_State * L, Unit * ptr);
int LuaUnit_FullCastSpellOnTarget(lua_State * L, Unit * ptr);
int LuaUnit_SpawnCreature(lua_State * L, Unit * ptr);
int LuaUnit_SpawnGameObject(lua_State * L, Unit * ptr);
int LuaUnit_RegisterEvent(lua_State * L, Unit * ptr);
int LuaUnit_CreateLuaEvent(lua_State * L, Unit * ptr);
int LuaUnit_RemoveEvents(lua_State * L, Unit * ptr);
int LuaUnit_AddEventHolder(lua_State * L, Unit * ptr);
int LuaUnit_SetFaction(lua_State * L, Unit * ptr);
int LuaUnit_SetStandState(lua_State * L, Unit * ptr);
int LuaUnit_IsInCombat(lua_State * L, Unit * ptr);
int LuaUnit_SetScale(lua_State * L, Unit * ptr);
int LuaUnit_SetModel(lua_State * L, Unit * ptr);
int LuaUnit_SetNPCFlags(lua_State * L, Unit * ptr);
int LuaUnit_SetCombatCapable(lua_State * L, Unit * ptr);
int LuaUnit_SetCombatMeleeCapable(lua_State * L, Unit * ptr);
int LuaUnit_SetCombatRangedCapable(lua_State * L, Unit * ptr);
int LuaUnit_SetCombatSpellCapable(lua_State * L, Unit * ptr);
int LuaUnit_SetCombatTargetingCapable(lua_State * L, Unit * ptr);
int LuaUnit_DestroyCustomWaypointMap(lua_State * L, Unit * ptr);
int LuaUnit_CreateCustomWaypointMap(lua_State * L, Unit * ptr);
int LuaUnit_CreateCustomWaypoint(lua_State * L, Unit * ptr);
int LuaUnit_DeleteAllWaypoints(lua_State * L, Unit * ptr);
int LuaUnit_CreateWaypoint(lua_State * L, Unit * ptr);
int LuaUnit_MoveToWaypoint(lua_State * L, Unit * ptr);
int LuaUnit_RemoveItem(lua_State * L, Unit * ptr);
int LuaUnit_AddItem(lua_State * L, Unit * ptr);
int LuaUnit_GetInstanceID(lua_State * L, Unit * ptr);
int LuaUnit_GetClosestPlayer(lua_State * L, Unit * ptr);
int LuaUnit_GetRandomPlayer(lua_State * L, Unit * ptr);
int LuaUnit_GetRandomFriend(lua_State * L, Unit * ptr);
int LuaUnit_GetRandomEnemy(lua_State * L, Unit * ptr);
int LuaUnit_StopMovement(lua_State * L, Unit * ptr);
int LuaUnit_RemoveAura(lua_State * L, Unit * ptr);
int LuaUnit_PlaySoundToSet(lua_State * L, Unit * ptr);
int LuaUnit_GetUnitBySqlId(lua_State * L, Unit * ptr);
int LuaUnit_Despawn(lua_State * L, Unit * ptr);
int LuaUnit_GetInRangeFriends(lua_State * L, Unit * ptr);
int LuaUnit_GetHealthPct(lua_State * L, Unit * ptr);
int LuaUnit_SetHealthPct(lua_State * L, Unit * ptr);
int LuaUnit_GetItemCount(lua_State * L, Unit * ptr);
int LuaUnit_GetMainTank(lua_State * L, Unit * ptr);
int LuaUnit_GetAddTank(lua_State * L, Unit * ptr);
int LuaUnit_ClearThreatList(lua_State * L, Unit * ptr);
int LuaUnit_GetTauntedBy(lua_State * L, Unit * ptr);
int LuaUnit_SetTauntedBy(lua_State * L, Unit * ptr);
int LuaUnit_ModThreat(lua_State * L, Unit * ptr);
int LuaUnit_GetThreatByPtr(lua_State * L, Unit * ptr);
int LuaUnit_ChangeTarget(lua_State * L, Unit * ptr);
int LuaUnit_HasFinishedQuest(lua_State * L, Unit * ptr);
int LuaUnit_UnlearnSpell(lua_State * L, Unit * ptr);
int LuaUnit_LearnSpell(lua_State * L, Unit* ptr);
int LuaUnit_MarkQuestObjectiveAsComplete(lua_State * L, Unit * ptr);
int LuaUnit_KnockBack(lua_State * L, Unit * ptr);
int LuaUnit_SendAreaTriggerMessage(lua_State * L, Unit * ptr);
int LuaUnit_SendBroadcastMessage(lua_State * L, Unit * ptr);
int LuaUnit_TeleportUnit(lua_State * L, Unit * ptr);
int LuaUnit_GetHealth(lua_State * L, Unit * ptr);
int LuaUnit_GetMaxHealth(lua_State * L, Unit * ptr);
int LuaUnit_SetHealth(lua_State * L, Unit * ptr);
int LuaUnit_SetMaxHealth(lua_State * L, Unit * ptr);
int LuaUnit_WipeHateList(lua_State * L, Unit * ptr);
int LuaUnit_WipeTargetList(lua_State * L, Unit * ptr);
int LuaUnit_WipeCurrentTarget(lua_State * L, Unit * ptr);
int LuaUnit_GetPlayerClass(lua_State * L, Unit * ptr);
int LuaUnit_ClearHateList(lua_State * L, Unit * ptr);
int LuaUnit_SetMana(lua_State * L, Unit * ptr);
int LuaUnit_SetMaxMana(lua_State * L, Unit * ptr);
int LuaUnit_GetPlayerRace(lua_State * L, Unit * ptr);
int LuaUnit_SetFlying(lua_State * L, Unit * ptr);
int LuaUnit_Land(lua_State * L, Unit * ptr);
int LuaUnit_HasAura(lua_State * L, Unit * ptr);
int LuaUnit_ReturnToSpawnPoint(lua_State * L, Unit * ptr);
int LuaUnit_GetGUID(lua_State * L, Unit * ptr);
int LuaUnit_GetDistance(lua_State * L, Unit * ptr);
int LuaUnit_GetCreatureNearestCoords(lua_State * L, Unit * ptr);
int LuaUnit_GetGameObjectNearestCoords(lua_State *L, Unit * ptr);
int LuaUnit_CastSpellAoF(lua_State * L, Unit * ptr);
int LuaUnit_SetInFront(lua_State * L, Unit * ptr);
int LuaUnit_RemoveAllAuras(lua_State *L, Unit * ptr);
int LuaUnit_CancelSpell(lua_State * L, Unit * ptr);
int LuaUnit_IsAlive(lua_State * L, Unit * ptr);
int LuaUnit_IsDead(lua_State * L, Unit * ptr);
int LuaUnit_IsInWorld(lua_State * L, Unit * ptr);
int LuaUnit_GetZoneId(lua_State *L, Unit * ptr);
int LuaUnit_GetMana(lua_State * L, Unit * ptr);
int LuaUnit_GetMaxMana(lua_State * L, Unit * ptr);
int LuaUnit_Root(lua_State * L, Unit * ptr);
int LuaUnit_Unroot(lua_State * L, Unit * ptr);
int LuaUnit_IsCreatureMoving(lua_State * L, Unit * ptr);
int LuaUnit_SetOutOfCombatRange(lua_State * L, Unit * ptr);
int LuaUnit_ModifyRunSpeed(lua_State * L, Unit * ptr);
int LuaUnit_ModifyWalkSpeed(lua_State * L, Unit * ptr);
int LuaUnit_ModifyFlySpeed(lua_State * L, Unit * ptr);
int LuaUnit_IsFlying(lua_State * L, Unit * ptr);
int LuaUnit_SetRotation(lua_State * L, Unit * ptr);
int LuaUnit_SetOrientation(lua_State * L, Unit * ptr);
int LuaUnit_CalcDistance(lua_State * L, Unit * ptr);
int LuaUnit_GetSpawnX(lua_State * L, Unit * ptr);
int LuaUnit_GetSpawnY(lua_State * L, Unit * ptr);
int LuaUnit_GetSpawnZ(lua_State * L, Unit * ptr);
int LuaUnit_GetSpawnO(lua_State * L, Unit * ptr);
int LuaUnit_GetInRangePlayersCount(lua_State * L, Unit * ptr);
int LuaUnit_GetEntry(lua_State * L, Unit * ptr);
int LuaUnit_SetMoveRunFlag(lua_State * L, Unit * ptr);
int LuaUnit_HandleEvent(lua_State * L, Unit * ptr);
int LuaUnit_GetCurrentSpellId(lua_State * L, Unit * ptr);
int LuaUnit_GetCurrentSpell(lua_State * L, Unit * ptr);
int LuaUnit_AddAssistTargets(lua_State * L, Unit * ptr);
int LuaUnit_GetAIState(lua_State * L, Unit * ptr);
int LuaUnit_GetFloatValue(lua_State * L, Unit * ptr);
int LuaUnit_SendPacket(lua_State * L, Unit * ptr);
int LuaUnit_InitPacket(lua_State * L, Unit * ptr);
int LuaUnit_AddDataToPacket(lua_State * L, Unit * ptr);
int LuaUnit_AddGuidDataToPacket(lua_State * L, Unit * ptr);
int LuaUnit_ModUInt32Value(lua_State * L, Unit * ptr);
int LuaUnit_ModFloatValue(lua_State * L, Unit * ptr);
int LuaUnit_SetUInt32Value(lua_State * L, Unit * ptr);
int LuaUnit_SetUInt64Value(lua_State * L, Unit * ptr);
int LuaUnit_SetFloatValue(lua_State * L, Unit * ptr);
int LuaUnit_GetUInt32Value(lua_State * L, Unit * ptr);
int LuaUnit_GetUInt64Value(lua_State * L, Unit * ptr);
int LuaUnit_AdvanceQuestObjective(lua_State * L, Unit * ptr);
int LuaUnit_Heal(lua_State * L, Unit * ptr);
int LuaUnit_Energize(lua_State * L, Unit * ptr);
int LuaUnit_SendChatMessageAlternateEntry(lua_State * L, Unit * ptr);
int LuaUnit_SendChatMessageToPlayer(lua_State * L, Unit * ptr);
int LuaUnit_Strike(lua_State * L, Unit * ptr);
int LuaUnit_SetAttackTimer(lua_State * L, Unit * ptr);
int LuaUnit_Kill(lua_State * L, Unit * ptr);
int LuaUnit_DealDamage(lua_State * L, Unit * ptr);
int LuaUnit_SetNextTarget(lua_State * L, Unit * ptr);
int LuaUnit_GetNextTarget(lua_State * L, Unit * ptr);
int LuaUnit_SetPetOwner(lua_State * L, Unit * ptr);
int LuaUnit_DismissPet(lua_State * L, Unit * ptr);
int LuaUnit_IsPet(lua_State * L, Unit * ptr);
int LuaUnit_GetPetOwner(lua_State * L, Unit * ptr);
int LuaUnit_SetUnitToFollow(lua_State * L, Unit * ptr);
int LuaUnit_GetUnitToFollow(lua_State * L, Unit * ptr);
int LuaUnit_IsInFront(lua_State * L, Unit * ptr);
int LuaUnit_IsInBack(lua_State * L, Unit * ptr);
int LuaUnit_IsPacified(lua_State * L, Unit * ptr);
int LuaUnit_SetPacified(lua_State * L, Unit * ptr);
int LuaUnit_IsFeared(lua_State * L, Unit * ptr);
int LuaUnit_IsStunned(lua_State * L, Unit * ptr);
int LuaUnit_CreateGuardian(lua_State * L, Unit * ptr);
int LuaUnit_IsInArc(lua_State * L, Unit * ptr);
int LuaUnit_IsInWater(lua_State * L, Unit * ptr);
int LuaUnit_GetAITargetsCount(lua_State * L, Unit * ptr);
int LuaUnit_GetUnitByGUID(lua_State * L, Unit * ptr);
int LuaUnit_GetInRangeObjectsCount(lua_State * L, Unit * ptr);
int LuaUnit_GetInRangePlayers(lua_State * L, Unit * ptr);
int LuaUnit_GetInRangeGameObjects(lua_State * L, Unit * ptr);
int LuaUnit_HasInRangeObjects(lua_State * L, Unit * ptr);
int LuaUnit_SetFacing(lua_State * L, Unit * ptr);
int LuaUnit_CalcToDistance(lua_State * L, Unit * ptr);
int LuaUnit_CalcAngle(lua_State * L, Unit * ptr);
int LuaUnit_CalcRadAngle(lua_State * L, Unit * ptr);
int LuaUnit_IsInvisible(lua_State * L, Unit * ptr);
int LuaUnit_MoveFly(lua_State * L, Unit * ptr);
int LuaUnit_IsInvincible(lua_State * L, Unit * ptr);
int LuaUnit_ResurrectPlayer(lua_State * L, Unit * ptr);
int LuaUnit_KickPlayer(lua_State * L, Unit * ptr);
int LuaUnit_CanCallForHelp(lua_State * L, Unit * ptr);
int LuaUnit_CallForHelpHp(lua_State * L, Unit * ptr);
int LuaUnit_SetDeathState(lua_State * L, Unit * ptr);
int LuaUnit_SetCreatureName(lua_State * L, Unit * ptr);
int LuaUnit_GetSpellId(lua_State * L, Unit * ptr);
int LuaUnit_SetNextSpell(lua_State * L, Unit * ptr);
int LuaUnit_Possess(lua_State * L, Unit * ptr);
int LuaUnit_Unpossess(lua_State * L, Unit * ptr);
int LuaUnit_RemoveFromWorld(lua_State * L, Unit * ptr);
int LuaUnit_GetFaction(lua_State * L, Unit * ptr);
int LuaUnit_SpellNonMeleeDamageLog(lua_State * L, Unit * ptr);
int LuaUnit_NoRespawn(lua_State * L, Unit * ptr);
int LuaUnit_GetMapId(lua_State * L, Unit * ptr);
int LuaUnit_AttackReaction(lua_State * L, Unit * ptr);
int LuaUnit_EventCastSpell(lua_State * L, Unit * ptr);
int LuaUnit_IsPlayerMoving(lua_State * L, Unit * ptr);
int LuaUnit_IsPlayerAttacking(lua_State * L, Unit * ptr);
int LuaUnit_GetFactionStanding(lua_State * L, Unit * ptr);
int LuaUnit_SetPlayerAtWar(lua_State * L, Unit * ptr);
int LuaUnit_SetPlayerStanding(lua_State * L, Unit * ptr);
int LuaUnit_GetStanding(lua_State * L, Unit * ptr);
int LuaUnit_RemoveThreatByPtr(lua_State * L, Unit * ptr);
int LuaUnit_HasItem(lua_State * L, Unit * ptr);
int LuaUnit_PlaySpellVisual(lua_State * L, Unit * ptr);
int LuaUnit_GetPlayerLevel(lua_State * L, Unit * ptr);
int LuaUnit_SetPlayerLevel(lua_State * L, Unit * ptr);
int LuaUnit_AddSkill(lua_State * L, Unit * ptr);
int LuaUnit_RemoveSkill(lua_State * L, Unit * ptr);
int LuaUnit_FlyCheat(lua_State * L, Unit * ptr);
int LuaUnit_AdvanceSkill(lua_State * L, Unit * ptr);
int LuaUnit_RemoveAurasByMechanic(lua_State * L, Unit * ptr);
int LuaUnit_RemoveAurasType(lua_State * L, Unit * ptr);
int LuaUnit_AddAura(lua_State * L, Unit * ptr);
int LuaUnit_SetAIState(lua_State * L, Unit * ptr);
int LuaUnit_SetStealth(lua_State * L, Unit * ptr);
int LuaUnit_GetStealthLevel(lua_State * L, Unit * ptr);
int LuaUnit_IsStealthed(lua_State * L, Unit * ptr);
int LuaUnit_RemoveStealth(lua_State * L, Unit * ptr);
int LuaUnit_InterruptSpell(lua_State * L, Unit * ptr);
int LuaUnit_IsPoisoned(lua_State * L, Unit * ptr);
int LuaUnit_RegisterAIUpdateEvent(lua_State * L, Unit * ptr);
int LuaUnit_ModifyAIUpdateEvent(lua_State * L, Unit * ptr);
int LuaUnit_RemoveAIUpdateEvent(lua_State * L, Unit * ptr);
int LuaUnit_deleteWaypoint(lua_State * L, Unit * ptr);
int LuaUnit_DealGoldCost(lua_State * L, Unit * ptr);
int LuaUnit_DealGoldMerit(lua_State * L, Unit * ptr);
int LuaUnit_DeMorph(lua_State * L, Unit * ptr);
int LuaUnit_Attack(lua_State * L, Unit * ptr);
int LuaUnit_CanUseCommand(lua_State * L, Unit * ptr);
int LuaUnit_GetTarget(lua_State * L, Unit * ptr);
int LuaUnit_GetSelection(lua_State * L, Unit * ptr);
int LuaUnit_GetSelectedGO(lua_State * L, Unit * ptr);
int LuaUnit_RepairAllPlayerItems(lua_State * L, Unit * ptr);
int LuaUnit_SetKnownTitle(lua_State * L, Unit * ptr);
int LuaUnit_UnsetKnownTitle(lua_State * L, Unit * ptr);
int LuaUnit_LifeTimeKills(lua_State * L, Unit * ptr);
int LuaUnit_HasTitle(lua_State * L, Unit * ptr);
int LuaUnit_GetMaxSkill(lua_State * L, Unit * ptr);
int LuaUnit_GetCurrentSkill(lua_State * L, Unit * ptr);
int LuaUnit_HasSkill(lua_State * L, Unit * ptr);
int LuaUnit_GetGuildName(lua_State * L, Unit * ptr);
int LuaUnit_ClearCooldownForSpell(lua_State * L, Unit * ptr);
int LuaUnit_HasSpell(lua_State * L, Unit * ptr);
int LuaUnit_ClearAllCooldowns(lua_State * L, Unit * ptr);
int LuaUnit_ResetAllTalents(lua_State * L, Unit * ptr);
int LuaUnit_GetAccountName(lua_State * L, Unit * ptr);
int LuaUnit_GetGmRank(lua_State * L, Unit * ptr);
int LuaUnit_IsGm(lua_State * L, Unit * ptr);
int LuaUnit_SavePlayer(lua_State * L, Unit * ptr);
int LuaUnit_HasQuest(lua_State * L, Unit * ptr);
int LuaUnit_CreatureHasQuest(lua_State * L, Unit * ptr);
int LuaUnit_RemovePvPFlag(lua_State * L, Unit * ptr);
int LuaUnit_RemoveNegativeAuras(lua_State * L, Unit * ptr);
int LuaUnit_GossipMiscAction(lua_State * L, Unit * ptr);
int LuaUnit_SendVendorWindow(lua_State * L, Unit * ptr);
int LuaUnit_SendTrainerWindow(lua_State * L, Unit * ptr);
int LuaUnit_SendInnkeeperWindow(lua_State * L, Unit * ptr);
int LuaUnit_SendBankWindow(lua_State * L, Unit * ptr);
int LuaUnit_SendAuctionWindow(lua_State * L, Unit * ptr);
int LuaUnit_SendBattlegroundWindow(lua_State * L, Unit * ptr);
int LuaUnit_SendLootWindow(lua_State * L, Unit * ptr);
int LuaUnit_AddLoot(lua_State * L, Unit * ptr);
int LuaUnit_VendorAddItem(lua_State * L, Unit * ptr);
int LuaUnit_VendorRemoveItem(lua_State * L, Unit * ptr);
int LuaUnit_VendorRemoveAllItems(lua_State * L, Unit * ptr);
int LuaUnit_EquipWeapons(lua_State * L, Unit * ptr);
int LuaUnit_Dismount(lua_State * L, Unit * ptr);
int LuaUnit_GiveXp(lua_State * L, Unit * ptr);
int LuaUnit_AdvanceAllSkills(lua_State * L, Unit * ptr);
int LuaUnit_GetTeam(lua_State * L, Unit * ptr);
int LuaUnit_StartTaxi(lua_State * L, Unit * ptr);
int LuaUnit_IsOnTaxi(lua_State * L, Unit * ptr);
int LuaUnit_GetTaxi(lua_State * L, Unit * ptr);
int LuaUnit_SetPlayerLock(lua_State * L, Unit * ptr);
int LuaUnit_MovePlayerTo(lua_State * L, Unit * ptr);
int LuaUnit_ChannelSpell(lua_State * L, Unit * ptr);
int LuaUnit_StopChannel(lua_State * L, Unit * ptr);
int LuaUnit_EnableFlight(lua_State * L, Unit * ptr);
int LuaUnit_GetCoinage(lua_State * L, Unit * ptr);
int LuaUnit_FlagPvP(lua_State * L, Unit * ptr);
int LuaUnit_IsMounted(lua_State * L, Unit * ptr);
int LuaUnit_SpawnVehicle(lua_State * L, Unit * ptr);
int LuaUnit_SetVehicle(lua_State * L, Unit * ptr);
int LuaUnit_GetVehicle(lua_State * L, Unit * ptr);
int LuaUnit_RemoveFromVehicle(lua_State * L, Unit * ptr);
int LuaUnit_GetVehicleSeat(lua_State * L, Unit * ptr);
int LuaUnit_IsVehicle(lua_State * L, Unit * ptr);
int LuaUnit_GetPassengerCount(lua_State * L, Unit * ptr);
int LuaUnit_MoveVehicle(lua_State * L, Unit * ptr);
int LuaUnit_IsGroupedWith(lua_State * L, Unit * ptr);
int LuaUnit_GetGroupType(lua_State * L, Unit * ptr);
int LuaUnit_GetTotalHonor(lua_State * L, Unit * ptr);
int LuaUnit_GetHonorToday(lua_State * L, Unit * ptr);
int LuaUnit_GetHonorYesterday(lua_State * L, Unit * ptr);
int LuaUnit_GetArenaPoints(lua_State * L, Unit * ptr);
int LuaUnit_AddArenaPoints(lua_State * L, Unit * ptr);
int LuaUnit_RemoveArenaPoints(lua_State * L, Unit * ptr);
int LuaUnit_AddLifetimeKills(lua_State * L, Unit * ptr);
int LuaUnit_GetGender(lua_State * L, Unit * ptr);
int LuaUnit_SetGender(lua_State * L, Unit * ptr);
int LuaUnit_SendPacketToGuild(lua_State * L, Unit * ptr);
int LuaUnit_GetGuildId(lua_State * L, Unit * ptr);
int LuaUnit_GetGuildRank(lua_State * L, Unit * ptr);
int LuaUnit_SetGuildRank(lua_State * L, Unit * ptr);
int LuaUnit_IsInGuild(lua_State * L, Unit * ptr);
int LuaUnit_SendGuildInvite(lua_State * L, Unit * ptr);
int LuaUnit_DemoteGuildMember(lua_State * L, Unit * ptr);
int LuaUnit_PromoteGuildMember(lua_State * L, Unit * ptr);
int LuaUnit_SetGuildMotd(lua_State * L, Unit * ptr);
int LuaUnit_GetGuildMotd(lua_State * L, Unit * ptr);
int LuaUnit_SetGuildInformation(lua_State * L, Unit * ptr);
int LuaUnit_AddGuildMember(lua_State * L, Unit * ptr);
int LuaUnit_RemoveGuildMember(lua_State * L, Unit * ptr);
int LuaUnit_SetPublicNote(lua_State * L, Unit * ptr);
int LuaUnit_SetOfficerNote(lua_State * L, Unit * ptr);
int LuaUnit_DisbandGuild(lua_State * L, Unit * ptr);
int LuaUnit_ChangeGuildMaster(lua_State * L, Unit * ptr);
int LuaUnit_SendGuildChatMessage(lua_State * L, Unit * ptr);
int LuaUnit_SendGuildLog(lua_State * L, Unit * ptr);
int LuaUnit_GuildBankDepositMoney(lua_State * L, Unit * ptr);
int LuaUnit_GuildBankWithdrawMoney(lua_State * L, Unit * ptr);
int LuaUnit_SetByteValue(lua_State * L, Unit * ptr);
int LuaUnit_GetByteValue(lua_State * L, Unit * ptr);
int LuaUnit_IsPvPFlagged(lua_State * L, Unit * ptr);
int LuaUnit_IsFFAPvPFlagged(lua_State * L, Unit * ptr);
int LuaUnit_GetGuildLeader(lua_State * L, Unit * ptr);
int LuaUnit_GetGuildMemberCount(lua_State * L, Unit * ptr);
int LuaUnit_IsFriendly(lua_State * L, Unit * ptr);
int LuaUnit_IsInChannel(lua_State * L, Unit * ptr);
int LuaUnit_JoinChannel(lua_State * L, Unit * ptr);
int LuaUnit_LeaveChannel(lua_State * L, Unit * ptr);
int LuaUnit_SetChannelName(lua_State * L, Unit * ptr);
int LuaUnit_SetChannelPassword(lua_State * L, Unit * ptr);
int LuaUnit_GetChannelPassword(lua_State * L, Unit * ptr);
int LuaUnit_KickFromChannel(lua_State * L, Unit * ptr);
int LuaUnit_BanFromChannel(lua_State * L, Unit * ptr);
int LuaUnit_UnbanFromChannel(lua_State * L, Unit * ptr);
int LuaUnit_GetChannelMemberCount(lua_State * L, Unit * ptr);
int LuaUnit_GetPlayerMovementVector(lua_State * L, Unit * ptr);
int LuaUnit_GetPlayerMovementFlags(lua_State * L, Unit * ptr);
int LuaUnit_Repop(lua_State * L, Unit * ptr);
int LuaUnit_SetMovementFlags(lua_State * L, Unit * ptr);
int LuaUnit_GetSpawnId(lua_State * L, Unit * ptr);
int LuaUnit_ResetTalents(lua_State * L, Unit * ptr);
int LuaUnit_SetTalentPoints(lua_State * L, Unit * ptr);
int LuaUnit_GetTalentPoints(lua_State * L, Unit * ptr);
int LuaUnit_EventChat(lua_State * L, Unit * ptr);
int LuaUnit_GetEquippedItemBySlot(lua_State * L, Unit * ptr);
int LuaUnit_GetGuildMembers(lua_State * L, Unit * ptr);
int LuaUnit_AddAchievement(lua_State * L, Unit * ptr);
int LuaUnit_RemoveAchievement(lua_State * L, Unit * ptr);
int LuaUnit_HasAchievement(lua_State * L, Unit * ptr);
int LuaUnit_GetAreaId(lua_State * L, Unit * ptr);
int LuaUnit_ResetPetTalents(lua_State * L, Unit * ptr);
int LuaUnit_IsDazed(lua_State * L, Unit * ptr);
int LuaUnit_GetAura(lua_State * L, Unit * ptr);
int LuaUnit_GetAuraObject(lua_State * L, Unit * ptr);
int LuaUnit_IsRooted(lua_State * L, Unit * ptr);
int LuaUnit_HasAuraWithMechanic(lua_State * L, Unit * ptr);
int LuaUnit_HasNegativeAura(lua_State * L, Unit * ptr);
int LuaUnit_HasPositiveAura(lua_State * L, Unit * ptr);
int LuaUnit_SetActionBar(lua_State * L, Unit * ptr);
int LuaUnit_GetClosestEnemy(lua_State * L, Unit * ptr);
int LuaUnit_GetClosestFriend(lua_State * L, Unit * ptr);
int LuaUnit_GetObjectType(lua_State * L, Unit * ptr);
int LuaUnit_GetCurrentWaypoint(lua_State * L, Unit * ptr);
int LuaUnit_DisableMelee(lua_State * L, Unit * ptr);
int LuaUnit_DisableSpells(lua_State * L, Unit * ptr);
int LuaUnit_DisableRanged(lua_State * L, Unit * ptr);
int LuaUnit_DisableCombat(lua_State * L, Unit * ptr);
int LuaUnit_DisableTargeting(lua_State * L, Unit * ptr);
int LuaUnit_IsInGroup(lua_State * L, Unit * ptr);
int LuaUnit_GetLocation(lua_State * L, Unit * ptr);
int LuaUnit_GetByte(lua_State * L, Unit * ptr);
int LuaUnit_SetByte(lua_State * L, Unit * ptr);
int LuaUnit_GetSpawnLocation(lua_State * L, Unit * ptr);
int LuaUnit_GetObject(lua_State * L, Unit * ptr);
int LuaUnit_GetSecondHated(lua_State * L, Unit * ptr);
int LuaUnit_UseAI(lua_State * L, Unit * ptr);
int LuaUnit_FlagFFA(lua_State * L, Unit * ptr);
int LuaUnit_TeleportCreature(lua_State * L, Unit * ptr);
int LuaUnit_IsInDungeon(lua_State * L, Unit * ptr);
int LuaUnit_IsInRaid(lua_State * L, Unit * ptr);
int LuaUnit_IsHostile(lua_State*  L, Unit * ptr);
int LuaUnit_IsAttackable(lua_State*  L, Unit * ptr);
int LuaUnit_GetNumWaypoints(lua_State * L, Unit * ptr);
int LuaUnit_GetMovementType(lua_State * L, Unit * ptr);
int LuaUnit_GetQuestLogSlot(lua_State * L, Unit * ptr);
int LuaUnit_GetAuraStackCount(lua_State * L, Unit * ptr);
int LuaUnit_AddAuraObject(lua_State * L, Unit * ptr);
int LuaUnit_GetAuraObjectById(lua_State * L, Unit * ptr);
int LuaUnit_GetNativeFaction(lua_State * L, Unit * ptr);
int LuaUnit_RemoveFlag(lua_State * L, Unit * ptr);
int LuaUnit_SetMount(lua_State * L, Unit * ptr);
int LuaUnit_StartQuest(lua_State * L, Unit * ptr);
int LuaUnit_FinishQuest(lua_State * L, Unit * ptr);
int LuaUnit_GetDisplay(lua_State * L, Unit * ptr);
int LuaUnit_GetNativeDisplay(lua_State * L, Unit * ptr);
int LuaUnit_GetGameTime(lua_State * L, Unit * ptr);
int LuaUnit_PlaySoundToPlayer(lua_State * L, Unit * ptr);
int LuaUnit_GetDuelState(lua_State * L, Unit * ptr);
int LuaUnit_SetPosition(lua_State * L, Unit * ptr);
int LuaUnit_GetLandHeight(lua_State * L, Unit * ptr);
int LuaUnit_QuestAddStarter(lua_State * L, Unit * ptr);
int LuaUnit_QuestAddFinisher(lua_State * L, Unit * ptr);
int LuaUnit_SetPlayerSpeed(lua_State * L, Unit * ptr);
int LuaUnit_GiveHonor(lua_State * L, Unit * ptr);
int LuaUnit_SetBindPoint(lua_State * L, Unit * ptr);
int LuaUnit_SoftDisconnect(lua_State * L, Unit * ptr);
int LuaUnit_GetInventoryItem(lua_State * L, Unit * ptr);
int LuaUnit_GetInventoryItemById(lua_State * L, Unit * ptr);
int LuaUnit_PhaseSet(lua_State * L, Unit * ptr);
int LuaUnit_PhaseAdd(lua_State * L, Unit * ptr);
int LuaUnit_PhaseDelete(lua_State * L, Unit * ptr);
int LuaUnit_GetPhase(lua_State * L, Unit * ptr);
int LuaUnit_GetPhaseMask(lua_State * L, Unit * ptr);
int LuaUnit_IsInPhase(lua_State * L, Unit * ptr);
int LuaUnit_SetZoneWeather(lua_State * L, Unit * ptr);
int LuaUnit_SetPlayerWeather(lua_State * L, Unit * ptr);
int LuaUnit_SendPacketToPlayer(lua_State * L, Unit * ptr);
int LuaUnit_PlayerSendChatMessage(lua_State * L, Unit * ptr);
int LuaUnit_AggroWithInRangeFriends(lua_State * L, Unit * ptr);
int LuaUnit_GetDistanceYards(lua_State * L, Unit * ptr);
int LuaUnit_MoveRandomArea(lua_State * L, Unit * ptr);
int LuaUnit_SendPacketToGroup(lua_State * L, Unit * ptr);
int LuaUnit_GetGroupPlayers(lua_State * L, Unit * ptr);
int LuaUnit_GetDungeonDifficulty(lua_State * L, Unit * ptr);
int LuaUnit_GetInstanceOwner(lua_State * L, Unit * ptr);
int LuaUnit_IsGroupFull(lua_State * L, Unit * ptr);
int LuaUnit_GetGroupLeader(lua_State * L, Unit * ptr);
int LuaUnit_SetGroupLeader(lua_State * L, Unit * ptr);
int LuaUnit_AddGroupMember(lua_State * L, Unit * ptr);
int LuaUnit_SetDungeonDifficulty(lua_State * L, Unit * ptr);
int LuaUnit_ExpandToRaid(lua_State * L, Unit * ptr);
int LuaUnit_CanAttack(lua_State * L, Unit * ptr);
int LuaUnit_GetInRangeEnemies(lua_State * L, Unit * ptr);
int LuaUnit_GetInRangeUnits(lua_State * L, Unit * ptr);
int LuaUnit_HasFlag(lua_State * L, Unit * ptr);
int LuaUnit_TakeHonor(lua_State * L, Unit * ptr);
int LuaUnit_GetPower(lua_State * L, Unit * ptr);
int LuaUnit_GetMaxPower(lua_State * L, Unit * ptr);
int LuaUnit_SetPowerType(lua_State * L, Unit * ptr);
int LuaUnit_SetMaxPower(lua_State * L, Unit * ptr);
int LuaUnit_SetPower(lua_State * L, Unit * ptr);
int LuaUnit_SetPowerPct(lua_State * L, Unit * ptr);
int LuaUnit_GetPowerType(lua_State * L, Unit * ptr);
int LuaUnit_GetPowerPct(lua_State * L, Unit * ptr);
int LuaUnit_LearnSpells(lua_State * L, Unit * ptr);

#endif // UNIT_FUNCTIONS_H
