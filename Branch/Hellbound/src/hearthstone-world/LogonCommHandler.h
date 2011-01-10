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

#ifndef LOGONCOMMHANDLER_H
#define LOGONCOMMHANDLER_H

typedef struct
{
	uint32 ID;
	string Name;
	string Address;
	uint32 Port;
	uint32 ServerID;
	uint32 RetryTime;
	bool   Registered;
}LogonServer;

typedef struct
{
	/* Ascent
	string Name;
	string Address;
	uint32 Colour;
	uint32 Icon;
	uint32 TimeZone;
	float Population;*/

	/* Aspire Hearthstone */
	/* Sandshroud */
	string Name;
	string Address;
	uint8 Icon;
	uint8 WorldRegion;
	float Population;
	uint8 Lock;

	/* ArcEmu
	string Arc_Name;
	string Arc_Address;
	uint32 Arc_Colour;
	uint32 Arc_Icon;
	uint32 Arc_TimeZone;
	float Arc_Population;
	uint8 Arc_Lock;*/
}Realm;

enum RealmType
{
	REALMTYPE_NORMAL	= 0,
	REALMTYPE_PVP		= 3,
	REALMTYPE_RP		= 6,
	REALMTYPE_RPPVP		= 8,
};

class SocketLoadBalancer;

class LogonCommHandler : public Singleton<LogonCommHandler>
{
#ifdef WIN32
	typedef HM_NAMESPACE::hash_map<string, string> ForcedPermissionMap;
#else
	typedef map<string, string> ForcedPermissionMap;
#endif

	ForcedPermissionMap forced_permissions;
	map<LogonServer*, LogonCommClientSocket*> logons;
	map<uint32, WorldSocket*> pending_logons;
	set<Realm*> realms;
	set<LogonServer*> servers;
	uint32 idhigh;
	uint32 next_request;
	Mutex mapLock;
	Mutex pendingLock;
	bool pings;
	int8 ReConCounter;
	uint32 _realmType;
	uint32 plrLimit;
	float server_population;

public:
	uint8 sql_passhash[20];

	LogonCommHandler();
	~LogonCommHandler();

	LogonCommClientSocket * ConnectToLogon(string Address, uint32 Port);
	void UpdateAccountCount(uint32 account_id, int8 add);
	void RequestAddition(LogonCommClientSocket * Socket);
	void CheckAllServers();
	void Startup();
	void ConnectionDropped(uint32 ID);
	void AdditionAck(uint32 ID, uint32 ServID);
	void UpdateSockets();
	void Connect(LogonServer * server);
	void ConnectAll();
	//void LogonDatabaseSQLExecute(const char* str, ...);
	//void LogonDatabaseReloadAccounts();
	void RefreshRealmPop();

	void Account_SetBanned(const char * account, uint32 banned, const char* reason);
	void Account_SetGM(const char * account, const char * flags);
	void Account_SetMute(const char * account, uint32 muted);
	void IPBan_Add(const char * ip, uint32 duration, const char* reason);
	void IPBan_Remove(const char * ip);

	void LoadRealmConfiguration();
	void AddServer(string Name, string Address, uint32 Port);

	float GetPopulation() { return server_population; }
	HEARTHSTONE_INLINE uint32 GetRealmType() { return _realmType; }
	void SetRealmType(uint32 type) { _realmType = type; }

	/////////////////////////////
	// Worldsocket stuff
	///////

	uint32 ClientConnected(string AccountName, WorldSocket * Socket);
	void UnauthedSocketClose(uint32 id);
	void RemoveUnauthedSocket(uint32 id);
	WorldSocket* GetSocketByRequest(uint32 id)
	{
		//pendingLock.Acquire();

		WorldSocket * sock;
		map<uint32, WorldSocket*>::iterator itr = pending_logons.find(id);
		sock = (itr == pending_logons.end()) ? 0 : itr->second;

		//pendingLock.Release();
		return sock;
	}
	HEARTHSTONE_INLINE Mutex & GetPendingLock() { return pendingLock; }
	const string* GetForcedPermissions(string& username);

	void TestConsoleLogon(string& username, string& password, uint32 requestnum);
};

#define sLogonCommHandler LogonCommHandler::getSingleton()

class LogonCommWatcherThread : public ThreadContext
{
	bool running;
#ifdef WIN32
	HANDLE hEvent;
#endif

public:
	LogonCommWatcherThread()
	{
		if(!(sWorld.LogonServerType & LOGON_MANGOS))
		{
#ifdef WIN32
			hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
#endif
			running = true;
		}
	}

	~LogonCommWatcherThread()
	{

	}

	void OnShutdown()
	{
		if(!(sWorld.LogonServerType & LOGON_MANGOS))
		{
			m_threadRunning = false;
#ifdef WIN32
			SetEvent( hEvent );
#endif
		}
	}

	bool run()
	{
		if(!(sWorld.LogonServerType & LOGON_MANGOS))
		{
			sLogonCommHandler.ConnectAll();
			while( m_threadRunning )
			{
				sLogonCommHandler.UpdateSockets();
#ifdef WIN32
				WaitForSingleObject( hEvent, 10 );
#else
				Sleep( 10 );
#endif
			}
		}
		return true;
	}
};

#endif
