#pragma once
#include "header.h"

enum DB_EVENT_TYPE { TRY_LOGIN };

class DB
{
private:
	SQLHENV henv;
	SQLHDBC hdbc;       // connect 할 떄 필요한 핸들
public:
	void InitDB();
	void ReleaseDB();
	bool Search_User(const char* id, const char* password);
	bool Search_Id(const char* id);
	void show_error(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
};

class DB_EVENT
{
public:
	int obj_id;
	std::chrono::system_clock::time_point		wakeup_time;
	DB_EVENT_TYPE event_id;
	int target_id;
public:
	DB_EVENT();
	DB_EVENT(int ob_id, std::chrono::system_clock::time_point time, DB_EVENT_TYPE et, int t_id);
	constexpr bool operator < (const DB_EVENT& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};