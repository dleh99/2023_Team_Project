#pragma once
#include "header.h"

enum DB_EVENT_TYPE { TRY_LOGIN, LOGIN_DISCONNECT };

constexpr int DB_SIGN_UP = 1;
constexpr int DB_LOGIN_SUCCESS = 2;
constexpr int DB_LOGIN_FAIL = 3;
constexpr int DB_ALREADY_INGAME = 4;

class DB
{
private:
	SQLHENV henv;
	SQLHDBC hdbc;       // connect 할 떄 필요한 핸들
public:
	void InitDB();
	void ReleaseDB();
	int Search_User(std::wstring id, std::wstring password);
	void Disconnect_User(std::wstring id);
	void show_error(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
};

class DB_EVENT
{
public:
	int obj_id;
	std::chrono::system_clock::time_point		wakeup_time;
	DB_EVENT_TYPE event_id;
	std::wstring _id;
	std::wstring _password;
public:
	DB_EVENT();
	DB_EVENT(int ob_id, std::chrono::system_clock::time_point time, DB_EVENT_TYPE et, std::wstring id, std::wstring password);
	constexpr bool operator < (const DB_EVENT& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};